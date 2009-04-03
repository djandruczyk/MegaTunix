/*
 * Copyright (C) 2007 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt curve widget
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
#include <curve.h>
#include <curve-private.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


static guint mtx_curve_signals[LAST_SIGNAL] = { 0 };

GType mtx_curve_get_type(void)
{
	static GType mtx_curve_type = 0;

	if (!mtx_curve_type)
	{
		static const GTypeInfo mtx_curve_info =
		{
			sizeof(MtxCurveClass),
			NULL,
			NULL,
			(GClassInitFunc) mtx_curve_class_init,
			NULL,
			NULL,
			sizeof(MtxCurve),
			0,
			(GInstanceInitFunc) mtx_curve_init,
		};
		mtx_curve_type = g_type_register_static(GTK_TYPE_DRAWING_AREA, "MtxCurve", &mtx_curve_info, 0);
	}
	return mtx_curve_type;
}

/*!
 \brief Initializes the mtx pie curve class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param klass (MtxCurveClass *) pointer to the class
 */
void mtx_curve_class_init (MtxCurveClass *klass)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;

	gobject_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_curve_configure;
	widget_class->expose_event = mtx_curve_expose;
	widget_class->button_press_event = mtx_curve_button_event;
	widget_class->button_release_event = mtx_curve_button_event;
	widget_class->enter_notify_event = mtx_curve_focus_event;
	widget_class->leave_notify_event = mtx_curve_focus_event;
	/* Motion event not needed, as unused currently */
	widget_class->motion_notify_event = mtx_curve_motion_event; 
	widget_class->size_request = mtx_curve_size_request;

	g_type_class_add_private (klass, sizeof (MtxCurvePrivate)); 
	mtx_curve_signals[CHANGED_SIGNAL] = 
		g_signal_new("coords-changed", G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (MtxCurveClass, coords_changed),
		NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


/*!
 \brief Initializes the curve attributes to sane defaults
 \param curve (MtxCurve *) pointer to the curve object
 */
void mtx_curve_init (MtxCurve *curve)
{
	/* The events the curve receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/ 
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	gtk_widget_add_events (GTK_WIDGET (curve),GDK_BUTTON_PRESS_MASK
			        |GDK_BUTTON_RELEASE_MASK
				|GDK_POINTER_MOTION_MASK
				|GDK_POINTER_MOTION_HINT_MASK
				|GDK_ENTER_NOTIFY_MASK
				|GDK_LEAVE_NOTIFY_MASK);
	priv->w = 100;		
	priv->h = 100;
	priv->cr = NULL;
	priv->colormap = gdk_colormap_get_system();
	priv->gc = NULL;
	priv->points = NULL;
	priv->coords = NULL;
	priv->num_points = 0;
	priv->border = 5;
	priv->x_precision = 0;
	priv->y_precision = 0;
	priv->x_scale = 1.0;
	priv->y_scale = 1.0;
	priv->locked_scale = 1.0;
	priv->font = g_strdup("Sans 10");
	priv->active_coord = -1;
	priv->vertex_selected = FALSE;
	priv->show_vertexes = FALSE;
	priv->show_grat = TRUE;
	priv->show_x_marker = FALSE;
	priv->show_y_marker = FALSE;
	priv->coord_changed = FALSE;
	priv->auto_hide = TRUE;
	priv->vertex_id = 0;
	mtx_curve_init_colors(curve);
}



/*!
 \brief Allocates the default colors for a curve with no options 
 \param widget (MegaCurve *) pointer to the curve object
 */
void mtx_curve_init_colors(MtxCurve *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	/*! Main Background */
	priv->colors[CURVE_COL_BG].red=0.0*65535;
	priv->colors[CURVE_COL_BG].green=0.0*65535;
	priv->colors[CURVE_COL_BG].blue=0.0*65535;
	/*! Trace */
	priv->colors[CURVE_COL_FG].red=0.8*65535;
	priv->colors[CURVE_COL_FG].green=0.8*65535;
	priv->colors[CURVE_COL_FG].blue=0.8*65535;
	/*! Selected Trace */
	priv->colors[CURVE_COL_SEL].red=0.9*65535;
	priv->colors[CURVE_COL_SEL].green=0.0*65535;
	priv->colors[CURVE_COL_SEL].blue=0.0*65535;
	/*! Graticule Color*/
	priv->colors[CURVE_COL_GRAT].red=0.50*65535;
	priv->colors[CURVE_COL_GRAT].green=0.50*65535;
	priv->colors[CURVE_COL_GRAT].blue=0.50*65535;
	/*! Text/Title Color */
	priv->colors[CURVE_COL_TEXT].red=1.0*65535;
	priv->colors[CURVE_COL_TEXT].green=1.0*65535;
	priv->colors[CURVE_COL_TEXT].blue=1.0*65535;
	/*! Marker Lines Color */
	priv->colors[CURVE_COL_MARKER].red=1.0*65535;
	priv->colors[CURVE_COL_MARKER].green=0.2*65535;
	priv->colors[CURVE_COL_MARKER].blue=0.2*65535;
}


/*!
 \brief updates the curve position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower than a raw GDK version
 \param widget (MtxCurve *) pointer to the curve object
 */
void update_curve_position (MtxCurve *curve)
{
	GtkWidget * widget = NULL;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	gchar * tmpbuf = NULL;
	gint i = 0;
	cairo_t *cr = NULL;
	gchar * message = NULL;
	cairo_text_extents_t extents;
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);

	widget = GTK_WIDGET(curve);

	/* Copy background pixmap to intermediary for final rendering */
	gdk_draw_drawable(priv->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);

	cr = gdk_cairo_create (priv->pixmap);
	cairo_set_font_options(cr,priv->font_options);

	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);

	/* curve  */

	cairo_set_source_rgb (cr, priv->colors[CURVE_COL_FG].red/65535.0,
			priv->colors[CURVE_COL_FG].green/65535.0,
			priv->colors[CURVE_COL_FG].blue/65535.0);
	cairo_set_line_width (cr, 1.5);

	/* The "curve" itself */
	for (i=0;i<priv->num_points-1;i++)
	{
		cairo_move_to (cr, priv->points[i].x,priv->points[i].y);
		cairo_line_to (cr, priv->points[i+1].x,priv->points[i+1].y);
	}
	cairo_stroke(cr);
	/* The circles for each vertex itself */
	if (priv->show_vertexes)
	{
		cairo_set_source_rgb (cr, 0.0,0.7,1.0);
		for (i=0;i<priv->num_points;i++)
		{
			cairo_arc(cr,priv->points[i].x,priv->points[i].y,3,0,2*M_PI);
			cairo_move_to (cr, priv->points[i].x,priv->points[i].y);
		}
		cairo_fill(cr);
	}
	/* Selected vertex drawn in alt color... */
	if (priv->vertex_selected)
	{
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_SEL].red/65535.0,
				priv->colors[CURVE_COL_SEL].green/65535.0,
				priv->colors[CURVE_COL_SEL].blue/65535.0);
		cairo_move_to (cr, priv->points[priv->active_coord].x,priv->points[priv->active_coord].y);
		cairo_new_sub_path(cr);
		cairo_arc(cr,priv->points[priv->active_coord].x,priv->points[priv->active_coord].y,4,0,2*M_PI);
	}
	cairo_stroke(cr);

	/* Vertical and Horizontal Markers */
	cairo_set_source_rgb (cr, priv->colors[CURVE_COL_MARKER].red/65535.0,
			priv->colors[CURVE_COL_MARKER].green/65535.0,
			priv->colors[CURVE_COL_MARKER].blue/65535.0);
	cairo_set_line_width (cr, 2.0);
	if (priv->show_x_marker)
	{
		cairo_move_to (cr, ((priv->x_marker-priv->lowest_x)*priv->x_scale) + priv->border,0);
		cairo_line_to (cr, ((priv->x_marker-priv->lowest_x)*priv->x_scale) + priv->border,priv->h);
		cairo_stroke(cr);
	}
	if (priv->show_y_marker)
	{
		cairo_move_to (cr, 0,((priv->y_marker-priv->lowest_y)*priv->x_scale) + priv->border);
		cairo_line_to (cr, priv->w,((priv->y_marker-priv->lowest_y)*priv->x_scale) + priv->border);
		cairo_stroke(cr);
	}

	/* Update the Title text */
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

	if (priv->vertex_selected)
	{
		cairo_set_font_size (cr, 9);
		message = g_strdup_printf("(%1$.*2$f,%3$.*4$f)",
				priv->coords[priv->active_coord].x,
				priv->x_precision,
				priv->coords[priv->active_coord].y,
				priv->y_precision);
		cairo_text_extents (cr, message, &extents);
		cairo_move_to(cr,priv->points[priv->active_coord].x,
				priv->points[priv->active_coord].y);
		cairo_set_source_rgb (cr, 0.1,1.0,0.1); 
		cairo_line_to (cr, 
				priv->w - (extents.width/2.0) - (3*priv->border),
				(extents.height*7)+ priv->border);
		cairo_stroke (cr);
		cairo_set_source_rgba (cr,0,0,0,0.75);
		cairo_rectangle(cr,priv->w - extents.width - (4*priv->border),
                                (extents.height*7) + (priv->border),
				extents.width + (2*priv->border),
				-(extents.height + (2*priv->border)));
		cairo_fill (cr);
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_TEXT].red/65535.0,
				priv->colors[CURVE_COL_TEXT].green/65535.0,
				priv->colors[CURVE_COL_TEXT].blue/65535.0);
		cairo_move_to (cr, 
				priv->w - extents.width - (3*priv->border),
				extents.height*7);
		cairo_show_text (cr, message);
		g_free(message);

		cairo_stroke (cr);
	}

	cairo_destroy(cr);
}


