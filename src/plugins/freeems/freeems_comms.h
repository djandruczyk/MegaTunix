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
#include <enums.h>


/* Prototypes */
gboolean able_to_read(GIOChannel *, GIOCondition, gpointer);
gboolean able_to_write(GIOChannel *, GIOCondition, gpointer);
gboolean serial_error(GIOChannel *, GIOCondition, gpointer);
void freeems_serial_enable(void);
gboolean comms_test(void);
void *serial_repair_thread(gpointer);
void *serial_reader(gpointer);
/* Prototypes */

#endif
