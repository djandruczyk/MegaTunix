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

/* MegaSquirt Linux tuning lookup tables for converting incoming values
 * to real world numbers. From Bruce Bowlings PCC tuning wsoftware.
 */

#ifndef __LOOKUPTABLES_H__
#define __LOOKUPTABLES_H__

#include <enums.h>
#include <structures.h>

gboolean load_table(gchar *, gchar *);
void get_table(gpointer, gpointer, gpointer );
gint reverse_lookup(GObject *, gint );
gint direct_reverse_lookup(gchar *, gint );
gfloat lookup_data(GObject *, gint );
gfloat direct_lookup_data(gchar *, gint );
gboolean lookuptables_configurator(GtkWidget *, gpointer );
gboolean lookuptables_configurator_hide(GtkWidget *, gpointer );
gboolean lookuptable_change(GtkCellRenderer *, gchar  *, gchar  *, gpointer );
void update_lt_config(gpointer , gpointer , gpointer );

#endif
