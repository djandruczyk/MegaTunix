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
  \file include/helpers.h
  \ingroup Headers
  \brief Headers for the global postfunctions common to all firmwares
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <defines.h>
#include <threads.h>
#include <gtk/gtk.h>

/* Prototypes */
void cleanup_pf(Io_Message *);
void conditional_start_rtv_tickler_pf(void);
void enable_3d_buttons_pf(void);
void enable_get_data_buttons_pf(void);
void disable_burner_buttons_pf(void);
void ready_msg_pf(void);
void reset_temps_pf(void);
void set_store_black_pf(void);
void start_statuscounts_pf(void);
void startup_default_timeouts_pf(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
