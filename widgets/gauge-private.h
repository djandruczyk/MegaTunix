/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire (czb)
 *
 * Megasquirt gauge widget
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

#ifndef __GAUGE_PRIVATE_H__
#define  __GAUGE_PRIVATE_H__

#include <gtk/gtk.h>
#include <gauge.h>


gboolean mtx_gauge_face_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_gauge_face_expose (GtkWidget *, GdkEventExpose *);
gboolean mtx_gauge_face_button_press (GtkWidget *,GdkEventButton *);
/* Not needed yet
* gboolean mtx_gauge_face_motion_event (GtkWidget *,GdkEventMotion *);
*/
void mtx_gauge_face_size_request (GtkWidget *, GtkRequisition *);
gboolean mtx_gauge_face_button_release (GtkWidget *,GdkEventButton *);
void cairo_generate_gauge_background(MtxGaugeFace *);
void cairo_update_gauge_position (MtxGaugeFace *);
void gdk_generate_gauge_background(MtxGaugeFace *);
void gdk_update_gauge_position (MtxGaugeFace *);
void mtx_gauge_face_init_colors(MtxGaugeFace *);
void mtx_gauge_face_init_name_bindings(MtxGaugeFace *);
void mtx_gauge_face_init_xml_hash(MtxGaugeFace *);
void mtx_gauge_face_init_default_tick_group(MtxGaugeFace *);


#endif
