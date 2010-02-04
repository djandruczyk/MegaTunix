/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix curve widget
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

#define MTX_CURVE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_CURVE, MtxCurvePrivate))
typedef struct _MtxCurvePrivate MtxCurvePrivate;
struct _MtxCurvePrivate
{
	MtxCurve *self;		/* Pointer to public opbject */
        GdkColor colors[CURVE_NUM_COLORS];    /* Colors Array */
        GdkPixmap *pixmap;      /*! Update/backing pixmap */
        GdkPixmap *bg_pixmap;   /*! Static rarely changing pixmap */
        MtxCurveCoord *coords;       /* Points array */
        GdkPoint *points;       /*! Onscreen coords array (for mouse) */
	gint proximity_threshold;	/*! Proximity threshold */
	gint proximity_vertex;	/*! Closest one to mouse.. */
	gint marker_proximity_vertex;	/*! Closest one to marker.. */
        gint num_points;        /*! Total Points*/
        gint w;                 /*! Width of full widget */
        gint h;                 /*! Height of full widget */
	gfloat x_lower_limit;	/*! hard limit to gauge */
	gfloat y_lower_limit;	/*! hard limit to gauge */
	gfloat x_upper_limit;	/*! hard limit to gauge */
	gfloat y_upper_limit;	/*! hard limit to gauge */
        gfloat lowest_x;	/*! Lowest X value in points[] */
        gfloat highest_x;	/*! Highest X value in points[] */
        gfloat lowest_y;	/*! Lowest Y value in points[] */
        gfloat highest_y;	/*! Highest Y value in points[] */
        gint x_peak_timeout;	/*! X Peak/Hold timeout */
        gint y_peak_timeout;	/*! Y Peak/Hold timeout */
        gint x_border;          /*! X Border in pixels */
        gint y_border;          /*! Y Border in pixels */
        gint active_coord;      /*! Active Coordinate */
	gint x_precision;	/*! Precision for X axis */
	gint y_precision;	/*! Precision for X axis */
	gint vertex_id;		/*! timeout ID */
        gfloat locked_scale;    /*! minimum fixed scale for both axis' */
        gfloat x_scale;         /*! X coord points->coords scaler */
        gfloat y_scale;         /*! Y coord points->coords scaler */
	gfloat x_marker;	/*! X marker (vertical line) */
	gfloat peak_x_marker;	/*! X marker (vertical line) */
	gfloat peak_y_at_x_marker;	/*! X marker (vertical line) */
	gfloat y_at_x_marker;	/*! X marker (vertical line) */
	gfloat y_marker;	/*! Y marker (horizontal line) */
	gfloat peak_y_marker;	/*! Y marker (horizontal line) */
	gfloat peak_x_at_y_marker;	/*! Y marker (horizontal line) */
	gfloat x_at_y_marker;	/*! Y marker (horizontal line) */
	gboolean x_draw_peak;	/*! X draw peak */
	gboolean y_draw_peak;	/*! Y draw peak */
        gboolean vertex_selected;/*! Do we have one selected? */
        gboolean auto_hide;	/*! Auto hide vertex on focus out */
        gboolean show_x_marker; /*! Show x_marker rectangles */
        gboolean show_y_marker; /*! Show y_marker rectangles */
	gboolean show_edit_marker;	/*! Show edit marker */
        gboolean show_vertexes; /*! Show vertex rectangles */
        gboolean show_grat;	/*! Draw graticule? */
        gboolean coord_changed;	/*! Flag */
	gboolean x_blocked_from_edit;	/* Prevent mouse edit of an axis */
	gboolean y_blocked_from_edit;	/* Prevent mouse edit of an axis */
        GdkGC *gc;              /*! Graphics Context */
        cairo_t *cr;            /*! Cairo context,  not sure if this is good
                                   too hold onto or not */
        cairo_font_options_t * font_options;
	gfloat x_label_border;	/*! Space taken by x axis marker */
	gfloat y_label_border;	/*! Space taken by x axis marker */
        gchar * pos_str;	/*! Position String */
        gchar * font;           /*! Font to use */
        gchar * axis_font;	/*! Axis Font to use */
        gchar * title;          /*! Title to use */
	gchar * x_axis_label;	/*! X Axis Label */
	gchar * y_axis_label;	/*! X Axis Label */

        GdkColormap *colormap;  /*! Colormap for GC's */
};

enum
{
	_X_ = 0x91837,
	_Y_
};

gboolean mtx_curve_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_curve_expose (GtkWidget *, GdkEventExpose *);
gboolean mtx_curve_button_event (GtkWidget *,GdkEventButton *);
gboolean mtx_curve_motion_event (GtkWidget *,GdkEventMotion *);
gboolean mtx_curve_focus_event (GtkWidget *, GdkEventCrossing *);
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
void recalc_extremes (MtxCurvePrivate *);
void recalc_points (MtxCurvePrivate *);
void draw_selected_msg(cairo_t *, MtxCurvePrivate *);
gboolean cancel_peak(gpointer);
gboolean auto_rescale(gpointer );
gboolean delay_turnoff_vertexes(gpointer);
gboolean proximity_test (GtkWidget *, GdkEventMotion *);
gboolean get_intersection(gfloat, gfloat, gfloat, gfloat, gfloat, gfloat, gfloat, gfloat, gfloat *, gfloat *);









#endif
