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
#include <jimstim_plugin.h>

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
	lookup_widget_f = (void *)DATA_GET(global_data,"lookup_widget_f");
	if (!lookup_widget_f)
		error_msg_f("JimStim plugin ERROR: lookup_widget_f pointer is not exported!\n\t BUG!");
	io_cmd_f = (void *)DATA_GET(global_data,"io_cmd_f");
	if (!io_cmd_f)
		error_msg_f("JimStim plugin ERROR: io_cmd_f pointer is not exported!\n\t BUG!");
	initialize_outputdata_f = (void *)DATA_GET(global_data,"initialize_outputdata_f");
	if (!initialize_outputdata_f)
		error_msg_f("JimStim plugin ERROR: initialize_outputdata_f pointer is not exported!\n\t BUG!");
	dbg_func_f = (void *)DATA_GET(global_data,"dbg_func_f");
	if (!dbg_func_f)
		error_msg_f("JimStim plugin ERROR: dbg_func_f pointer is not exported!\n\t BUG!");
	start_tickler_f = (void *)DATA_GET(global_data,"start_tickler_f");
	if (!start_tickler_f)
		error_msg_f("JimStim plugin ERROR: start_tickler_f pointer is not exported!\n\t BUG!");
	stop_tickler_f = (void *)DATA_GET(global_data,"stop_tickler_f");
	if (!stop_tickler_f)
		error_msg_f("JimStim plugin ERROR: stop_tickler_f pointer is not exported!\n\t BUG!");
	get_list_f = (void *)DATA_GET(global_data,"get_list_f");
	if (!get_list_f)
		error_msg_f("JimStim plugin ERROR: get_list_f pointer is not exported!\n\t BUG!");
	set_widget_sensitive_f = (void *)DATA_GET(global_data,"set_widget_sensitive_f");
	if (!set_widget_sensitive_f)
		error_msg_f("JimStim plugin ERROR: set_widget_sensitive_f pointer is not exported!\n\t BUG!");
	update_logbar_f = (void *)DATA_GET(global_data,"update_logbar_f");
	if (!update_logbar_f)
		error_msg_f("JimStim plugin ERROR: update_logbar_f pointer is not exported!\n\t BUG!");
}
