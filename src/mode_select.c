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
#include <mode_select.h>
#include <serialio.h>

extern gboolean fahrenheit;

void set_dt_table_mapping_state(gboolean state)
{
        extern GList *table_map_widgets;
        g_list_foreach(table_map_widgets, set_widget_state,(gpointer)state);
}

void set_ignition_mode(gboolean state)
{
	extern gboolean ign_variant;
        extern GList *ign_widgets;
        extern GList *inv_ign_widgets;
        ign_variant = state;
        g_list_foreach(ign_widgets, set_widget_state,(gpointer)state);
        g_list_foreach(inv_ign_widgets, set_widget_state,(gpointer)(!state));
        reset_temps(GINT_TO_POINTER(fahrenheit));
}

void set_iac_mode(gboolean state)
{
        extern GList *enh_idle_widgets;
	extern gboolean using_pwm_idle;
        using_pwm_idle = state;
        g_list_foreach(enh_idle_widgets, set_widget_state,(gpointer)state);
}

void set_dualtable_mode(gboolean state)
{
        extern GList *dt_widgets;
        extern GList *inv_dt_widgets;
	extern gboolean dualtable;
        dualtable = state;

        g_list_foreach(dt_widgets, set_widget_state,(gpointer)state);
        g_list_foreach(inv_dt_widgets, set_widget_state,(gpointer)(!state));

        /* fahrenheit is a FLAG... */
        reset_temps(GINT_TO_POINTER(fahrenheit));
}

void set_widget_state(gpointer widget, gpointer state)
{
        gtk_widget_set_sensitive(GTK_WIDGET(widget),(gboolean)state);
}

gboolean drain_hashtable(gpointer offset, gpointer value, gpointer data)
{
        /* called per element from the hash table to drain and send to ECU */
        write_ve_const((gint)value,(gint)offset);
        return TRUE;
}

