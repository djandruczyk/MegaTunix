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

#ifndef __LOGVIEWER_CORE_H__
#define __LOGVIEWER_CORE_H__

#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
EXPORT gboolean select_datalog_for_import(GtkWidget *, gpointer );
EXPORT gboolean logviewer_scroll_speed_change(GtkWidget *, gpointer );
void load_logviewer_file(GIOChannel * );
void read_log_header(GIOChannel *, Log_Info * );
void read_log_data(GIOChannel *, Log_Info * );
Log_Info * initialize_log_info(void);
void allocate_buffers(Log_Info *);
void populate_limits(Log_Info *);
void free_log_info(void);

/* Prototypes */

#endif
