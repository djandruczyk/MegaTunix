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
gint create_3d_view(GtkWidget *, gpointer );
gint reset_3d_winstat(GtkWidget *);
GdkGLConfig* get_gl_config(void);
void ve_realize (GtkWidget *, gpointer );
gboolean ve_configure_event(GtkWidget *, GdkEventConfigure *,gpointer);
gboolean ve_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean ve_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean ve_motion_notify_event(GtkWidget *, GdkEventMotion *,gpointer);
gboolean ve_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean ve_button_press_event(GtkWidget *, GdkEventButton *, gpointer);
gboolean ve_key_press_event (GtkWidget *, GdkEventKey *, gpointer );
gboolean ve_focus_in_event (GtkWidget *, GdkEventFocus *, gpointer );
void ve_draw_ve_grid(void *);
void ve_reset_ve_grid(void);
void ve_normalize(float );
void ve_draw_grid_point(float , float , float , int );
void ve_draw_labels(void);
void ve_draw_active_indicator(void *);
void ve_calculate_scaling(void *);
void ve_draw_axis(void *);
void ve_drawtext(char* text, float x, float y, float z);
void ve_load_font_metrics(void);
void reset_3d_view(GtkWidget *);
void initialize_ve_view(void *);
void ve_draw_runtime_indicator(void *);

/* Prototypes */

/* Datastructures... */
struct Ve_View_3D
{
	gint beginX;
	gint beginY;
	gint active_load;
	gint active_rpm;
	gfloat dt;
	gfloat sphi;
	gfloat stheta;
	gfloat sdepth;
	gfloat zNear;
	gfloat zFar;
	gfloat aspect;
	gfloat rpm_div;
	gfloat load_div;
	gfloat ve_div;
	gfloat h_strafe;
	gfloat v_strafe;
	gint rpm_max;
	gint load_max;
	gint ve_max;
	GtkWidget *drawing_area;
	GtkWidget *window;
	GtkWidget *burn_but;
	gint table;
};

#endif
