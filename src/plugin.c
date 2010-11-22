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
#include <plugin.h>


extern gconstpointer *global_data;

gboolean plugin_function(GtkWidget *widget, gpointer data)
{
	GModule * module = NULL;
	gchar * func_name = NULL;
	gboolean (*func)(GtkWidget *, gpointer) =  NULL;
	gboolean res = FALSE;

	module = (GModule *)DATA_GET(global_data,"plugin_module");
	func_name = (gchar *)OBJ_GET(widget,"function_name");
	func = (void *)OBJ_GET(widget,"function");

	if ((func_name) && (func == NULL))
	{
		if (g_module_symbol(module,func_name,(void *)&func))
			OBJ_SET(widget,"function",(gpointer)func);
		else
			dbg_func(PLUGINS|CRITICAL,g_strdup_printf(_("ERROR, Cannot locate function \"%s\" within plugin %s\n"),func_name,DATA_GET(global_data,"ecu_library")));
	}
	if(func)
		res = func(widget,data);

	return res;
}
