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
  \file include/logviewer_core.h
  \ingroup Headers
  \brief Header for core logviewer parsing/loading functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LOGVIEWER_CORE_H__
#define __LOGVIEWER_CORE_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <watches.h>


/* Type definitions */
typedef struct _Log_Info Log_Info;
typedef struct _Logview_Data Logview_Data;


/*!
 \brief The _Logview_Data struct is a ontainer used within the logviewer_gui.c
 file used to store settings specific to the logviewer including th pointer to
 the drawing area, and a hashtable and list of pointers to the trace 
 datastructures.
 */
struct _Logview_Data
{
	GdkGC *highlight_gc;	/*!< GC used for the highlight */
	GtkWidget *darea;	/*!< Trace drawing area... */
	GdkPixmap *pixmap;	/*!< pointer to backing pixmap... */
	GdkPixmap *pmap;	/*!< pointer to Win32 pixmap hack!!! */
	GHashTable *traces;	/*!< Hash table of v_values key'd by name */
	GList *tlist;		/*!< Doubly linked lists of v_Values*/
	GList *used_colors;	/*!< List of colors in use.... */
	gint active_traces;	/*!< how many are active */
	gint spread;		/*!< Pixel spread between trace info blocks */
	guint tselect;		/*!< Trace that is currently selected */
	PangoFontDescription *font_desc; /*!< Font used for text... */
	gint info_width;	/*!< Width of left info area */
};


/*!
 \brief The _Log_Info datastructure is populated when a datalog file is opened
 for viewing in the Datalog viewer.
 */
struct _Log_Info
{
	guint field_count;	/*!< How many fields in the logfile */
	gchar *delimiter;	/*!< delimiter between fields for this logfile */
	gchar *signature;	/*!< ECU signature of log */
	GPtrArray *log_list;	/*!< List of objects */
};


/* Prototypes */
void allocate_buffers(Log_Info *);
void create_stripchart(GtkWidget *);
void free_log_info(Log_Info *);
Log_Info * initialize_log_info(void);
void load_logviewer_file(GIOChannel * );
gboolean logviewer_scroll_speed_change(GtkWidget *, gpointer );
void populate_limits(Log_Info *);
void read_log_data(GIOChannel *, Log_Info * );
void read_log_header(GIOChannel *, Log_Info * );
gboolean select_datalog_for_import(GtkWidget *, gpointer );
void update_stripchart_data(RtvWatch *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
