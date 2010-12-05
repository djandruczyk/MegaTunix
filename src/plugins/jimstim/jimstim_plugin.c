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
	get_symbol_f = (void *)DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("lookup_widget",(void *)&lookup_widget_f);
	get_symbol_f("io_cmd",(void *)&io_cmd_f);
	get_symbol_f("initialize_outputdata",(void *)&initialize_outputdata_f);
	get_symbol_f("dbg_func",(void *)&dbg_func_f);
	get_symbol_f("start_tickler",(void *)&start_tickler_f);
	get_symbol_f("stop_tickler",(void *)&stop_tickler_f);
	get_symbol_f("get_list",(void *)&get_list_f);
	get_symbol_f("set_widget_sensitive",(void *)&set_widget_sensitive_f);
	get_symbol_f("update_logbar",(void *)&update_logbar_f);

}

void plugin_shutdown(void)
{
	return;
}


void register_enums(void)
{
	return;
}
