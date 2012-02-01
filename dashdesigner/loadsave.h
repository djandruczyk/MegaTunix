/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Electronic Fuel Injection tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean load_handler(GtkWidget *, gpointer );
gboolean save_handler(GtkWidget *, gpointer );
gboolean save_as_handler(GtkWidget *, gpointer );
void setup_file_filters(GtkFileChooser *);
gboolean check_datasources_set(GtkWidget *);
void prompt_to_save(void);
/* Prototypes */

#endif
