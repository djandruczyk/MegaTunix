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

#ifndef __TUNING_GUI_H__
#define __TUNING_GUI_H__

#include <gtk/gtk.h>

/* GL includes */
#include <gtk/gtkwidget.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkglglext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <pango/pangoft2.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms.h>

/* Prototypes */
int build_tuning(GtkWidget *);
GdkGLConfig* get_gl_config(void);
void tuning_gui_realize (GtkWidget *, gpointer );
gboolean tuning_gui_configure_event(GtkWidget *, GdkEventConfigure *,gpointer);
gboolean tuning_gui_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean tuning_gui_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean tuning_gui_motion_notify_event(GtkWidget *, GdkEventMotion *,gpointer);
gboolean tuning_gui_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean tuning_gui_button_press_event(GtkWidget *, GdkEventButton *, gpointer);
gboolean tuning_gui_key_press_event (GtkWidget *, GdkEventKey *, gpointer );
gboolean tuning_gui_focus_in_event (GtkWidget *, GdkEventFocus *, gpointer );
void tuning_gui_draw_ve_grid(void);
void tuning_gui_reset_ve_grid(void);
void tuning_gui_normalize(float );
void tuning_gui_draw_grid_point(float , float , float , int );
void tuning_gui_draw_labels(void);
void reset_3d_view(void);
/* Prototypes */

#endif
