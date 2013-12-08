/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/ms1/ms1_plugin.c
  \ingroup MS1Plugin,Plugins
  \brief MS1 Plugin init/shutdown functions
  \author David Andruczyk
  */

#define __MS1_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <ms1_gui_handlers.h>
#include <ms1_plugin.h>
#include <ms1_tlogger.h>
#include <gtk/gtk.h>


gconstpointer *global_data;


/*!
  \brief Initializes the MS1 ecu firmware plugin. This links up to all the 
  needed functions within core MtX, and registers any ecu specific ENUMS
  or other datastructures
  \param data is a pointer to the global_data container
  \see plugin_shutdown
  */
G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;
	*(void **)(&error_msg_f) = DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	*(void **)(&get_symbol_f) = DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("dbg_func",(void **)&dbg_func_f);
	ENTER();
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	get_symbol_f("alter_widget_state",(void **)&alter_widget_state_f);
	get_symbol_f("convert_after_upload",(void **)&convert_after_upload_f);
	get_symbol_f("convert_before_download",(void **)&convert_before_download_f);
	get_symbol_f("entry_changed_handler",(void **)&entry_changed_handler_f);
	get_symbol_f("get_colors_from_hue",(void **)&get_colors_from_hue_f);
	get_symbol_f("get_bitshift",(void **)&get_bitshift_f);
	get_symbol_f("get_ecu_data",(void **)&get_ecu_data_f);
	get_symbol_f("get_essentials",(void **)&get_essentials_f);
	get_symbol_f("get_essential_bits",(void **)&get_essential_bits_f);
	get_symbol_f("get_extreme_from_size",(void **)&get_extreme_from_size_f);
	get_symbol_f("initialize_gc",(void **)&initialize_gc_f);
	get_symbol_f("io_cmd",(void **)&io_cmd_f);
	get_symbol_f("lookup_current_value",(void **)&lookup_current_value_f);
	get_symbol_f("lookup_widget",(void **)&lookup_widget_f);
	get_symbol_f("lookuptables_configurator",(void **)&lookuptables_configurator_f);
	get_symbol_f("ms_get_ecu_data",(void **)&ms_get_ecu_data_f);
	get_symbol_f("ms_send_to_ecu",(void **)&ms_send_to_ecu_f);
	get_symbol_f("recalc_table_limits",(void **)&recalc_table_limits_f);
	get_symbol_f("signal_read_rtvars",(void **)&signal_read_rtvars_f);
	get_symbol_f("start_tickler",(void **)&start_tickler_f);
	get_symbol_f("std_entry_handler",(void **)&std_entry_handler_f);
	get_symbol_f("stop_tickler",(void **)&stop_tickler_f);

	register_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief stops any resources assocated with the MS1 plugin and de-registers
  the enums/resource in prep for plugin unload
  */
G_MODULE_EXPORT void plugin_shutdown()
{
	ENTER();
	stop(TOOTHMON_TICKLER);
	stop(TRIGMON_TICKLER);
	deregister_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief registers firmware specific enumerations so this plugin can work
  */
void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;
	ENTER();
	str_2_enum = (GHashTable *)DATA_GET(global_data,"str_2_enum");
	if (str_2_enum)
	{
		/* MSnS-E Tooth/Trigger monitor */
		g_hash_table_insert(str_2_enum,(gpointer)"_START_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(START_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_STOP_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(STOP_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_START_TRIGMON_LOGGER_",
				GINT_TO_POINTER(START_TRIGMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_STOP_TRIGMON_LOGGER_",
				GINT_TO_POINTER(STOP_TRIGMON_LOGGER));
		/* Oddball Trigger angle/oddfire angle special handlers */
		g_hash_table_insert(str_2_enum,(gpointer)"_TRIGGER_ANGLE_",
				GINT_TO_POINTER(TRIGGER_ANGLE));
		g_hash_table_insert(str_2_enum,(gpointer)"_ODDFIRE_ANGLE_",
				GINT_TO_POINTER(ODDFIRE_ANGLE));
		/* Std button handlers */
		g_hash_table_insert(str_2_enum,(gpointer)"_REBOOT_GETERR_",
				GINT_TO_POINTER(REBOOT_GETERR));		
	}
	EXIT();
	return;
}


/*!
  \brief deregisters firmware specific enumerations so this plugin can 
  be unloaded without leaking excessive amounts of memory
  */
void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;
	ENTER();
	str_2_enum = (GHashTable *)DATA_GET(global_data,"str_2_enum");
	if (str_2_enum)
	{
		/* MSnS-E Tooth/Trigger monitor */
		g_hash_table_remove(str_2_enum,"_START_TOOTHMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_TOOTHMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_START_TRIGMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_TRIGMON_LOGGER_");
		/* Oddball Trigger angle/oddfire angle special handlers */
		g_hash_table_remove(str_2_enum,"_TRIGGER_ANGLE_");
		g_hash_table_remove(str_2_enum,"_ODDFIRE_ANGLE_");
		/* Std button handlers */
		g_hash_table_remove(str_2_enum,"_REBOOT_GETERR_");
	}
	EXIT();
	return;
}