/*!
 \brief handles configure events whe nthe curve gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget (GtkWidget *) pointer to the curve object
 \param event (GdkEventConfigure *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_curve_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	MtxCurve * curve = MTX_CURVE(widget);
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);

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
	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	priv->bg_pixmap=gdk_pixmap_new(widget->window,
			priv->w,priv->h,
			gtk_widget_get_visual(widget)->depth);
	gdk_draw_rectangle(priv->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			priv->w,priv->h);

	gdk_window_set_back_pixmap(widget->window,priv->pixmap,0);
	priv->gc = gdk_gc_new(priv->bg_pixmap);
	gdk_gc_set_colormap(priv->gc,priv->colormap);


	if (priv->font_options)
		cairo_font_options_destroy(priv->font_options);
	priv->font_options = cairo_font_options_create();
	cairo_font_options_set_antialias(priv->font_options,
			CAIRO_ANTIALIAS_GRAY);

	recalc_extremes(priv);
	generate_curve_background(curve);
	update_curve_position(curve);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget (GtkWidget *) pointer to the curve object
 \param event (GdkEventExpose *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_curve_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxCurve * curve = MTX_CURVE(widget);
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}


/*!
 \brief draws the static elements of the curve (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param widget (MtxCurve *) pointer to the curve object
 */
