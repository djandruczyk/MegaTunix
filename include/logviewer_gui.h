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
#include <structures.h>

/* Prototypes */
void present_viewer_choices(void);
void scroll_logviewer_traces(void);
void reset_logviewer_state(void);
gboolean view_value_set(GtkWidget *, gpointer );
gboolean set_lview_choices_state(GtkWidget *, gpointer);
gboolean slider_key_press_event(GtkWidget *, GdkEventKey *, gpointer);
struct Viewable_Value * build_v_value(GObject * );
GdkGC * initialize_gc(GdkDrawable *, GcType );
GdkColor get_colors_from_hue(gfloat, gfloat, gfloat);
void draw_infotext();
void draw_valtext(gboolean);
gboolean rt_update_logview_traces(gboolean);
gboolean pb_update_logview_traces(gboolean);
void trace_update(gboolean );
gboolean logviewer_log_position_change(GtkWidget *, gpointer);
void set_realtime_mode(void);
void set_playback_mode(void);
void finish_logviewer(void);
void populate_viewer(void);
gboolean reenable_select_params_button(GtkWidget *);

/* Prototypes */

#endif
