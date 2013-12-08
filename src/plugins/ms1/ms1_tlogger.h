/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/plugins/ms1/ms1_tlogger.h
  \ingroup MS1Plugin,Headers
  \brief MS1 tooth/trigger logger code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MS1_T_LOGGER_H__
#define __MS1_T_LOGGER_H__

#include <gtk/gtk.h>
#include <defines.h>

typedef enum {
	TOOTHMON_TICKLER=0x91,
	TRIGMON_TICKLER
}EcuPluginTickler;

typedef struct _TTMon_Data TTMon_Data;
/*!
 * \brief _TTMon_Data struct is a container used to hold private data
 * for the Trigger and Tooth Loggers (MSnS-E only)
 */
struct _TTMon_Data
{
	gboolean stop;		/*!< Stop display */
	gfloat zoom;		/*!< Zoom */
	gint page;		/*!< page used to discern them apart */
	GdkPixmap *pixmap;	/*!< Pixmap */
	GtkWidget *darea;	/*!< Pointer to drawing area */
	gint min_time;		/*!< Minimum, trigger/tooth time */
	gint num_maxes;		/*!< Hot many long pips per block */
	gint mins_inbetween;	/*!< How many normal teeth */
	gint max_time;		/*!< Maximum, trigger/tooth time */
	gint midpoint_time;	/*!< avg between min and max */
	gint est_teeth;		/*!< Estimated number of teeth */
	gint units;		/*!< Units multiplier */
	gint missing;		/*!< Number of missing teeth */
	gint sample_time;	/*!< Time delay between reads.. */
	gint capabilities;	/*!< Enum of ECU capabilities */
	gfloat usable_begin;	/*!< Usable begin point for bars */
	gfloat font_height;	/*!< Font height needed for some calcs */
	gfloat rpm;		/*!< Current RPM */
	gushort *current;	/*!< Current block of times */
	gushort *last;		/*!< Last block of times */
	gushort *captures;	/*!< Array of capture points */
	gint wrap_pt;		/*!< Wrap point */
	gint vdivisor;		/*!< Vertical scaling divisor */
	gfloat peak;		/*!< Vertical Peak Value */
	PangoFontDescription *font_desc;	/*!< Pango Font Descr */
	PangoLayout *layout;	/*!< Pango Layout */
};

/* Prototypes */
void bind_ttm_to_page(gint page);
void crunch_trigtooth_data(void);
void crunch_trigtooth_data_pf(void);
gboolean logger_display_config_event(GtkWidget *, GdkEventConfigure *, gpointer);
gboolean logger_display_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean ms1_tlogger_button_handler(GtkWidget *, gpointer);
void reset_ttm_buttons(void);
void setup_logger_display(GtkWidget *);
gboolean signal_toothtrig_read(EcuPluginTickler);
void start(EcuPluginTickler);
void stop(EcuPluginTickler);
gboolean update_trigtooth_display(gpointer);
void update_trigtooth_display_pf(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
