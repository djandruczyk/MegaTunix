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

/*! @file widgets/progress.c
 *
 * @brief ...
 *
 *
 */


#undef GTK_DISABLE_DEPRECATED
#undef GDK_DISABLE_DEPRECATED
#undef G_DISABLE_DEPRECATED

#include <progress-private.h>


/* Prototypes */

gboolean mtx_progress_bar_peak_reset(gpointer data);

/*!
 \brief gets called when a user wants a new mtx pbar
 \returns a pointer to a newly created pbar widget
 */
GtkWidget *mtx_progress_bar_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_PROGRESS_BAR, NULL));
}


/*!
 \brief sets current fraction
 If fraction is greater that peak, set peak to match, If there was a hold 
 timeout running,  cancel it.
 If fraction is LESS than peak, check for hold function, if none, start one.
 \param pbar is a pointer to the progressbar object
 \param fraction is the fractional value to set the bar to
 */
void mtx_progress_bar_set_fraction (MtxProgressBar *pbar, gfloat fraction)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_if_fail ((MTX_IS_PROGRESS_BAR (pbar)));
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	g_return_if_fail(priv);

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


/*!
 \brief Resets the peak/hold of the progressbar
 \param data is a pointer to the progressbar object
 */
gboolean mtx_progress_bar_peak_reset(gpointer data)
{
	MtxProgressBar *pbar = (MtxProgressBar *)data;
	MtxProgressBarPrivate *priv = NULL; 

	g_return_val_if_fail(GTK_IS_WIDGET(pbar),FALSE);
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	g_return_val_if_fail(priv,FALSE);

	priv->peak = 0;
	priv->hold_id = 0;
	mtx_progress_bar_real_update(GTK_PROGRESS(pbar));
	return FALSE;
}


/*!
 \brief gets current fraction
 \param pbar is a pointer to the progressbar object
 \returns current fraction from the progressbar (0-1.0)
 */
gfloat mtx_progress_bar_get_fraction (MtxProgressBar *pbar)
{
	g_return_val_if_fail ((MTX_IS_PROGRESS_BAR (pbar)),0.0);
	return gtk_progress_get_current_percentage (GTK_PROGRESS (pbar));
}


/*!
 \brief gets the progressbar peak hold time
 \param pbar is a pointer to the progressbar object
 \returns hold time in ms
 */
gint mtx_progress_bar_get_hold_time (MtxProgressBar *pbar)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_val_if_fail ((MTX_IS_PROGRESS_BAR (pbar)),0.0);
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	g_return_val_if_fail(priv,0);
	return priv->hold_time;
}


/*!
 \brief sets hold time
 \param pbar is a pointer to the progressbar object
 \param hold_time is the time to hold peaks in milliseconds
 */
void mtx_progress_bar_set_hold_time (MtxProgressBar *pbar, gint hold_time)
{
	MtxProgressBarPrivate *priv = NULL; 
	g_return_if_fail ((MTX_IS_PROGRESS_BAR (pbar)));
	priv = MTX_PROGRESS_BAR_GET_PRIVATE(pbar);
	g_return_if_fail(priv);
	priv->hold_time = hold_time;
}
