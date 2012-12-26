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

/*!
  \file widgets/stripchart-private.c
  \brief MtxStripChart private functions
  \author David Andruczyk
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

/*!
  \brief The Enumeration for the Text Placement within the chart
  */
static enum
{
	BOTTOM,
	QUARTER,
	HALF,
	THREEQUARTER,
	TOP,
	NUM_TXTS
}TxtPlacement;


/*!
  \brief Returns the Gtype for a MtxStripChart object, or cretes one and 
  registers it if it hasn't already been
  */
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
		mtx_stripchart_type = g_type_register_static(GTK_TYPE_DRAWING_AREA, "MtxStripChart", &mtx_stripchart_info, (GTypeFlags)0);
	}
	return mtx_stripchart_type;
}


/*!
 \brief Initializes the mtx strip chart class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name is a pointer to the MtxStripChartClass structure
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

	widget_class->enter_notify_event = mtx_stripchart_enter_leave_event; 
	widget_class->leave_notify_event = mtx_stripchart_enter_leave_event; 
	widget_class->motion_notify_event = mtx_stripchart_motion_event; 
	widget_class->size_request = mtx_stripchart_size_request;
	obj_class->finalize = mtx_stripchart_finalize;

	g_type_class_add_private (class_name, sizeof (MtxStripChartPrivate)); 
}


/*!
 \brief Finalizes the chart object
 \param chart is the pointer to the chart object
 */
void mtx_stripchart_finalize (GObject *chart)
{
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	if (priv->trace_pixmap)
		g_object_unref(priv->trace_pixmap);
	if (priv->grat_pixmap)
		g_object_unref(priv->grat_pixmap);
	if (priv->font)
		g_free(priv->font);
	if (priv->traces)
		mtx_stripchart_cleanup_traces(priv->traces);
}


/*!
 \brief cleans up each trace object
 \param traces is the pointer to the array of trace structures
 */
void mtx_stripchart_cleanup_traces (GArray *traces)
{
	guint i=0;
	MtxStripChartTrace *trace = NULL;
	for(i=0;i<traces->len;i++)
	{
		trace = g_array_index(traces,MtxStripChartTrace *,i);
		if (trace->name)
			g_free(trace->name);
		if (trace->history)
			g_array_free(trace->history,TRUE);
		g_free(trace);
	}
	g_array_free(traces,TRUE);
}


/*!
 \brief Initializes the chart attributes to sane defaults
 \param chart is the pointer to the chart object
 */
void mtx_stripchart_init (MtxStripChart *chart)
{
	/* The events the chart receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/

	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	gtk_widget_add_events (GTK_WIDGET (chart),GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK |
			GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK);
        gtk_widget_set_double_buffered (GTK_WIDGET(chart), FALSE);

	priv->num_traces = 0;
	priv->w = 130;		
	priv->h = 20;
	priv->justification = GTK_JUSTIFY_RIGHT;
	priv->font = g_strdup("Bitstream Vera Sans");
	priv->traces = g_array_new(FALSE,TRUE,sizeof(MtxStripChartTrace *));
	mtx_stripchart_init_colors(chart);
/*	if (GTK_WIDGET_REALIZED(chart))
		mtx_stripchart_redraw (chart);
*/
}


