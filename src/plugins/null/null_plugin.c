/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#define __NULL_PLUGIN_C__
#include <config.h>
#include <defines.h>
#include <null_plugin.h>
#include <gtk/gtk.h>


gconstpointer *global_data = NULL;


G_MODULE_EXPORT void plugin_init(gconstpointer *data)
{
	global_data = data;
	/* Initializes function pointers since on Winblows was can NOT
	   call functions within the program that loaded this DLL, so
	   we need to pass pointers over and assign them here.
	 */
	register_ecu_enums();
}


G_MODULE_EXPORT void plugin_shutdown()
{
	deregister_ecu_enums();
	return;
}


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
