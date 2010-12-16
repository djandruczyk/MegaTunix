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
#include <firmware.h>
#include <freeems_plugin.h>
#include <gtk/gtk.h>
#include <interrogate.h>


gconstpointer *global_data = NULL;


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
	get_symbol_f("_get_sized_data",(void *)&_get_sized_data_f);
	get_symbol_f("_set_sized_data",(void *)&_set_sized_data_f);
	get_symbol_f("alter_widget_state",(void *)&alter_widget_state_f);
	get_symbol_f("bind_to_lists",(void *)&bind_to_lists_f);
	get_symbol_f("cleanup",(void *)&cleanup_f);
	get_symbol_f("convert_after_upload",(void *)&convert_after_upload_f);
	get_symbol_f("convert_before_download",(void *)&convert_before_download_f);
	get_symbol_f("dbg_func",(void *)&dbg_func_f);
	get_symbol_f("entry_changed_handler",(void *)&entry_changed_handler_f);
	get_symbol_f("eval_create",(void *)&eval_create_f);
	get_symbol_f("eval_destroy",(void *)&eval_destroy_f);
	get_symbol_f("eval_x",(void *)&eval_x_f);
	get_symbol_f("flush_serial",(void *)&flush_serial_f);
	get_symbol_f("get_bitshift",(void *)&get_bitshift_f);
	get_symbol_f("get_colors_from_hue",(void *)&get_colors_from_hue_f);
	get_symbol_f("get_extreme_from_size",(void *)&get_extreme_from_size_f);
	get_symbol_f("get_file_api",(void *)&get_file_api_f);
	get_symbol_f("get_list",(void *)&get_list_f);
	get_symbol_f("get_multiplier",(void *)&get_multiplier_f);
	get_symbol_f("get_table",(void *)&get_table_f);
	get_symbol_f("io_cmd",(void *)&io_cmd_f);
	get_symbol_f("initialize_outputdata",(void *)&initialize_outputdata_f);
	get_symbol_f("key_event",(void *)&key_event_f);
	get_symbol_f("lookup_current_value",(void *)&lookup_current_value_f);
	get_symbol_f("lookup_data",(void *)&lookup_data_f);
	get_symbol_f("lookup_precision",(void *)&lookup_precision_f);
	get_symbol_f("lookup_widget",(void *)&lookup_widget_f);
	get_symbol_f("mem_alloc",(void *)&mem_alloc_f);
	get_symbol_f("parse_keys",(void *)&parse_keys_f);
	get_symbol_f("process_rt_vars",(void *)&process_rt_vars_f);
	get_symbol_f("queue_function",(void *)&queue_function_f);
	get_symbol_f("register_widget",(void *)&register_widget_f);
	get_symbol_f("remove_watch",(void *)&remove_watch_f);
	get_symbol_f("remove_from_lists",(void *)&remove_from_lists_f);
	get_symbol_f("reverse_lookup",(void *)&reverse_lookup_f);
	get_symbol_f("search_model",(void *)&search_model_f);
	get_symbol_f("set_file_api",(void *)&set_file_api_f);
	get_symbol_f("set_group_color",(void *)&set_group_color_f);
	get_symbol_f("set_reqfuel_color",(void *)&set_reqfuel_color_f);
	get_symbol_f("set_title",(void *)&set_title_f);
	get_symbol_f("set_widget_sensitive",(void *)&set_widget_sensitive_f);
	get_symbol_f("spin_button_handler",(void *)&spin_button_handler_f);
	get_symbol_f("start_tickler",(void *)&start_tickler_f);
	get_symbol_f("std_entry_handler",(void *)&std_entry_handler_f);
	get_symbol_f("stop_tickler",(void *)&stop_tickler_f);
	get_symbol_f("thread_refresh_widget",(void *)&thread_refresh_widget_f);
	get_symbol_f("thread_refresh_widget_range",(void *)&thread_refresh_widget_range_f);
	get_symbol_f("thread_update_logbar",(void *)&thread_update_logbar_f);
	get_symbol_f("thread_update_widget",(void *)&thread_update_widget_f);
	get_symbol_f("thread_widget_set_sensitive",(void *)&thread_widget_set_sensitive_f);
	get_symbol_f("translate_string",(void *)&translate_string_f);
	get_symbol_f("update_logbar",(void *)&update_logbar_f);
	get_symbol_f("update_ve3d_if_necessary",(void *)&update_ve3d_if_necessary_f);
	get_symbol_f("warn_user",(void *)&warn_user_f);
	register_common_enums();
}


G_MODULE_EXPORT void plugin_shutdown()
{
	return;
}


void register_common_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* Firmware capabilities */
		g_hash_table_insert (str_2_enum, "_FREEEMS_",
				GINT_TO_POINTER (FREEEMS));
		g_hash_table_insert (str_2_enum, "_COUNT_", 
				GINT_TO_POINTER (COUNT));
		g_hash_table_insert (str_2_enum, "_SUBMATCH_",
				GINT_TO_POINTER (SUBMATCH));
		g_hash_table_insert (str_2_enum, "_NUMMATCH_",
				GINT_TO_POINTER (NUMMATCH));
		g_hash_table_insert (str_2_enum, "_FULLMATCH_",
				GINT_TO_POINTER (FULLMATCH));
		g_hash_table_insert (str_2_enum, "_REGEX_", 
				GINT_TO_POINTER (REGEX));
		/* Interrogation Test Results */
		g_hash_table_insert (str_2_enum, "_RESULT_DATA_",
				GINT_TO_POINTER (RESULT_DATA));
		g_hash_table_insert (str_2_enum, "_RESULT_TEXT_",
				GINT_TO_POINTER (RESULT_TEXT));
		/* Special Common Handlers */
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
}

