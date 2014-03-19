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
  \file src/plugins/freeems/freeems_helpers.h
  \ingroup FreeEMSPlugin,Headers
  \brief FreeEMS comm.xml helpers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __FREEEMS_HELPERS_H__
#define __FREEEMS_HELPERS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>

typedef enum
{
	FREEEMS_ALL = 0x290,
	CALLBACK,
	EMPTY_PAYLOAD,
	GENERIC_READ,
	GENERIC_RAM_WRITE,
	GENERIC_FLASH_WRITE,
	GENERIC_BURN,
	BENCHTEST_RESPONSE,
	LAST_XML_FUNC_CALL_TYPE
}FuncCall;


/* Prototypes */
gboolean freeems_burn_all(void *, FuncCall);
void handle_transaction_hf(void *, FuncCall);
void hard_boot_ecu(void);
gboolean read_freeems_data(void *, FuncCall);
void reset_counters(void);
FreeEMS_Packet *retrieve_packet(gconstpointer *, const gchar *);
void soft_boot_ecu(void);
void start_streaming(void);
void stop_streaming(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
