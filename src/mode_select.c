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

gchar *states[] = {"FALSE","TRUE"};

void parse_ecu_capabilities(unsigned int ecu_caps)
{
	extern struct DynamicButtons buttons;
	set_dualtable_mode((ecu_caps & DUALTABLE) == 0 ? FALSE:TRUE);
	set_enhanced_mode((ecu_caps & ENHANCED) == 0 ? FALSE:TRUE);
	set_iac_mode((ecu_caps & (IAC_PWM|IAC_STEPPER)) == 0 ? FALSE:TRUE);
	set_ignition_mode((ecu_caps & (S_N_SPARK|S_N_EDIS)) == 0 ? FALSE:TRUE);
	set_launch_ctrl_mode((ecu_caps & LAUNCH_CTRL) == 0 ? FALSE:TRUE);
	set_raw_memory_mode((ecu_caps & RAW_MEMORY) == 0 ? FALSE:TRUE);
	if (ecu_caps & IAC_STEPPER)
		gtk_button_set_label(GTK_BUTTON(buttons.pwm_idle_but),
				"Stepper Controlled");
	else
		gtk_button_set_label(GTK_BUTTON(buttons.pwm_idle_but),
				"PWM Controlled");
}

void set_dt_table_mapping_state(gboolean state)
{
        extern GList *table_map_controls;

	dbg_func(g_strdup_printf(__FILE__": set_dt_table_mapping_state()\n\tSetting DT map controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(table_map_controls, set_widget_state,(gpointer)state);
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

void set_ignition_mode(gboolean state)
{
	extern gint ecu_caps;
	extern GList *ign_controls;
	extern GList *inv_ign_controls;
	extern gint temp_units;
	extern struct DynamicLabels labels;
	extern struct DynamicButtons buttons;

	dbg_func(g_strdup_printf(__FILE__": set_ignition_mode()\n\tSetting Ignition controls state to %s\n",states[state]),INTERROGATOR);
	g_list_foreach(ign_controls, set_widget_state,(gpointer)state);
	g_list_foreach(inv_ign_controls, set_widget_state,(gpointer)(!state));
	reset_temps(GINT_TO_POINTER(temp_units));
	if (ecu_caps & S_N_EDIS)
	{
		gtk_label_set_text(GTK_LABEL(labels.timing_multi_lab),"EDIS Multi-Spark:");
		gtk_label_set_text(GTK_LABEL(labels.output_boost_lab),"Boost Retard:");
		gtk_button_set_label(GTK_BUTTON(buttons.multi_spark_but),"Enabled");
		gtk_button_set_label(GTK_BUTTON(buttons.norm_spark_but),"Disabled");
		gtk_button_set_label(GTK_BUTTON(buttons.boost_retard_but),"Enabled");
		gtk_button_set_label(GTK_BUTTON(buttons.noboost_retard_but),"Disabled");
	}
	else if (ecu_caps & S_N_SPARK)
	{
		gtk_label_set_text(GTK_LABEL(labels.timing_multi_lab),"Crank Timing:");
		gtk_label_set_text(GTK_LABEL(labels.output_boost_lab),"Ignition Output:");
		gtk_button_set_label(GTK_BUTTON(buttons.time_based_but),"Time Based");
		gtk_button_set_label(GTK_BUTTON(buttons.trig_return_but),"Trigger Return");
		gtk_button_set_label(GTK_BUTTON(buttons.normal_out_but),"Normal");
		gtk_button_set_label(GTK_BUTTON(buttons.invert_out_but),"Inverted");
	}
	else
		dbg_func(__FILE__": set_ignition_mode, ecu_caps doesn't have an ignition variant enabled\n",CRITICAL);

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
	extern gint temp_units;

	dbg_func(g_strdup_printf(__FILE__": set_dualtable_mode()\n\tSetting Dual Table controls state to %s\n",states[state]),INTERROGATOR);
        g_list_foreach(dt_controls, set_widget_state,(gpointer)state);
        g_list_foreach(inv_dt_controls, set_widget_state,(gpointer)(!state));

        /* temp_units is a FLAG... */
        reset_temps(GINT_TO_POINTER(temp_units));
}

void set_widget_state(gpointer widget, gpointer state)
{
        gtk_widget_set_sensitive(GTK_WIDGET(widget),(gboolean)state);
}

gboolean drain_hashtable(gpointer offset, gpointer value, gpointer data)
{
        /* called per element from the hash table to drain and send to ECU */
        write_ve_const((gint)value,(gint)offset, FALSE);
        return TRUE;
}

