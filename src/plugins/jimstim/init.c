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
#include <jimstim.h>

gconstpointer global_data;

G_MODULE_EXPORT void plugin_init(gconstpointer data)
{
	global_data = data;
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	   */
	lookup_widget_f = (void *)DATA_GET(global_data,"lookup_widget_f");
	io_cmd_f = (void *)DATA_GET(global_data,"io_cmd_f");
	initialize_outputdata_f = (void *)DATA_GET(global_data,"initialize_outputdata_f");
	dbg_func_f = (void *)DATA_GET(global_data,"dbg_func_f");
	start_tickler_f = (void *)DATA_GET(global_data,"start_tickler_f");
	stop_tickler_f = (void *)DATA_GET(global_data,"stop_tickler_f");
	get_list_f = (void *)DATA_GET(global_data,"get_list_f");
	set_widget_sensitive_f = (void *)DATA_GET(global_data,"set_widget_sensitive_f");
	update_logbar_f = (void *)DATA_GET(global_data,"update_logbar_f");
}
