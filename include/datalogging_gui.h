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


/* Prototypes */
void build_datalogging(GtkWidget *);
void start_datalogging(void);
void stop_datalogging(void);
void clear_logables(void);
int log_value_set(GtkWidget *, gpointer);
void write_log_header(void *);
void run_datalog(void);
/* Prototypes */

#endif
