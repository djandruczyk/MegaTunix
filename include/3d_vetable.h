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

#ifndef __3D_VETABLE_H__
#define __3D_VETABLE_H__

#include <gtk/gtk.h>
#include <structures.h>

/* GL includes */
#include <gtk/gtkwidget.h>
#include <gtk/gtkgl.h>
#ifdef __WIN32__
 #include <gdk/gdkglglext.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

/* Prototypes */
gint create_ve3d_view(GtkWidget *, gpointer );
gint free_ve3d_view(GtkWidget *);
GdkGLConfig* get_gl_config(void);
void ve3d_realize (GtkWidget *, gpointer );
gboolean ve3d_configure_event(GtkWidget *, GdkEventConfigure *,gpointer);
gboolean ve3d_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean ve3d_motion_notify_event(GtkWidget *, GdkEventMotion *,gpointer);
gboolean ve3d_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean ve3d_button_press_event(GtkWidget *, GdkEventButton *, gpointer);
void ve3d_calculate_scaling(struct Ve_View_3D *);
void ve3d_draw_ve_grid(struct Ve_View_3D *);
void ve3d_draw_active_indicator(struct Ve_View_3D *);
void ve3d_draw_runtime_indicator(struct Ve_View_3D *);
void ve3d_draw_axis(struct Ve_View_3D *);
void ve3d_draw_text(gchar * text, gfloat x, gfloat y, gfloat z);
void ve3d_load_font_metrics(void);
void reset_3d_view(GtkWidget *);
struct Ve_View_3D * initialize_ve3d_view();
void update_ve3d_if_necessary(int , int );

/* Prototypes */

#endif
