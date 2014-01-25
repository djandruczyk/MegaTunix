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
  \file include/loader_common.h
  \ingroup Headers
  \brief Header for the common routines of the firmware loader tool(s)
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LOADER_COMMON_H__
#define __LOADER_COMMON_H__

#include <winserialio.h>
#include <gtk/gtk.h>

typedef enum
{
	MS1=0xcca,
	MS2,
	LIBREEMS
}FirmwareType;

/* Prototypes */
void close_port(gint);
FirmwareType detect_firmware(gchar *);
void flush_serial(gint, FlushDirection);
gboolean get_ecu_signature(gint);
gboolean lock_port(gchar *);
gint open_port(gchar *);
void progress_update(gfloat);
gint read_wrapper(gint, gchar *, gint);
gint setup_port(gint, gint);
void unlock_port(void);
gint write_wrapper(gint, guchar *, gint);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
