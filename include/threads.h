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

#ifndef __THREADS_H__
#define __THREADS_H__

#include <gtk/gtk.h>
#include <enums.h>

/* Prototypes */
void dealloc_message(void * );		/* Deallocate memory for message struct */
void io_cmd(IoCommands, gpointer);	/* Send message down the queue */
void *serial_io_handler(gpointer);	/* thread that processes messages */
void comms_test(void);			/* new check_ecu_comms function */
void readfrom_ecu(void *);		/* Function to get data FROM ecu */
void writeto_ecu(void *);		/* Func to send data to the ECU */
void write_ve_const(gint, gint, gint, gboolean);
void burn_ms_flash(void);		/* run after burn completion */
/* Prototypes */

#endif
