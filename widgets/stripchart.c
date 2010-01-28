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
gboolean mtx_stripchart_get_latest_values (MtxStripChart *stripchart, gfloat *values)
{
	gint i = 0;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(stripchart);
	g_return_val_if_fail ((MTX_IS_STRIPCHART (stripchart)),FALSE);
	for (i=0;i<priv->num_traces;i++)
		if (values[i])
		{
			trace = g_array_index(priv->traces, MtxStripChartTrace *, i);
			values[i] = g_array_index(trace->history,gfloat, trace->history->len-1);
		}
	return TRUE;
}


/*!
 \brief sets the current value 
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param value (gfloat) new value
 */
gboolean mtx_stripchart_set_values (MtxStripChart *stripchart, gfloat* values)
{
	gint i = 0;
	gfloat val = 0.0;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(stripchart);
	g_return_if_fail (MTX_IS_STRIPCHART (stripchart));
	g_object_freeze_notify (G_OBJECT (stripchart));
	for (i=0;i<priv->num_traces;i++)
	{
		if (values[i])
		{
			trace = g_array_index(priv->traces, MtxStripChartTrace *, i);
			val = values[i] > trace->max ? trace->max:values[i];
			val = val < trace->min ? trace->min:val;
			trace->history = g_array_append_val(trace->history,val);
		}
	}
	g_object_thaw_notify (G_OBJECT (stripchart));
	mtx_stripchart_redraw(stripchart);
	return TRUE;
}


/*!
 \brief Adds an additional trace
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param value (gfloat) new value
 */
gint mtx_stripchart_add_trace(MtxStripChart *chart, gfloat min, gfloat max, gint precision, const gchar *name, GdkColor *color)
{
	gint i = 0;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart),-1);
	g_return_val_if_fail (min <= max,-1);
	g_return_val_if_fail (name != NULL ,-1);
	g_object_freeze_notify (G_OBJECT (chart));
	
	/* add a trace.. */
	trace = g_new0(MtxStripChartTrace, 1);
	trace->min = min;
	trace->max = max;
	trace->precision = precision;
	trace->lwidth = 2.0;
	if (name)
		trace->name = g_strdup(name);
	if (color)
		trace->color = *color;
	else
		trace->color = priv->tcolors[priv->num_traces];

	trace->show_val = TRUE;
	trace->id = g_random_int_range(1000,100000);
	trace->history = g_array_sized_new(FALSE,TRUE,sizeof(gfloat),100);
	priv->num_traces++;
	priv->traces = g_array_append_val(priv->traces,trace);

	g_object_thaw_notify (G_OBJECT (chart));
	mtx_stripchart_redraw(chart);
	/** FIXME!!! **/
	return trace->id;
}


/*!
 \brief Removes trace per specifed index
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param value (gfloat) new value
 */
gboolean mtx_stripchart_delete_trace(MtxStripChart *chart, gint index)
{
	gint i = 0;
	gboolean retval = FALSE;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart),FALSE);
	g_object_freeze_notify (G_OBJECT (chart));


	for (i=0;i<priv->traces->len;i++)
	{
		trace = g_array_index(priv->traces,MtxStripChartTrace *, i);
		if (!trace)
			continue;
		if (trace->id == index) /* Found it! */
		{
			priv->traces = g_array_remove_index(priv->traces,i);
			if (trace->name)
				g_free(trace->name);
			if (trace->history)
				g_array_free(trace->history,TRUE);
			g_free(trace);
			retval = TRUE;
			priv->num_traces--;
		}
		trace = NULL;
	}
	g_object_thaw_notify (G_OBJECT (chart));
	mtx_stripchart_redraw(chart);
	return retval;
}


/*!
 \brief Sets trace name justification
 \param stripchart (MtxStripChart *) pointer to stripchart
 \param justification 
 */
gboolean mtx_stripchart_set_name_justification(MtxStripChart *chart, GtkJustification justification)
{
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_if_fail (MTX_IS_STRIPCHART (chart));
	g_return_if_fail ((justification != GTK_JUSTIFY_LEFT) ||
			(justification != GTK_JUSTIFY_RIGHT) ||
			(justification != GTK_JUSTIFY_CENTER) ||
			(justification != GTK_JUSTIFY_FILL));
	g_object_freeze_notify (G_OBJECT (chart));
	priv->justification = justification;
	g_object_thaw_notify (G_OBJECT (chart));
	mtx_stripchart_redraw(chart);
}


/*!
 \brief Gets trace name justification
 \param stripchart (MtxStripChart *) pointer to stripchart
 */
GtkJustification mtx_stripchar_get_name_justification(MtxStripChart *chart)
{
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart), GTK_JUSTIFY_RIGHT);
	return priv->justification;
}
