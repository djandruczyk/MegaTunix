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

#ifndef __REQ_FUEL_H__
#define __REQ_FUEL_H__

#include <gtk/gtk.h>

/* Prototypes */
int reqd_fuel_popup();
int update_reqd_fuel(GtkWidget *, gpointer);
int close_popup(GtkWidget *, gpointer);
void req_fuel_change(void *);
/* Prototypes */

#endif
