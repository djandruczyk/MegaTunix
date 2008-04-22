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

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
EXPORT gboolean select_file_for_ecu_backup(GtkWidget *, gpointer );
EXPORT gboolean select_file_for_ecu_restore(GtkWidget *, gpointer );
void backup_all_ecu_settings(gchar  *);
void restore_all_ecu_settings(gchar  *);
/* Prototypes */

#endif