/*!
 \brief Allocates the default colors for a chart with no options 
 \param chart is the pointer to the chart object
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
 \brief updates the chart position,  This is the CAIRO implementation
 \param chart is the pointer to the chart object
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
	gint shift = 0;
	gfloat start_x = 0.0;
	gfloat start_y = 0.0;
	gfloat buffer = 0.0;
	gboolean draw_quarters = TRUE;
	gint i = 0;
	gint j = 0;
	gfloat x = 0.0;
	gfloat y = 0.0;
	gfloat text_offset[NUM_TXTS] = {0.0,0.0,0.0,0.0,0.0};
	GdkPoint tip;
	cairo_t *cr = NULL;
	cairo_t *cr2 = NULL;
	cairo_text_extents_t extents;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);

	widget = GTK_WIDGET(chart);
	shift = priv->newsamples;
	
	if (shift == 0)
		return;

	/* Scroll trace pixmap */
	cr = gdk_cairo_create(priv->trace_pixmap);
	gdk_cairo_set_source_pixmap(cr,priv->trace_pixmap,-shift,0);
	cairo_rectangle(cr,0,0,priv->w,priv->h);
	cairo_fill(cr);
	cairo_set_source_rgb(cr,0,0,0);
	cairo_rectangle(cr,priv->w-shift,0,shift,priv->h);
	cairo_fill(cr);
	/* Render new data */

	/* Draw new data to trace pixmap */
	for (i=0;i<priv->num_traces;i++)
	{
		trace = g_array_index(priv->traces,MtxStripChartTrace *,i);

		cairo_set_line_width(cr,trace->lwidth);
		cairo_set_source_rgb (cr, 
				trace->color.red/65535.0,
				trace->color.green/65535.0,
				trace->color.blue/65535.0);
		if (trace->history->len > 1)
		{
			shift = shift >= trace->history->len ? trace->history->len-1:shift;
			for(j=shift;j>0;j--)
			{
				start_x = priv->w - (j-1);
				start_y = priv->h - (((g_array_index(trace->history,gfloat,trace->history->len-(j))-trace->min) / (trace->max - trace->min))*priv->h);
				cairo_move_to(cr,start_x,start_y);
				x = priv->w-j;
				y = priv->h - (((g_array_index(trace->history,gfloat,trace->history->len-(j+1))-trace->min) / (trace->max - trace->min))*priv->h);
				/*
				if (i == 1)
					printf("history->len is %i, J is %i, Line from (%f - %f)\n",trace->history->len,j, g_array_index(trace->history,gfloat,trace->history->len-(j))),g_array_index(trace->history,gfloat,trace->history->len-(j+1));
					*/
				cairo_line_to(cr,x,y);
				cairo_stroke(cr);
			}
		}
	}
	cairo_destroy(cr);

	/* Copy background trace pixmap to grat for grat rendering */

	cr = gdk_cairo_create(priv->grat_pixmap);
	gdk_cairo_set_source_pixmap(cr,priv->trace_pixmap,0,0);
	cairo_rectangle(cr,0,0,priv->w,priv->h);
	cairo_fill(cr);

	/* Render the graticule lines */

	cairo_set_source_rgba (cr, 
			priv->colors[COL_GRAT].red/65535.0,
			priv->colors[COL_GRAT].green/65535.0,
			priv->colors[COL_GRAT].blue/65535.0,
			0.5);
	cairo_move_to(cr,0,priv->h/4);
	cairo_line_to(cr,priv->w,priv->h/4);
	cairo_stroke(cr);
	cairo_move_to(cr,0,priv->h/2);
	cairo_line_to(cr,priv->w,priv->h/2);
	cairo_stroke(cr);
	cairo_move_to(cr,0,priv->h*3/4);
	cairo_line_to(cr,priv->w,priv->h*3/4);
	cairo_stroke(cr);

	cairo_set_font_options(cr,priv->font_options);
	tmpbuf = g_utf8_strup(priv->font,-1);
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
	cairo_select_font_face (cr, priv->font,  slant, weight);

	cairo_set_font_size (cr, 12);

	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	buffer = 0;
	text_offset[BOTTOM] = 0.0;
	message = g_strdup_printf("123");
	cairo_text_extents(cr,message,&extents);
	if ((extents.height * 4) > (priv->h/4))
		draw_quarters = FALSE;
	else
		draw_quarters = TRUE;
	g_free(message);
	/* render the new data */

	cr2 = gdk_cairo_create(priv->grat_pixmap);
	cairo_set_source_rgba(cr2,0.13,0.13,0.13,0.75);

	for (i=0;i<priv->num_traces;i++)
	{
		trace = g_array_index(priv->traces,MtxStripChartTrace *,i);
		cairo_set_source_rgb (cr, 
				trace->color.red/65535.0,
				trace->color.green/65535.0,
				trace->color.blue/65535.0
				);
		message = g_strdup_printf("%1$.*2$f", trace->min,trace->precision);
		cairo_text_extents (cr, message, &extents);
		cairo_rectangle(cr2,2.0+text_offset[BOTTOM],priv->h-2.0,extents.width,-extents.height);
		cairo_fill(cr2);
		cairo_move_to(cr,2.0+text_offset[BOTTOM],priv->h-2.0);

		cairo_show_text (cr, message);
		g_free(message);
		text_offset[BOTTOM] += extents.width + 7;

		if (draw_quarters)
		{
			message = g_strdup_printf("%1$.*2$f", trace->min+((trace->max-trace->min)/4),trace->precision);
			cairo_text_extents (cr, message, &extents);
			cairo_rectangle(cr2,2.0+text_offset[QUARTER],priv->h*3/4-2.0,extents.width,-extents.height);
			cairo_fill(cr2);
			cairo_move_to(cr,2.0+text_offset[QUARTER],(priv->h*3/4)-2.0);
			cairo_show_text (cr, message);
			g_free(message);
			text_offset[QUARTER] += extents.width + 7;
		}

		message = g_strdup_printf("%1$.*2$f", trace->min+((trace->max-trace->min)/2),trace->precision);
		cairo_text_extents (cr, message, &extents);
		cairo_rectangle(cr2,2.0+text_offset[HALF],(priv->h/2.0)-2.0,extents.width,-extents.height);
		cairo_fill(cr2);
		cairo_move_to(cr,2.0+text_offset[HALF],(priv->h/2.0)-2.0);
		cairo_show_text (cr, message);
		g_free(message);
		text_offset[HALF] += extents.width + 7;

		if (draw_quarters)
		{
			message = g_strdup_printf("%1$.*2$f", trace->min+((trace->max-trace->min)*3/4),trace->precision);
			cairo_text_extents (cr, message, &extents);
			cairo_rectangle(cr2,2.0+text_offset[THREEQUARTER],priv->h/4-2.0,extents.width,-extents.height);
			cairo_fill(cr2);
			cairo_move_to(cr,2.0+text_offset[THREEQUARTER],(priv->h/4)-2.0);
			cairo_show_text (cr, message);
			g_free(message);
			text_offset[THREEQUARTER] += extents.width + 7;
		}

		message = g_strdup_printf("%1$.*2$f", trace->max,trace->precision);
		cairo_text_extents (cr, message, &extents);
		cairo_rectangle(cr2,2.0+text_offset[TOP],2.0,extents.width,extents.height);
		cairo_fill(cr2);
		cairo_move_to(cr,2.0+text_offset[TOP],extents.height+2.0);
		cairo_show_text (cr, message);
		g_free(message);
		text_offset[TOP] += extents.width + 7;

		/* Trace names */

		message = g_strdup_printf("%s", trace->name);
		cairo_text_extents (cr, message, &extents);
		cairo_rectangle(cr2,priv->w-extents.width - 20, 2.0+buffer +extents.height,extents.width,-extents.height);
		cairo_fill(cr2);
		cairo_move_to(cr,priv->w-extents.width - 20, 2.0 + buffer + extents.height);
		cairo_show_text (cr, message);
		g_free(message);
		buffer += (extents.height + 3);

	}
	cairo_destroy(cr);
	cairo_destroy(cr2);
}


