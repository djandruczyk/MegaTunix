/*
 * Copyright (C) 2007 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix pie pbar widget
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


#undef GTK_DISABLE_DEPRECATED
#undef GDK_DISABLE_DEPRECATED
#undef G_DISABLE_DEPRECATED
#include <config.h>
#include <cairo/cairo.h>
#include <progress.h>
#include <progress-private.h>
#include <gtk/gtk.h>
#include <gtk/gtkprogressbar.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>



G_DEFINE_TYPE (MtxProgressBar, mtx_progress_bar, GTK_TYPE_PROGRESS_BAR)

/*
GType mtx_progress_bar_get_type(void)
{
	static GType mtx_progress_bar_type = 0;

	if (!mtx_progress_bar_type)
	{
		static const GTypeInfo mtx_progress_bar_info =
		{
			sizeof(MtxProgressBarClass),
			NULL,
			NULL,
			(GClassInitFunc) mtx_progress_bar_class_init,
			NULL,
			NULL,
			sizeof(MtxProgressBar),
			0,
			(GInstanceInitFunc) mtx_progress_bar_init,
		};
		mtx_progress_bar_type = g_type_register_static(GTK_TYPE_PROGRESS_BAR, "MtxProgressBar", &mtx_progress_bar_info, 0);
	}
	return mtx_progress_bar_type;
}
*/

/*!
 \brief Initializes the mtx pie pbar class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name (MtxProgressBarClass *) pointer to the class
 */
void mtx_progress_bar_class_init (MtxProgressBarClass *class)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;
	GtkProgressClass *progress_class;

	gobject_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);
	progress_class = (GtkProgressClass *) class;

	/* GtkWidget signals */
	/*widget_class->button_press_event = mtx_progress_bar_button_press; */

	widget_class->expose_event = mtx_progress_bar_expose;

	progress_class->update = mtx_progress_bar_real_update;
	progress_class->paint = mtx_progress_bar_paint;

	g_type_class_add_private (class, sizeof (MtxProgressBarPrivate)); 
}


/*!
 \brief Initializes the pbar attributes to sane defaults
 \param pbar (MtxProgressBar *) pointer to the pbar object
 */
