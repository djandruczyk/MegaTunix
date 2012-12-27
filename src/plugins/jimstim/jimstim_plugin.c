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

/*!
  \file src/plugins/jimstim/jimstim_plugin.c
  \ingroup JimStimPlugin,Plugins
  \brief JimStim Plugin init/shutdown functions
  \author David Andruczyk
  */

#define __JIMSTIM_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <jimstim_gui_handlers.h>
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
	*(void **)(&error_msg_f) = DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	*(void**)(&get_symbol_f) = DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("dbg_func",(void **)&dbg_func_f);
	ENTER();

	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	get_symbol_f("convert_after_upload",(void **)&convert_after_upload_f);
	get_symbol_f("convert_before_download",(void **)&convert_before_download_f);
	get_symbol_f("get_essential_bits",(void **)&get_essential_bits_f);
	get_symbol_f("get_list",(void **)&get_list_f);
	get_symbol_f("initialize_outputdata",(void **)&initialize_outputdata_f);
	get_symbol_f("io_cmd",(void **)&io_cmd_f);
	get_symbol_f("lookup_widget",(void **)&lookup_widget_f);
	get_symbol_f("ms_send_to_ecu",(void **)&ms_send_to_ecu_f);
	get_symbol_f("search_model",(void **)&search_model_f);
	get_symbol_f("set_widget_sensitive",(void **)&set_widget_sensitive_f);
	get_symbol_f("start_tickler",(void **)&start_tickler_f);
	get_symbol_f("std_combo_handler",(void **)&std_combo_handler_f);
	get_symbol_f("stop_tickler",(void **)&stop_tickler_f);
	get_symbol_f("update_logbar",(void **)&update_logbar_f);

	register_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief deregisters plugin resources in prep for plugin shutdown
  */
G_MODULE_EXPORT void plugin_shutdown(void)
{
	ENTER();
	deregister_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief registers enumeration into the global table for this plugin
  */
void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		/* Ecu specific Handler Enumerations */
		g_hash_table_insert (str_2_enum, (void *)"_SWEEP_START_",
				GINT_TO_POINTER (SWEEP_START));
		g_hash_table_insert (str_2_enum, (void *)"_SWEEP_STOP_",
				GINT_TO_POINTER (SWEEP_STOP));
		g_hash_table_insert (str_2_enum, (void *)"_RPM_MODE_",
				GINT_TO_POINTER (RPM_MODE));
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}


/*!
  \brief deregisters enumeration from the global table for this plugin
  */
void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
		g_hash_table_remove (str_2_enum, "_SWEEP_START_");
		g_hash_table_remove (str_2_enum, "_SWEEP_STOP_");
		g_hash_table_remove (str_2_enum, "_RPM_MODE_");

	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}

