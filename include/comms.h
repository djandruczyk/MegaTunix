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

#ifndef __COMMS_H__
#define __COMMS_H__

#include <gtk/gtk.h>
#include <enums.h>
#include <threads.h>

/* Prototypes */
gint comms_test(void);			/* new check_ecu_comms function */
void update_comms_status(void);
void update_write_status(void *);
void update_write_status(void *);	/* gui updater for write status */
void readfrom_ecu(Io_Message *);	/* Function to get data FROM ecu */
void writeto_ecu(Io_Message *);		/* Func to send data to the ECU */
void queue_burn_ecu_flash(gint);	/* run after burn completion */
void write_data(Io_Message *);
/* Prototypes */

#endif
