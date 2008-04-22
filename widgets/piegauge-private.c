/*
 * Copyright (C) 2007 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt pie gauge widget
 * Inspired by Phil Tobins MegaLogViewer
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 *  
 */


#include <config.h>
#ifdef HAVE_CAIRO
#include <cairo/cairo.h>
#endif
#include <piegauge.h>
#include <piegauge-private.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>


G_DEFINE_TYPE (MtxPieGauge, mtx_pie_gauge, GTK_TYPE_DRAWING_AREA)

/*!
 \brief Initializes the mtx pie gauge class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name (MtxPieGaugeClass *) pointer to the class
 */
void mtx_pie_gauge_class_init (MtxPieGaugeClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_pie_gauge_configure;
	widget_class->expose_event = mtx_pie_gauge_expose;
	/*widget_class->button_press_event = mtx_pie_gauge_button_press; */
	/*widget_class->button_release_event = mtx_pie_gauge_button_release; */
	/* Motion event not needed, as unused currently */
	/*widget_class->motion_notify_event = mtx_pie_gauge_motion_event; */
	widget_class->size_request = mtx_pie_gauge_size_request;

	/*g_type_class_add_private (obj_class, sizeof (MtxPieGaugePrivate)); */
}


/*!
 \brief Initializes the gauge attributes to sane defaults
 \param gauge (MtxPieGauge *) pointer to the gauge object
 */
void mtx_pie_gauge_init (MtxPieGauge *gauge)
{
	/* The events the gauge receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/ 
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK |GDK_POINTER_MOTION_MASK);
	gauge->w = 130;		
	gauge->h = 20;
	gauge->pie_xc = 17;		/* pie x center coord from LL corner */
	gauge->pie_yc = gauge->h-3;	/* pie y center coord from LL corner */
	gauge->pie_radius = 14;		/* pie is 180deg swep so 14x28 pixels */
	gauge->value = 0.0;		/* default values */
	gauge->min = 0.0;
	gauge->max = 100.0;
	gauge->precision = 2;
	gauge->start_angle = 180;	/* lower left quadrant */
	gauge->sweep_angle = 180;	/* CW sweep */
	gauge->value_font = g_strdup("Bitstream Vera Sans");
	gauge->value_xpos = 0;
	gauge->value_ypos = 0;
	gauge->value_font_scale = 0.2;
#ifdef HAVE_CAIRO
	gauge->cr = NULL;
	gauge->antialias = TRUE;
#else
	gauge->antialias = FALSE;
#endif
	gauge->colormap = gdk_colormap_get_system();
	gauge->gc = NULL;
	mtx_pie_gauge_init_colors(gauge);
	/*mtx_pie_gauge_redraw (gauge);*/
}



/*!
 \brief Allocates the default colors for a gauge with no options 
 \param widget (MegaPieGauge *) pointer to the gauge object
 */
void mtx_pie_gauge_init_colors(MtxPieGauge *gauge)
{
	/*! Main Background */
	gauge->colors[COL_BG].red=0.914*65535;
	gauge->colors[COL_BG].green=0.914*65535;
	gauge->colors[COL_BG].blue=0.859*65535;
	/*! Needle */
	gauge->colors[COL_NEEDLE].red=0.0*65535;
	gauge->colors[COL_NEEDLE].green=0.0*65535;
	gauge->colors[COL_NEEDLE].blue=0.0*65535;
	/*! Text Color*/
	gauge->colors[COL_VALUE_FONT].red=0.1*65535;
	gauge->colors[COL_VALUE_FONT].green=0.1*65535;
	gauge->colors[COL_VALUE_FONT].blue=0.1*65535;
	/*! Gauge BG Color Begin */
	gauge->colors[COL_LOW].red=0.0*65535;
	gauge->colors[COL_LOW].green=1.0*65535;
	gauge->colors[COL_LOW].blue=0.0*65535;
	/*! Gauge BG Color Middle */
	gauge->colors[COL_MID].red=1.0*65535;
	gauge->colors[COL_MID].green=1.0*65535;
	gauge->colors[COL_MID].blue=0.0*65535;
	/*! Gauge BG Color End */
	gauge->colors[COL_HIGH].red=1.0*65535;
	gauge->colors[COL_HIGH].green=0.0*65535;
	gauge->colors[COL_HIGH].blue=0.0*65535;

}


/*!
 \brief updates the gauge position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param widget (MtxPieGauge *) pointer to the gauge object
 */
