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
void run_post_function_with_arg(gchar *, GtkWidget *);
void run_post_function(gchar * );
/* Prototypes */

#endif
