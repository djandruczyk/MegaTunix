/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __LOGVIEWER_GUI_H__
#define __LOGVIEWER_GUI_H__

#include <gtk/gtk.h>

/* Prototypes */
void build_logviewer(GtkWidget *);
void present_viewer_choices(void *);
void scroll_logviewer_traces(void);
gboolean lv_configure_event(GtkWidget *, GdkEventConfigure *, gpointer);
gboolean lv_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean view_value_set(GtkWidget *, gpointer );
gboolean populate_viewer(GtkWidget * );
struct Viewable_Value * build_v_value(GtkWidget *, gint );
GdkGC * initialize_gc(GdkDrawable *, GcType );
void draw_graticule(void * );
void draw_infotext(void * );
void draw_old_data(void * );
gboolean update_logview_traces(void);
void trace_update(gpointer, gpointer, gpointer );

/* Prototypes */

#endif