void cairo_update_pie_gauge_position (MtxPieGauge *gauge)
{
#ifdef HAVE_CAIRO
	GtkWidget * widget = NULL;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * tmpbuf = NULL;
	gchar * message = NULL;
	GdkPoint tip;
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;

	widget = GTK_WIDGET(gauge);

	/* Copy background pixmap to intermediary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);

	cr = gdk_cairo_create (gauge->pixmap);
	cairo_set_font_options(cr,gauge->font_options);

	if (gauge->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
	/* Update the VALUE text */
	cairo_set_source_rgb (cr, 
			gauge->colors[COL_VALUE_FONT].red/65535.0,
			gauge->colors[COL_VALUE_FONT].green/65535.0,
			gauge->colors[COL_VALUE_FONT].blue/65535.0);
	tmpbuf = g_utf8_strup(gauge->value_font,-1);
	if (g_strrstr(tmpbuf,"BOLD"))
		weight = CAIRO_FONT_WEIGHT_BOLD;
	else
		weight = CAIRO_FONT_WEIGHT_NORMAL;
	if (g_strrstr(tmpbuf,"OBLIQUE"))
		slant = CAIRO_FONT_SLANT_OBLIQUE;
	else if (g_strrstr(tmpbuf,"ITALIC"))
		slant = CAIRO_FONT_SLANT_ITALIC;
	else
		slant = CAIRO_FONT_SLANT_NORMAL;
	g_free(tmpbuf);
	cairo_select_font_face (cr, gauge->value_font,  slant, weight);

	cairo_set_font_size (cr, 11);

	message = g_strdup_printf("%s:%.*f", gauge->valname,gauge->precision,gauge->value);

	cairo_text_extents (cr, message, &extents);

	cairo_move_to (cr, 
			gauge->pie_radius*2 + 5 +gauge->value_xpos,
			gauge->pie_yc - (extents.height/4) + gauge->value_ypos);
	cairo_show_text (cr, message);
	g_free(message);

	cairo_stroke (cr);

	/* gauge hands */
	tmpf = (gauge->value-gauge->min)/(gauge->max-gauge->min);
	needle_pos = (gauge->start_angle+(tmpf*gauge->sweep_angle))*(M_PI/180);


	cairo_set_source_rgb (cr, gauge->colors[COL_NEEDLE].red/65535.0,
			gauge->colors[COL_NEEDLE].green/65535.0,
			gauge->colors[COL_NEEDLE].blue/65535.0);
	cairo_set_line_width (cr, 1.5);


	tip.x = gauge->pie_xc + (gauge->pie_radius * cos (needle_pos));
	tip.y = gauge->pie_yc + (gauge->pie_radius * sin (needle_pos));

	cairo_move_to (cr, gauge->pie_xc,gauge->pie_yc);
	cairo_line_to (cr, tip.x,tip.y);
	cairo_stroke(cr);
	cairo_destroy(cr);
#endif
}


/*!
 \brief updates the gauge position,  This is the GDK implementation that
 doesn't do antialiasing,  but is the fastest one.
 \param widget (MtxPieGauge *) pointer to the gauge object
 */
void gdk_update_pie_gauge_position (MtxPieGauge *gauge)
{
#ifndef HAVE_CAIRO
	GtkWidget *widget = NULL;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	GdkPoint tip;
	PangoRectangle logical_rect;

	widget = GTK_WIDGET(gauge);

	/* Check if in alert bounds and alert as necessary */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);
	/* the text */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_VALUE_FONT]);
	message = g_strdup_printf("%s:%.*f", gauge->valname,gauge->precision,gauge->value);

	tmpbuf = g_strdup_printf("%s 8",gauge->value_font);
	gauge->font_desc = pango_font_description_from_string(tmpbuf);
	g_free(tmpbuf);
	pango_layout_set_font_description(gauge->layout,gauge->font_desc);
	pango_layout_set_text(gauge->layout,message,-1);
	pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);
	g_free(message);

	gdk_draw_layout(gauge->pixmap,gauge->gc,
			gauge->pie_radius*2 + 5 +gauge->value_xpos,
			gauge->pie_yc - logical_rect.height+gauge->value_ypos,gauge->layout);

	gdk_gc_set_line_attributes(gauge->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_ROUND,
			GDK_JOIN_ROUND);

	/* gauge hands */
	tmpf = (gauge->value-gauge->min)/(gauge->max-gauge->min);
	needle_pos = (gauge->start_angle+(tmpf*gauge->sweep_angle))*(M_PI/180.0);
	tip.x = gauge->pie_xc + (gauge->pie_radius * cos (needle_pos));
	tip.y = gauge->pie_yc + (gauge->pie_radius * sin (needle_pos));

	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_NEEDLE]);
	gdk_draw_line(gauge->pixmap,
			gauge->gc,
			gauge->pie_xc,gauge->pie_yc,
			tip.x,tip.y);
	
