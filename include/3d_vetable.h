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
 *
 * changelog
 * Ben Pierre 05/21/08
 * - define _RGB3f RGB3f rgb float triplet
 * - _RGB3f RGB3f structure
 * - prototype rgb_from_hue()
 * - set_shading_mode()
 * - drawFrameRate()
 */

#ifndef __3D_VETABLE_H__
#define __3D_VETABLE_H__

#include <enums.h>
#include <gtk/gtk.h>

/* GL includes */
#include <gtk/gtkwidget.h>
#include <gtk/gtkgl.h>
#ifdef __WIN32__
 #include <gdk/gdkglglext.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

/* Private structures */
typedef enum
{
	_X_,
	_Y_,
	_Z_
}Axis;

typedef struct _RGB3f RGB3f;
typedef struct _Quad Quad;
typedef struct _Cur_Vals Cur_Vals;
typedef struct _Ve_View_3D Ve_View_3D;

/*!
 \brief the _RGB3f structure contains rgb color values in standard floats
 */
struct _RGB3f
{
	float red;
	float green;
	float blue;
};


/*! 
 \brief _Quad describes a quad forthe 3D mesh grid
 */
struct _Quad
{
	gfloat x[4];
	gfloat y[4];
	gfloat z[4];
	RGB3f color[4];
};

/*!
 \brief the _Ve_View_3D structure contains all the field to create and 
 manipulate a 3D view of a MegaSquirt VE/Spark table, and should work in
 theory for any sized table
 */
struct _Ve_View_3D
{
	gint beginX;
	gint beginY;
	gint active_y;
	gint active_x;
	gfloat dt;
	gfloat sphi;
	gfloat stheta;
	gfloat sdepth;
	gfloat zNear;
	gfloat zFar;
	gfloat aspect;
	gfloat h_strafe;
	gfloat v_strafe;
	gfloat z_offset;
	gfloat x_trans;
	gfloat y_trans;
	gfloat z_trans;
	gfloat x_scale;
	gfloat y_scale;
	gfloat z_scale;
	gfloat x_max;
	gfloat y_max;
	gfloat z_max;
	gint x_precision;
	gint y_precision;
	gint z_precision;
	gint x_mult;
	gint y_mult;
	gint z_mult;
	/* Simple sources*/
	gchar *x_source;
	gchar *x_suffix;
	gchar *x_conv_expr;
	void *x_eval;
	gchar *y_source;
	gchar *y_suffix;
	gchar *y_conv_expr;
	void *y_eval;
	gchar *z_source;
	gchar *z_suffix;
	gchar *z_conv_expr;
	void *z_eval;
	/* Multi-sources */
	gchar * x_source_key;
	gboolean x_multi_source;
	GHashTable *x_multi_hash;
	gchar * y_source_key;
	gboolean y_multi_source;
	GHashTable *y_multi_hash;
	gchar * z_source_key;
	gboolean z_multi_source;
	GHashTable *z_multi_hash;

	GtkWidget *drawing_area;
	GtkWidget *window;
	GtkWidget *burn_but;
	GObject *dep_obj;
	gint x_base;
	gint x_page;
	gint x_bincount;
	DataSize x_size;
	gint y_base;
	gint y_page;
	gint y_bincount;
	DataSize y_size;
	gint z_base;
	gint z_page;
	DataSize z_size;
	gchar *table_name;
	gint table_num;
	gfloat opacity;
	gboolean tracking_focus;
	gboolean fixed_scale;
	gboolean wireframe;
	GtkWidget *tracking_button;
	Quad ***quad_mesh;
	gboolean mesh_created;
};


/*!
 \brief the _Cur_Vals structure contains The current data that pertains to the
 3D table view.
 */
struct _Cur_Vals
{
	gfloat x_val;
	gfloat x_edit_value;
	gchar *x_edit_text;
	gchar *x_runtime_text;
	void *x_eval;
	gfloat y_val;
	gfloat y_edit_value;
	gchar *y_edit_text;
	gchar *y_runtime_text;
	void *y_eval;
	gfloat z_val;
	gfloat z_edit_value;
	gchar *z_edit_text;
	gchar *z_runtime_text;
	void *z_eval;
};

/* Prototypes */
RGB3f rgb_from_hue(gfloat, gfloat, gfloat);
gint create_ve3d_view(GtkWidget *, gpointer );
gint free_ve3d_view(GtkWidget *);
GdkGLConfig* get_gl_config(void);
void ve3d_realize (GtkWidget *, gpointer );
gboolean ve3d_configure_event(GtkWidget *, GdkEventConfigure *,gpointer);
gboolean ve3d_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean ve3d_motion_notify_event(GtkWidget *, GdkEventMotion *,gpointer);
gboolean ve3d_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean ve3d_button_press_event(GtkWidget *, GdkEventButton *, gpointer);
void ve3d_calculate_scaling(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_ve_grid(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_edit_indicator(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_runtime_indicator(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_axis(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_active_vertexes_marker(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_text(gchar * text, gfloat x, gfloat y, gfloat z);
void ve3d_load_font_metrics(GtkWidget *);
void reset_3d_view(GtkWidget *);
Ve_View_3D * initialize_ve3d_view();
void update_ve3d_if_necessary(int , int );
Cur_Vals * get_current_values(Ve_View_3D *);
void free_current_values(Cur_Vals *);
gboolean set_opacity(GtkWidget *, gpointer );
gboolean set_tracking_focus(GtkWidget *, gpointer );
gboolean set_scaling_mode(GtkWidget *, gpointer );
gboolean set_rendering_mode(GtkWidget *, gpointer );
gboolean set_shading_mode(GtkWidget *, gpointer );
gfloat get_fixed_pos(Ve_View_3D *, void *,gfloat, Axis);
gint get_multiplier(DataSize );
void drawOrthoText(char *, GLclampf, GLclampf, GLclampf, GLfloat, GLfloat);
void generate_quad_mesh(Ve_View_3D *, Cur_Vals *);

/* Prototypes */

#endif
