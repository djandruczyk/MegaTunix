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
  \file src/plugins/null/null_plugin.c
  \ingroup NullPlugin,Plugins
  \brief Null plugin init/shutdown stubs
  \author David Andruczyk
  */

#define __NULL_PLUGIN_C__
#include <null_plugin.h>
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
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	register_ecu_enums();
}


/*!
  \brief prepares the plugin to be unloaded
  */
G_MODULE_EXPORT void plugin_shutdown()
{
	deregister_ecu_enums();
	return;
}


/*!
  \brief registers common enumerations in the global table for this plugin
  */
void register_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
}


/*!
  \brief deregisters common enumerations from the global table for this plugin
  */
void deregister_ecu_enums(void)
{
	GHashTable *str_2_enum = NULL;

	str_2_enum = (GHashTable *)DATA_GET (global_data, "str_2_enum");
	if (str_2_enum)
	{
	}
	else
		printf ("COULD NOT FIND global pointer to str_2_enum table\n!");
}
