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

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>

/* Prototypes */
EXPORT gboolean legacy_tick_attributes_handler(GtkWidget *, gpointer );
EXPORT gboolean text_attributes_handler(GtkWidget *, gpointer );
EXPORT gboolean tick_groups_handler(GtkWidget *, gpointer );
EXPORT gboolean general_attributes_handler(GtkWidget *, gpointer );
EXPORT gboolean warning_ranges_handler(GtkWidget *, gpointer );
EXPORT gboolean about_handler(GtkWidget *, gpointer );
void update_tickmark_controls();
void update_text_controls();
void update_general_controls();
void reset_tickmark_controls();
void reset_text_controls();
void reset_tick_group_controls();
void reset_general_controls();
/* Prototypes */

#endif