void generate_curve_background(MtxCurve *curve)
{
	cairo_t *cr = NULL;
	gint w = 0;
	gint h = 0;
	gfloat tmpf = 0.0;
	gint max_lines;
	gint spread = 0;
	gint i = 0;
	gchar * message = NULL;
	cairo_text_extents_t extents;
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);

	w = GTK_WIDGET(curve)->allocation.width;
	h = GTK_WIDGET(curve)->allocation.height;

	if (!priv->bg_pixmap)
		return;
	/* get a cairo_t */
	cr = gdk_cairo_create (priv->bg_pixmap);
	cairo_set_font_options(cr,priv->font_options);
	cairo_set_source_rgb (cr, 
			priv->colors[CURVE_COL_BG].red/65535.0,
			priv->colors[CURVE_COL_BG].green/65535.0,
			priv->colors[CURVE_COL_BG].blue/65535.0);
	/* Background Rectangle */
	cairo_rectangle (cr,
			0,0,w,h);
	cairo_fill(cr);
	if (priv->show_grat)
	{
		cairo_set_source_rgba (cr, 
				priv->colors[CURVE_COL_GRAT].red/65535.0,
				priv->colors[CURVE_COL_GRAT].green/65535.0,
				priv->colors[CURVE_COL_GRAT].blue/65535.0,
				0.5);
		max_lines = (priv->w - 2*priv->border)/50;
		tmpf = ((priv->w - 2*priv->border)%50)/50.0;
		if (tmpf > 0.5)
			max_lines++;
		if (max_lines == 0)
			max_lines = 1;
		spread = (priv->w - 2*priv->border)/max_lines;
		for(i = 0;i<max_lines;i++)
		{
			cairo_move_to (cr,priv->border+i*spread,0);
			cairo_line_to (cr,priv->border+i*spread,priv->h);
			cairo_stroke(cr);
		}
		max_lines = (priv->h - 2*priv->border)/50;
		tmpf = ((priv->h - 2*priv->border)%50)/50.0;
		if (tmpf > 0.5)
			max_lines++;
		if (max_lines == 0)
			max_lines = 1;
		spread = (priv->h - 2*priv->border)/max_lines;
		for(i = 0;i<max_lines;i++)
		{
			cairo_move_to (cr,0,priv->border+i*spread);
			cairo_line_to (cr,priv->w,priv->border+i*spread);
			cairo_stroke(cr);
		}

	}
	cairo_set_font_size (cr, 15);
	if (priv->title)
	{
		message = g_strdup_printf("%s", priv->title);

		cairo_text_extents (cr, message, &extents);

		cairo_set_source_rgba (cr,0.0,0,0,0.75);
		cairo_rectangle(cr,priv->w/2 - (extents.width/2)-priv->border,
				extents.height-priv->border,
				extents.width+(2*priv->border),
				extents.height+(2*priv->border));
		cairo_fill (cr);
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_TEXT].red/65535.0,
				priv->colors[CURVE_COL_TEXT].green/65535.0,
				priv->colors[CURVE_COL_TEXT].blue/65535.0);
		cairo_move_to (cr, 
				priv->w/2-(extents.width/2),
				(extents.height*2));

		cairo_show_text (cr, message);
		g_free(message);
	}
	cairo_stroke (cr);

}


