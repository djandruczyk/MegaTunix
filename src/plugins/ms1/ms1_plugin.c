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
#include <ms1_plugin.h>
#include <ms1-t-logger.h>
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
	dbg_func_f = (void *)DATA_GET(global_data,"dbg_func_f");
	if (!dbg_func_f)
		error_msg_f("MS1 plugin ERROR: dbg_func_f pointer is not exported!\n\t BUG!");
	lookup_widget_f = (void *)DATA_GET(global_data,"lookup_widget_f");
	if (!lookup_widget_f)
		error_msg_f("MS1 plugin ERROR: lookup_widget_f pointer is not exported!\n\t BUG!");
	io_cmd_f = (void *)DATA_GET(global_data,"io_cmd_f");
	if (!io_cmd_f)
		error_msg_f("MS1 plugin ERROR: io_cmd_f pointer is not exported!\n\t BUG!");
	start_tickler_f = (void *)DATA_GET(global_data,"start_tickler_f");
	if (!start_tickler_f)
		error_msg_f("MS1 plugin ERROR: start_tickler_f pointer is not exported!\n\t BUG!");
	stop_tickler_f = (void *)DATA_GET(global_data,"stop_tickler_f");
	if (!stop_tickler_f)
		error_msg_f("MS1 plugin ERROR: stop_tickler_f pointer is not exported!\n\t BUG!");
	signal_read_rtvars_f = (void *)DATA_GET(global_data,"signal_read_rtvars_f");
	if (!signal_read_rtvars_f)
		error_msg_f("MS1 plugin ERROR: signal_read_rtvars_f pointer is not exported!\n\t BUG!");
	get_ecu_data_f = (void *)DATA_GET(global_data,"get_ecu_data_f");
	if (!get_ecu_data_f)
		error_msg_f("MS1 plugin ERROR: get_ecu_data_f pointer is not exported!\n\t BUG!");
	initialize_gc_f = (void *)DATA_GET(global_data,"initialize_gc_f");
	if (!initialize_gc_f)
		error_msg_f("MS1 plugin ERROR: initialize_gc_f pointer is not exported!\n\t BUG!");
	lookup_current_value_f = (void *)DATA_GET(global_data,"lookup_current_value_f");
	if (!lookup_current_value_f)
		error_msg_f("MS1 plugin ERROR: lookup_current_value_f pointer is not exported!\n\t BUG!");
}


G_MODULE_EXPORT void plugin_shutdown()
{
	stop(TOOTHMON_TICKLER);
	stop(TRIGMON_TICKLER);
}
