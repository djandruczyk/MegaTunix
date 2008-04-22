/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __LOGVIEWER_GUI_H__
#define __LOGVIEWER_GUI_H__

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
	GdkGC *font_gc;			/*! GC used for the fonts */
	GdkGC *trace_gc;		/*! GC used for the trace */
	PangoRectangle *log_rect;	/*! Logcial rectangle around text */
	PangoRectangle *ink_rect;	/*! Ink rectangle around text */
	GObject *object;		/*! object */
	gchar *vname;			/*! Name of widget being logged */
	gboolean is_float;		/*! TRUE or FALSE */
	gboolean force_update;		/*! flag to force update on addition */
	gboolean highlight;		/*! flag it highlight it.. */
	gint last_y;			/*! Last point on screen of trace */
	gint last_index;		/*! latest entryu into data array */
	gchar *data_source;		/*! Textual name of source */
	gfloat min;			/*! for auto-scaling */
	gfloat max;			/*! for auto-scaling */
	gfloat lower;			/*! hard limits to use for scaling */
	gfloat upper;			/*! hard limits to use for scaling */
	gfloat cur_low;			/*! User limits to use for scaling */
	gfloat cur_high;		/*! User limits to use for scaling */
	GArray *data_array;		/*! History of all values recorded */
	Log_Info *log_info;	/*! important */
};
	

/* Prototypes */
void present_viewer_choices(void);
void scroll_logviewer_traces(void);
void reset_logviewer_state(void);
gboolean view_value_set(GtkWidget *, gpointer );
gboolean set_lview_choices_state(GtkWidget *, gpointer);
gboolean slider_key_press_event(GtkWidget *, GdkEventKey *, gpointer);
Viewable_Value * build_v_value(GObject * );
GdkGC * initialize_gc(GdkDrawable *, GcType );
GdkColor get_colors_from_hue(gfloat, gfloat, gfloat);
void draw_infotext();
void draw_valtext(gboolean);
gboolean rt_update_logview_traces(gboolean);
gboolean pb_update_logview_traces(gboolean);
void trace_update(gboolean );
gboolean logviewer_log_position_change(GtkWidget *, gpointer);
void set_logviewer_mode(Lv_Mode);
void finish_logviewer(void);
void populate_viewer(void);
gboolean reenable_select_params_button(GtkWidget *);

/* Prototypes */

#endif
