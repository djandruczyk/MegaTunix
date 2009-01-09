/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt curve widget
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

#ifndef __CURVE_PRIVATE_H__
#define __CURVE_PRIVATE_H__

#include <gtk/gtk.h>
#include <curve.h>


gboolean mtx_curve_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_curve_expose (GtkWidget *, GdkEventExpose *);
gboolean mtx_curve_button_event (GtkWidget *,GdkEventButton *);
gboolean mtx_curve_motion_event (GtkWidget *,GdkEventMotion *);
void mtx_curve_size_request (GtkWidget *, GtkRequisition *);
void mtx_curve_class_init (MtxCurveClass *class_name);
void mtx_curve_init (MtxCurve *gauge);
gboolean mtx_curve_button_release (GtkWidget *,GdkEventButton *);
void generate_curve_background(MtxCurve *);
void update_curve_position (MtxCurve *);
void mtx_curve_init_colors(MtxCurve *);
void mtx_curve_redraw (MtxCurve *gauge);
void generate_curve_background(MtxCurve *);
void update_curve_position (MtxCurve *);


#endif