#endif
}


/*!
 \brief handles configure events whe nthe gauge gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget (GtkWidget *) pointer to the gauge object
 \param event (GdkEventConfigure *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_pie_gauge_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	MtxPieGauge * gauge = MTX_PIE_GAUGE(widget);

	gauge->w = widget->allocation.width;
	gauge->h = widget->allocation.height;

	if (gauge->gc)
		g_object_unref(gauge->gc);
	if (gauge->bm_gc)
		g_object_unref(gauge->bm_gc);
	if (gauge->layout)
		g_object_unref(gauge->layout);
	/* Backing pixmap (copy of window) */
	if (gauge->pixmap)
		g_object_unref(gauge->pixmap);
	gauge->pixmap=gdk_pixmap_new(widget->window,
			gauge->w,gauge->h,
			gtk_widget_get_visual(widget)->depth);
	gdk_draw_rectangle(gauge->pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			gauge->w,gauge->h);
	/* Static Background pixmap */
	if (gauge->bg_pixmap)
		g_object_unref(gauge->bg_pixmap);
	gauge->bg_pixmap=gdk_pixmap_new(widget->window,
			gauge->w,gauge->h,
			gtk_widget_get_visual(widget)->depth);
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			gauge->w,gauge->h);

	gdk_window_set_back_pixmap(widget->window,gauge->pixmap,0);
	gauge->layout = gtk_widget_create_pango_layout(GTK_WIDGET(&gauge->parent),NULL);	
	gauge->gc = gdk_gc_new(gauge->bg_pixmap);
	gdk_gc_set_colormap(gauge->gc,gauge->colormap);


#ifdef HAVE_CAIRO
	if (gauge->font_options)
		cairo_font_options_destroy(gauge->font_options);
	gauge->font_options = cairo_font_options_create();
	cairo_font_options_set_antialias(gauge->font_options,
			CAIRO_ANTIALIAS_GRAY);
