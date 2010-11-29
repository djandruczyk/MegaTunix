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
#include <datamgmt.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <init.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <mode_select.h>
#include <notifications.h>
#include <plugin.h>
#include <rtv_processor.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>
#include <widgetmgmt.h>
#include <timeout_handlers.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT gboolean plugin_function(GtkWidget *widget, gpointer data)
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
			dbg_func(PLUGINS|CRITICAL,g_strdup_printf(_("ERROR, Cannot locate function \"%s\" within plugin %s\n"),func_name,(gchar *)DATA_GET(global_data,"ecu_library")));
	}
	if(func)
		res = func(widget,data);

	return res;
}


G_MODULE_EXPORT void plugin_init()
{
	GModule *module;

	void (*plugin_init)(gconstpointer *);

	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (!module)
		dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": plugin_init()\n\tUnable to call g_module_open for MegaTunix itself, error: %s\n",g_module_error()));
	DATA_SET(global_data,"error_msg_f",(gpointer)&error_msg);
	DATA_SET(global_data,"megatunix_module",(gpointer)module);

	if (g_module_symbol(DATA_GET(global_data,"plugin_module"),"plugin_init",(void *)&plugin_init))
		plugin_init(global_data);
}


G_MODULE_EXPORT void plugin_shutdown()
{
	void (*plugin_shutdown)(void);

	if (g_module_symbol(DATA_GET(global_data,"plugin_module"),"plugin_shutdown",(void *)&plugin_shutdown))
		plugin_shutdown();
}


G_MODULE_EXPORT gboolean get_symbol(const gchar *name, void **function_p)
{
	GModule **module = NULL;
	gint i = 0;
	gboolean found = FALSE;
#ifdef __WIN32__
	gchar * libname = NULL;
#endif
	gchar * libpath = NULL;
	extern gconstpointer *global_data;

	/* Mtx common and ecu specific libs */
	module = g_new0(GModule *, 3);
	module[0] = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (!module[0])
		dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": get_symbol()\n\tUnable to call g_module_open for MegaTunix itself, error: %s\n",g_module_error()));
	/* Common library */
	if (strlen((gchar *)DATA_GET(global_data,"common_lib")) > 1)
	{
#ifdef __WIN32__
		libname = g_strdup_printf("%s-0",(gchar *)DATA_GET(global_data,"common_lib"));
		libpath = g_module_build_path(MTXPLUGINDIR,libname);
		g_free(libname);
#else
		libpath = g_module_build_path(MTXPLUGINDIR,(gchar *)DATA_GET(global_data,"ecu_lib"));
#endif
		module[1] = g_module_open(libpath,G_MODULE_BIND_LAZY);
		g_free(libpath);
	}
	/* ECU Specific library */
	if (strlen((gchar *)DATA_GET(global_data,"ecu_lib")) > 1)
	{
#ifdef __WIN32__
		libname = g_strdup_printf("%s-0",(gchar *)DATA_GET(global_data,"ecu_lib"));
		libpath = g_module_build_path(MTXPLUGINDIR,libname);
		g_free(libname);
#else
		libpath = g_module_build_path(MTXPLUGINDIR,(gchar *)DATA_GET(global_data,"ecu_lib"));
#endif
		module[2] = g_module_open(libpath,G_MODULE_BIND_LAZY);
		g_free(libpath);
	}

	for (i=0;i<3;i++)
	{
		if ((module[i]) && (!found))
		{
			g_module_symbol(module[i],name,function_p);
			found = TRUE;
		}
	}
	if (!found)
		dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": get_symbol()\n\tError finding symbol \"%s\", error:\n\t%s\n",name,g_module_error()));
	for (i=0;i<3;i++)
	{
		if (!g_module_close(module[i]))
			dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": get_symbol()\n\t Failure calling \"g_module_close()\", error %s\n",g_module_error()));
	}
	g_free(module);
	return found;
}
