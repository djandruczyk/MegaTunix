/*
 * Copyright (C) 2007 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix stripchart widget
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
#include <cairo/cairo.h>
#include <stripchart.h>
#include <stripchart-private.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>

GType mtx_stripchart_get_type(void)
{
	static GType mtx_stripchart_type = 0;

	if (!mtx_stripchart_type)
	{
		static const GTypeInfo mtx_stripchart_info =
		{
			sizeof(MtxStripChartClass),
			NULL,
			NULL,
			(GClassInitFunc) mtx_stripchart_class_init,
			NULL,
			NULL,
			sizeof(MtxStripChart),
			0,
			(GInstanceInitFunc) mtx_stripchart_init,
		};
		mtx_stripchart_type = g_type_register_static(GTK_TYPE_DRAWING_AREA, "MtxStripChart", &mtx_stripchart_info, 0);
	}
	return mtx_stripchart_type;
}

/*!
 \brief Initializes the mtx pie chart class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name (MtxStripChartClass *) pointer to the class
 */
void mtx_stripchart_class_init (MtxStripChartClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_stripchart_configure;
	widget_class->expose_event = mtx_stripchart_expose;
	/*widget_class->button_press_event = mtx_stripchart_button_press; */
	/*widget_class->button_release_event = mtx_stripchart_button_release; */
	/* Motion event not needed, as unused currently */
	/*widget_class->motion_notify_event = mtx_stripchart_motion_event; */
	widget_class->size_request = mtx_stripchart_size_request;

	g_type_class_add_private (class_name, sizeof (MtxStripChartPrivate)); 
}


/*!
 \brief Initializes the chart attributes to sane defaults
 \param chart (MtxStripChart *) pointer to the chart object
 */
void mtx_stripchart_init (MtxStripChart *chart)
{
	/* The events the chart receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/ 
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	gtk_widget_add_events (GTK_WIDGET (chart),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK |GDK_POINTER_MOTION_MASK);
	priv->num_traces = 0;
	priv->w = 130;		
	priv->h = 20;
	priv->justification = GTK_JUSTIFY_RIGHT;
	priv->value_font = g_strdup("Bitstream Vera Sans");
	priv->value_font_scale = 0.2;
	priv->cr = NULL;
	priv->colormap = gdk_colormap_get_system();
	priv->gc = NULL;
	priv->traces = g_array_new(FALSE,TRUE,sizeof(MtxStripChartTrace *));
	mtx_stripchart_init_colors(chart);
	if (GTK_WIDGET_REALIZED(chart))
		mtx_stripchart_redraw (chart);
}



/*!
 \brief Allocates the default colors for a chart with no options 
 \param widget (MegaStripChart *) pointer to the chart object
 */
void mtx_stripchart_init_colors(MtxStripChart *chart)
{
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	/*! Main Background */
	priv->colors[COL_BG].red=0.*65535;
	priv->colors[COL_BG].green=0.*65535;
	priv->colors[COL_BG].blue=0.*65535;
	/*! Needle */
	priv->colors[COL_GRAT].red=0.8*65535;
	priv->colors[COL_GRAT].green=0.8*65535;
	priv->colors[COL_GRAT].blue=0.8*65535;
	/*! Trace 1 */
	priv->tcolors[0].red=0.0*65535;
	priv->tcolors[0].green=1.0*65535;
	priv->tcolors[0].blue=1.0*65535;
	/*! Trace 2 */
	priv->tcolors[1].red=1.0*65535;
	priv->tcolors[1].green=0.0*65535;
	priv->tcolors[1].blue=0.0*65535;
	/*! Trace 3 */
	priv->tcolors[2].red=1.0*65535;
	priv->tcolors[2].green=1.0*65535;
	priv->tcolors[2].blue=0.0*65535;
	/*! Trace 4 */
	priv->tcolors[3].red=0.0*65535;
	priv->tcolors[3].green=1.0*65535;
	priv->tcolors[3].blue=0.0*65535;
	/*! Trace 5 */
	priv->tcolors[4].red=1.0*65535;
	priv->tcolors[4].green=0.0*65535;
	priv->tcolors[4].blue=1.0*65535;
	/*! Trace 6 */
	priv->tcolors[5].red=0.0*65535;
	priv->tcolors[5].green=0.0*65535;
	priv->tcolors[5].blue=1.0*65535;

}


/*!
 \brief updates the chart position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param widget (MtxStripChart *) pointer to the chart object
 */
void update_stripchart_position (MtxStripChart *chart)
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
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);

	widget = GTK_WIDGET(chart);

	printf("updating position, copying pixmap over\n");
	/* Copy background pixmap to intermediary for final rendering */
	gdk_draw_drawable(priv->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->trace_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);
/*

	cr = gdk_cairo_create (priv->pixmap);
	cairo_set_font_options(cr,priv->font_options);

	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	// Update the VALUE text 
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

	message = g_strdup_printf("%s:%.*f", priv->valname,priv->precision,priv->value);

	cairo_text_extents (cr, message, &extents);

	cairo_move_to (cr, 
			priv->pie_radius*2 + 5 +priv->value_xpos,
			priv->pie_yc - (extents.height/4) + priv->value_ypos);
	cairo_show_text (cr, message);
	g_free(message);

	cairo_stroke (cr);
	cairo_destroy(cr);
*/
}


