/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt curve widget
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
        GdkColor colors[CURVE_NUM_COLORS];    /* Colors Array */
        GdkPixmap *pixmap;      /*! Update/backing pixmap */
        GdkPixmap *bg_pixmap;   /*! Static rarely changing pixmap */
        MtxCurveCoord *coords;       /* Points array */
        GdkPoint *points;       /*! Onscreen coords array (for mouse) */
        gint num_points;        /*! Total Points*/
        gint w;                 /*! Width of full widget */
        gint h;                 /*! Height of full widget */
        gint lowest_x;          /*! Lowest X value in points[] */
        gint highest_x;         /*! Highest X value in points[] */
        gint lowest_y;          /*! Lowest Y value in points[] */
        gint highest_y;         /*! Highest Y value in points[] */
        gint border;            /*! Border in pixels */
        gint active_coord;      /*! Active Coordinate */
	gint x_precision;	/*! Precision for X axis */
	gint y_precision;	/*! Precision for X axis */
	gint vertex_id;		/*! timeout ID */
        gfloat locked_scale;    /*! minimum fixed scale for both axis' */
        gfloat x_scale;         /*! X coord points->coords scaler */
        gfloat y_scale;         /*! Y coord points->coords scaler */
	gfloat x_marker;	/*! X marker (vertical line) */
	gfloat y_marker;	/*! X marker (horizontal line) */
        gboolean vertex_selected;/*! Do we have one selected? */
        gboolean auto_hide;	/*! Auto hide vertex on focus out */
        gboolean show_x_marker; /*! Show x_marker rectangles */
        gboolean show_y_marker; /*! Show y_marker rectangles */
        gboolean show_vertexes; /*! Show vertex rectangles */
        gboolean show_grat;	/*! Draw graticule? */
        gboolean coord_changed;	/*! Flag */
        GdkGC *gc;              /*! Graphics Context */
        cairo_t *cr;            /*! Cairo context,  not sure if this is good
                                   too hold onto or not */
        cairo_font_options_t * font_options;
        gchar * font;           /*! Font to use */
        gchar * title;          /*! Title to use */

        GdkColormap *colormap;  /*! Colormap for GC's */
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
gboolean delay_turnoff_vertexes(gpointer);


#endif
