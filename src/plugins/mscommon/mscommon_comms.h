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
  \file src/plugins/mscommon/mscommon_comms.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon comms routines
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MSCOMMON_COMMS_H__
#define __MSCOMMON_COMMS_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>


/* Prototypes */
void queue_burn_ecu_flash(gint);
void queue_ms1_page_change(gint);
gint comms_test(void);
void ms_handle_page_change(gint , gint );
void ms_table_write(gint, gint, guint8 *);
void ms_send_to_ecu(gint, gint, gint, DataSize, gint, gboolean);
void send_to_ecu(gpointer, gint, gboolean);
void ms_chunk_write(gint, gint, gint, gint, guint8 *);
void ecu_chunk_write(gint, gint, gint, gint, guint8 *);
void chunk_write(gpointer, gint, guint8 *);
void send_to_slaves(void *);
void slaves_set_color(GuiColor,const gchar *);
void update_write_status(void *);
void *restore_update(gpointer);
void start_restore_monitor(void);
void *serial_repair_thread(gpointer);
void signal_read_rtvars(void);
void build_output_message(Io_Message *, Command *, gpointer);
gboolean setup_rtv(void);
gboolean teardown_rtv(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
