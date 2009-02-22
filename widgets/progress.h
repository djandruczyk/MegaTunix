/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * 
 *
 * Megasquirt progress_bar widget
 * Inspired by Phil Tobin's MegaLogViewer 
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef MTX_PROGRESS_BAR_H
#define MTX_PROGRESS_BAR_H

#include <config.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define MTX_TYPE_PROGRESS_BAR		(mtx_progress_bar_get_type ())
#define MTX_PROGRESS_BAR(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MTX_TYPE_PROGRESS_BAR, MtxProgressBar))
#define MTX_PROGRESS_BAR_CLASS(obj)		(G_TYPE_CHECK_CLASS_CAST ((obj), MTX_PROGRESS_BAR, MtxProgressBarClass))
#define MTX_IS_PROGRESS_BAR(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTX_TYPE_PROGRESS_BAR))
#define MTX_IS_PROGRESS_BAR_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MTX_TYPE_PROGRESS_BAR))
#define MTX_PROGRESS_BAR_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), MTX_TYPE_PROGRESS_BAR, MtxProgressBarClass))


typedef struct _MtxProgressBar		MtxProgressBar;
typedef struct _MtxProgressBarClass	MtxProgressBarClass;

/*! ColorIndex enum,  for indexing into the color arrays */
typedef enum  
{
	COL_BG = 0,
	COL_BAR,
	COL_PEAK,
	NUM_COLORS
}ColorIndex;


struct _MtxProgressBar
{	/* public data */
	GtkProgressBar parent;
};

struct _MtxProgressBarClass
{
	GtkProgressBarClass parent_class;
};

GType mtx_progress_bar_get_type (void) G_GNUC_CONST;

GtkWidget* mtx_progress_bar_new ();
void mtx_progress_bar_set_fraction(MtxProgressBar *, gfloat);
gfloat mtx_progress_bar_get_fraction(MtxProgressBar *);


G_END_DECLS

#endif
