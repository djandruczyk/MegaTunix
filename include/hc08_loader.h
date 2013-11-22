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
  \file include/hc08_loader.h
  \ingroup Headers
  \brief Header for the Motorola HC08 series firmware loader
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HC08_LOADER_H__
#define __HC08_LOADER_H__

#include <winserialio.h>
#include <gtk/gtk.h>

typedef enum
{
	NOT_LISTENING=0xcba,
	IN_BOOTLOADER,
	LIVE_MODE
}EcuState;

/* Prototypes */
EcuState detect_ecu(gint);
gboolean do_ms1_load(gint, gint);
gboolean jump_to_bootloader(gint);
void output(gchar *, gboolean);
gboolean prepare_for_upload(gint);
void reboot_ecu(gint);
void upload_firmware(gint, gint);

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
