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

#ifndef __DATALOGGING_GUI_H__
#define __DATALOGGING_GUI_H__

#include <gtk/gtk.h>
#include <structures.h>


/* Prototypes */
void populate_dlog_choices(void);
void start_datalogging(void);
void stop_datalogging(void);
void clear_logables(void);
gboolean log_value_set(GtkWidget *, gpointer);
void write_log_header(struct Io_File *);
void run_datalog(void);
gboolean set_logging_mode(GtkWidget * , gpointer);
/* Prototypes */

#endif
