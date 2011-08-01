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

/*! @file widgets/stripchart.c
 *
 * @brief ...
 *
 *
 */


#include <stripchart-private.h>


/*!
 \brief gets called when a user wants a new stripchart
 \returns a pointer to a newly created stripchart widget
 */
GtkWidget *mtx_stripchart_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_STRIPCHART, NULL));
}


/*!
 \brief gets the current value 
 \param stripchart is the pointer to the stripchart object
 \param values is a pointer to store the latest values into
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_stripchart_get_latest_values (MtxStripChart *stripchart, gfloat *values)
{
	gint i = 0;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(stripchart);
	g_return_val_if_fail ((MTX_IS_STRIPCHART (stripchart)),FALSE);
	g_return_val_if_fail (values,FALSE);
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
 \param chart is the pointer to the stripchart object
 \param values is the new set of values. This code makes some bad assumptions
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_stripchart_set_values (MtxStripChart *chart, gfloat* values)
{
	gint i = 0;
	static gint count = 0;
	gfloat val = 0.0;
	MtxStripChartTrace *trace = NULL;
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart),FALSE);
	g_return_val_if_fail (values,FALSE);

	g_object_freeze_notify (G_OBJECT (chart));
	for (i=0;i<priv->num_traces;i++)
	{
		trace = g_array_index(priv->traces, MtxStripChartTrace *, i);
		val = values[i] > trace->max ? trace->max:values[i];
		val = val < trace->min ? trace->min:val;
		trace->history = g_array_append_val(trace->history,val);
		if (trace->history->len > 2*priv->w)
			trace->history = g_array_remove_range(trace->history,0,trace->history->len-(2*priv->w));
	}
	g_object_thaw_notify (G_OBJECT (chart));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(chart)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(chart)))
		return TRUE;
#endif

	mtx_stripchart_redraw(chart);
	return TRUE;
}


/*!
 \brief Adds an additional trace
 \param chart is the pointer to the stripchart object
 \param min is the minimum value for this trace
 \param max is the maximum value for this trace
 \param precision is the precision for this trace
 \param name is the name to display for this trace
 \param color is the color to draw this trace
 \returns the trace id number or -1 on error
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
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(chart)))
		return trace->id;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(chart)))
		return trace->id;
#endif
	mtx_stripchart_redraw(chart);
	return trace->id;
}


/*!
 \brief Removes trace per specifed index
 \param chart is the pointer to the stripchart object
 \param index is the trace ID to delete
 \returns TRUE on success, FALSE otherwise
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
	generate_stripchart_static_traces(chart);
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(chart)))
		return retval;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(chart)))
		return retval;
#endif
	mtx_stripchart_redraw(chart);
	return retval;
}


/*!
 \brief Sets trace name justification
 \param chart is the pointer to the stripchart object
 \param justification is the justification enum for the stripchart object
 \returns TRUE on success, FALSE otherwise
 */
gboolean mtx_stripchart_set_name_justification(MtxStripChart *chart, GtkJustification justification)
{
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart), FALSE);
	g_return_val_if_fail ((justification != GTK_JUSTIFY_LEFT) ||
			(justification != GTK_JUSTIFY_RIGHT) ||
			(justification != GTK_JUSTIFY_CENTER) ||
			(justification != GTK_JUSTIFY_FILL),FALSE);
	g_object_freeze_notify (G_OBJECT (chart));
	priv->justification = justification;
	g_object_thaw_notify (G_OBJECT (chart));
#if GTK_MINOR_VERSION >= 18
	if (!gtk_widget_is_sensitive(GTK_WIDGET(chart)))
		return TRUE;
#else
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(chart)))
		return TRUE;
#endif
	mtx_stripchart_redraw(chart);
	return TRUE;
}


/*!
 \brief Gets trace name justification
 \param chart is the pointer to the stripchart object
 \returns the justification enum for the stripchart object
 */
GtkJustification mtx_stripchar_get_name_justification(MtxStripChart *chart)
{
	MtxStripChartPrivate *priv = MTX_STRIPCHART_GET_PRIVATE(chart);
	g_return_val_if_fail (MTX_IS_STRIPCHART (chart), GTK_JUSTIFY_RIGHT);
	return priv->justification;
}