/*!
 \brief handles configure events when the chart gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget is the pointer to the chart object
 \param event is the pointer to the GdkEventConfigure structure that
 encodes important info like window dimensions and depth.
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_stripchart_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	MtxStripChart * chart = MTX_STRIPCHART(widget);
	MtxStripChartPrivate *priv = NULL;
	cairo_t *cr = NULL;
	GtkAllocation allocation;
	GdkWindow *window = gtk_widget_get_window(widget);

	g_return_val_if_fail(MTX_IS_STRIPCHART(widget),FALSE);
	gtk_widget_get_allocation(widget,&allocation);
	priv = MTX_STRIPCHART_GET_PRIVATE(chart);

	priv->w = allocation.width;
	priv->h = allocation.height;

	/* Backing pixmap (copy of window) */

	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	priv->bg_pixmap=gdk_pixmap_new(window,
			priv->w,priv->h,
			-1);
	cr = gdk_cairo_create(priv->bg_pixmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_destroy(cr);
	/* Trace pixmap */

	if (priv->trace_pixmap)
		g_object_unref(priv->trace_pixmap);
	priv->trace_pixmap=gdk_pixmap_new(window,
			priv->w,priv->h,
			-1);
	cr = gdk_cairo_create(priv->trace_pixmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_destroy(cr);
	/* Grat pixmap */

	if (priv->grat_pixmap)
		g_object_unref(priv->grat_pixmap);
	priv->grat_pixmap=gdk_pixmap_new(window,
			priv->w,priv->h,
			-1);
	cr = gdk_cairo_create(priv->grat_pixmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_destroy(cr);

	gdk_window_set_back_pixmap(window,priv->bg_pixmap,0);

	if (priv->font_options)
		cairo_font_options_destroy(priv->font_options);
	priv->font_options = cairo_font_options_create();
	cairo_font_options_set_antialias(priv->font_options,
			CAIRO_ANTIALIAS_GRAY);

	generate_stripchart_static_traces(chart);
	render_marker (chart);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget is the pointer to the chart object
 \param event is the pointer to the GdkEventExpose structure that
 encodes important info like window dimensions and depth.
 \returns FALSE
 */
gboolean mtx_stripchart_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxStripChart * chart = MTX_STRIPCHART(widget);
	MtxStripChartPrivate *priv = NULL;
	cairo_t *cr = NULL;
	GdkWindow *window = gtk_widget_get_window(widget);

	g_return_val_if_fail(MTX_IS_STRIPCHART(widget),FALSE);
	priv = MTX_STRIPCHART_GET_PRIVATE(chart);

#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_is_sensitive(GTK_WIDGET(widget)))
#else
		if (GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(widget)))
#endif
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_pixmap(cr,priv->bg_pixmap,0,0);
			cairo_rectangle(cr,event->area.x, event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
		else
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_pixmap(cr,priv->bg_pixmap,0,0);
			cairo_rectangle(cr,event->area.x, event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
			cairo_paint(cr);
			/*
			   cairo_rectangle (cr,
			   0,0,priv->w,priv->h);
			   cairo_fill(cr);
			 */

			cairo_destroy(cr);
		}
	return FALSE;
}


/*!
 \brief draws the static part of the stripchart,  i.e. this is only the 
 history of the trace, as there's no point to re-render the whole fucking thing
 every damn time when only 1 datapoint is being appended,  This writes to a
 pixmap, which is shifted and new trace data rendered, before graticaule/text
 overlay is added on.
 \param chart is the pointer to the chart object
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
	GtkAllocation allocation;

	gtk_widget_get_allocation(GTK_WIDGET(chart),&allocation);

	w = allocation.width;
	h = allocation.height;

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
		if (!trace)
			continue;


		cairo_set_line_width(cr,trace->lwidth);
		cairo_set_source_rgb (cr, 
				trace->color.red/65535.0,
				trace->color.green/65535.0,
				trace->color.blue/65535.0);
		points = trace->history->len < (guint)priv->w ? trace->history->len-1:priv->w;
		if (points < 1)
			continue;
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


/*!
  \brief handler for mouse motion on the stripchart, stores the current mouse
  coords
  \param chart is the pointer to the chart object
  \param event is the pointer to the GdkEventMotion structure containing
  details about the motion event
  \returns FALSE so other handlers run
  */
gboolean mtx_stripchart_motion_event (GtkWidget *chart,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */

	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(MTX_STRIPCHART(chart));
	priv->mouse_x = event->x;
	priv->mouse_y = event->y;
	return FALSE;
}
					       


/*!
 \brief sets the INITIAL sizeof the widget
 \param widget is the pointer to the chart widget
 \param requisition is the pointer to the GdkRequisition structure
 */
void mtx_stripchart_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = 140;
	requisition->height = 80;
}


/*!
 \brief gets called to redraw the entire display manually
 \param chart is the pointer to the chart object
 */
void mtx_stripchart_redraw (MtxStripChart *chart)
{
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(chart));
	if (!window) 
		return;

	update_stripchart_position(chart);
	render_marker(chart);
	gdk_window_clear(window);
}


