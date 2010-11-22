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
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <template.h>


extern gconstpointer *global_data;

gboolean plugin_function(GtkWidget *widget, gpointer data)
{
	GModule * module = NULL;
	gchar * func_name = NULL;
	gboolean (*func)(GtkWidget *, gpointer) =  NULL;
	gboolean res = FALSE;

	printf ("plugin_function!\n");
	module = (GModule *)DATA_GET(global_data,"plugin_module");
	printf("module pointer %p\n",module);
	func_name = (gchar *)OBJ_GET(widget,"function_name");
	func = (void *)OBJ_GET(widget,"function");
	printf("func_name %s\n",func_name);

	if ((func_name) && (!func))
	{
		printf("search for func_name %s\n",func_name);
		if (g_module_symbol(module,func_name,(void *)&func))
		{
			printf("found it, storing ptr\n");
			DATA_SET(widget,"function",(gpointer)&func);
		}
	}
	if(func)
		res = func(widget,data);

	return res;
}
