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
void build_output_message(Io_Message *, Command *, gpointer);
void chunk_write(gpointer, gint, guint8 *);
unsigned long crc32_computebuf(unsigned long, const void *, size_t);
gint comms_test(void);
void ecu_chunk_write(gint, gint, gint, gint, guint8 *);
void ms_chunk_write(gint, gint, gint, gint, guint8 *);
void ms_handle_page_change(gint , gint );
void ms_send_to_ecu(gint, gint, gint, DataSize, gint, gboolean);
void ms_table_write(gint, gint, guint8 *);
void send_to_ecu(gpointer, gint, gboolean);
void send_to_slaves(void *);
void *serial_repair_thread(gpointer);
gboolean setup_rtv(void);
void signal_read_rtvars(void);
void slaves_set_color(GuiColor,const gchar *);
gboolean teardown_rtv(void);
void queue_burn_ecu_flash(gint);
void queue_ms1_page_change(gint);
void update_write_status(void *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