gboolean mtx_curve_motion_event (GtkWidget *curve,GdkEventMotion *event)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	gint i = 0;
	gchar * tmpbuf = 0;
	if (!priv->vertex_selected)
		return FALSE;
	i = priv->active_coord;
	priv->points[i].x = event->x;
	priv->points[i].y = event->y;

	tmpbuf = g_strdup_printf("%1$.*2$f",(gfloat)((event->x- priv->border)/priv->x_scale) + priv ->lowest_x, priv->x_precision);
	priv->coords[i].x = (gfloat)g_strtod(tmpbuf,NULL);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%1$.*2$f",(gfloat)(-((event->y - priv->h + priv->border)/priv->y_scale) + priv ->lowest_y),priv->y_precision);
	priv->coords[i].y = (gfloat)g_strtod(tmpbuf,NULL);
	g_free(tmpbuf);
	recalc_extremes(priv);
	priv->coord_changed = TRUE;
	mtx_curve_redraw(MTX_CURVE(curve));
	return TRUE;
}
					       

gboolean mtx_curve_button_event (GtkWidget *curve,GdkEventButton *event)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	gint i = 0;
	if ((event->button == 1 ) && (event->type == GDK_BUTTON_PRESS))	
	{
		for (i=0;i<priv->num_points;i++)
		{
			if (((abs(event->x - priv->points[i].x)) < 10) && (abs(event->y - priv->points[i].y) < 10))
			{
				priv->active_coord = i;
				priv->vertex_selected = TRUE;
				priv->coord_changed = FALSE;
				mtx_curve_redraw(MTX_CURVE(curve));
				return TRUE;
			}

		}
		priv->active_coord = -1;
		priv->vertex_selected = FALSE;
		mtx_curve_redraw(MTX_CURVE(curve));
	}
	if ((event->button == 1 ) && 
			(event->type == GDK_BUTTON_RELEASE) &&
			(priv->vertex_selected))	
	{
		priv->vertex_selected = FALSE;

		mtx_curve_redraw(MTX_CURVE(curve));
		if (priv->coord_changed)
			g_signal_emit_by_name((gpointer)curve, "coords-changed");
		return TRUE;
	}
	return FALSE;
}
					       