void mtx_progress_bar_init (MtxProgressBar *pbar)
{
	/* The events the pbar receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/ 
	MtxProgressBarPrivate *priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	priv->peak = 0.0;
	priv->hold_id = 0;
	priv->hold_time = 750;
	mtx_progress_bar_init_colors(pbar);
}


/*!
 \brief Allocates the default colors for a pbar with no options 
 \param widget (MegaProgressBar *) pointer to the pbar object
 */
void mtx_progress_bar_init_colors(MtxProgressBar *pbar)
{
	MtxProgressBarPrivate *priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	/*! Main Background */
	priv->colors[PROGRESS_COL_BG].red=0.95;
	priv->colors[PROGRESS_COL_BG].green=0.95;
	priv->colors[PROGRESS_COL_BG].blue=0.95;
	/*! Bar */
	priv->colors[PROGRESS_COL_BAR].red=0.2;
	priv->colors[PROGRESS_COL_BAR].green=0.2;
	priv->colors[PROGRESS_COL_BAR].blue=1.0;
	/*! Peak */
	priv->colors[PROGRESS_COL_PEAK].red=1.0;
	priv->colors[PROGRESS_COL_PEAK].green=0.0;
	priv->colors[PROGRESS_COL_PEAK].blue=0.0;
}


/*!
 \brief gets called to redraw the entire display manually
 \param pbar (MtxProgressBar *) pointer to the pbar object
 */
void mtx_progress_bar_real_update (GtkProgress *progress)
{
	GtkProgressBar *pbar;

	g_return_if_fail (GTK_IS_WIDGET (progress));

	pbar = GTK_PROGRESS_BAR (progress);

	pbar->dirty = TRUE;
	gtk_widget_queue_draw (GTK_WIDGET (progress));
}

void mtx_progress_bar_paint (GtkProgress *progress)
{
	GtkProgressBar *pbar;
	MtxProgressBarPrivate *priv = NULL;
	GtkWidget *widget;
        cairo_t *cr = NULL;
	gint current;
	gint space;
	gint peak;

	GtkProgressBarOrientation orientation;

	g_return_if_fail (GTK_IS_PROGRESS_BAR (progress));

	pbar = GTK_PROGRESS_BAR (progress);
	widget = GTK_WIDGET (progress);
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(MTX_PROGRESS_BAR(pbar));

	orientation = pbar->orientation;
	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
	{
		if (pbar->orientation == GTK_PROGRESS_LEFT_TO_RIGHT)
			orientation = GTK_PROGRESS_RIGHT_TO_LEFT;
		else if (pbar->orientation == GTK_PROGRESS_RIGHT_TO_LEFT)
			orientation = GTK_PROGRESS_LEFT_TO_RIGHT;
	}

	if (progress->offscreen_pixmap)
	{
		cr = gdk_cairo_create (GTK_PROGRESS (pbar)->offscreen_pixmap);
		cairo_set_source_rgb(cr,priv->colors[PROGRESS_COL_BG].red,
				priv->colors[PROGRESS_COL_BG].green,
				priv->colors[PROGRESS_COL_BG].blue);
		cairo_rectangle (cr,1,1,
				widget->allocation.width-2,
				widget->allocation.height-2);
		cairo_fill(cr);
		cairo_set_source_rgb(cr,0,0,0);
		cairo_rectangle (cr,0,0,
				widget->allocation.width,
				widget->allocation.height);
		cairo_stroke(cr);
		cairo_destroy(cr);

		if (orientation == GTK_PROGRESS_LEFT_TO_RIGHT ||
				orientation == GTK_PROGRESS_RIGHT_TO_LEFT)
			space = widget->allocation.width - 2;
		else
			space = widget->allocation.height - 2;

		current = space *
			gtk_progress_get_current_percentage (GTK_PROGRESS (pbar));

		peak = space *
			mtx_progress_get_peak_percentage (GTK_PROGRESS (pbar));

		if (pbar->bar_style == GTK_PROGRESS_CONTINUOUS)
		{
			mtx_progress_bar_paint_continuous (pbar, current, peak, orientation);

			/*
			   if (GTK_PROGRESS (pbar)->show_text)
			   mtx_progress_bar_paint_text (pbar, -1, current, orientation);
			   */
		}
		pbar->dirty = FALSE;

	}
}


void mtx_progress_bar_paint_continuous (GtkProgressBar *pbar, gint current,gint peak, GtkProgressBarOrientation orientation)
{
	GdkRectangle b_area;
	GdkRectangle p_area;
	GtkWidget *widget = GTK_WIDGET (pbar);
	MtxProgressBarPrivate *priv = NULL;
	cairo_t *cr = NULL;


	priv = MTX_PROGRESS_BAR_GET_PRIVATE(MTX_PROGRESS_BAR(pbar));
	if (current < 0)
		return;

	switch (orientation)
	{
		case GTK_PROGRESS_LEFT_TO_RIGHT:
		case GTK_PROGRESS_RIGHT_TO_LEFT:
			b_area.width = current;
			b_area.height = widget->allocation.height - 2;
			b_area.y = 1;
			b_area.x = 1;

			p_area.width = peak;
			p_area.height = widget->allocation.height - 2;
			p_area.y = 1;
			p_area.x = 1;
			if (orientation == GTK_PROGRESS_RIGHT_TO_LEFT)
			{
				b_area.x = widget->allocation.width - current - b_area.x;
				p_area.x = widget->allocation.width - peak - p_area.x;
			}
			break;

		case GTK_PROGRESS_TOP_TO_BOTTOM:
		case GTK_PROGRESS_BOTTOM_TO_TOP:
			b_area.width = widget->allocation.width - 2;
			b_area.height = current;
			b_area.x = 1;
			b_area.y = 1;

			p_area.width = widget->allocation.width - 2;
			p_area.height = peak;
			p_area.x = 1;
			p_area.y = 1;
			if (orientation == GTK_PROGRESS_BOTTOM_TO_TOP)
			{
				b_area.y = widget->allocation.height - current - b_area.y;
				p_area.y = widget->allocation.height - peak - p_area.y;
			}
			break;

		default:
			return;
			break;
	}
	cr = gdk_cairo_create (GTK_PROGRESS (pbar)->offscreen_pixmap);
	if (peak > current)
	{
		cairo_set_source_rgb(cr,priv->colors[PROGRESS_COL_PEAK].red,
				priv->colors[PROGRESS_COL_PEAK].green,
				priv->colors[PROGRESS_COL_PEAK].blue);
		cairo_rectangle (cr,p_area.x,p_area.y,p_area.width,p_area.height);
		cairo_fill(cr);
	}

	/* Show the immediate value */
	cairo_set_source_rgb(cr,priv->colors[PROGRESS_COL_BAR].red,
			priv->colors[PROGRESS_COL_BAR].green,
			priv->colors[PROGRESS_COL_BAR].blue);
	cairo_rectangle (cr,b_area.x,b_area.y,b_area.width,b_area.height);
	cairo_fill(cr);
	cairo_destroy(cr);

}


gboolean mtx_progress_bar_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkProgressBar *pbar;
	cairo_t *cr = NULL;
	GdkPixmap *pmap = NULL;
	GtkStateType state = GTK_STATE_NORMAL;

	g_return_val_if_fail (MTX_IS_PROGRESS_BAR (widget), FALSE);

	pbar = GTK_PROGRESS_BAR (widget);
#if GTK_MINOR_VERSION >= 20
	state = gtk_widget_get_state(GTK_WIDGET(widget));
#else
	state = GTK_WIDGET_STATE (widget);
#endif

#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_is_sensitive(GTK_WIDGET(widget)))
#else
		if (GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(widget)))
#endif
		{


#if GTK_MINOR_VERSION >= 20
			if (gtk_widget_is_drawable (widget) && pbar->dirty)
#else
				if (GTK_WIDGET_DRAWABLE (widget) && pbar->dirty)
#endif
					mtx_progress_bar_paint (GTK_PROGRESS (pbar));

			g_return_val_if_fail (event != NULL, FALSE);

#if GTK_MINOR_VERSION >= 20
			if (gtk_widget_is_drawable (widget))
#else
				if (GTK_WIDGET_DRAWABLE (widget))
#endif
					cr = gdk_cairo_create(widget->window);
			gdk_cairo_set_source_pixmap(cr,GTK_PROGRESS (widget)->offscreen_pixmap,0,0);
			cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
		else
		{
			cr = gdk_cairo_create(widget->window);
			gdk_cairo_set_source_pixmap(cr,GTK_PROGRESS (widget)->offscreen_pixmap,0,0);
			cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
			cairo_fill(cr);
			cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
			cairo_rectangle (cr,
					0,0,widget->allocation.width,widget->allocation.height);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
	return TRUE;
}

gfloat mtx_progress_get_peak_percentage (GtkProgress *progress)
{
	MtxProgressBar *pbar = MTX_PROGRESS_BAR (progress);
	MtxProgressBarPrivate *priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	return priv->peak;

}
