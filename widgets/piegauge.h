/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * 
 *
 * Megasquirt pie_gauge widget
 * Inspired by Phil Tobin's MegaLogViewer 
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef MTX_PIE_GAUGE_H
#define MTX_PIE_GAUGE_H

#include <config.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define MTX_TYPE_PIE_GAUGE		(mtx_pie_gauge_get_type ())
#define MTX_PIE_GAUGE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MTX_TYPE_PIE_GAUGE, MtxPieGauge))
#define MTX_PIE_GAUGE_CLASS(obj)		(G_TYPE_CHECK_CLASS_CAST ((obj), MTX_PIE_GAUGE, MtxPieGaugeClass))
#define MTX_IS_PIE_GAUGE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTX_TYPE_PIE_GAUGE))
#define MTX_IS_PIE_GAUGE_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MTX_TYPE_PIE_GAUGE))
#define MTX_PIE_GAUGE_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), MTX_TYPE_PIE_GAUGE, MtxPieGaugeClass))
/*#define MTX_PIE_GAUGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_PIE_GAUGE, MtxPieGaugePrivate))*/


typedef struct _MtxPieGauge		MtxPieGauge;
typedef struct _MtxPieGaugeClass	MtxPieGaugeClass;

/*! ColorIndex enum,  for indexing into the color arrays */
typedef enum  
{
	COL_BG = 0,
	COL_NEEDLE,
	COL_VALUE_FONT,
	COL_LOW,
	COL_MID,
	COL_HIGH,
	NUM_COLORS
}ColorIndex;

struct _MtxPieGauge
{	/* public data */
	GtkDrawingArea parent;
	GdkPixmap *pixmap;	/*! Update/backing pixmap */
	GdkPixmap *bg_pixmap;	/*! Static rarely changing pixmap */
	gfloat min;		/*! Minimum Value */
	gfloat max;		/* MAximum Value */
	gfloat value;		/* Current value */
	gint w;			/* Width of full widget */
	gint h;			/* Height of full widget */
	gint pie_xc;		/* Pie arc x center */
	gint pie_yc;		/* Pie arc y center */
	gint pie_radius;	/*! Pie Radius */
	gint precision;		/* Text precision */
	gint start_angle;	/* Start angle */
	gint sweep_angle;	/* Sweep angle */
	gint value_xpos;	/* Text X position offset */
	gint value_ypos;	/* Text X position offset */
	gchar *value_font;	/* Font string for value */
	gfloat value_font_scale;/* Font scale */
	gchar *valname;		/* Value text to the let of the number */
#ifdef HAVE_CAIRO
	cairo_t *cr;		/*! Cairo context,  not sure if this is good
				   too hold onto or not */
	cairo_font_options_t * font_options;
#endif
	PangoLayout *layout;	/*! Pango TextLayout object */
	PangoFontDescription *font_desc;/*! Pango Font description */
	GdkGC * bm_gc;		/*! Graphics Context for bitmap */
	GdkGC * gc;		/*! Graphics Context for drawing */
	GdkColormap *colormap;	/*! Colormap for GC's */
	gboolean antialias;	/*! AA Flag (used in Cairo ONLY */
	GdkColor colors[NUM_COLORS];
};

struct _MtxPieGaugeClass
{
	GtkDrawingAreaClass parent_class;
};

GType mtx_pie_gauge_get_type (void) G_GNUC_CONST;

GtkWidget* mtx_pie_gauge_new ();
void mtx_pie_gauge_set_value (MtxPieGauge *gauge, gfloat value);
float mtx_pie_gauge_get_value (MtxPieGauge *gauge);

G_END_DECLS

#endif
