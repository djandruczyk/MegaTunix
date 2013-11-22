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
  \file src/plugins/mscommon/mscommon_helpers.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon helpers for comm.xml
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MSCOMMON_HELPERS_H__
#define __MSCOMMON_HELPERS_H__

#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <threads.h>

typedef enum
{
	WRITE_VERIFY=0x290,
	MISMATCH_COUNT,
	MS1_CLOCK,
	MS2_CLOCK,
	NUM_REV,
	TEXT_REV,
	SIGNATURE,
	MS1_VECONST,
	MS2_VECONST,
	MS2_BOOTLOADER,
	MS1_RT_VARS,
	MS2_RT_VARS,
	MS1_GETERROR,
	MS1_E_TRIGMON,
	MS1_E_TOOTHMON,
	MS2_E_TRIGMON,
	MS2_E_TOOTHMON,
	MS2_E_COMPOSITEMON,
	LAST_XML_FUNC_CALL_TYPE
}FuncCall;

/* Prototypes */
gboolean burn_all_helper(void *, FuncCall);
void ecu_info_update(Firmware_Details *);
void enable_get_data_buttons_pf(void);
void post_burn_pf(void);
void post_single_burn_pf(void *data);
gboolean read_ve_const(void *, FuncCall);
void simple_read_hf(void *, FuncCall);
void spawn_read_all_pf(void);
void startup_tcpip_sockets_pf(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
