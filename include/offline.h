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

#ifndef __OFFLINE_H__
#define __OFFLINE_H__

#include <gtk/gtk.h>


/* Prototypes */
void set_offline_mode(void);
gchar * present_firmware_choices(void);
gint ptr_sort(gconstpointer , gconstpointer );
gboolean offline_ecu_restore(GtkWidget *, gpointer );
/* Prototypes */

#endif
