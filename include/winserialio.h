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
  \file include/winserialio.h
  \ingroup Headers
  \brief Header for the windows serial I/O init functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WINSERIALIO_H__
#define __WINSERIALIO_H__

#include <gtk/gtk.h>
#include <serialio.h>


/* Prototypes */
void win32_flush_serial(int, FlushDirection);
void win32_setup_serial_params(gint, gint, gint, Parity, gint);
void win32_toggle_serial_control_lines(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
