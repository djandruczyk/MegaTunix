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
 */

#ifndef MTX_CURVE_H
#define MTX_CURVE_H

#include <config.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define MTX_TYPE_CURVE		(mtx_curve_get_type ())
#define MTX_CURVE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MTX_TYPE_CURVE, MtxCurve))
#define MTX_CURVE_CLASS(obj)		(G_TYPE_CHECK_CLASS_CAST ((obj), MTX_CURVE, MtxCurveClass))
#define MTX_IS_CURVE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTX_TYPE_CURVE))
#define MTX_IS_CURVE_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MTX_TYPE_CURVE))
#define MTX_CURVE_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), MTX_TYPE_CURVE, MtxCurveClass))


typedef struct _MtxCurve		MtxCurve;
typedef struct _MtxCurveClass	MtxCurveClass;
typedef struct _MtxCurveCoord	MtxCurveCoord;

/*! CurveColorIndex enum,  for indexing into the color arrays */
typedef enum  
{
	CURVE_COL_BG = 0,
	CURVE_COL_FG,
	CURVE_COL_SEL,
	CURVE_COL_GRAT,
	CURVE_COL_TEXT,
	CURVE_COL_MARKER,
	CURVE_NUM_COLORS
}CurveColorIndex;

enum
{	
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

struct _MtxCurve
{	/* public data */
	GtkDrawingArea parent;
};

struct _MtxCurveClass
{
	GtkDrawingAreaClass parent_class;
	void (*coords_changed) (MtxCurve *curve);
};

struct _MtxCurveCoord
{
	gfloat x;
	gfloat y;
};

GType mtx_curve_get_type (void) G_GNUC_CONST;

GtkWidget* mtx_curve_new (void);

/* Point manipulation */
gboolean mtx_curve_get_coords (MtxCurve *, gint *, MtxCurveCoord *);
/* Do NOT free array of returned points! */
gboolean mtx_curve_set_coords (MtxCurve *, gint , MtxCurveCoord *);
gboolean mtx_curve_get_coords_at_index (MtxCurve *, gint , MtxCurveCoord * );
gboolean mtx_curve_set_coords_at_index (MtxCurve *, gint , MtxCurveCoord );
gboolean mtx_curve_set_empty_array(MtxCurve *, gint);
gint mtx_curve_get_active_coord_index(MtxCurve *);

/* Precision of data */
gboolean mtx_curve_set_x_precision(MtxCurve *, gint);
gboolean mtx_curve_set_y_precision(MtxCurve *, gint);
gint mtx_curve_get_x_precision(MtxCurve *);
gint mtx_curve_get_y_precision(MtxCurve *);


/* Title */
gboolean mtx_curve_set_title (MtxCurve *,gchar *);
const gchar * mtx_curve_get_title (MtxCurve *);

/* Colors */
gboolean mtx_curve_set_color (MtxCurve *, CurveColorIndex , GdkColor );
gboolean mtx_curve_get_color (MtxCurve *, CurveColorIndex , GdkColor *);

/* Rendering */
gboolean mtx_curve_set_auto_hide_vertexes (MtxCurve *, gboolean);
gboolean mtx_curve_get_get_auto_hide_vertexes (MtxCurve *);
gboolean mtx_curve_set_show_vertexes (MtxCurve *, gboolean);
gboolean mtx_curve_get_get_show_vertexes (MtxCurve *);
gboolean mtx_curve_set_show_graticule (MtxCurve *, gboolean );
gboolean mtx_curve_get_show_graticule (MtxCurve *);
gboolean mtx_curve_get_show_x_marker (MtxCurve *);
gboolean mtx_curve_get_show_y_marker (MtxCurve *);
gboolean mtx_curve_set_show_x_marker (MtxCurve *, gboolean );
gboolean mtx_curve_set_show_y_marker (MtxCurve *, gboolean );
gboolean mtx_curve_set_x_marker_value (MtxCurve *, gfloat );
gboolean mtx_curve_set_y_marker_value (MtxCurve *, gfloat );
gboolean mtx_curve_set_hard_limits (MtxCurve *, gint, gint, gint, gint);
gboolean mtx_curve_get_hard_limits (MtxCurve *, gint *, gint *, gint *, gint *);

G_END_DECLS

#endif
