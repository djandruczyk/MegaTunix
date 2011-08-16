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

/*!
  \file src/plugins/freeems/freeems_helpers.h
  \ingroup FreeEMSPlugin,Headers
  \brief FreeEMS comm.xml helpers
  \author David Andruczyk
  */

#ifndef __FREEEMS_HELPERS_H__
#define __FREEEMS_HELPERS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>

typedef enum
{
	FREEEMS_ALL = 0x290,
	GENERIC_READ,
	GENERIC_RAM_WRITE,
	GENERIC_FLASH_WRITE,
	GENERIC_BURN,
	LAST_XML_FUNC_CALL_TYPE
}FuncCall;


/* Prototypes */
void stop_streaming(void);
void start_streaming(void);
void soft_boot_ecu(void);
void hard_boot_ecu(void);
gboolean freeems_burn_all(void *, FuncCall);
gboolean read_freeems_data(void *, FuncCall);
void handle_transaction(void *, FuncCall);
FreeEMS_Packet * retrieve_packet(gconstpointer *, const gchar *);


/* Prototypes */

#endif
