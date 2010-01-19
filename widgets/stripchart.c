/*
 * Copyright (C) 2010 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix stripchart widget
 * Inspired by Phil Tobin's MegaLogViewer
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
#include <stripchart.h>
#include <stripchart-private.h>
#include <gtk/gtk.h>


/*!
 \brief gets called when  a user wants a new stripchart
 \returns a pointer to a newly created stripchart widget
 */
GtkWidget *mtx_stripchart_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_STRIPCHART, NULL));
}


/*!
 \brief gets the current value 
 \param stripchart (MtxStripChart *) pointer to stripchart
 */
gboolean mtx_stripchart_get_values (MtxStripChart *stripchart, gfloat *values)
{
	gint i = 0;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(stripchart);
	g_return_val_if_fail ((MTX_IS_STRIPCHART (stripchart)),FALSE);
	for (i=0;i<priv->num_traces;i++)
		if (values[i])
			values[i]=priv->current[i];
	return TRUE;
}


/*!
 \brief sets the current value 
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param value (gfloat) new value
 */
void mtx_stripchart_set_values (MtxStripChart *stripchart, gfloat* values)
{
	gint i = 0;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(stripchart);
	g_return_if_fail (MTX_IS_STRIPCHART (stripchart));
	g_object_freeze_notify (G_OBJECT (stripchart));
	for (i=0;i<priv->num_traces;i++)
		if (values[i])
			if (priv->current[i])
				priv->current[i] = values[i];
	g_object_thaw_notify (G_OBJECT (stripchart));
	mtx_stripchart_redraw(stripchart);
}


/*!
 \brief Adds an additional trace
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param value (gfloat) new value
 */
gint mtx_stripchart_add_trace(MtxStripChart *chart, gfloat min, gfloat max, GdkColor color)
{
	gint i = 0;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart),-1);
	g_object_freeze_notify (G_OBJECT (chart));
	
	/* add a trace.. */

	g_object_thaw_notify (G_OBJECT (chart));
	mtx_stripchart_redraw(chart);
	/** FIXME!!! **/
	return 0;
}




/*!
 \brief Removes trace per specifed index
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param value (gfloat) new value
 */
gboolean mtx_stripchart_delete_trace(MtxStripChart *chart, gint index)
{
	printf("Not implemented yet...\n");

	return FALSE;
}
