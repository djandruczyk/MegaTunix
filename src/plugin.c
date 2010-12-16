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
	gchar * func_name = NULL;
	gboolean (*func)(GtkWidget *, gpointer) =  NULL;
	gboolean res = FALSE;

	func_name = (gchar *)OBJ_GET(widget,"function_name");
	func = (void *)OBJ_GET(widget,"function");

	if ((func_name) && (func == NULL))
	{
		if (get_symbol(func_name,(void *)&func))
			OBJ_SET(widget,"function",(gpointer)func);
		else
			dbg_func(PLUGINS|CRITICAL,g_strdup_printf(_("ERROR, Cannot locate function \"%s\" within plugin %s\n"),func_name,(gchar *)DATA_GET(global_data,"ecu_library")));
	}
	if(func)
		res = func(widget,data);

	return res;
}


G_MODULE_EXPORT void plugins_init()
{
	GModule *module[3] = {NULL,NULL,NULL};
	GThread *id = NULL;
#ifdef __WIN32__
	gchar * libname = NULL;
#endif 
	gchar * libpath = NULL;
	void (*plugin_init)(gconstpointer *);
	void (*common_gui_init)(void) = NULL;

	/* MegaTunix itself */
	module[0] = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (!module[0])
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugin_init()\n\tUnable to call g_module_open for MegaTunix itself, error: %s\n",g_module_error()));
	DATA_SET(global_data,"megatunix_module",(gpointer)module[0]);

	/* ECU library */
	if (DATA_GET(global_data,"ecu_lib"))
	{
#ifdef __WIN32__
		libname = g_strdup_printf("%s-0",(gchar *)DATA_GET(global_data,"ecu_lib"));
		libpath = g_module_build_path(MTXPLUGINDIR,libname);
		g_free(libname);
#else
		libpath = g_module_build_path(MTXPLUGINDIR,(gchar *)DATA_GET(global_data,"ecu_lib"));
#endif
		module[1] = g_module_open(libpath,G_MODULE_BIND_LAZY);
		if (!module[1])
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugins_init()\n\tOpening ECU library module error:\n\t%s\n",g_module_error()));
		g_free(libpath);
		DATA_SET(global_data,"ecu_module",(gpointer)module[1]);
	}

	/* Common Library */
	if (DATA_GET(global_data,"common_lib"))
	{
#ifdef __WIN32__
		libname = g_strdup_printf("%s-0",(gchar *)DATA_GET(global_data,"common_lib"));
		libpath = g_module_build_path(MTXPLUGINDIR,libname);
		g_free(libname);
#else
		libpath = g_module_build_path(MTXPLUGINDIR,(gchar *)DATA_GET(global_data,"common_lib"));
#endif
		module[2] = g_module_open(libpath,G_MODULE_BIND_LAZY);
		if (!module[2])
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugins_init()\n\tOpening Common library module error:\n\t%s\n",g_module_error()));
		g_free(libpath);
		DATA_SET(global_data,"common_module",(gpointer)module[2]);
	}

	/* Set pointer to error message function global data container is
	   passed to plugin(s) so they have access to get to global functions
	   by looking up the symbols
	 */
	DATA_SET(global_data,"error_msg_f",(gpointer)&error_msg);
	DATA_SET(global_data,"get_symbol_f",(gpointer)&get_symbol);

	/* Common module init */
	if (module[2])
		if (g_module_symbol(module[2],"plugin_init",(void *)&plugin_init))
			plugin_init(global_data);
	/* ECU Specific module init */
	if (module[1])

		if (g_module_symbol(module[1],"plugin_init",(void *)&plugin_init))
			plugin_init(global_data);

	if (get_symbol("common_gui_init",(void *)&common_gui_init))
		common_gui_init();

	/* Startup dispatcher thread */
	id =  g_thread_create(thread_dispatcher,
			NULL, /* Thread args */
			TRUE, /* Joinable */
			NULL); /*GError Pointer */
	DATA_SET(global_data,"thread_dispatcher_id",id);
}


G_MODULE_EXPORT void plugins_shutdown()
{
	GModule *module = NULL;
	GThread *id = NULL;
	void (*plugin_shutdown)(void);

	id = DATA_GET(global_data,"thread_dispatcher_id");
	if (id)
	{
		DATA_SET(global_data,"thread_dispatcher_exit",GINT_TO_POINTER(TRUE));
		g_thread_join(id);
	}
	/* Shutdown ECU module */
	module = DATA_GET(global_data,"ecu_module");
	if (module)
	{
		if (g_module_symbol(module,"plugin_shutdown",(void *)&plugin_shutdown))
			plugin_shutdown();
		if (!g_module_close(module))
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugins_shutdown()\n\tClosing module error:\n\t%s\n",g_module_error()));
		DATA_SET(global_data,"ecu_module",NULL);
	}
	/* Shutdown Common module */
	module = DATA_GET(global_data,"common_module");
	if (module)
	{
		if (g_module_symbol(module,"plugin_shutdown",(void *)&plugin_shutdown))
			plugin_shutdown();
		if (!g_module_close(module))
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugins_shutdown()\n\tClosing module error:\n\t%s\n",g_module_error()));
		DATA_SET(global_data,"common_module",NULL);
	}
}


G_MODULE_EXPORT gboolean get_symbol(const gchar *name, void **function_p)
{
	GModule *module[3];
	gint i = 0;
	gboolean found = FALSE;
	extern gconstpointer *global_data;

	/* Megatunix itself */
	module[0] = DATA_GET(global_data,"megatunix_module");
	/* Common library */
	module[1] = DATA_GET(global_data,"common_module");
	/* ECU Specific library */
	module[2] = DATA_GET(global_data,"ecu_module");

	for (i=0;i<3;i++)
	{
		if (!module[i])
			printf("module %i is not found!\n",i);
		if ((module[i]) && (!found))
			if (g_module_symbol(module[i],name,function_p))
				found = TRUE;
	}
	if (!found)
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": get_symbol()\n\tError finding symbol \"%s\" in any plugins\n",name));

	return found;
}
