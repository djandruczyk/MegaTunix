/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __LOADER_COMMON_H__
#define __LOADER_COMMON_H__

#include <winserialio.h>
#include <gtk/gtk.h>

typedef enum
{
	MS1=0xcca,
	MS2,
	FREEEMS
}FirmwareType;


/* Prototypes */
gint open_port(gchar *);
gint setup_port(gint, gint);
void flush_serial(gint, FlushDirection);
gboolean get_ecu_signature(gint);
void close_port(gint);
void unlock_port(void);
gboolean lock_port(gchar *);
FirmwareType detect_firmware(gchar *);
void progress_update(gfloat);


#endif
