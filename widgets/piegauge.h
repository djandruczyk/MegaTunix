/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * 
 *
 * MegaTunix pie_gauge widget
 * Inspired by Phil Tobin's MegaLogViewer 
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file widgets/piegauge.h
  \ingroup WidgetHeaders,Headers
  \brief Public header for the MtxPieGauge widget
  \author David Andruczyk
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


typedef struct _MtxPieGauge		MtxPieGauge;
typedef struct _MtxPieGaugeClass	MtxPieGaugeClass;

/*! 
  \brief ColorIndex enum, for indexing into the color arrays 
 */
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


/*!
  \brief _MtxPieGauge structure
  */
struct _MtxPieGauge
{	/* public data */
	GtkDrawingArea parent;		/*!< Parent Widget */
};

/*!
  \brief _MtxPieGaugeClass structure
  */
struct _MtxPieGaugeClass
{
	GtkDrawingAreaClass parent_class;	/*!< Parent Class */
};

GType mtx_pie_gauge_get_type (void) G_GNUC_CONST;

GtkWidget* mtx_pie_gauge_new ();
void mtx_pie_gauge_set_value (MtxPieGauge *gauge, gfloat value);
float mtx_pie_gauge_get_value (MtxPieGauge *gauge);

G_END_DECLS

#endif
