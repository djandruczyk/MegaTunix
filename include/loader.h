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

#ifndef __LOADER_H__
#define __LOADER_H__

#include <enums.h>
#include <gtk/gtk.h>

typedef enum
{
	NOT_LISTENING=0xcba,
	IN_BOOTLOADER,
	LIVE_MODE
}EcuState;

/* Prototypes */
gint setup_port(gchar * );
void flush_serial(gint, FlushDirection);
EcuState detect_ecu(gint);
gboolean jump_to_bootloader(gint);
gboolean prepare_for_upload(gint);
void upload_firmware(gint, gint);
void reboot_ecu(gint);
void close_port(gint);
void output(gchar *);


#endif
