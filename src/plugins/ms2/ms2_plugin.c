/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#define __MS2_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <ms2_plugin.h>
#include <ms2_gui_handlers.h>
#include <ms2_tlogger.h>
#include <gtk/gtk.h>


gconstpointer *global_data;


G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	error_msg_f = (void *)DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	get_symbol_f = (void *)DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);

	get_symbol_f("bind_to_lists",(void *)&bind_to_lists_f);
	get_symbol_f("convert_before_download",(void *)&convert_before_download_f);
	get_symbol_f("convert_after_upload",(void *)&convert_after_upload_f);
	get_symbol_f("create_single_bit_state_watch",(void *)&create_single_bit_state_watch_f);
	get_symbol_f("dbg_func",(void *)&dbg_func_f);
	get_symbol_f("evaluator_create",(void *)&evaluator_create_f);
	get_symbol_f("evaluator_destroy",(void *)&evaluator_destroy_f);
	get_symbol_f("evaluator_evaluate_x",(void *)&evaluator_evaluate_x_f);
	get_symbol_f("get_essential_bits",(void *)&get_essential_bits_f);
	get_symbol_f("get_bitshift",(void *)&get_bitshift_f);
	get_symbol_f("initialize_gc",(void *)&initialize_gc_f);
	get_symbol_f("io_cmd",(void *)&io_cmd_f);
	get_symbol_f("lookup_current_value",(void *)&lookup_current_value_f);
	get_symbol_f("lookup_widget",(void *)&lookup_widget_f);
	get_symbol_f("mask_entry_new_with_mask_w",(void *)&mask_entry_new_with_mask_f);
	get_symbol_f("ms_get_ecu_data",(void *)&ms_get_ecu_data_f);
	get_symbol_f("ms_send_to_ecu",(void *)&ms_send_to_ecu_f);
	get_symbol_f("register_widget",(void *)&register_widget_f);
	get_symbol_f("search_model",(void *)&search_model_f);
	get_symbol_f("signal_read_rtvars",(void *)&signal_read_rtvars_f);
	get_symbol_f("start_tickler",(void *)&start_tickler_f);
	get_symbol_f("stop_tickler",(void *)&stop_tickler_f);
	get_symbol_f("update_widget",(void *)&update_widget_f);

	register_ecu_enums();
}


G_MODULE_EXPORT void plugin_shutdown(void)
{
	extern MS2_TTMon_Data *ttm_data;
	if (ttm_data)
		ttm_data->stop = TRUE;
	deregister_ecu_enums();
}

void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET(global_data,"str_2_enum");
	if (str_2_enum)
	{
		g_hash_table_insert(str_2_enum,"_START_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(START_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_STOP_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(STOP_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_START_TRIGMON_LOGGER_",
				GINT_TO_POINTER(START_TRIGMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_STOP_TRIGMON_LOGGER_",
				GINT_TO_POINTER(STOP_TRIGMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_START_COMPOSITEMON_LOGGER_",
				GINT_TO_POINTER(START_COMPOSITEMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_STOP_COMPOSITEMON_LOGGER_",
				GINT_TO_POINTER(STOP_COMPOSITEMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_GET_CURR_TPS_",
				GINT_TO_POINTER(GET_CURR_TPS));
	}
}

void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET(global_data,"str_2_enum");
	if (str_2_enum)
	{
		g_hash_table_remove(str_2_enum,"_START_TOOTHMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_TOOTHMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_START_TRIGMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_TRIGMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_START_COMPOSITEMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_STOP_COMPOSITEMON_LOGGER_");
		g_hash_table_remove(str_2_enum,"_GET_CURR_TPS_");
	}
}

