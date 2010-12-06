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
#include <defines.h>
#include <mscommon_plugin.h>
#include <mscommon_helpers.h>
#include <mtxsocket.h>
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
	get_symbol_f("dbg_func",(void *)&dbg_func_f);
	get_symbol_f("get_list",(void *)&get_list_f);
	get_symbol_f("io_cmd",(void *)&io_cmd_f);
	get_symbol_f("initialize_outputdata",(void *)&initialize_outputdata_f);
	get_symbol_f("set_title",(void *)&set_title_f);
	get_symbol_f("set_widget_sensitive",(void *)&set_widget_sensitive_f);
	get_symbol_f("thread_update_logbar",(void *)&thread_update_logbar_f);
	get_symbol_f("process_rt_vars",(void *)&process_rt_vars_f);
	get_symbol_f("thread_update_widget",(void *)&thread_update_widget_f);
	get_symbol_f("cleanup",(void *)&cleanup_f);
	get_symbol_f("queue_function",(void *)&queue_function_f);
	get_symbol_f("lookup_precision",(void *)&lookup_precision_f);
	get_symbol_f("lookup_current_value",(void *)&lookup_current_value_f);
	get_symbol_f("translate_string",(void *)&translate_string_f);
	get_symbol_f("lookup_widget",(void *)&lookup_widget_f);
	get_symbol_f("set_group_color",(void *)&set_group_color_f);
	get_symbol_f("thread_refresh_widgets_at_offset",(void *)&thread_refresh_widgets_at_offset_f);
	get_symbol_f("get_multiplier",(void *)&get_multiplier_f);
	get_symbol_f("recalc_table_limits",(void *)&recalc_table_limits_f);
	get_symbol_f("get_bitshift",(void *)&get_bitshift_f);
	get_symbol_f("spin_button_handler",(void *)&spin_button_handler_f);
	get_symbol_f("set_file_api",(void *)&set_file_api_f);
	get_symbol_f("get_file_api",(void *)&get_file_api_f);
	get_symbol_f("parse_keys",(void *)&parse_keys_f);
	get_symbol_f("start_tickler",(void *)&start_tickler_f);
	get_symbol_f("stop_tickler",(void *)&stop_tickler_f);
	get_symbol_f("get_multiplier",(void *)&get_multiplier_f);
	get_symbol_f("get_extreme_from_size",(void *)&get_extreme_from_size_f);
	get_symbol_f("convert_after_upload",(void *)&convert_after_upload_f);
	get_symbol_f("entry_changed_handler",(void *)&entry_changed_handler_f);
	get_symbol_f("get_colors_from_hue",(void *)&get_colors_from_hue_f);
}


G_MODULE_EXPORT void plugin_shutdown()
{
	return;
}


void register_enums(void)
{
	extern gconstpointer *global_data;
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET(global_data,"str2_enum");
	if (str_2_enum)
	{
		/* TCP Socket Commands */
		g_hash_table_insert(str_2_enum,"HELP",
				GINT_TO_POINTER(HELP));
		g_hash_table_insert(str_2_enum,"QUIT",
				GINT_TO_POINTER(QUIT));
		g_hash_table_insert(str_2_enum,"GET_REVISION",
				GINT_TO_POINTER(GET_REVISION));
		g_hash_table_insert(str_2_enum,"GET_SIGNATURE",
				GINT_TO_POINTER(GET_SIGNATURE));
		g_hash_table_insert(str_2_enum,"GET_RT_VARS",
				GINT_TO_POINTER(GET_RT_VARS));
		g_hash_table_insert(str_2_enum,"GET_RTV_LIST",
				GINT_TO_POINTER(GET_RTV_LIST));
		g_hash_table_insert(str_2_enum,"GET_ECU_VARS",
				GINT_TO_POINTER(GET_ECU_VARS));
		g_hash_table_insert(str_2_enum,"GET_ECU_VAR_U08",
				GINT_TO_POINTER(GET_ECU_VAR_U08));
		g_hash_table_insert(str_2_enum,"GET_ECU_VAR_S08",
				GINT_TO_POINTER(GET_ECU_VAR_S08));
		g_hash_table_insert(str_2_enum,"GET_ECU_VAR_U16",
				GINT_TO_POINTER(GET_ECU_VAR_U16));
		g_hash_table_insert(str_2_enum,"GET_ECU_VAR_S16",
				GINT_TO_POINTER(GET_ECU_VAR_S16));
		g_hash_table_insert(str_2_enum,"GET_ECU_VAR_U32",
				GINT_TO_POINTER(GET_ECU_VAR_U32));
		g_hash_table_insert(str_2_enum,"GET_ECU_VAR_S32",
				GINT_TO_POINTER(GET_ECU_VAR_S32));
		g_hash_table_insert(str_2_enum,"SET_ECU_VAR_U08",
				GINT_TO_POINTER(SET_ECU_VAR_U08));
		g_hash_table_insert(str_2_enum,"SET_ECU_VAR_S08",
				GINT_TO_POINTER(SET_ECU_VAR_S08));
		g_hash_table_insert(str_2_enum,"SET_ECU_VAR_U16",
				GINT_TO_POINTER(SET_ECU_VAR_U16));
		g_hash_table_insert(str_2_enum,"SET_ECU_VAR_S16",
				GINT_TO_POINTER(SET_ECU_VAR_S16));
		g_hash_table_insert(str_2_enum,"SET_ECU_VAR_U32",
				GINT_TO_POINTER(SET_ECU_VAR_U32));
		g_hash_table_insert(str_2_enum,"SET_ECU_VAR_S32",
				GINT_TO_POINTER(SET_ECU_VAR_S32));
		g_hash_table_insert(str_2_enum,"BURN_FLASH",
				GINT_TO_POINTER(BURN_FLASH));
		g_hash_table_insert(str_2_enum,"GET_RAW_ECU",
				GINT_TO_POINTER(GET_RAW_ECU));
		g_hash_table_insert(str_2_enum,"SET_RAW_ECU",
				GINT_TO_POINTER(SET_RAW_ECU));
	}
}