#endif

	generate_pie_gauge_background(gauge);
	update_pie_gauge_position(gauge);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget (GtkWidget *) pointer to the gauge object
 \param event (GdkEventExpose *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_pie_gauge_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxPieGauge * gauge = MTX_PIE_GAUGE(widget);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param widget (MtxPieGauge *) pointer to the gauge object
 */
void cairo_generate_pie_gauge_background(MtxPieGauge *gauge)
{
#ifdef HAVE_CAIRO
	cairo_t *cr = NULL;
	gint w = 0;
	gint h = 0;

	w = GTK_WIDGET(gauge)->allocation.width;
	h = GTK_WIDGET(gauge)->allocation.height;

	if (!gauge->bg_pixmap)
		return;
	/* get a cairo_t */
	cr = gdk_cairo_create (gauge->bg_pixmap);
	cairo_set_font_options(cr,gauge->font_options);
	cairo_set_source_rgb (cr, 
			gauge->colors[COL_BG].red/65535.0,
			gauge->colors[COL_BG].green/65535.0,
			gauge->colors[COL_BG].blue/65535.0);
	/* Background Rectangle */
	cairo_rectangle (cr,
			0,0,w,h);
	cairo_fill(cr);
	/* first one big yellow, with the green and red on top
	 * This prevents seeing BG pixels due to antialiasing errors */
	cairo_move_to(cr,gauge->pie_xc,gauge->pie_yc);
	cairo_set_source_rgb (cr, 
			gauge->colors[COL_MID].red/65535.0,
			gauge->colors[COL_MID].green/65535.0,
			gauge->colors[COL_MID].blue/65535.0);
	cairo_arc(cr, gauge->pie_xc, gauge->pie_yc, gauge->pie_radius, 
			gauge->start_angle*(M_PI/180.0), 
			(gauge->start_angle+gauge->sweep_angle)*(M_PI/180.0));
	cairo_fill(cr);
	/* Low green Arc (*/
	cairo_move_to(cr,gauge->pie_xc,gauge->pie_yc);
	cairo_set_source_rgb (cr, 
			gauge->colors[COL_LOW].red/65535.0,
			gauge->colors[COL_LOW].green/65535.0,
			gauge->colors[COL_LOW].blue/65535.0);
	cairo_arc(cr, gauge->pie_xc, gauge->pie_yc, gauge->pie_radius, 
			gauge->start_angle*(M_PI/180.0), 
			(gauge->start_angle+45)*(M_PI/180.0));
	cairo_fill(cr);
	/* High red Arc */
	cairo_move_to(cr,gauge->pie_xc,gauge->pie_yc);
	cairo_set_source_rgb (cr, 
			gauge->colors[COL_HIGH].red/65535.0,
			gauge->colors[COL_HIGH].green/65535.0,
			gauge->colors[COL_HIGH].blue/65535.0);
	cairo_arc(cr, gauge->pie_xc, gauge->pie_yc, gauge->pie_radius, 
			(gauge->start_angle+135)*(M_PI/180.0), 
			(gauge->start_angle+180)*(M_PI/180.0));
	cairo_fill(cr);
	/* Pie Gauge Arcs */
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgb (cr, 
			gauge->colors[COL_NEEDLE].red/65535.0,
			gauge->colors[COL_NEEDLE].green/65535.0,
			gauge->colors[COL_NEEDLE].blue/65535.0);
	cairo_arc(cr, gauge->pie_xc, gauge->pie_yc, gauge->pie_radius, 
			gauge->start_angle*(M_PI/180.0), 
			(gauge->start_angle+gauge->sweep_angle)*(M_PI/180.0));
	cairo_stroke(cr);
	cairo_destroy (cr);
#endif
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the gdk version.
 \param widget (MtxPieGauge *) pointer to the gauge object
 */
void gdk_generate_pie_gauge_background(MtxPieGauge *gauge)
{
#ifndef HAVE_CAIRO
	gint w = 0;
	gint h = 0;

	if (!gauge->bg_pixmap)
		return;

	w = GTK_WIDGET(gauge)->allocation.width;
	h = GTK_WIDGET(gauge)->allocation.height;

	/* Wipe the display, selected BG color */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_BG]);
	gdk_draw_rectangle(gauge->bg_pixmap,
			gauge->gc,
			TRUE, 0,0,
			w,
			h);

	/* Black Border pie gauge  */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_NEEDLE]);
	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->pie_xc-gauge->pie_radius-1,
			gauge->pie_yc-gauge->pie_radius-1,
			2*gauge->pie_radius+2,
			2*gauge->pie_radius+2,
			180*64,-180*64);
	/* Yellow pie gauge  */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_MID]);
	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->pie_xc-gauge->pie_radius,
			gauge->pie_yc-gauge->pie_radius,
			2*gauge->pie_radius,
			2*gauge->pie_radius,
			180*64,-180*64);

	/* Green Slice */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_LOW]);
	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->pie_xc-gauge->pie_radius,
			gauge->pie_yc-gauge->pie_radius,
			2*gauge->pie_radius,
			2*gauge->pie_radius,
			180*64,-45*64);

	/* Red Slice */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_HIGH]);
	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->pie_xc-gauge->pie_radius,
			gauge->pie_yc-gauge->pie_radius,
			2*gauge->pie_radius,
			2*gauge->pie_radius,
			45*64,-45*64);

#endif
}


gboolean mtx_pie_gauge_motion_event (GtkWidget *gauge,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */
	/*	printf("motion in gauge, returning false\n");*/
	return FALSE;
}
					       


/*!
 \brief sets the INITIAL sizeof the widget
 \param gauge (GtkWidget *) pointer to the gauge widget
 \param requisition (GdkRequisition *) struct to set the vars within
 \returns void
 */
void mtx_pie_gauge_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = 80;
	requisition->height = 20;
}


/*!
 \brief gets called to redraw the entire display manually
 \param gauge (MtxPieGauge *) pointer to the gauge object
 */
void mtx_pie_gauge_redraw (MtxPieGauge *gauge)
{
	update_pie_gauge_position(gauge);
	gdk_window_clear(GTK_WIDGET(gauge)->window);
}


/*!
 \brief generates the gauge background, This is a wrapper function 
 conditionally compiled to call a corresponsing GDK or cairo function.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void generate_pie_gauge_background(MtxPieGauge *gauge)
{
	g_return_if_fail (MTX_IS_PIE_GAUGE (gauge));
#ifdef HAVE_CAIRO
	cairo_generate_pie_gauge_background(gauge);
#else
	gdk_generate_pie_gauge_background(gauge);
#endif
}


/*!
 \brief updates the gauge position,  This is a wrapper function conditionally
 compiled to call a corresponsing GDK or cairo function.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void update_pie_gauge_position(MtxPieGauge *gauge)
{
	g_return_if_fail (MTX_IS_PIE_GAUGE (gauge));
#ifdef HAVE_CAIRO
	cairo_update_pie_gauge_position (gauge);
#else
	gdk_update_pie_gauge_position (gauge);
#endif
}
