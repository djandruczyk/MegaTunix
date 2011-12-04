/*
 * Copyright (C) 2007 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix pie gauge widget
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

/*!
  \file widgets/piegauge-private.c
  \brief MtxPieGauge private functions
  \author David Andruczyk
 */

#include <defines.h>
#include <piegauge-private.h>
#include <math.h>
#include <stdio.h>

/*!
  \brief returns the GType associated with a MtxPieGauge, registering it if 
  needed
  \returns the GType for a MtxPieGauge
  */
GType mtx_pie_gauge_get_type(void)
{
	static GType mtx_pie_gauge_type = 0;

	if (!mtx_pie_gauge_type)
	{
		static const GTypeInfo mtx_pie_gauge_info =
		{
			sizeof(MtxPieGaugeClass),
			NULL,
			NULL,
			(GClassInitFunc) mtx_pie_gauge_class_init,
			NULL,
			NULL,
			sizeof(MtxPieGauge),
			0,
			(GInstanceInitFunc) mtx_pie_gauge_init,
		};
		mtx_pie_gauge_type = g_type_register_static(GTK_TYPE_DRAWING_AREA, "MtxPieGauge", &mtx_pie_gauge_info, 0);
	}
	return mtx_pie_gauge_type;
}

/*!
 \brief Initializes the mtx pie gauge class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name is a pointer to the MtxPieGaugeClass structure
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
	obj_class->finalize = mtx_pie_gauge_finalize;

	g_type_class_add_private (class_name, sizeof (MtxPieGaugePrivate)); 
}


/*!
 \brief Frees up private data
 \param object is the pointer to the gauge object
 */
void mtx_pie_gauge_finalize (GObject *object)
{
	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(object);
	if (priv->pixmap)
		g_object_unref(priv->pixmap);
	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	if (priv->value_font)
		g_free(priv->value_font);
	if (priv->valname)
		g_free(priv->valname);
}

/*!
 \brief Initializes the gauge attributes to sane defaults
 \param gauge is the pointer to the gauge object
 */
void mtx_pie_gauge_init (MtxPieGauge *gauge)
{
	/* The events the gauge receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/

	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(gauge);
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK |GDK_POINTER_MOTION_MASK);
	gtk_widget_set_double_buffered (GTK_WIDGET(gauge), FALSE);
	priv->w = 130;		
	priv->h = 20;
	priv->pie_xc = 17;		/* pie x center coord from LL corner */

	priv->pie_yc = priv->h-3;	/* pie y center coord from LL corner */

	priv->pie_radius = 14;		/* pie is 180deg swep so 14x28 pixels */

	priv->value = 0.0;		/* default values */

	priv->min = 0.0;
	priv->max = 100.0;
	priv->precision = 2;
	priv->start_angle = 180;	/* lower left quadrant */

	priv->sweep_angle = 180;	/* CW sweep */

	priv->value_font = g_strdup("Bitstream Vera Sans");
	priv->value_font_scale = 0.2;
	mtx_pie_gauge_init_colors(gauge);
	/*mtx_pie_gauge_redraw (gauge);*/

}


/*!
 \brief Allocates the default colors for a gauge with no options 
 \param gauge is the pointer to the gauge object
 */
void mtx_pie_gauge_init_colors(MtxPieGauge *gauge)
{
	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(gauge);
	/*! Main Background */

	priv->colors[COL_BG].red=0.914*65535;
	priv->colors[COL_BG].green=0.914*65535;
	priv->colors[COL_BG].blue=0.859*65535;
	/*! Needle */

	priv->colors[COL_NEEDLE].red=0.0*65535;
	priv->colors[COL_NEEDLE].green=0.0*65535;
	priv->colors[COL_NEEDLE].blue=0.0*65535;
	/*! Text Color*/

	priv->colors[COL_VALUE_FONT].red=0.1*65535;
	priv->colors[COL_VALUE_FONT].green=0.1*65535;
	priv->colors[COL_VALUE_FONT].blue=0.1*65535;
	/*! Gauge BG Color Begin */

	priv->colors[COL_LOW].red=0.0*65535;
	priv->colors[COL_LOW].green=1.0*65535;
	priv->colors[COL_LOW].blue=0.0*65535;
	/*! Gauge BG Color Middle */

	priv->colors[COL_MID].red=1.0*65535;
	priv->colors[COL_MID].green=1.0*65535;
	priv->colors[COL_MID].blue=0.0*65535;
	/*! Gauge BG Color End */

	priv->colors[COL_HIGH].red=1.0*65535;
	priv->colors[COL_HIGH].green=0.0*65535;
	priv->colors[COL_HIGH].blue=0.0*65535;

}


