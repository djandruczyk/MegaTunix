/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix progress bar widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <config.h>
#include <cairo/cairo.h>
#include <progress.h>
#include <progress-private.h>
#include <gtk/gtk.h>


/* Prototypes */
gboolean mtx_progress_bar_peak_reset(gpointer data);

/*!
 \brief gets called when  a user wants a new pbar
 \returns a pointer to a newly created pbar widget
 */
GtkWidget *mtx_progress_bar_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_PROGRESS_BAR, NULL));
}


/*!
 \brief sets current fraction
 \returns nothing
 * If fraction is greater that peak, set peak to match, If there was a hold timeout
 * running,  cancel it.
 * If fraction is LESS than peak, check for hold function, if none, start one.
 */
void mtx_progress_bar_set_fraction (MtxProgressBar *pbar, gfloat fraction)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_if_fail ((MTX_IS_PROGRESS_BAR (pbar)));
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);

	if (fraction > priv->peak)
	{
		priv->peak = fraction;
		if (priv->hold_id > 0)
		{
			g_source_remove(priv->hold_id);
			priv->hold_id = 0;
		}
	}
	else
		if (priv->hold_id == 0)
			priv->hold_id = g_timeout_add(priv->hold_time,mtx_progress_bar_peak_reset,pbar);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar),fraction);
	return;
}


gboolean mtx_progress_bar_peak_reset(gpointer data)
{
	MtxProgressBar *pbar = (MtxProgressBar *)data;
	MtxProgressBarPrivate *priv = NULL; 
	if (!GTK_IS_WIDGET(pbar))
		return FALSE;

	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	priv->peak =0;
	priv->hold_id = 0;
	mtx_progress_bar_real_update(GTK_PROGRESS(pbar));
	return FALSE;
}


/*!
 \brief gets current fraction
 \returns current fraction (0-1.0)
 */
gfloat mtx_progress_bar_get_fraction (MtxProgressBar *pbar)
{
	g_return_val_if_fail ((MTX_IS_PROGRESS_BAR (pbar)),0.0);
	return gtk_progress_get_current_percentage (GTK_PROGRESS (pbar));
}


/*!
 \brief gets hold time
 \returns hold time in ms
 */
gint mtx_progress_bar_get_hold_time (MtxProgressBar *pbar)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_val_if_fail ((MTX_IS_PROGRESS_BAR (pbar)),0.0);
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	return priv->hold_time;
}


/*!
 \brief sets hold time
 \returns nothing
 */
void mtx_progress_bar_set_hold_time (MtxProgressBar *pbar, gint hold_time)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_if_fail ((MTX_IS_PROGRESS_BAR (pbar)));
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	priv->hold_time = hold_time;
}
