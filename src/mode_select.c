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

void parse_ecu_capabilities(gint ecu_caps)
{
	set_raw_memory_mode((ecu_caps & RAW_MEMORY) == 0 ? FALSE:TRUE);
}

void set_raw_memory_mode(gboolean state)
{
        extern GList *raw_mem_controls;
	dbg_func(g_strdup_printf(__FILE__": set_raw_memory_mode()\n\tSetting RAW Memory controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(raw_mem_controls, set_widget_state,(gpointer)state);
}

void set_widget_state(gpointer widget, gpointer state)
{
        gtk_widget_set_sensitive(GTK_WIDGET(widget),(gboolean)state);
}

gboolean drain_hashtable(gpointer offset, gpointer value, gpointer data)
{
	/* called per element from the hash table to drain and send to ECU */
	write_ve_const((gint)data, (gint)offset,(gint)value, FALSE);
	return TRUE;
}
