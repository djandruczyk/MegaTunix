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
#include <ms1_gui_handlers.h>
#include <ms1_plugin.h>
#include <ms1_tlogger.h>
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
	get_symbol_f("lookup_widget",(void *)&lookup_widget_f);
	get_symbol_f("io_cmd",(void *)&io_cmd_f);
	get_symbol_f("start_tickler",(void *)&start_tickler_f);
	get_symbol_f("stop_tickler",(void *)&stop_tickler_f);
	get_symbol_f("signal_read_rtvars",(void *)&signal_read_rtvars_f);
	get_symbol_f("ms_get_ecu_data",(void *)&ms_get_ecu_data_f);
	get_symbol_f("ms_send_to_ecu",(void *)&ms_send_to_ecu_f);
	get_symbol_f("initialize_gc",(void *)&initialize_gc_f);
	get_symbol_f("lookup_current_value",(void *)&lookup_current_value_f);
	get_symbol_f("std_entry_handler",(void *)&std_entry_handler_f);
	get_symbol_f("entry_changed_handler",(void *)&entry_changed_handler_f);
	get_symbol_f("recalc_table_limits",(void *)&recalc_table_limits_f);
	get_symbol_f("get_extreme_from_size",(void *)&get_extreme_from_size_f);
	get_symbol_f("convert_before_download",(void *)&convert_before_download_f);
	get_symbol_f("get_colors_from_hue",(void *)&get_colors_from_hue_f);
	get_symbol_f("get_bitshift",(void *)&get_bitshift_f);

	register_ecu_enums();
}


G_MODULE_EXPORT void plugin_shutdown()
{
	stop(TOOTHMON_TICKLER);
	stop(TRIGMON_TICKLER);
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
	}
}

