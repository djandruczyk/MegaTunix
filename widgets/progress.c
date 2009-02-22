/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt progress bar widget
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
 \returns a pointer to a newly created progressbar widget
 */
void mtx_progress_bar_set_fraction (MtxProgressBar *pbar, gfloat fraction)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_if_fail ((MTX_IS_PROGRESS_BAR (pbar)));
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);

	if (fraction > priv->peak)
	{
		priv->peak = fraction;
		if (priv->peak_timeout == 0)
		{
			priv->peak_timeout = g_timeout_add(800,mtx_progress_bar_peak_reset,pbar);
		}
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar),fraction);
	return;
}


gboolean mtx_progress_bar_peak_reset(gpointer data)
{
	MtxProgressBar* pbar = MTX_PROGRESS_BAR(data);
	MtxProgressBarPrivate *priv = NULL; 
	g_return_val_if_fail ((MTX_IS_PROGRESS_BAR (pbar)),FALSE);
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	priv->peak =0;
	priv->peak_timeout = 0;
	mtx_progress_bar_real_update(GTK_PROGRESS(pbar));
	return FALSE;
}


/*!
 \brief sets current fraction
 \returns a pointer to a newly created pbar widget
 */
gfloat mtx_progress_bar_get_fraction (MtxProgressBar *pbar)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_val_if_fail ((MTX_IS_PROGRESS_BAR (pbar)),0.0);
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	return gtk_progress_get_current_percentage (GTK_PROGRESS (pbar));
}
