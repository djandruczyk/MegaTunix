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

#define __JIMSTIM_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <jimstim_plugin.h>

gconstpointer *global_data;

/*!
  \brief initializes the jimstim plugin, sets up all function references
  and enumerations
  \param data is the pointer to the global data structure
  */
G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;

	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */

	error_msg_f = DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	get_symbol_f = DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);

	get_symbol_f("dbg_func",(void *)&dbg_func_f);
	g_assert(dbg_func_f);
	get_symbol_f("get_list",(void *)&get_list_f);
	g_assert(get_list_f);
	get_symbol_f("initialize_outputdata",(void *)&initialize_outputdata_f);
	g_assert(initialize_outputdata_f);
	get_symbol_f("io_cmd",(void *)&io_cmd_f);
	g_assert(io_cmd_f);
	get_symbol_f("lookup_widget",(void *)&lookup_widget_f);
	g_assert(lookup_widget_f);
	get_symbol_f("set_widget_sensitive",(void *)&set_widget_sensitive_f);
	g_assert(set_widget_sensitive_f);
	get_symbol_f("start_tickler",(void *)&start_tickler_f);
	g_assert(start_tickler_f);
	get_symbol_f("stop_tickler",(void *)&stop_tickler_f);
	g_assert(stop_tickler_f);
	get_symbol_f("update_logbar",(void *)&update_logbar_f);
	g_assert(update_logbar_f);

	register_ecu_enums();
}


/*!
  \brief deregisters plugin resources in prep for plugin shutdown
  */
G_MODULE_EXPORT void plugin_shutdown(void)
{
	deregister_ecu_enums();
	return;
}


/*!
  \brief registers enumeration into the global table for this plugin
  */
void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
}


/*!
  \brief deregisters enumeration from the global table for this plugin
  */
void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
}

