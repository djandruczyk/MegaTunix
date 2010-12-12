/*
 * Copyright (C) 2007 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix curve widget
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 *  
 */


#include <config.h>
#include <defines.h>
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
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_curve_configure;
	widget_class->expose_event = mtx_curve_expose;
	widget_class->button_press_event = mtx_curve_button_event;
	widget_class->button_release_event = mtx_curve_button_event;
	widget_class->enter_notify_event = mtx_curve_focus_event;
	widget_class->leave_notify_event = mtx_curve_focus_event;
	widget_class->motion_notify_event = mtx_curve_motion_event; 
	widget_class->size_request = mtx_curve_size_request;
	obj_class->finalize = mtx_curve_finalize;

	g_type_class_add_private (klass, sizeof (MtxCurvePrivate)); 
	mtx_curve_signals[CHANGED_SIGNAL] = 
		g_signal_new("coords-changed", G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (MtxCurveClass, coords_changed),
		NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	mtx_curve_signals[VERTEX_PROXIMITY_SIGNAL] = 
		g_signal_new("vertex-proximity", G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (MtxCurveClass, vertex_proximity),
		NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	mtx_curve_signals[MARKER_PROXIMITY_SIGNAL] = 
		g_signal_new("marker-proximity", G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (MtxCurveClass, marker_proximity),
		NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


/*!
 \brief Finalizes the curve object on destruction
 \param curve (GObject *) pointer to the curve object
 */
void mtx_curve_finalize (GObject *curve)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	if (priv->pixmap)
		g_object_unref(priv->pixmap);
	if (priv->bg_pixmap)
		g_object_unref(priv->bg_pixmap);
	if (priv->coords)
		g_free(priv->coords);
	if (priv->points)
		g_free(priv->points);
	if (priv->gc)
		g_object_unref(priv->gc);
	if (priv->cr)
		cairo_destroy(priv->cr);
	if (priv->font_options)
		cairo_font_options_destroy(priv->font_options);
	if (priv->pos_str)
		g_free(priv->pos_str);
	if (priv->font)
		g_free(priv->font);
	if (priv->axis_font)
		g_free(priv->axis_font);
	if (priv->title)
		g_free(priv->title);
	if (priv->x_axis_label)
		g_free(priv->x_axis_label);
	if (priv->y_axis_label)
		g_free(priv->y_axis_label);
	if (priv->colormap)
		g_object_unref(priv->colormap);
}


/*!
 \brief Initializes the curve attributes to sane defaults
 \param curve (MtxCurve *) pointer to the curve object
 */
void mtx_curve_init (MtxCurve *curve)
{
	/* The events the curve receives
	* Need events for button press/release AND motion AND enter/leave
	*/ 
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	gtk_widget_add_events (GTK_WIDGET (curve),GDK_BUTTON_PRESS_MASK
			        |GDK_BUTTON_RELEASE_MASK
				|GDK_POINTER_MOTION_MASK
				|GDK_POINTER_MOTION_HINT_MASK
				|GDK_ENTER_NOTIFY_MASK
				|GDK_LEAVE_NOTIFY_MASK);
	priv->self = curve;
	priv->auto_rescale_id = 0;
	priv->w = 100;		
	priv->h = 100;
	priv->cr = NULL;
	priv->colormap = gdk_colormap_get_system();
	priv->gc = NULL;
	priv->points = NULL;
	priv->coords = NULL;
	priv->num_points = 0;
	priv->proximity_threshold = 10;
	priv->proximity_vertex = -1;
	priv->marker_proximity_vertex = -1;
	priv->x_border = 30;
	priv->y_border = 30;
	priv->x_precision = 0;
	priv->y_precision = 0;
	priv->x_scale = 1.0;
	priv->y_scale = 1.0;
	priv->locked_scale = 1.0;
	priv->font = g_strdup("Sans 10");
	priv->axis_font = g_strdup("Sans 6");
	priv->active_coord = -1;
	priv->vertex_selected = FALSE;
	priv->show_vertexes = FALSE;
	priv->show_grat = TRUE;
	priv->show_edit_marker = TRUE;
	priv->show_x_marker = FALSE;
	priv->show_y_marker = FALSE;
	priv->coord_changed = FALSE;
	priv->x_blocked_from_edit = FALSE;
	priv->y_blocked_from_edit = FALSE;
	priv->auto_hide = TRUE;
	priv->vertex_id = 0;
	priv->x_marker = 0.0;
	priv->y_marker = 0.0;
	priv->x_lower_limit=-65536;
	priv->y_lower_limit=-65536;
	priv->x_upper_limit=65536;
	priv->y_upper_limit=65536;
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
	/*! Proximity */
	priv->colors[CURVE_COL_PROX].red=0.9*65535;
	priv->colors[CURVE_COL_PROX].green=0.0*65535;
	priv->colors[CURVE_COL_PROX].blue=0.9*65535;
	/*! Graticule Color*/
	priv->colors[CURVE_COL_GRAT].red=0.50*65535;
	priv->colors[CURVE_COL_GRAT].green=0.50*65535;
	priv->colors[CURVE_COL_GRAT].blue=0.50*65535;
	/*! Text/Title Color */
	priv->colors[CURVE_COL_TEXT].red=1.0*65535;
	priv->colors[CURVE_COL_TEXT].green=1.0*65535;
	priv->colors[CURVE_COL_TEXT].blue=1.0*65535;
	/*! Marker Lines Color */
	priv->colors[CURVE_COL_MARKER].red=0.2*65535;
	priv->colors[CURVE_COL_MARKER].green=1.0*65535;
	priv->colors[CURVE_COL_MARKER].blue=0.2*65535;
	/*! Marker Lines Color */
	priv->colors[CURVE_COL_EDIT].red=1.0*65535;
	priv->colors[CURVE_COL_EDIT].green=0.2*65535;
	priv->colors[CURVE_COL_EDIT].blue=0.2*65535;
}


/*!
 \brief updates the curve position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower than a raw GDK version
 \param widget (MtxCurve *) pointer to the curve object
 */
void update_curve (MtxCurve *curve)
{
	GtkWidget * widget = NULL;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	gchar * tmpbuf = NULL;
	gint i = 0;
	cairo_t *cr = NULL;
	double dashes[2] = {4.0,4.0};
	gchar * message = NULL;
	cairo_text_extents_t extents;
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	GtkStateType state = GTK_STATE_NORMAL;

	widget = GTK_WIDGET(curve);

#if GTK_MINOR_VERSION >= 20
	state = gtk_widget_get_state(GTK_WIDGET(widget));
#else
	state = GTK_WIDGET_STATE (widget);
#endif
	/* Copy background pixmap to intermediary for final rendering */
	gdk_draw_drawable(priv->pixmap,
			widget->style->fg_gc[state],
			priv->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);

	cr = gdk_cairo_create (priv->pixmap);
	cairo_set_font_options(cr,priv->font_options);

	/* The circles for each vertex itself */
	if (priv->show_vertexes)
	{
		cairo_set_source_rgb (cr, 0.0,0.7,1.0);
		for (i=0;i<priv->num_points;i++)
		{
			cairo_arc(cr,priv->points[i].x,priv->points[i].y,3,0,2*G_PI);
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
		cairo_arc(cr,priv->points[priv->active_coord].x,priv->points[priv->active_coord].y,4,0,2*G_PI);
	}
	cairo_stroke(cr);

	if (priv->proximity_vertex >= 0)
	{
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_PROX].red/65535.0,
				priv->colors[CURVE_COL_PROX].green/65535.0,
				priv->colors[CURVE_COL_PROX].blue/65535.0);
		cairo_move_to (cr, priv->points[priv->proximity_vertex].x,priv->points[priv->proximity_vertex].y);
		cairo_new_sub_path(cr);
		cairo_arc(cr,priv->points[priv->proximity_vertex].x,priv->points[priv->proximity_vertex].y,4.5,0,2*G_PI);
	}
	cairo_stroke(cr);

	/* Edit Markers */
	/*
	if (priv->show_edit_marker)
	{
	cairo_set_source_rgb (cr, priv->colors[CURVE_COL_EDIT].red/65535.0,
	priv->colors[CURVE_COL_EDIT].green/65535.0,
	priv->colors[CURVE_COL_EDIT].blue/65535.0);
	cairo_set_line_width (cr, 1.0);
	cairo_move_to (cr,0 ,((priv->y_edit-priv->lowest_y)*priv->y_scale) + priv->y_border);
	cairo_line_to (cr, ((priv->x_edit-priv->lowest_x)*priv->x_scale) + priv->x_border,((priv->y_edit-priv->lowest_y)*priv->y_scale) + priv->y_border);
	cairo_line_to (cr, ((priv->x_edit-priv->lowest_x)*priv->x_scale) + priv->x_border,0);
	cairo_stroke(cr);
	}
	*/
	/* Vertical and Horizontal Markers */
	cairo_set_source_rgb (cr, priv->colors[CURVE_COL_MARKER].red/65535.0,
			priv->colors[CURVE_COL_MARKER].green/65535.0,
			priv->colors[CURVE_COL_MARKER].blue/65535.0);
	if (priv->show_x_marker)
	{
		cairo_move_to (cr,0 ,priv->h-(((priv->y_at_x_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
		cairo_line_to (cr, ((priv->x_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h - (((priv->y_at_x_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
		cairo_line_to (cr, ((priv->x_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h);
		cairo_stroke(cr);
		if (priv->x_draw_peak)
		{
			cairo_save(cr);
			cairo_set_dash(cr,dashes,2,0);
			cairo_move_to (cr,0 ,priv->h-(((priv->peak_y_at_x_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
			cairo_line_to (cr, ((priv->peak_x_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h - (((priv->peak_y_at_x_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
			cairo_line_to (cr, ((priv->peak_x_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h);
			cairo_stroke(cr);
			cairo_restore(cr);
		}

	}
	if (priv->show_y_marker)
	{
		cairo_move_to (cr, 0,priv->h-(((priv->y_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
		cairo_line_to (cr, ((priv->x_at_y_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h - (((priv->y_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
		cairo_line_to (cr, ((priv->x_at_y_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h);
		cairo_stroke(cr);
		if (priv->y_draw_peak)
		{
			cairo_save(cr);
			cairo_set_dash(cr,dashes,2,0);
			cairo_move_to (cr, 0,priv->h-(((priv->peak_y_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
			cairo_line_to (cr, ((priv->peak_x_at_y_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h - (((priv->peak_y_marker-priv->lowest_y)*priv->y_scale) + priv->y_border));
			cairo_line_to (cr, ((priv->peak_x_at_y_marker-priv->lowest_x)*priv->x_scale) + priv->x_border,priv->h);
			cairo_stroke(cr);
			cairo_restore(cr);
		}
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
		draw_selected_msg(cr,priv);
	cairo_destroy(cr);
}


void draw_selected_msg(cairo_t *cr, MtxCurvePrivate *priv)
{
	gchar * x_msg = NULL;
	gchar * y_msg = NULL;
	gdouble h = 0.0;
	gdouble w = 0.0;
	gdouble x_pad = 6.0;
	gdouble y_pad = 6.0;
	cairo_text_extents_t x_extents;
	cairo_text_extents_t y_extents;

	cairo_set_font_size (cr, 9);
	x_msg = g_strdup_printf("%1$s: %2$.*3$f",
			priv->x_axis_label,
			priv->coords[priv->active_coord].x,
			priv->x_precision);
	y_msg = g_strdup_printf("%1$s: %2$.*3$f",
			priv->y_axis_label,
			priv->coords[priv->active_coord].y,
			priv->y_precision);
	cairo_text_extents (cr, x_msg, &x_extents);
	cairo_text_extents (cr, y_msg, &y_extents);

	/* Determine where the bounding box will fit for this
	 * popup. If too close to right side, put it to left of
	 * pointer, otherwise prefer upper right side
	 */
	w = MAX(x_extents.width,y_extents.width);
	h = x_extents.height+y_extents.height;
	cairo_set_source_rgb (cr, 
			priv->colors[CURVE_COL_TEXT].red/65535.0,
			priv->colors[CURVE_COL_TEXT].green/65535.0,
			priv->colors[CURVE_COL_TEXT].blue/65535.0);
	if ((priv->points[priv->active_coord].x + w + x_pad) > priv->w)
	{
		if ((priv->points[priv->active_coord].y - h - y_pad - priv->y_border) < 0)
		{
			/*printf("lower left?\n"); */
			cairo_set_source_rgba (cr,0.13,0.13,0.13,0.75);
			cairo_rectangle(cr,priv->points[priv->active_coord].x - x_pad/2,
					priv->points[priv->active_coord].y + y_pad/2,
					-(w + x_pad),
					(h + 2*y_pad));
			cairo_fill (cr);
			cairo_set_source_rgb (cr, 
					priv->colors[CURVE_COL_TEXT].red/65535.0,
					priv->colors[CURVE_COL_TEXT].green/65535.0,
					priv->colors[CURVE_COL_TEXT].blue/65535.0);
			cairo_move_to(cr,priv->points[priv->active_coord].x - w - x_pad,priv->points[priv->active_coord].y + x_extents.height + y_pad);
			cairo_show_text (cr, x_msg);
			cairo_move_to(cr,priv->points[priv->active_coord].x - w - x_pad,priv->points[priv->active_coord].y +h + 2*y_pad);
			cairo_show_text (cr, y_msg);
		}

		else
		{
		/*	printf("upper left?\n"); */
			cairo_set_source_rgba (cr,0.13,0.13,0.13,0.75);
			cairo_rectangle(cr,priv->points[priv->active_coord].x - x_pad/2,
					priv->points[priv->active_coord].y - y_pad/2,
					-(w + x_pad),
					-(h + 2*y_pad));
			cairo_fill (cr);
			cairo_set_source_rgb (cr, 
					priv->colors[CURVE_COL_TEXT].red/65535.0,
					priv->colors[CURVE_COL_TEXT].green/65535.0,
					priv->colors[CURVE_COL_TEXT].blue/65535.0);
			cairo_move_to(cr,priv->points[priv->active_coord].x - w - x_pad,priv->points[priv->active_coord].y - y_extents.height - 2*y_pad);
			cairo_show_text (cr, x_msg);
			cairo_move_to(cr,priv->points[priv->active_coord].x - w - x_pad,priv->points[priv->active_coord].y - y_pad);
			cairo_show_text (cr, y_msg);
		}
	}
	else
	{
		if ((priv->points[priv->active_coord].y - h - y_pad - priv->y_border) < 0)
		{
			/* printf("lower right?\n"); */
			cairo_set_source_rgba (cr,0.13,0.13,0.13,0.75);
			cairo_rectangle(cr,priv->points[priv->active_coord].x + x_pad/2,
					priv->points[priv->active_coord].y + y_pad/2,
					w + x_pad,
					(h + 2*y_pad));
			cairo_fill (cr);
			cairo_set_source_rgb (cr, 
					priv->colors[CURVE_COL_TEXT].red/65535.0,
					priv->colors[CURVE_COL_TEXT].green/65535.0,
					priv->colors[CURVE_COL_TEXT].blue/65535.0);
			cairo_move_to(cr,priv->points[priv->active_coord].x + x_pad,priv->points[priv->active_coord].y + x_extents.height + y_pad);
			cairo_show_text (cr, x_msg);
			cairo_move_to(cr,priv->points[priv->active_coord].x + x_pad,priv->points[priv->active_coord].y + h + 2*y_pad);
			cairo_show_text (cr, y_msg);
		}
		else
		{
			/*printf("upper right?\n"); */
			cairo_set_source_rgba (cr,0.13,0.13,0.13,0.75);
			cairo_rectangle(cr,priv->points[priv->active_coord].x + x_pad/2,
					priv->points[priv->active_coord].y - y_pad/2,
					w + x_pad,
					-(h + 2*y_pad));
			cairo_fill (cr);
			cairo_set_source_rgb (cr, 
					priv->colors[CURVE_COL_TEXT].red/65535.0,
					priv->colors[CURVE_COL_TEXT].green/65535.0,
					priv->colors[CURVE_COL_TEXT].blue/65535.0);
			cairo_move_to(cr,priv->points[priv->active_coord].x + x_pad,priv->points[priv->active_coord].y - y_extents.height - 2*y_pad);
			cairo_show_text (cr, x_msg);
			cairo_move_to(cr,priv->points[priv->active_coord].x + x_pad,priv->points[priv->active_coord].y - y_pad);
			cairo_show_text (cr, y_msg);
		}
	}
	/*cairo_set_source_rgba (cr,0.0,0.0,0.0,0.75);
	cairo_rectangle(cr,priv->w*0.80 - extents.width,
			(extents.height*7),
			extents.width,
			-extents.height);
	cairo_fill (cr);
	*/
	g_free(x_msg);
	g_free(y_msg);

	cairo_stroke (cr);
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
	update_curve(curve);

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
	GtkStateType state = GTK_STATE_NORMAL;
	cairo_t *cr = NULL;
	GdkPixmap *pmap = NULL;

#if GTK_MINOR_VERSION >= 20
	state = gtk_widget_get_state(GTK_WIDGET(widget));
#else
	state = GTK_WIDGET_STATE (widget);
#endif
#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_is_sensitive(GTK_WIDGET(curve)))
#else
	if (GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(curve)))
#endif
	{
		gdk_draw_drawable(widget->window,
				widget->style->fg_gc[state],
				priv->pixmap,
				event->area.x, event->area.y,
				event->area.x, event->area.y,
				event->area.width, event->area.height);
	}
	else
	{
		pmap=gdk_pixmap_new(widget->window,
				priv->w,priv->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_drawable(pmap,
				widget->style->fg_gc[state],
				priv->pixmap,
				event->area.x, event->area.y,
				event->area.x, event->area.y,
				event->area.width, event->area.height);
		cr = gdk_cairo_create (pmap);
		cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
		cairo_rectangle (cr,
				0,0,priv->w,priv->h);
		cairo_fill(cr);
		cairo_destroy(cr);
		gdk_draw_drawable(widget->window,
				widget->style->fg_gc[state],
				pmap,
				event->area.x, event->area.y,
				event->area.x, event->area.y,
				event->area.width, event->area.height);
		g_object_unref(pmap);
	}
	return FALSE;
}


/*!
 \brief draws the static elements of the curve (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param widget (MtxCurve *) pointer to the curve object
 */
void generate_static_curve(MtxCurve *curve)
{
	cairo_t *cr = NULL;
	gint w = 0;
	gint h = 0;
	gfloat tmpf = 0.0;
	gint max_lines;
	gfloat spread = 0;
	gint i = 0;
	gfloat longest = 0.0;
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
	/* X Axis Label */
	cairo_set_source_rgb (cr, 
			priv->colors[CURVE_COL_TEXT].red/65535.0,
			priv->colors[CURVE_COL_TEXT].green/65535.0,
			priv->colors[CURVE_COL_TEXT].blue/65535.0);
	cairo_select_font_face (cr, priv->font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	if (priv->x_axis_label)
	{
		message = g_strdup_printf("%s",priv->x_axis_label);
		cairo_text_extents (cr, message, &extents);
		/*cairo_move_to(cr,(priv->w/2) + priv->x_border - (extents.width/2.0),priv->h - (extents.height/2));*/
		cairo_move_to(cr,(priv->w/2) - (extents.width/2.0),priv->h - (extents.height/2));
		cairo_show_text (cr, message);
		g_free(message);
		priv->x_label_border = 2 * extents.height;
		priv->y_border = priv->x_label_border + (1.5*extents.height);
	}
	if (priv->y_axis_label)
	{
		message = g_strdup_printf("%s",priv->y_axis_label);
		cairo_text_extents (cr, message, &extents);
		/*cairo_move_to(cr,1.5*extents.height,(priv->h/2) - priv->y_border + (extents.width/2.0));*/
		cairo_move_to(cr,1.5*extents.height,(priv->h/2) + (extents.width/2.0));
		cairo_save(cr);
		cairo_rotate(cr,-G_PI/2.0);
		cairo_show_text (cr, message);
		cairo_restore(cr);
		g_free(message);
		priv->y_label_border = 2 * extents.height;
	}
	cairo_select_font_face (cr, priv->font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);

	if (priv->show_grat)
	{
		/* Need to draw text markers FIRST... */
		/* Y axis text markers */
		max_lines = (priv->h - 2*priv->y_border)/50;
		tmpf = ((priv->h - 2*priv->y_border)%50)/50.0;
		if (tmpf > 0.5)
			max_lines++;
		if (max_lines == 0)
			max_lines = 1;
		spread = (priv->h - 2*priv->y_border)/(gfloat)max_lines;
		longest = 0.0;
		/* Find longest one */
		for (i = 0;i <= max_lines;i++)
		{
			message = g_strdup_printf("%1$.*2$f",((i*spread)/priv->y_scale)+priv->lowest_y,priv->y_precision==0?1:priv->y_precision);
			cairo_text_extents (cr, message, &extents);
			g_free(message);
			if (extents.width > longest)
				longest = extents.width;
		}
		/* Render text labels, right justified */
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_TEXT].red/65535.0,
				priv->colors[CURVE_COL_TEXT].green/65535.0,
				priv->colors[CURVE_COL_TEXT].blue/65535.0);
		for(i = 0;i <= max_lines;i++)
		{
			message = g_strdup_printf("%1$.*2$f",((i*spread)/priv->y_scale)+priv->lowest_y,priv->y_precision==0?1:priv->y_precision);
			cairo_text_extents (cr, message, &extents);
			cairo_move_to(cr,priv->y_label_border  + (longest - extents.width) ,priv->h - (priv->y_border+i*spread-(extents.height/2.0)));
			cairo_show_text (cr, message);
			g_free(message);
		}
		/* Calc out X border */
		priv->x_border = priv->y_label_border + longest + extents.height;
		/* Render Left to Right Lines for Y axis */
		cairo_set_source_rgba (cr, 
				priv->colors[CURVE_COL_GRAT].red/65535.0,
				priv->colors[CURVE_COL_GRAT].green/65535.0,
				priv->colors[CURVE_COL_GRAT].blue/65535.0,
				0.5);
		/* Render Horizontal Lines */
		for(i = 0;i <= max_lines;i++)
		{
			cairo_move_to (cr,priv->x_border-extents.height/2.0,priv->y_border+i*spread);
			cairo_line_to (cr,priv->w,priv->y_border+i*spread);
		}
		cairo_stroke(cr);
		/* top to Bottom (X axis) graticule lines */
		max_lines = (priv->w - priv->x_border - 25)/50;
		tmpf = ((priv->w - priv->x_border - 25)%50)/50.0;
		if (tmpf > 0.5)
			max_lines++;
		if (max_lines == 0)
			max_lines = 1;
		spread = (priv->w - priv->x_border - 25)/(gfloat)max_lines;
		cairo_set_font_size (cr, 8);
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_TEXT].red/65535.0,
				priv->colors[CURVE_COL_TEXT].green/65535.0,
				priv->colors[CURVE_COL_TEXT].blue/65535.0);
		for(i = 0;i <= max_lines;i++)
		{
			message = g_strdup_printf("%1$.*2$f",((i*spread)/priv->x_scale)+priv->lowest_x,priv->x_precision==0?1:priv->x_precision);
			cairo_text_extents (cr, message, &extents);
			cairo_move_to(cr,priv->x_border+i*spread-(extents.width/2.0),priv->h - priv->y_border + 2*extents.height);
			cairo_show_text (cr, message);
			g_free(message);
		}
		priv->y_border = priv->x_label_border + 2*extents.height;
		cairo_stroke (cr);
		cairo_set_source_rgba (cr, 
				priv->colors[CURVE_COL_GRAT].red/65535.0,
				priv->colors[CURVE_COL_GRAT].green/65535.0,
				priv->colors[CURVE_COL_GRAT].blue/65535.0,
				0.5);
		/* Render Vertical Lines */
		for(i = 0;i <= max_lines;i++)
		{
			cairo_move_to (cr,priv->x_border+i*spread,0);
			cairo_line_to (cr,priv->x_border+i*spread,priv->h-priv->y_border+(extents.height/2));
		}
		cairo_stroke (cr);

	}
	recalc_points(priv);
	/* THE curve */
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
	cairo_select_font_face (cr, priv->font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 15);
	if (priv->title)
	{
		message = g_strdup_printf("%s", priv->title);
		cairo_text_extents (cr, message, &extents);

		cairo_set_source_rgba (cr,0.0,0,0,0.75);
		cairo_rectangle(cr,priv->w/2 - (extents.width/2),
				1.5*extents.height,
				extents.width,
				-extents.height);
		cairo_fill (cr);
		cairo_set_source_rgb (cr, 
				priv->colors[CURVE_COL_TEXT].red/65535.0,
				priv->colors[CURVE_COL_TEXT].green/65535.0,
				priv->colors[CURVE_COL_TEXT].blue/65535.0);
		cairo_move_to (cr, 
				priv->w/2-(extents.width/2),
				1.5*extents.height);

		cairo_show_text (cr, message);
		g_free(message);
	}
	cairo_stroke (cr);
	cairo_destroy(cr);
}


gboolean mtx_curve_motion_event (GtkWidget *curve,GdkEventMotion *event)
{
	MtxCurvePrivate *priv = MTX_CURVE_GET_PRIVATE(curve);
	gint i = 0;
	guint index = 0;
	gfloat tmp_x = 0.0;
	gfloat tmp_y = 0.0;
	gchar * tmpbuf = 0;

	if (!priv->vertex_selected)
	{
		if (priv->pos_str)
			g_free(priv->pos_str);
		priv->pos_str = g_strdup_printf("%1$.*2$f, %3$.*4$f",(gfloat)((event->x - priv->x_border)/priv->x_scale) + priv ->lowest_x, priv->x_precision, (gfloat)(-((event->y - priv->h + priv->y_border)/priv->y_scale) + priv ->lowest_y),priv->y_precision);
		if (proximity_test(curve,event))
		{
			g_signal_emit_by_name((gpointer)curve, "vertex-proximity");
			priv->active_coord = priv->proximity_vertex;
			mtx_curve_redraw(MTX_CURVE(curve),FALSE);
		}
		else
		{
			priv->active_coord = -1;
			mtx_curve_redraw(MTX_CURVE(curve),FALSE);
		}
		gdk_event_request_motions(event);
		return TRUE;
	}

	i = priv->active_coord;

	/*printf("motion, active vertex is %i, coords %f,%f\n",i,priv->coords[i].x,priv->coords[i].y);*/

	/* Honor Axis locks */
	if (priv->x_blocked_from_edit)
		tmp_x = priv->points[i].x;
	else
	{
		tmp_x = event->x;
		/* Clamp X movement within adjoining points */
		if (i == 0) /* lower left */
			if (event->x > priv->points[i+1].x)
				tmp_x = priv->points[i+1].x;
		if ((i > 0) && (i < (priv->num_points-1)))
		{
			if (event->x < priv->points[i-1].x)
				tmp_x = priv->points[i-1].x;
			if (event->x > priv->points[i+1].x)
				tmp_x = priv->points[i+1].x;
		}
		if (i == (priv->num_points-1))
			if (event->x < priv->points[i-1].x)
				tmp_x = priv->points[i-1].x;
	}

	if (priv->y_blocked_from_edit)
		tmp_y = priv->points[i].y;
	else
		tmp_y = event->y;

	/* Limit clamps */

	/* IF not highest or lowest point, clamp within X bounds */
	tmpbuf = g_strdup_printf("%1$.*2$f",(gfloat)((tmp_x - priv->x_border)/priv->x_scale) + priv ->lowest_x, priv->x_precision);
	priv->coords[i].x = (gfloat)g_strtod(tmpbuf,NULL);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%1$.*2$f",(gfloat)(-((tmp_y - priv->h + priv->y_border)/priv->y_scale) + priv ->lowest_y),priv->y_precision);
	priv->coords[i].y = (gfloat)g_strtod(tmpbuf,NULL);
	g_free(tmpbuf);

	if ((gfloat)((tmp_x - priv->x_border)/priv->x_scale) + priv ->lowest_x < priv->x_lower_limit)
		priv->coords[i].x = priv->x_lower_limit;
	if ((gfloat)((tmp_x - priv->x_border)/priv->x_scale) + priv ->lowest_x > priv->x_upper_limit)
		priv->coords[i].x = priv->x_upper_limit;

	if ((gfloat)(-((tmp_y - priv->h + priv->y_border)/priv->y_scale) + priv ->lowest_y) < priv->y_lower_limit)
		priv->coords[i].y = priv->y_lower_limit;
	if ((gfloat)(-((tmp_y - priv->h + priv->y_border)/priv->y_scale) + priv ->lowest_y) > priv->y_upper_limit)
		priv->coords[i].y = priv->y_upper_limit;

	
	if (( (gint)priv->coords[i].x > priv->highest_x) ||
			( (gint) priv->coords[i].y > priv->highest_y) ||
			( (gint) priv->coords[i].x < priv->lowest_x) ||
			( (gint) priv->coords[i].y < priv->lowest_y))
		recalc_extremes(priv);
	priv->coord_changed = TRUE;
	mtx_curve_redraw(MTX_CURVE(curve), TRUE);
	gdk_event_request_motions(event);
	return TRUE;
}
					       

gboolean auto_rescale(gpointer data)
{
	MtxCurvePrivate *priv = (MtxCurvePrivate *)data;
	recalc_extremes(priv);
	mtx_curve_redraw(MTX_CURVE(priv->self),TRUE);
	priv->auto_rescale_id = 0;
	return FALSE;
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
				mtx_curve_redraw(MTX_CURVE(curve),FALSE);
				return TRUE;
			}

		}
		priv->active_coord = -1;
		priv->vertex_selected = FALSE;
		mtx_curve_redraw(MTX_CURVE(curve),FALSE);
	}
	if ((event->button == 1 ) && 
			(event->type == GDK_BUTTON_RELEASE) &&
			(priv->vertex_selected))	
	{
		priv->vertex_selected = FALSE;

		if (priv->auto_rescale_id == 0)
			priv->auto_rescale_id = g_timeout_add(500,(GSourceFunc)auto_rescale,priv);
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
void mtx_curve_redraw (MtxCurve *curve, gboolean full)
{
	if (!GTK_WIDGET(curve)->window) return;

	if (full)
		generate_static_curve(curve);
	update_curve (curve);
	gdk_window_clear(GTK_WIDGET(curve)->window);
}


/*!
 \brief Recalculates the extremes of all coords in the graph
 \param priv (MtxCurvePrivate *) pointer to private data
 */
void recalc_extremes(MtxCurvePrivate *priv)
{
        gint i = 0;
        priv->highest_x = G_MINFLOAT;
        priv->highest_y = G_MINFLOAT;
        priv->lowest_x = G_MAXFLOAT;
        priv->lowest_y = G_MAXFLOAT;

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
	/*printf("X range (%f<->%f), Y range (%f<->%f)\n",priv->lowest_x, priv->highest_x, priv->lowest_y, priv->highest_y);*/
	if  (fabs(priv->lowest_x-priv->highest_x) < 2) /* Vertical Line */
	{
		if ( priv->lowest_x == 0.0)	/* Special case */
		{
			priv->lowest_x -= 5;
			priv->highest_x += 5;
		}
		else
		{
			priv->lowest_x *= 0.75;
			priv->highest_x *= 1.25;
		}
	}
	/*
	else
	{
		priv->lowest_x *= 0.95;
		priv->highest_x *= 1.05;
	}
	*/
	if  (fabs(priv->lowest_y-priv->highest_y) < 2) /* Horizontal Line */
	{
		if ( priv->lowest_y == 0.0)	/* Special case */
		{
			priv->lowest_y -= 5;
			priv->highest_y += 5;
		}
		else
		{
			priv->lowest_y *= 0.75;
			priv->highest_y *= 1.25;
		}
	}
	/*
	else
	{
		priv->lowest_y *= 0.95;
		priv->highest_y *= 1.05;
	}
	*/
	priv->x_scale = (gfloat)(priv->w-(1.25*priv->x_border))/((priv->highest_x - (priv->lowest_x + 0.000001)));
	priv->y_scale = (gfloat)(priv->h-(2*priv->y_border))/((priv->highest_y - (priv->lowest_y + 0.000001)));
	priv->locked_scale = (priv->x_scale < priv->y_scale) ? priv->x_scale:priv->y_scale;
	generate_static_curve(priv->self);
}


/*!
 \brief Recalculates the points of all coords in the graph
 \param priv (MtxCurvePrivate *) pointer to private data
 */
void recalc_points(MtxCurvePrivate *priv)
{
	gint i = 0;

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
		priv->points[i].x = (gint)(((priv->coords[i].x - priv->lowest_x)*priv->x_scale) + priv->x_border);
		priv->points[i].y = (gint)(priv->h - (((priv->coords[i].y - priv->lowest_y)*priv->y_scale) + priv->y_border));
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
		mtx_curve_redraw(MTX_CURVE(curve),FALSE);
	}
	if ((event->type == GDK_LEAVE_NOTIFY) && (priv->auto_hide))
		priv->vertex_id = g_timeout_add(2000,delay_turnoff_vertexes,curve);
	return FALSE;
}


gboolean delay_turnoff_vertexes(gpointer data)
{
	MtxCurve *curve = (MtxCurve*) data;
	MtxCurvePrivate *priv = NULL;
	if (!GTK_IS_WIDGET(curve))
		return FALSE;

	priv = MTX_CURVE_GET_PRIVATE(curve);
	priv->show_vertexes = FALSE;
	mtx_curve_redraw(MTX_CURVE(curve),FALSE);
	return FALSE;
}


gboolean proximity_test (GtkWidget *curve, GdkEventMotion *event)
{
	MtxCurvePrivate *priv = NULL;
	guint i = 0;
	gint thresh = 0;

	priv = MTX_CURVE_GET_PRIVATE(curve);
	thresh = priv->proximity_threshold;

	for(i=0;i<priv->num_points;i++)
	{
		if ((abs((gint)event->x - priv->points[i].x) < thresh) && (abs((gint)event->y - priv->points[i].y) < thresh))
		{
			if (priv->proximity_vertex == i)
				return FALSE;
			priv->proximity_vertex = i;
			return TRUE;
		}
	}
	if (priv->proximity_vertex == -1)
		return FALSE;
	priv->proximity_vertex = -1;
	return TRUE;
}


gboolean get_intersection(
	gfloat Ax, gfloat Ay,
	gfloat Bx, gfloat By,
	gfloat Cx, gfloat Cy,
	gfloat Dx, gfloat Dy,
	gfloat *X, gfloat *Y) 
{
	gdouble  distAB, theCos, theSin, newX, ABpos ;

	/*  Fail if either line segment is zero-length. */
	if (((Ax==Bx) && (Ay==By)) || ((Cx==Dx) && (Cy==Dy)))
		return FALSE;

	/*  (1) Translate the system so that point A is on the origin. */
	Bx-=Ax; By-=Ay;
	Cx-=Ax; Cy-=Ay;
	Dx-=Ax; Dy-=Ay;

	/*  Discover the length of segment A-B.*/
	distAB=sqrt(Bx*Bx+By*By);

	/*  (2) Rotate the system so that point B is on the positive X axis.*/
	theCos=Bx/distAB;
	theSin=By/distAB;
	newX=Cx*theCos+Cy*theSin;
	Cy  =Cy*theCos-Cx*theSin; Cx=newX;
	newX=Dx*theCos+Dy*theSin;
	Dy  =Dy*theCos-Dx*theSin; Dx=newX;

	/*  Fail if lines are parallel */
	if (Cy == Dy) 
		return FALSE;

	/*  (3) Discover the position of the intersection point along line A-B.*/
	ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

	/*  (4) Apply the discovered position to line A-B in the original coordinate system. */
	*X=Ax+ABpos*theCos;
	*Y=Ay+ABpos*theSin;

	/*  Success. */
	return TRUE; 
} 


gboolean cancel_peak(gpointer data)
{
	MtxCurve *curve = NULL;
	MtxCurvePrivate *priv = NULL;
	gint axis = 0;

	if (!MTX_IS_CURVE(data))
		return  FALSE;

	curve = MTX_CURVE(data);

	priv = MTX_CURVE_GET_PRIVATE(curve);
	axis = (GINT)g_object_get_data(G_OBJECT(curve),"axis");

	switch (axis)
	{
		case _X_:
			priv->x_draw_peak = FALSE;
			priv->peak_x_marker = priv->x_lower_limit;
			priv->x_peak_timeout = 0;
			break;
		case _Y_:
			priv->y_draw_peak = FALSE;
			priv->peak_y_marker = priv->y_lower_limit;
			priv->y_peak_timeout = 0;
			break;
		default:
			break;
	}
	mtx_curve_redraw(MTX_CURVE(curve),FALSE);
	return FALSE;
}
