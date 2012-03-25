/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * 
 *
 * MegaTunix progress_bar widget
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
  \file widgets/progress.h
  \ingroup WidgetHeaders,Headers
  \brief Public header for the MtxProgress subclass of GtkProgress widget
  \author David Andruczyk
  */

#ifndef MTX_PROGRESS_BAR_H
#define MTX_PROGRESS_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

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

/*! ProgressColorIndex enum,  for indexing into the color arrays */
typedef enum  
{
	PROGRESS_COL_BG = 0,
	PROGRESS_COL_BAR,
	PROGRESS_COL_PEAK,
	PROGRESS_NUM_COLORS
}ProgressColorIndex;


/*!
  \brief MtxProgressBar structure
  */
struct _MtxProgressBar
{	/* public data */
	GtkProgressBar parent;		/*!< Parent Widget */
};

/*!
  \brief MtxProgressBarClass structure
  */
struct _MtxProgressBarClass
{
	GtkProgressBarClass parent_class;/*!< Parent Class */
};

GType mtx_progress_bar_get_type (void) G_GNUC_CONST;

GtkWidget* mtx_progress_bar_new ();
void mtx_progress_bar_set_fraction(MtxProgressBar *, gfloat);
gfloat mtx_progress_bar_get_fraction(MtxProgressBar *);
void mtx_progress_bar_set_hold_time(MtxProgressBar *, gint);
gint mtx_progress_bar_get_hold_time(MtxProgressBar *);


G_END_DECLS
#ifdef __cplusplus
}
#endif

#endif
