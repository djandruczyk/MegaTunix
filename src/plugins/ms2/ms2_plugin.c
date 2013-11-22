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
  \file src/plugins/ms2/ms2_plugin.c
  \ingroup MS2Plugin,Plugins
  \brief MS2 Plugin init/shutdown functions
  \author David Andruczyk
  */

#define __MS2_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <ms2_plugin.h>
#include <ms2_gui_handlers.h>
#include <ms2_tlogger.h>
#include <gtk/gtk.h>


gconstpointer *global_data;


/*!
  \brief initializes the MS2 plugin, connects to all functions needed in
  core megatunix and registers any needed enumerations
  \param data is the pointer to the global data container
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
	get_symbol_f("bind_to_lists",(void **)&bind_to_lists_f);
	get_symbol_f("convert_before_download",(void **)&convert_before_download_f);
	get_symbol_f("calc_value",(void **)&calc_value_f);
	get_symbol_f("convert_temps",(void **)&convert_temps_f);
	get_symbol_f("convert_after_upload",(void **)&convert_after_upload_f);
	get_symbol_f("create_rtv_single_bit_state_watch",(void **)&create_rtv_single_bit_state_watch_f);
	get_symbol_f("evaluator_create",(void **)&evaluator_create_f);
	get_symbol_f("evaluator_destroy",(void **)&evaluator_destroy_f);
	get_symbol_f("evaluator_evaluate_x",(void **)&evaluator_evaluate_x_f);
	get_symbol_f("f_to_c",(void **)&f_to_c_f);
	get_symbol_f("f_to_k",(void **)&f_to_k_f);
	get_symbol_f("get_essential_bits",(void **)&get_essential_bits_f);
	get_symbol_f("get_extreme_from_size",(void **)&get_extreme_from_size_f);
	get_symbol_f("get_bitshift",(void **)&get_bitshift_f);
	get_symbol_f("initialize_gc",(void **)&initialize_gc_f);
	get_symbol_f("io_cmd",(void **)&io_cmd_f);
	get_symbol_f("lookup_current_value",(void **)&lookup_current_value_f);
	get_symbol_f("lookup_widget",(void **)&lookup_widget_f);
	get_symbol_f("mask_entry_new_with_mask_w",(void **)&mask_entry_new_with_mask_f);
	get_symbol_f("ms_get_ecu_data",(void **)&ms_get_ecu_data_f);
	get_symbol_f("ms_send_to_ecu",(void **)&ms_send_to_ecu_f);
	get_symbol_f("register_widget",(void **)&register_widget_f);
	get_symbol_f("remove_from_lists",(void **)&remove_from_lists_f);
	get_symbol_f("search_model",(void **)&search_model_f);
	get_symbol_f("signal_read_rtvars",(void **)&signal_read_rtvars_f);
	get_symbol_f("start_tickler",(void **)&start_tickler_f);
	get_symbol_f("stop_tickler",(void **)&stop_tickler_f);
	get_symbol_f("temp_to_host",(void **)&temp_to_host_f);
	get_symbol_f("update_widget",(void **)&update_widget_f);

	register_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief Shuts down the MS2 plugin and deregisters the enums in prep for 
  plugin unload
  */
G_MODULE_EXPORT void plugin_shutdown(void)
{
	extern MS2_TTMon_Data *ttm_data;
	ENTER();
	if (ttm_data)
		ttm_data->stop = TRUE;
	deregister_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief Registers the MS2 specific enumerations in the global table
  \see deregister_ecu_enums
  */
void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET(global_data,"str_2_enum");
	if (str_2_enum)
	{
		g_hash_table_insert(str_2_enum,(gpointer)"_START_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(START_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_STOP_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(STOP_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_START_TRIGMON_LOGGER_",
				GINT_TO_POINTER(START_TRIGMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_STOP_TRIGMON_LOGGER_",
				GINT_TO_POINTER(STOP_TRIGMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_START_COMPOSITEMON_LOGGER_",
				GINT_TO_POINTER(START_COMPOSITEMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_STOP_COMPOSITEMON_LOGGER_",
				GINT_TO_POINTER(STOP_COMPOSITEMON_LOGGER));
		g_hash_table_insert(str_2_enum,(gpointer)"_GET_CURR_TPS_",
				GINT_TO_POINTER(GET_CURR_TPS));
		g_hash_table_insert(str_2_enum,(gpointer)"_MS2_USER_OUTPUTS_",
				GINT_TO_POINTER(MS2_USER_OUTPUTS));
	}
	EXIT();
	return;
}


/*!
  \brief Deregister the MS2 specific enumerations in the global table
  \see register_ecu_enums
  */
void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET(global_data,"str_2_enum");
	if (str_2_enum)
	{
		g_hash_table_remove(str_2_enum,"_START_TOOTHMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_TOOTHMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_START_TRIGMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_TRIGMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_START_COMPOSITEMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_COMPOSITEMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_GET_CURR_TPS_");
		g_hash_table_remove(str_2_enum,"_MS2_USER_OUTPUTS_");
	}
	EXIT();
	return;
}