/*!
 \brief updates the gauge position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param gauge is the pointer to the gauge object
 */
void update_pie_gauge_position (MtxPieGauge *gauge)
{
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
	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(gauge);
	GtkAllocation allocation;

	widget = GTK_WIDGET(gauge);
	gtk_widget_get_allocation(widget,&allocation);

	/* Copy background pixmap to intermediary for final rendering */

	cr = gdk_cairo_create(priv->pixmap);
	gdk_cairo_set_source_pixmap(cr,priv->bg_pixmap,0,0);
	cairo_rectangle(cr,0,0,allocation.width,allocation.height);
	cairo_fill(cr);

	cairo_set_font_options(cr,priv->font_options);

	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	/* Update the VALUE text */

	cairo_set_source_rgb (cr, 
			priv->colors[COL_VALUE_FONT].red/65535.0,
			priv->colors[COL_VALUE_FONT].green/65535.0,
			priv->colors[COL_VALUE_FONT].blue/65535.0);
	tmpbuf = g_utf8_strup(priv->value_font,-1);
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
	cairo_select_font_face (cr, priv->value_font,  slant, weight);

	cairo_set_font_size (cr, 11);

	if (priv->valname)
		message = g_strdup_printf("%s:%.*f", priv->valname,priv->precision,priv->value);
	else
		message = g_strdup_printf("%.*f", priv->precision,priv->value);

	cairo_text_extents (cr, message, &extents);

	cairo_move_to (cr, 
			((priv->w-(priv->pie_radius*2.0 + 5)-extents.width)/2.0) +priv->pie_radius*2.0,
			priv->pie_yc - (extents.height/4.0));
	cairo_show_text (cr, message);
	g_free(message);

	cairo_stroke (cr);

	/* gauge hands */

	tmpf = (priv->value-priv->min)/(priv->max-priv->min);
	needle_pos = (priv->start_angle+(tmpf*priv->sweep_angle))*(M_PI/180);


	cairo_set_source_rgb (cr, priv->colors[COL_NEEDLE].red/65535.0,
			priv->colors[COL_NEEDLE].green/65535.0,
			priv->colors[COL_NEEDLE].blue/65535.0);
	cairo_set_line_width (cr, 1.5);


	tip.x = priv->pie_xc + (priv->pie_radius * cos (needle_pos));
	tip.y = priv->pie_yc + (priv->pie_radius * sin (needle_pos));

	cairo_move_to (cr, priv->pie_xc,priv->pie_yc);
	cairo_line_to (cr, tip.x,tip.y);
	cairo_stroke(cr);
	cairo_destroy(cr);
}


/*!
 \brief handles configure events whe nthe gauge gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget is the pointer to the gauge object
 \param event is a pointer to the GdkEventConfigure structure that 
 encodes important info like window dimensions and depth.
 \returns TRUE
 */
gboolean mtx_pie_gauge_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	MtxPieGauge * gauge = MTX_PIE_GAUGE(widget);
	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(gauge);
	cairo_t *cr = NULL;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	gtk_widget_get_allocation(widget,&allocation);
	window = (GdkWindow *)gtk_widget_get_window(widget);
	priv->w = allocation.width;
	priv->h = allocation.height;

	/* Backing pixmap (copy of window) */

	if (priv->pixmap)
		g_object_unref(priv->pixmap);
	priv->pixmap=gdk_pixmap_new(window,
			priv->w,priv->h,
			gtk_widget_get_visual(widget)->depth);
	gdk_window_set_back_pixmap(window,priv->pixmap,0);
	cr = gdk_cairo_create(priv->pixmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_destroy(cr);
	/* Static Background pixmap */

	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	priv->bg_pixmap=gdk_pixmap_new(window,
			priv->w,priv->h,
			gtk_widget_get_visual(widget)->depth);
	cr = gdk_cairo_create(priv->bg_pixmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_destroy(cr);


	if (priv->font_options)
		cairo_font_options_destroy(priv->font_options);
	priv->font_options = cairo_font_options_create();
	cairo_font_options_set_antialias(priv->font_options,
			CAIRO_ANTIALIAS_GRAY);

	generate_pie_gauge_background(gauge);
	update_pie_gauge_position(gauge);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget is the pointer to the gauge object
 \param event is the pointer to the GdkEventExpose structure that 
 encodes important info like window dimensions and depth.
 \returns FALSE
 */
gboolean mtx_pie_gauge_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxPieGauge * gauge = MTX_PIE_GAUGE(widget);
	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(gauge);
	cairo_t *cr = NULL;
	GdkWindow *window = gtk_widget_get_window(widget);

#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_is_sensitive(GTK_WIDGET(widget)))
#else
		if (GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(widget)))
#endif
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_pixmap(cr,priv->pixmap,0,0);
			cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
		}
		else
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_pixmap(cr,priv->pixmap,0,0);
			cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
			cairo_rectangle (cr,
					0,0,priv->w,priv->h);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
	return FALSE;
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param gauge is the pointer to the gauge object
 */
