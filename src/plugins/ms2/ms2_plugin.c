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
#include <enums.h>
#include <ms2_plugin.h>
#include <ms2-t-logger.h>
#include <gtk/gtk.h>


gconstpointer *global_data;


G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	GModule *module = NULL;
	global_data = data;
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	error_msg_f = (void *)DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	module = DATA_GET(global_data,"megatunix_module");
	if (!module)
		error_msg_f(__FILE__": Plugin ERROR: pointer to megatunix module is invalid!\n\tBUG, contact author!");

	g_module_symbol(module,"dbg_func",(void *)&dbg_func_f);
	g_module_symbol(module,"lookup_widget",(void *)&lookup_widget_f);
	g_module_symbol(module,"io_cmd",(void *)&io_cmd_f);
	g_module_symbol(module,"start_tickler",(void *)&start_tickler_f);
	g_module_symbol(module,"stop_tickler",(void *)&stop_tickler_f);
	g_module_symbol(module,"signal_read_rtvars",(void *)&signal_read_rtvars_f);
	g_module_symbol(module,"get_ecu_data",(void *)&get_ecu_data_f);
	g_module_symbol(module,"initialize_gc",(void *)&initialize_gc_f);
	g_module_symbol(module,"lookup_current_value",(void *)&lookup_current_value_f);
	g_module_symbol(module,"create_single_bit_state_watch",(void *)&create_single_bit_state_watch_f);

	register_enums();
}


G_MODULE_EXPORT void plugin_shutdown(void)
{
	extern MS2_TTMon_Data *ttm_data;
	ttm_data->stop = TRUE;
}

void register_enums(void)
{
	extern gconstpointer *global_data;
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET(global_data,"str2_enum");
	if (str_2_enum)
	{
		g_hash_table_insert(str_2_enum,"_START_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(START_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_START_TRIGMON_LOGGER_",
				GINT_TO_POINTER(START_TRIGMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_START_COMPOSITEMON_LOGGER_",
				GINT_TO_POINTER(START_COMPOSITEMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_STOP_TOOTHMON_LOGGER_",
				GINT_TO_POINTER(STOP_TOOTHMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_STOP_COMPOSITEMON_LOGGER_",
				GINT_TO_POINTER(STOP_COMPOSITEMON_LOGGER));
		g_hash_table_insert(str_2_enum,"_STOP_TRIGMON_LOGGER_",
				GINT_TO_POINTER(STOP_TRIGMON_LOGGER));
	}
}
