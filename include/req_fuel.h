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
gint reqd_fuel_popup(GtkWidget *);
gboolean save_reqd_fuel(GtkWidget *, gpointer);
gboolean close_popup(GtkWidget *);
void req_fuel_change(GtkWidget *);
void check_req_fuel_limits(void);
void initialize_reqd_fuel(void *, gint );
/* Prototypes */

#endif
