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
void load_gui_tabs(void);
void bind_data(gpointer,gpointer,gpointer);
void populate_master(gpointer,gpointer,gpointer);
gchar ** parse_keys(gchar * , gint * );
void parse_keytypes(gchar * , gint *, gint * );
/* Prototypes */

#endif
