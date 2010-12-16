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

#ifndef __FREEMS_COMMS_H__
#define __FEEEMS_COMMS_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <configfile.h>
#include <enums.h>
#include <threads.h>


/* Prototypes */
gboolean data_to_be_read(GIOChannel *, GIOCondition, gpointer);
gboolean data_can_be_written(GIOChannel *, GIOCondition, gpointer);
void freeems_serial_setup(void);
/* Prototypes */

#endif
