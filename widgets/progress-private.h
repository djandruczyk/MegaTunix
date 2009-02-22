/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt pie gauge widget
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


struct _MtxColor
{
	gfloat red;
	gfloat green;
	gfloat blue;
};

struct _MtxProgressBarPrivate
{
	gfloat peak;
	gint hold_id;
	gint hold_time;
        MtxColor colors[NUM_COLORS];
};

void mtx_progress_bar_init_colors(MtxProgressBar *);
gboolean mtx_progress_bar_expose (GtkWidget *widget, GdkEventExpose *event);
void mtx_progress_bar_real_update(GtkProgress *);
void mtx_progress_bar_paint (GtkProgress *);
void mtx_progress_bar_paint_continuous (GtkProgressBar *, gint, gint, GtkProgressBarOrientation);
gfloat mtx_progress_get_peak_percentage (GtkProgress *progress);

#endif