void generate_pie_gauge_background(MtxPieGauge *gauge)
{
	cairo_t *cr = NULL;
	gint w = 0;
	gint h = 0;
	MtxPieGaugePrivate *priv = MTX_PIE_GAUGE_GET_PRIVATE(gauge);
	GtkAllocation allocation;

	gtk_widget_get_allocation(GTK_WIDGET(gauge),&allocation);

	w = allocation.width;
	h = allocation.height;

	if (!priv->bg_pixmap)
		return;
	/* get a cairo_t */

	cr = gdk_cairo_create (priv->bg_pixmap);
	cairo_set_font_options(cr,priv->font_options);
	cairo_set_source_rgb (cr, 
			priv->colors[COL_BG].red/65535.0,
			priv->colors[COL_BG].green/65535.0,
			priv->colors[COL_BG].blue/65535.0);
	/* Background Rectangle */

	cairo_rectangle (cr,
			0,0,w,h);
	cairo_fill(cr);
	/* first one big yellow, with the green and red on top
	 * This prevents seeing BG pixels due to antialiasing errors */

	cairo_move_to(cr,priv->pie_xc,priv->pie_yc);
	cairo_set_source_rgb (cr, 
			priv->colors[COL_MID].red/65535.0,
			priv->colors[COL_MID].green/65535.0,
			priv->colors[COL_MID].blue/65535.0);
	cairo_arc(cr, priv->pie_xc, priv->pie_yc, priv->pie_radius, 
			priv->start_angle*(M_PI/180.0), 
			(priv->start_angle+priv->sweep_angle)*(M_PI/180.0));
	cairo_fill(cr);
	/* Low green Arc (*/

	cairo_move_to(cr,priv->pie_xc,priv->pie_yc);
	cairo_set_source_rgb (cr, 
			priv->colors[COL_LOW].red/65535.0,
			priv->colors[COL_LOW].green/65535.0,
			priv->colors[COL_LOW].blue/65535.0);
	cairo_arc(cr, priv->pie_xc, priv->pie_yc, priv->pie_radius, 
			priv->start_angle*(M_PI/180.0), 
			(priv->start_angle+45)*(M_PI/180.0));
	cairo_fill(cr);
	/* High red Arc */

	cairo_move_to(cr,priv->pie_xc,priv->pie_yc);
	cairo_set_source_rgb (cr, 
			priv->colors[COL_HIGH].red/65535.0,
			priv->colors[COL_HIGH].green/65535.0,
			priv->colors[COL_HIGH].blue/65535.0);
	cairo_arc(cr, priv->pie_xc, priv->pie_yc, priv->pie_radius, 
			(priv->start_angle+135)*(M_PI/180.0), 
			(priv->start_angle+180)*(M_PI/180.0));
	cairo_fill(cr);
	/* Pie Gauge Arcs */

	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgb (cr, 
			priv->colors[COL_NEEDLE].red/65535.0,
			priv->colors[COL_NEEDLE].green/65535.0,
			priv->colors[COL_NEEDLE].blue/65535.0);
	cairo_arc(cr, priv->pie_xc, priv->pie_yc, priv->pie_radius, 
			priv->start_angle*(M_PI/180.0), 
			(priv->start_angle+priv->sweep_angle)*(M_PI/180.0));
	cairo_stroke(cr);
	cairo_destroy (cr);
}


/*!
  \brief Unused handler for motion events
  \param gauge is the pointer to the gauge widget
  \param event is a pointe to a GdkEventMotion structure
  \returns FALSE so other handlers run
  */
gboolean mtx_pie_gauge_motion_event (GtkWidget *gauge,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */

	/*	printf("motion in gauge, returning false\n");*/

	return FALSE;
}
					       


/*!
 \brief sets the INITIAL size of the widget
 \param widget is the pointer to the gauge widget
 \param requisition is a pointer to the GdkRequisition struct to set the 
 vars within forthe size requisition
 */
void mtx_pie_gauge_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(requisition);
	requisition->width = 80;
	requisition->height = 20;
}


/*!
 \brief gets called to redraw the entire display manually
 \param gauge is the pointer to the gauge object
 */
void mtx_pie_gauge_redraw (MtxPieGauge *gauge)
{
	GdkWindow *window = (GdkWindow *)gtk_widget_get_window(GTK_WIDGET(gauge));

	if (!window) return;

	update_pie_gauge_position(gauge);
	gdk_window_clear(window);
}