/*!
  \brief Event handler for when the mouse enters/leaves the window
  \param widget is the pointer to the chart object
  \param event is the pointer to the GdkEventCrossing structure
  \returns TRUE on success FALSE otherwise
  */
gboolean mtx_stripchart_enter_leave_event(GtkWidget *widget, GdkEventCrossing *event)
{
	MtxStripChartPrivate *priv = NULL;
	g_return_val_if_fail(MTX_IS_STRIPCHART(widget),FALSE);
	priv = MTX_STRIPCHART_GET_PRIVATE(MTX_STRIPCHART(widget));

	if (event->type == GDK_ENTER_NOTIFY)
		priv->mouse_tracking = TRUE;
	else
		priv->mouse_tracking = FALSE;

	return TRUE;
}


/*!
  \brief  renders a marker at the current mouse position
  \param chart is the pointer to the chart object
  */
void render_marker(MtxStripChart *chart)
{
	cairo_t *cr = NULL;
	gint i = 0;
	gint buffer = 0;
	gfloat val = 0.0;
	cairo_text_extents_t extents;
	gchar *message = NULL;
	GtkWidget *widget = GTK_WIDGET(chart);
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	GtkAllocation allocation;

	gtk_widget_get_allocation(widget,&allocation);
	/* Copy trace+graticule to backing pixmap */

	cr = gdk_cairo_create(priv->bg_pixmap);
	gdk_cairo_set_source_pixmap(cr,priv->grat_pixmap,0,0);
	cairo_rectangle(cr,0,0,allocation.width,allocation.height);
	cairo_fill(cr);

	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	cairo_select_font_face (cr, priv->font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size (cr, 12);

	cairo_set_line_width(cr,2);
	cairo_set_source_rgb (cr, 1.0,1.0,1.0);
	message = g_strdup_printf("123");
	cairo_text_extents(cr,message,&extents);
	g_free(message);
	buffer = extents.height + 3;
	if (priv->mouse_tracking)
	{
		cairo_move_to(cr,priv->mouse_x,0);
		cairo_line_to(cr,priv->mouse_x,priv->h);
		cairo_stroke(cr);
		for (i=0; i<priv->num_traces;i++)
		{
			trace = g_array_index(priv->traces,MtxStripChartTrace *, i);
			if ((guint)(priv->w-(gint)priv->mouse_x) > trace->history->len)
				val = trace->min;
			else
				val = g_array_index(trace->history, gfloat, trace->history->len-(priv->w-(gint)priv->mouse_x));

			message = g_strdup_printf("%1$.*2$f", val,trace->precision);
			cairo_set_source_rgb (cr, 
					trace->color.red/65535.0,
					trace->color.green/65535.0,
					trace->color.blue/65535.0);
			cairo_text_extents(cr,message,&extents);
			cairo_move_to(cr,priv->w-20-extents.width, priv->h-(priv->num_traces-i-1)*buffer - extents.height);
			cairo_show_text(cr,message);
			g_free(message);
		}
	}
	else
	{
		for (i=0; i<priv->num_traces;i++)
		{
			trace = g_array_index(priv->traces,MtxStripChartTrace *, i);
			if (trace->history->len < 1)
				continue;
			val = g_array_index(trace->history, gfloat, trace->history->len-1);
			message = g_strdup_printf("%1$.*2$f", val,trace->precision);
			cairo_set_source_rgb (cr, 
					trace->color.red/65535.0,
					trace->color.green/65535.0,
					trace->color.blue/65535.0);
			cairo_text_extents(cr,message,&extents);
			cairo_move_to(cr,priv->w-20-extents.width, priv->h-(priv->num_traces-i-1)*buffer - extents.height);
			cairo_show_text(cr,message);
			g_free(message);
		}
	}
	cairo_destroy(cr);
}
