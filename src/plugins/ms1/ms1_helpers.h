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

#ifndef __MS1_HELPERS_H__
#define __MS1_HELPERS_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Externs */
extern GtkWidget *(*lookup_widget_f)(const gchar *);
/* Prototypes */
void enable_reboot_button_pf(void);
/* Prototypes */

#endif
