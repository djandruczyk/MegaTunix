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

#ifndef __THERM_H__
#define __THERM_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

/* Prototypes */
EXPORT gboolean clamp_value(GtkWidget *, gpointer);
EXPORT gboolean flip_table_gen_temp_label(GtkWidget *, gpointer);
EXPORT gboolean table_gen_process_and_dl(GtkWidget *, gpointer);

/* Prototypes */

#endif
