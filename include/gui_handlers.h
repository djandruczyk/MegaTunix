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

#ifndef __GUI_HANDLERS_H__
#define __GUI_HANDLERS_H__

#include <gtk/gtk.h>

/* Prototypes */
void leave(GtkWidget *, gpointer);
int std_button_handler(GtkWidget *, gpointer);
int toggle_button_handler(GtkWidget *, gpointer);
int bitmask_button_handler(GtkWidget *, gpointer);
int spinner_changed(GtkWidget *, gpointer);
int classed_spinner_changed(GtkWidget *, gpointer);
void check_req_fuel_limits(void);
void check_config11(gint);
void check_config13(gint);
/* Prototypes */

#endif
