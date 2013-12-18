/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
 * - prototype rgb_from_hue(void)
 * - set_shading_mode(void)
 * - drawFrameRate(void)
 */

/*!
  \file include/3d_vetable.h
  \ingroup Headers
  \brief Header for the 3D VE|Spark|Afr|Boost|Any Tables
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __3D_VETABLE_H__
#define __3D_VETABLE_H__

#include <enums.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <multi_expr_loader.h>

/* GL includes */

typedef struct _RGB3f RGB3f;
typedef struct _Quad Quad;
typedef struct _Cur_Vals Cur_Vals;
typedef struct _Ve_View_3D Ve_View_3D;

/*!
 \brief the _RGB3f structure contains rgb color values in standard floats
 */
struct _RGB3f
{
	float red;		/*!< Red color in 0<-X->1.0 */
	float green;		/*!< Green color in 0<-X->1.0 */
	float blue;		/*!< Blue color in 0<-X->1.0 */
};


/*!
 \brief _Quad describes a quad forthe 3D mesh grid
 */
struct _Quad
{
	gfloat x[4];		/*!< X coords array for the quad */
	gfloat y[4];		/*!< Y coords array for the quad */
	gfloat z[4];		/*!< Z coords array for the quad */
	RGB3f color[4];		/*!< Color at each point */
};

/*!
 \brief the _Ve_View_3D structure contains all the field to create and 
 manipulate a 3D view of a MegaSquirt VE/Spark table, and should work in
 theory for any sized table
 */
struct _Ve_View_3D
{
	gint beginX;		/*!< X Click coordinate */
	gint beginY;		/*!< Y Click coordinate */
	gint active_x;		/*!< Current active X position */
	gint active_y;		/*!< Current active X position */
	gfloat sphi;		/*!< Camera rotation horiz angle */
	gfloat stheta;		/*!< Camera rotation vertical angle */
	gfloat sdepth;		/*!< Zoom factor */
	gfloat zNear;		/*!< Near clip */
	gfloat zFar;		/*!< Far clip */
	gfloat aspect;		/*!< Aspect ratio */
	gfloat h_strafe;	/*!< Amount of horizotal offset */
	gfloat v_strafe;	/*!< Amount of vertical offset */
	gfloat z_offset;	/*!< offst from bottom of grid */
	gfloat x_trans;		/*!< factors to handle axis's */
	gfloat y_trans;		/*!< factors to handle axis's */
	gfloat z_trans;		/*!< factors to handle axis's */
	gfloat x_scale;		/*!< X Scale factor */
	gfloat y_scale;		/*!< Y Scale factor */
	gfloat z_scale;		/*!< Z Scale factor */
	gfloat x_max;		/*!< X Max Value */
	gfloat y_max;		/*!< Y Max Value */
	gfloat z_max;		/*!< Z Max Value */
	gint x_precision;	/*!< X Precision */
	gint y_precision;	/*!< Y Precision */
	gint z_precision;	/*!< Z Precision */
	gint x_mult;		/*!< X Multiplier */
	gint y_mult;		/*!< Y Multiplier */
	gint z_mult;		/*!< Z Multiplier */
	gint z_minval;		/*!< Z Minimum Value */
	gint z_maxval;		/*!< Z Maximum Value */
	gint x_smallstep;	/*!< X Smallstep for hotkeys */
	gint x_bigstep;		/*!< X Bigstep for hotkeys */
	gint y_smallstep;	/*!< Y smallstep for hotkeys */
	gint y_bigstep;		/*!< Y bigstep for hotkeys */
	gint z_smallstep;	/*!< Z smallstep for hotkeys */
	gint z_bigstep;		/*!< Z bigstep for hotkeys */
	/* Simple sources*/
	gchar *x_source;	/*!< X Source name */
	gchar *x_suffix;	/*!< X Suffix */
	gchar *x_conv_expr;	/*!< X conversion expression */
	void *x_eval;		/*!< X Evaluator */
	gchar *y_source;	/*!< Y source name */
	gchar *y_suffix;	/*!< Y suffix */
	gchar *y_conv_expr;	/*!< Y Conversion Expression */
	void *y_eval;		/*!< Y evaluator */
	gchar *z_source;	/*!< Z source name */
	gchar *z_suffix;	/*!< Z Suffix */
	gchar *z_conv_expr;	/*!< Z Conversion Expression */
	void *z_eval;		/*!< Z Evaluator */
	gchar * z_depend_on;	/*!< Z Dependancy */
	GObject *x_container;	/*!< X Container */
	GObject *y_container;	/*!< Y Container */
	GObject *z_container;	/*!< Z Container */
	GObject **x_objects;	/*!< Array of X Objects */
	GObject **y_objects;	/*!< Array of Y Objects */
	GObject ***z_objects;	/*!< Array of Z Objects */
	/* Multi-sources */
	gchar * x_source_key;	/*!< X Source key (multi-source only) */
	gboolean x_ignore_algorithm;/*!< ignore algorithms */
	gboolean x_multi_source;/*!< X Multi-source or not */
	GHashTable *x_multi_hash;/*!< X Multi Hash */
	gchar * y_source_key;	/*!< Y Source key (multi-source only) */
	gboolean y_ignore_algorithm;/*!< ignore algorithms */
	gboolean y_multi_source;/*!< Y Multi-source or not */
	GHashTable *y_multi_hash;/*!< Y Multi Hash */
	gchar * z_source_key;	/*!< Z Source key (multi-source only) */
	gboolean z_ignore_algorithm;/*!< ignore algorithms */
	gboolean z_multi_source;/*!< Z Multi-source or not */
	GHashTable *z_multi_hash;/*!< Z Multi Hash */

