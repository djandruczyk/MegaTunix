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
#include <jimstim.h>

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
	g_module_symbol(module,"lookup_widget",(void *)&lookup_widget_f);
	g_module_symbol(module,"io_cmd",(void *)&io_cmd_f);
	g_module_symbol(module,"initialize_outputdata",(void *)&initialize_outputdata_f);
	g_module_symbol(module,"dbg_func",(void *)&dbg_func_f);
	g_module_symbol(module,"start_tickler",(void *)&start_tickler_f);
	g_module_symbol(module,"stop_tickler",(void *)&stop_tickler_f);
	g_module_symbol(module,"get_list",(void *)&get_list_f);
	g_module_symbol(module,"set_widget_sensitive",(void *)&set_widget_sensitive_f);
	g_module_symbol(module,"update_logbar",(void *)&update_logbar_f);

}

void plugin_shutdown(void)
{
	return;
}


void register_enums(void)
{
	return;
}
