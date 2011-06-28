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

#ifndef __PIEGAUGE_PRIVATE_H__
#define __PIEGAUGE_PRIVATE_H__

#include <gtk/gtk.h>
#include <piegauge.h>

#define MTX_PIE_GAUGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_PIE_GAUGE, MtxPieGaugePrivate))

typedef struct _MtxPieGaugePrivate      MtxPieGaugePrivate;

/*!
  \struct _MtxPieGaugePrivate
  \brief Private variables for the PieGauge Widget
  */
struct _MtxPieGaugePrivate
{
        GdkPixmap *pixmap;      /*! Update/backing pixmap */
        GdkPixmap *bg_pixmap;   /*! Static rarely changing pixmap */
        gfloat min;             /*! Minimum Value */
        gfloat max;             /* MAximum Value */
        gfloat value;           /* Current value */
        gint w;                 /* Width of full widget */
        gint h;                 /* Height of full widget */
        gint pie_xc;            /* Pie arc x center */
        gint pie_yc;            /* Pie arc y center */
        gint pie_radius;        /*! Pie Radius */
        gint precision;         /* Text precision */
        gint start_angle;       /* Start angle */
        gint sweep_angle;       /* Sweep angle */
        gchar *value_font;      /* Font string for value */
        gfloat value_font_scale;/* Font scale */
        gchar *valname;         /* Value text to the let of the number */
        cairo_t *cr;            /*! Cairo context,  not sure if this is good
                                   too hold onto or not */
        cairo_font_options_t * font_options;
        GdkColor colors[NUM_COLORS];
};


gboolean mtx_pie_gauge_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_pie_gauge_expose (GtkWidget *, GdkEventExpose *);
/* Not needed yet
* gboolean mtx_pie_gauge_button_press (GtkWidget *,GdkEventButton *);
* gboolean mtx_pie_gauge_motion_event (GtkWidget *,GdkEventMotion *);
*/
void mtx_pie_gauge_size_request (GtkWidget *, GtkRequisition *);
void mtx_pie_gauge_class_init (MtxPieGaugeClass *class_name);
void mtx_pie_gauge_init (MtxPieGauge *gauge);
gboolean mtx_pie_gauge_button_release (GtkWidget *,GdkEventButton *);
void generate_pie_gauge_background(MtxPieGauge *);
void update_pie_gauge_position (MtxPieGauge *);
void mtx_pie_gauge_init_colors(MtxPieGauge *);
void mtx_pie_gauge_redraw (MtxPieGauge *gauge);
void mtx_pie_gauge_finalize (GObject *gauge);



#endif