/*!
 \brief handles configure events whe nthe chart gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget (GtkWidget *) pointer to the chart object
 \param event (GdkEventConfigure *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_stripchart_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	MtxStripChart * chart = MTX_STRIPCHART(widget);
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);

	priv->w = widget->allocation.width;
	priv->h = widget->allocation.height;

	if (priv->gc)
		g_object_unref(priv->gc);
	/* Backing pixmap (copy of window) */
	if (priv->pixmap)
		g_object_unref(priv->pixmap);
	priv->pixmap=gdk_pixmap_new(widget->window,
			priv->w,priv->h,
			gtk_widget_get_visual(widget)->depth);
	gdk_draw_rectangle(priv->pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			priv->w,priv->h);
	/* Static Background pixmap */
	if (priv->trace_pixmap)
		g_object_unref(priv->trace_pixmap);
	priv->trace_pixmap=gdk_pixmap_new(widget->window,
			priv->w,priv->h,
			gtk_widget_get_visual(widget)->depth);
	gdk_draw_rectangle(priv->trace_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			priv->w,priv->h);

	gdk_window_set_back_pixmap(widget->window,priv->pixmap,0);
	priv->gc = gdk_gc_new(priv->trace_pixmap);
	gdk_gc_set_colormap(priv->gc,priv->colormap);


	if (priv->font_options)
		cairo_font_options_destroy(priv->font_options);
	priv->font_options = cairo_font_options_create();
	cairo_font_options_set_antialias(priv->font_options,
			CAIRO_ANTIALIAS_GRAY);

	generate_stripchart_static_traces(chart);
	update_stripchart_position(chart);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget (GtkWidget *) pointer to the chart object
 \param event (GdkEventExpose *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_stripchart_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxStripChart * chart = MTX_STRIPCHART(widget);
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}


/*!
 \brief draws the static part of the stripchart,  i.e. this is only the 
 history of the trace, as there's no point to re-renderthe whole fucking thing
 every damn time when only 1 datapoint is being appended,  This writes to a
 pixmap, which is shifted and new trace data rendered, before graticaule/text
 overlay is added on.
 \param widget (MtxStripChart *) pointer to the chart object
 */
void generate_stripchart_static_traces(MtxStripChart *chart)
{
	cairo_t *cr = NULL;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gfloat x = 0.0;
	gfloat y = 0.0;
	gfloat start_x = 0.0;
	gfloat start_y = 0.0;
	gint points = 0;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);

	w = GTK_WIDGET(chart)->allocation.width;
	h = GTK_WIDGET(chart)->allocation.height;

	if (!priv->trace_pixmap)
		return;
	/* get a cairo_t */
	cr = gdk_cairo_create (priv->trace_pixmap);
	cairo_set_font_options(cr,priv->font_options);
	cairo_set_source_rgb (cr, 
			priv->colors[COL_BG].red/65535.0,
			priv->colors[COL_BG].green/65535.0,
			priv->colors[COL_BG].blue/65535.0);
	/* Background Rectangle */
	cairo_rectangle (cr,
			0,0,w,h);
	cairo_fill(cr);

	for (i=0;i<priv->num_traces;i++)
	{
		trace = g_array_index(priv->traces,MtxStripChartTrace *,i);

		cairo_set_line_width(cr,trace->lwidth);
		cairo_set_source_rgb (cr, 
				trace->color.red/65535.0,
				trace->color.green/65535.0,
				trace->color.blue/65535.0);
		points = trace->history->len > priv->w ? priv->w:trace->history->len;
		start_x = priv->w - points;
		start_y = priv->h - (((g_array_index(trace->history,gfloat,trace->history->len - points)-trace->min) / (trace->max - trace->min))*priv->h);
		cairo_move_to(cr,start_x,start_y);
		for (j=0;j<points;j++)
		{
			x = priv->w - points + j;
			y = priv->h - (((g_array_index(trace->history,gfloat,trace->history->len - points + j)-trace->min) / (trace->max - trace->min))*priv->h);
			cairo_line_to(cr,x,y);
		}
		cairo_stroke(cr);
	}
	cairo_destroy (cr);
}


gboolean mtx_stripchart_motion_event (GtkWidget *chart,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */
	/*	printf("motion in chart, returning false\n");*/
	return FALSE;
}
					       


/*!
 \brief sets the INITIAL sizeof the widget
 \param chart (GtkWidget *) pointer to the chart widget
 \param requisition (GdkRequisition *) struct to set the vars within
 \returns void
 */
void mtx_stripchart_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = 120;
	requisition->height = 60;
}


/*!
 \brief gets called to redraw the entire display manually
 \param chart (MtxStripChart *) pointer to the chart object
 */
void mtx_stripchart_redraw (MtxStripChart *chart)
{
	update_stripchart_position(chart);
	gdk_window_clear(GTK_WIDGET(chart)->window);
}


