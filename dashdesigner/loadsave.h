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

#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>

/* Prototypes */
EXPORT gboolean load_handler(GtkWidget *, gpointer );
EXPORT gboolean save_handler(GtkWidget *, gpointer );
EXPORT gboolean save_as_handler(GtkWidget *, gpointer );
void setup_file_filters(GtkFileChooser *);
gboolean check_datasources_set(GtkWidget *);
/* Prototypes */

#endif
