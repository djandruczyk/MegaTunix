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

#ifndef __TABLOADER_H__
#define __TABLOADER_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean load_gui_tabs(void);
void bind_data(GtkWidget *, gpointer );
void populate_master(GtkWidget *, gpointer );
gchar ** parse_keys(gchar * , gint * );
gint * parse_keytypes(gchar * , gint * );
GList * get_list(gchar * );
void store_list(gchar * , GList * );

/* Prototypes */

#endif
