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
	set_dualtable_mode((ecu_caps & DUALTABLE) == 0 ? FALSE:TRUE);
	set_enhanced_mode((ecu_caps & ENHANCED) == 0 ? FALSE:TRUE);
	set_iac_mode((ecu_caps & (IAC_PWM|IAC_STEPPER)) == 0 ? FALSE:TRUE);
	set_launch_ctrl_mode((ecu_caps & LAUNCH_CTRL) == 0 ? FALSE:TRUE);
	set_raw_memory_mode((ecu_caps & RAW_MEMORY) == 0 ? FALSE:TRUE);
}

void set_raw_memory_mode(gboolean state)
{
        extern GList *raw_mem_controls;
	dbg_func(g_strdup_printf(__FILE__": set_raw_memory_mode()\n\tSetting RAW Memory controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(raw_mem_controls, set_widget_state,(gpointer)state);
}

void set_enhanced_mode(gboolean state)
{
        extern GList *enhanced_controls;
	dbg_func(g_strdup_printf(__FILE__": set_enhanced_mode()\n\tSetting Enhanced controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(enhanced_controls, set_widget_state,(gpointer)state);
}

void set_launch_ctrl_mode(gboolean state)
{
        extern GList *launch_controls;
	dbg_func(g_strdup_printf(__FILE__": set_launch_ctrl_mode()\n\tSetting Launch-Ctrl controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(launch_controls, set_widget_state,(gpointer)state);
}

void set_iac_mode(gboolean state)
{
        extern GList *iac_idle_controls;
        extern GList *enh_idle_controls;

	dbg_func(g_strdup_printf(__FILE__": set_iac_mode()\n\tSetting Idle-Ctrl controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(enh_idle_controls, set_widget_state,(gpointer)state);
        g_list_foreach(iac_idle_controls, set_widget_state,(gpointer)state);
}

void set_dualtable_mode(gboolean state)
{
        extern GList *dt_controls;
        extern GList *inv_dt_controls;

	dbg_func(g_strdup_printf(__FILE__": set_dualtable_mode()\n\tSetting Dual Table controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(dt_controls, set_widget_state,(gpointer)state);
        g_list_foreach(inv_dt_controls, set_widget_state,(gpointer)(!state));

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

