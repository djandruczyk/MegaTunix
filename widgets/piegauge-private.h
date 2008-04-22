/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt pie gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 * This is a the PRIVATE implementation header file for INTERNAL functions
 * of the widget.  Public functions as well the the gauge structure are 
 * defined in the gauge.h header file
 *
 */

#ifndef __PIEGAUGE_PRIVATE_H__
#define __PIEGAUGE_PRIVATE_H__

#include <gtk/gtk.h>
#include <piegauge.h>


gboolean mtx_pie_gauge_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_pie_gauge_expose (GtkWidget *, GdkEventExpose *);
/* Not needed yet
* gboolean mtx_pie_gauge_button_press (GtkWidget *,GdkEventButton *);
* gboolean mtx_pie_gauge_motion_event (GtkWidget *,GdkEventMotion *);
*/
void mtx_pie_gauge_size_request (GtkWidget *, GtkRequisition *);
gboolean mtx_pie_gauge_button_release (GtkWidget *,GdkEventButton *);
void cairo_generate_pie_gauge_background(MtxPieGauge *);
void cairo_update_pie_gauge_position (MtxPieGauge *);
void gdk_generate_pie_gauge_background(MtxPieGauge *);
void gdk_update_pie_gauge_position (MtxPieGauge *);
void mtx_pie_gauge_init_colors(MtxPieGauge *);
void mtx_pie_gauge_redraw (MtxPieGauge *gauge);
void generate_pie_gauge_background(MtxPieGauge *);
void update_pie_gauge_position (MtxPieGauge *);


#endif
