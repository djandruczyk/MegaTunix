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
#define MTX_CURVE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_CURVE, MtxCurvePrivate))


typedef struct _MtxCurve		MtxCurve;
typedef struct _MtxCurvePrivate	MtxCurvePrivate;
typedef struct _MtxCurveClass	MtxCurveClass;

/*! ColorIndex enum,  for indexing into the color arrays */
typedef enum  
{
	COL_BG = 0,
	COL_FG,
	COL_AXIS,
	COL_TEXT,
	NUM_COLORS
}ColorIndex;


struct _MtxCurvePrivate
{
	GdkColor colors[NUM_COLORS];	/* Colors Array */
	GdkPixmap *pixmap;	/*! Update/backing pixmap */
	GdkPixmap *bg_pixmap;	/*! Static rarely changing pixmap */
	GdkPoint *points;	/* Points array */
	GtkUpdateType type;	/* Update mode */
	gint num_points;	/* Total Points*/
	gint w;			/* Width of full widget */
	gint h;			/* Height of full widget */
	GdkGC *gc;		/* Graphics Context */
	cairo_t *cr;		/*! Cairo context,  not sure if this is good
				   too hold onto or not */
        cairo_font_options_t * font_options;
	gchar * font;		/*! Font to use */
	gchar * title;		/*! Title to use */

	GdkColormap *colormap;	/*! Colormap for GC's */
};


struct _MtxCurve
{	/* public data */
	GtkDrawingArea parent;
};

struct _MtxCurveClass
{
	GtkDrawingAreaClass parent_class;
};

GType mtx_curve_get_type (void) G_GNUC_CONST;

GtkWidget* mtx_curve_new (void);
void mtx_curve_set_points (MtxCurve *, gint , GdkPoint *);
/* Do NOT free array of returned points! */
void mtx_curve_get_points (MtxCurve *, gint *, GdkPoint *);
gboolean mtx_curve_set_point_at_index (MtxCurve *, gint , GdkPoint );
gboolean mtx_curve_set_color (MtxCurve *, ColorIndex , GdkColor );
gboolean mtx_curve_get_color (MtxCurve *, ColorIndex , GdkColor *);
void mtx_curve_set_update_policy (MtxCurve *, GtkUpdateType );
GtkUpdateType mtx_curve_get_update_policy (MtxCurve *);


G_END_DECLS

#endif
