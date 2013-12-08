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
  \file include/binlogger.h
  \ingroup Headers
  \brief Header for binary Logging debug functionality
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BINLOGGER_H__
#define __BINLOGGER_H__

#include <gtk/gtk.h>

/* Prototypes */
void close_binary_logs(void);
gboolean flush_binary_logs(gpointer);
void log_inbound_data(const void *, size_t);
void log_outbound_data(const void *, size_t);
void open_binary_logs(void);

/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
