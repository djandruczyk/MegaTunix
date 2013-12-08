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
  \file include/logviewer_gui.h
  \ingroup Headers
  \brief Header for logviewer gui creation/management
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LOGVIEWER_GUI_H__
#define __LOGVIEWER_GUI_H__

#include <configfile.h>
#include <enums.h>
#include <logviewer_core.h>
#include <gtk/gtk.h>

typedef struct _Viewable_Value Viewable_Value;
/*!
 \brief _Viewable_Value is the datastructure bound 
 to every trace viewed in the logviewer. 
 */
struct _Viewable_Value
{
	GdkGC *font_gc;			/*!< GC used for the fonts */
	GdkGC *trace_gc;		/*!< GC used for the trace */
	PangoRectangle *log_rect;	/*!< Logcial rectangle around text */
	PangoRectangle *ink_rect;	/*!< Ink rectangle around text */
	gconstpointer *object;			/*!< object */
	gchar *vname;			/*!< Name of widget being logged */
	gint precision;			/*!< number of digits */
	gboolean force_update;		/*!< flag to force update on addition */
	gboolean highlight;		/*!< flag it highlight it.. */
	gint last_y;			/*!< Last point on screen of trace */
	gint last_index;		/*!< latest entryu into data array */
	gchar *data_source;		/*!< Textual name of source */
	gfloat min;			/*!< for auto-scaling */
	gfloat max;			/*!< for auto-scaling */
	gfloat lower;			/*!< hard limits to use for scaling */
	gfloat upper;			/*!< hard limits to use for scaling */
	gfloat cur_low;			/*!< User limits to use for scaling */
	gfloat cur_high;		/*!< User limits to use for scaling */
	GArray *data_array;		/*!< History of all values recorded */
	Log_Info *log_info;	/*!< important */
};
	

/* Prototypes */
Viewable_Value * build_v_value(gconstpointer * );
void draw_infotext(void);
void draw_valtext(gboolean);
void enable_playback_controls(gboolean );
void finish_logviewer(void);
GdkColor get_colors_from_hue(gfloat, gfloat, gfloat);
GdkGC * initialize_gc(GdkDrawable *, GcType );
gboolean logviewer_log_position_change(GtkWidget *, gpointer);
gboolean pb_update_logview_traces(gpointer);
gboolean pb_update_logview_traces_wrapper(gpointer);
void populate_viewer(void);
void present_viewer_choices(void);
void read_logviewer_defaults(ConfigFile *);
gboolean reenable_select_params_button(GtkWidget *);
void reset_logviewer_state(void);
gboolean save_default_choices(GtkWidget *);
void scroll_logviewer_traces(void);
gboolean set_all_lview_choices_state(GtkWidget *, gpointer);
void set_default_lview_choices_state(void);
void set_logviewer_mode(Lv_Mode);
gboolean slider_key_press_event(GtkWidget *, GdkEventKey *, gpointer);
void trace_update(gboolean );
gboolean update_logview_traces_pf(gboolean);
gboolean view_value_set(GtkWidget *, gpointer );
void write_logviewer_defaults(ConfigFile *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
