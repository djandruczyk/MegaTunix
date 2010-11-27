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
	dbg_func_f = (void *)DATA_GET(global_data,"dbg_func_f");
	g_assert(dbg_func_f);
	io_cmd_f = (void *)DATA_GET(global_data,"io_cmd_f");
	g_assert(io_cmd_f);
	start_tickler_f = (void *)DATA_GET(global_data,"start_tickler_f");
	g_assert(start_tickler_f);
	stop_tickler_f = (void *)DATA_GET(global_data,"stop_tickler_f");
	g_assert(stop_tickler_f);
	signal_read_rtvars_f = (void *)DATA_GET(global_data,"signal_read_rtvars_f");
	g_assert(signal_read_rtvars_f);
	get_ecu_data_f = (void *)DATA_GET(global_data,"get_ecu_data_f");
	g_assert(get_ecu_data_f);
}


G_MODULE_EXPORT void plugin_shutdown()
{
	stop(TOOTHMON_TICKLER);
	stop(TRIGMON_TICKLER);
}
