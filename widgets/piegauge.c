/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt pie gauge widget
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
#ifdef HAVE_CAIRO
#include <cairo/cairo.h>
#endif
#include <piegauge.h>
#include <piegauge-private.h>
#include <gtk/gtk.h>


/*!
 \brief gets called when  a user wants a new gauge
 \returns a pointer to a newly created gauge widget
 */
GtkWidget *mtx_pie_gauge_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_PIE_GAUGE, NULL));
}


/*!
 \brief gets the current value 
 \param gauge (MtxPieGauge *) pointer to gauge
 */
gfloat mtx_pie_gauge_get_value (MtxPieGauge *gauge)
{
	g_return_val_if_fail ((MTX_IS_PIE_GAUGE (gauge)),0.0);
	return	gauge->value;
}


/*!
 \brief sets the current value 
 \param gauge (MtxPieGauge *) pointer to gauge
 \param value (gfloat) new value
 */
void mtx_pie_gauge_set_value (MtxPieGauge *gauge, gfloat value)
{
	g_return_if_fail (MTX_IS_PIE_GAUGE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->value = value;
	g_object_thaw_notify (G_OBJECT (gauge));
	mtx_pie_gauge_redraw(gauge);
}


