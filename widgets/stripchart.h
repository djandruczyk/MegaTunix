/*
 * Copyright (C) 2010 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix stripchart widget
 * Inspired by Phil Tobin's MegaLogViewer 
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef MTX_STRIPCHART_H
#define MTX_STRIPCHART_H

#include <config.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define MTX_TYPE_STRIPCHART		(mtx_stripchart_get_type ())
#define MTX_STRIPCHART(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MTX_TYPE_STRIPCHART, MtxStripChart))
#define MTX_STRIPCHART_CLASS(obj)		(G_TYPE_CHECK_CLASS_CAST ((obj), MTX_STRIPCHART, MtxStripChartClass))
#define MTX_IS_STRIPCHART(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTX_TYPE_STRIPCHART))
#define MTX_IS_STRIPCHART_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MTX_TYPE_STRIPCHART))
#define MTX_STRIPCHART_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), MTX_TYPE_STRIPCHART, MtxStripChartClass))


typedef struct _MtxStripChart		MtxStripChart;
typedef struct _MtxStripChartClass	MtxStripChartClass;

/*! ColorIndex enum,  for indexing into the color arrays */
typedef enum  
{
	COL_BG = 0,
	COL_GRAT,
	COL_TRACE1,
	COL_TRACE2,
	COL_TRACE3,
	COL_TRACE4,
	NUM_COLORS
}ColorIndex;


struct _MtxStripChart
{	/* public data */
	GtkDrawingArea parent;
};

struct _MtxStripChartClass
{
	GtkDrawingAreaClass parent_class;
};

GType mtx_stripchart_get_type (void) G_GNUC_CONST;

/* Initializing */
GtkWidget* mtx_stripchart_new ();

/* Params tweaking */
gint mtx_stripchart_add_trace(MtxStripChart *, gfloat min, gfloat max, GdkColor color);
gboolean mtx_stripchart_delete_trace(MtxStripChart *, gint index);

/* Set/Get latest values */
void mtx_stripchart_set_values (MtxStripChart *, gfloat *);
gboolean mtx_stripchart_get_values (MtxStripChart *, gfloat *);

G_END_DECLS

#endif
