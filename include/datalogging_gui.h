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
  \file include/datalogging_gui.h
  \ingroup Headers
  \brief Handler for datalogging core management
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DATALOGGING_GUI_H__
#define __DATALOGGING_GUI_H__

#include <defines.h>
#include <gtk/gtk.h>


/* Prototypes */
gboolean autolog_dump(gpointer);
void clear_logables(void);
void dlog_deselect_all(void);
void dlog_select_all(void);
void dlog_select_defaults(void);
void dump_log_to_disk(GIOChannel *);
gboolean internal_datalog_dump(GtkWidget *, gpointer );
gboolean log_value_set(GtkWidget *, gpointer);
void populate_dlog_choices(void);
gboolean run_datalog(void);
gboolean select_datalog_for_export(GtkWidget *, gpointer );
gboolean set_logging_mode(GtkWidget * , gpointer);
void start_datalogging(void);
void stop_datalogging(void);
void write_log_header(GIOChannel *, gboolean);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