	GtkWidget *drawing_area;/*!< where the magic happens */
	GtkWidget *window;	/*!< main window */
	GtkWidget *burn_but;	/*!< Burn button */
	GObject *dep_obj;	/*!< Dependant object */
	gint x_base;		/*!< X base offset */
	gint x_page;		/*!< X page */
	gint x_bincount;	/*!< X bincount */
	DataSize x_size;	/*!< X variable size (8,16,32 signed/unsigned)*/
	gint y_base;		/*!< Y base offset */
	gint y_page;		/*!< Y page */
	gint y_bincount;	/*!< Y bincount */
	DataSize y_size;	/*!< Y variable size (8,16,32 signed/unsigned)*/
	gint z_base;		/*!< Z base offset */
	gint z_page;		/*!< Z page */
	gint z_raw_lower;	/*!< Raw low Z value */
	gint z_raw_upper;	/*!< Raw upper Z value */
	DataSize z_size;	/*!< Z variable size (8,16,32 signed/unsigned)*/
	gchar *table_name;	/*!< Table name */
	gint table_num;		/*!< Table number */
	gfloat opacity;		/*!< Opacity factor */
	gboolean tracking_focus;/*!< Is tracking focus turned on? */
	gboolean fixed_scale;	/*!< IS this using a fixed scale? */
	gboolean wireframe;	/*!< IS this using wireframe rendering? */
	GtkWidget *tracking_button;/*!< Tracking button widget */
	Quad ***quad_mesh;	/*!< Array of Quads for the mesh */
	gboolean mesh_created;	/*!< Is the mesh calculated yet? */
	gboolean gl_initialized;/*!< Flag for init of the window */
	gint font_ascent;	/*!< GL Font ascent value */
	gint font_descent;	/*!< GL Font ascent value */
	gint y_offset_bitmap_render_pango_units; /*!< GL font y offset */
	PangoContext *ft2_context; /*!< GL Font pango context */
	gboolean font_created;	/*!< GL Font created flag */
	gfloat fps;		/*!< Frames per second */
	glong lasttime;		/*!< Time of last render */
	gchar strfps[50];	/*!< FPS String */
	gint render_id;		/*!< Render thread ID */
	gint requested_fps;	/*!< Requested Frames per second */
	GMutex *mutex;		/*!< Protection Mutex */
};


/*!
 \brief the _Cur_Vals structure contains The current data that pertains to the
 3D table view.
 */
