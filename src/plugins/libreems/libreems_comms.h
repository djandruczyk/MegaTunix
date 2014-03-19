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
  \file src/plugins/freeems/freeems_comms.h
  \ingroup FreeEMSPlugin,Headers
  \brief FreeEMS Comms functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __FREEEMS_COMMS_H__
#define __FREEEMS_COMMS_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <enums.h>


/* Prototypes */
gboolean comms_test(void);
void ecu_chunk_write(gint, gint, gint, gint, guint8 *);
void freeems_chunk_write(gint, gint, gint, gint, guint8 *);
void freeems_send_to_ecu(gint, gint, gint, DataSize, gint, gboolean);
void freeems_serial_enable(void);
void freeems_serial_disable(void);
void post_single_burn_pf(void *);
void *rtv_subscriber(gpointer);
void *serial_repair_thread(gpointer);
void send_to_ecu(gpointer, gint, gboolean);
void setup_rtv_pf(void);
void signal_read_rtvars(void);
gboolean teardown_rtv(void);
void update_write_status(void *);
void *unix_reader(gpointer);
void *win32_reader(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
