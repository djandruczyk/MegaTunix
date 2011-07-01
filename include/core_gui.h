/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __CORE_GUI_H__
#define __CORE_GUI_H__

#include <gtk/gtk.h>
#include <glade/glade.h>

/* Prototypes */
gint setup_gui(void);
void finalize_core_gui(GladeXML *);
void setup_main_status(GtkWidget *);
void set_connected_icons_state(gboolean);

/* Prototypes */

#endif
