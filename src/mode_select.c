/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <mode_select.h>
#include <serialio.h>
#include <structures.h>
#include <threads.h>

gchar *states[] = {"FALSE","TRUE"};


void set_widget_sensitive(gpointer widget, gpointer state)
{
        gtk_widget_set_sensitive(GTK_WIDGET(widget),(gboolean)state);
}


void set_widget_active(gpointer widget, gpointer state)
{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gboolean)state);
}


gboolean drain_hashtable(gpointer offset, gpointer value, gpointer page)
{
	/* called per element from the hash table to drain and send to ECU */
	write_ve_const((gint)page, (gint)offset,(gint)value, FALSE);
	return TRUE;
}