struct _Cur_Vals
{
	gfloat x_val;		/*!< Current X value */
	gfloat p_x_vals[3];	/*!< Previous 3 X values */
	gfloat x_edit_value;	/*!< X Edit value */
	gchar *x_edit_text;	/*!< X Edit Text */
	gchar *x_runtime_text;	/*!< X Runtime Text */
	void *x_eval;		/*!< X Evaluator */
	gint x_precision;	/*!< X Precision */
	gfloat y_val;		/*!< Current Y Value */
	gfloat p_y_vals[3];	/*!< Previous 3 Y values */
	gfloat y_edit_value;	/*!< Y Edit value */
	gchar *y_edit_text;	/*!< Y Edit Text */
	gchar *y_runtime_text;	/*!< Y Runtime Text */
	void *y_eval;		/*!< Y Evalutator */
	gint y_precision;	/*!< Y Precision */
	gfloat z_val;		/*!< Current Z Value */
	gfloat p_z_vals[3];	/*!< Previous 3 Z values */
	gfloat z_edit_value;	/*!< Z Edit value */
	gchar *z_edit_text;	/*!< Z Edit Text */
	gchar *z_runtime_text;	/*!< Z Runtime Text */
	void *z_eval;		/*!< Z Evaluator */
	gint z_precision;	/*!< Z Precision */
};

/* Prototypes */
gboolean create_ve3d_view(GtkWidget *, gpointer );
gboolean call_ve3d_shutdown(GtkWidget *, gpointer);
gboolean delayed_expose(gpointer);
gboolean delayed_expose_wrapper(gpointer);
gboolean delayed_reconfigure(gpointer);
gboolean delayed_reconfigure_wrapper(gpointer);
void free_current_values(Cur_Vals *);
void generate_quad_mesh(Ve_View_3D *, Cur_Vals *);
Cur_Vals * get_current_values(Ve_View_3D *);
gfloat get_fixed_pos(Ve_View_3D *, gfloat, Axis);
void gl_print_string(GtkWidget *, const gchar *);
void gl_destroy_font(GtkWidget *);
void gl_create_font(GtkWidget *);
Ve_View_3D * initialize_ve3d_view(void);
gfloat multi_lookup_and_compute(MultiSource *);
void multi_lookup_and_compute_n(MultiSource *, gint, gint, gfloat *);
void queue_ve3d_update(Ve_View_3D *);
RGB3f rgb_from_hue(gfloat, gfloat, gfloat);
void reset_3d_view(GtkWidget *);
gboolean set_fps(GtkWidget *, gpointer );
gboolean set_opacity(GtkWidget *, gpointer );
gboolean set_rendering_mode(GtkWidget *, gpointer );
gboolean set_scaling_mode(GtkWidget *, gpointer );
gboolean set_shading_mode(GtkWidget *, gpointer );
gboolean set_tracking_focus(GtkWidget *, gpointer );
gboolean sleep_and_redraw(gpointer);
gboolean sleep_and_redraw_wrapper(gpointer);
gboolean update_ve3d(gpointer);
void update_ve3d_if_necessary(int , int );
gboolean update_ve3d_wrapper(gpointer);
gboolean ve3d_button_press_event(GtkWidget *, GdkEventButton *, gpointer);
void ve3d_calculate_scaling(Ve_View_3D *, Cur_Vals *);
gboolean ve3d_configure_event(GtkWidget *, GdkEventConfigure *,gpointer);
void ve3d_draw_active_vertexes_marker(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_axis(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_edit_indicator(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_runtime_indicator(Ve_View_3D *, Cur_Vals *);
void ve3d_draw_text(GtkWidget *, gchar * text, gfloat x, gfloat y, gfloat z);
void ve3d_draw_ve_grid(Ve_View_3D *, Cur_Vals *);
gboolean ve3d_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
void ve3d_grey_window(Ve_View_3D *);
gboolean ve3d_key_press_event (GtkWidget *, GdkEventKey *, gpointer);
gboolean ve3d_motion_notify_event(GtkWidget *, GdkEventMotion *,gpointer);
gint ve3d_realize (GtkWidget *, gpointer );
gboolean ve3d_shutdown(GtkWidget *, GdkEvent *,  gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
