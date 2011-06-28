/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * MegaTunix stripchart widget
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

#ifndef __STRIPCHART_PRIVATE_H__
#define __STRIPCHART_PRIVATE_H__

#include <gtk/gtk.h>
#include <stripchart.h>

#define MTX_STRIPCHART_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_STRIPCHART, MtxStripChartPrivate))

typedef struct _MtxStripChartPrivate      MtxStripChartPrivate;
typedef struct _MtxStripChartTrace      MtxStripChartTrace;

/*!
  \brief _MtxStripChartTrace holds detailed information about each trace in
  the stripchart widget
  */
struct _MtxStripChartTrace
{
	gfloat min;		/*! minimum clamp valued */
	gfloat max;		/*! maximum clamped value */
	gint precision;		/*! numeric precision */
	gint id;		/*! numeric ID */
	gfloat lwidth;		/*! linewidth */
       	gchar * name;		/*! textual name onscreen */
	GdkColor color;		/*! Trace color */
	GArray *history;	/*! Previous values */
	gboolean show_val;	/*! Show the value or not */
};

/*!
  \brief _MtxStripChartPrivate holds the private information for the 
  MtxStripChart widget
  */
struct _MtxStripChartPrivate
{
        GdkPixmap *bg_pixmap;   /*! Update/backing pixmap */
        GdkPixmap *trace_pixmap;/*! Static part of traces */
	GdkPixmap *grat_pixmap;	/*! Graticule pixmap */
        gint w;                 /*! Width of full widget */
        gint h;                 /*! Height of full widget */
	gfloat mouse_x;		/*! motion event X coord */
	gfloat mouse_y;		/*! motion event X coord */
	gint num_traces;	/*! Number of active traces */
	gboolean mouse_tracking;/*! If true, render tracer line at mouse pos */
	gboolean update_pending;/*! If true, don't schedule another */
	GArray *traces;		/*! Array of trace specific data */
        gchar *font;		/*! Font string for value */
        cairo_font_options_t * font_options;
        GdkColor colors[NUM_COLORS];
        GdkColor tcolors[6];	/*! Trace colors */
	GtkJustification justification;	/*! Where to put the trace names */
};


gboolean mtx_stripchart_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_stripchart_expose (GtkWidget *, GdkEventExpose *);
/* Not needed yet
* gboolean mtx_stripchart_button_press (GtkWidget *,GdkEventButton *);
*/
gboolean mtx_stripchart_enter_leave_event(GtkWidget *, GdkEventCrossing *);
gboolean mtx_stripchart_motion_event (GtkWidget *,GdkEventMotion *);
void mtx_stripchart_size_request (GtkWidget *, GtkRequisition *);
void mtx_stripchart_class_init (MtxStripChartClass *class_name);
void mtx_stripchart_init (MtxStripChart *gauge);
gboolean mtx_stripchart_button_release (GtkWidget *,GdkEventButton *);
void generate_stripchart_static_traces(MtxStripChart *);
void update_stripchart_position (MtxStripChart *);
void render_marker(MtxStripChart *);
void mtx_stripchart_init_colors(MtxStripChart *);
void mtx_stripchart_redraw (MtxStripChart *gauge);
void mtx_stripchart_finalize (GObject *);
void mtx_stripchart_cleanup_traces (GArray *);




#endif
