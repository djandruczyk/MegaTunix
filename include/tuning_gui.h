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

/* Prototypes */
int build_tuning(GtkWidget *);
GdkGLConfig* get_gl_config(void);
void tuning_gui_realize (GtkWidget *widget, gpointer data);
gboolean tuning_gui_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
gboolean tuning_gui_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean tuning_gui_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data);
gboolean tuning_gui_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
void tuning_gui_draw_ve_grid(void);
void tuning_gui_reset_ve_grid(void);
void tuning_gui_normalize(float v[3]);
void tuning_gui_draw_grid_point(float i, float j, float k, int normalise);
void tuning_gui_draw_labels(void);
/* Prototypes */

#endif