/*!
 \brief sets the INITIAL sizeof the widget
 \param curve (GtkWidget *) pointer to the curve widget
 \param requisition (GdkRequisition *) struct to set the vars within
 \returns void
 */
void mtx_curve_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = 100;
	requisition->height = 100;
}


/*!
 \brief gets called to redraw the entire display manually
 \param curve (MtxCurve *) pointer to the curve object
 */
void mtx_curve_redraw (MtxCurve *curve)
{
	update_curve_position (curve);
	gdk_window_clear(GTK_WIDGET(curve)->window);
}


/*!
 \brief Recalculates the extremes of all coords in the graph
 \param priv (MtxCurvePrivate *) pointer to private data
 */
void recalc_extremes(MtxCurvePrivate *priv)
{
        gint i = 0;
        priv->highest_x = -10000;
        priv->highest_y = -10000;
        priv->lowest_x = 10000;
        priv->lowest_y = 10000;
        for (i=0;i<priv->num_points;i++)
        {
                if (priv->coords[i].x < priv->lowest_x)
                        priv->lowest_x = priv->coords[i].x;
                if (priv->coords[i].x > priv->highest_x)
                        priv->highest_x = priv->coords[i].x;
                if (priv->coords[i].y < priv->lowest_y)
                        priv->lowest_y = priv->coords[i].y;
                if (priv->coords[i].y > priv->highest_y)
                        priv->highest_y = priv->coords[i].y;
        }
	priv->x_scale = (gfloat)(priv->w-(2*priv->border))/(priv->highest_x - priv->lowest_x + 0.000001);
	priv->y_scale = (gfloat)(priv->h-(2*priv->border))/(priv->highest_y - priv->lowest_y + 0.000001);
	priv->locked_scale = (priv->x_scale < priv->y_scale) ? priv->x_scale:priv->y_scale;

	if(priv->points)
		g_free(priv->points);
	priv->points = g_new0(GdkPoint, priv->num_points);

	/* Convert from user provided floating point coords to integer X,Y 
 	 * coords so things display nicely.  NOTE: motion event will do a 
 	 * reverse conversion to take screen coords to original values, and
 	 * take into account specified precision so that signals feed useful
 	 * data back to connected handlers
 	 */
	for (i=0;i<priv->num_points;i++)
	{
		priv->points[i].x = (gint)(((priv->coords[i].x - priv->lowest_x)*priv->x_scale) + priv->border);
		priv->points[i].y = (gint)(priv->h - (((priv->coords[i].y - priv->lowest_y)*priv->y_scale) + priv->border));
	}

        /*printf("Extremes, X %i,%i, Y %i,%i\n",priv->lowest_x,priv->highest_x, priv->lowest_y, priv->highest_y);
 	*/
}


gboolean mtx_curve_focus_event (GtkWidget *curve, GdkEventCrossing *event)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	if ((event->type == GDK_ENTER_NOTIFY) && (priv->auto_hide))
	{
		if (priv->vertex_id > 0)
			g_source_remove(priv->vertex_id);
		priv->show_vertexes = TRUE;
		mtx_curve_redraw(MTX_CURVE(curve));
	}
	if ((event->type == GDK_LEAVE_NOTIFY) && (priv->auto_hide))
		priv->vertex_id = g_timeout_add(2000,delay_turnoff_vertexes,curve);
	return FALSE;
}


gboolean delay_turnoff_vertexes(gpointer data)
{
	MtxCurve *curve = (MtxCurve*) data;
	if (!GTK_IS_WIDGET(curve))
		return FALSE;

	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	priv->show_vertexes = FALSE;
	mtx_curve_redraw(MTX_CURVE(curve));
	return FALSE;
}
