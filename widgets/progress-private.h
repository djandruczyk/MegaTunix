/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix pie gauge widget
 * 
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

#ifndef __PROGRESS_PRIVATE_H__
#define __PROGRESS_PRIVATE_H__

#include <gtk/gtk.h>
#include <progress.h>

#define MTX_PROGRESS_BAR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_PROGRESS_BAR, MtxProgressBarPrivate))

typedef struct _MtxProgressBarPrivate      MtxProgressBarPrivate;
typedef struct _MtxColor      MtxColor;


/*!
  \brief _MtxColor holds float representation of a RGB color
 */
struct _MtxColor
{
	gfloat red;		/*!< Red value 0-1.0 */
	gfloat green;		/*!< Green value 0-1.0 */
	gfloat blue;		/*!< Blue value 0-1.0 */
};

/*!
  \brief _MtxProgressBarPrivate holds all the private variables for 
  MtxProgressBar
 */
struct _MtxProgressBarPrivate
{
	gfloat peak;		/*!< Peak value */
	gint hold_id;		/*!< ID for hold timeout func */
	gint hold_time;		/*!< Hold time in ms */
        MtxColor colors[PROGRESS_NUM_COLORS];	/*!< Colors for the widget */
};

void mtx_progress_bar_init_colors(MtxProgressBar *);
gboolean mtx_progress_bar_expose (GtkWidget *widget, GdkEventExpose *event);
void mtx_progress_bar_real_update(GtkProgress *);
void mtx_progress_bar_paint (GtkProgress *);
void mtx_progress_bar_paint_continuous (GtkProgressBar *, gint, gint, GtkProgressBarOrientation);
gfloat mtx_progress_get_peak_percentage (GtkProgress *progress);
void mtx_progress_bar_finalize (GObject *);


#endif
