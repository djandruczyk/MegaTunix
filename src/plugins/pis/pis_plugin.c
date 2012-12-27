/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file src/plugins/pis/pis_plugin.c
  \ingroup PisPlugin,Plugins
  \brief PIS Plugin stubs
  \author David Andruczyk
  */

#define __PIS_PLUGIN_C__
#include <pis_plugin.h>
#include <gtk/gtk.h>
#include <stdio.h>


gconstpointer *global_data = NULL;


/*!
  \brief initializes the plugin, referencing all needed functions
  \param data is the pointer to the global data container
  */
G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;
	*(void **)(&error_msg_f) = (void **)DATA_GET(global_data,"error_msg_f");
	g_assert(error_msg_f);
	*(void **)(&get_symbol_f) = (void **)DATA_GET(global_data,"get_symbol_f");
	g_assert(get_symbol_f);
	get_symbol_f("dbg_func",(void **)&dbg_func_f);
	ENTER();
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	register_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief prepares the plugin to be unloaded
  */
G_MODULE_EXPORT void plugin_shutdown()
{
	ENTER();
	deregister_ecu_enums();
	EXIT();
	return;
}


/*!
  \brief registers common enumerations in the global table for this plugin
  */
void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}


/*!
  \brief deregisters common enumerations from the global table for this plugin
  */
void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	ENTER();
	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
	EXIT();
	return;
}
