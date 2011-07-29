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

#include <debugging.h>
#include <defines.h>
#include <notifications.h>
#include <plugin.h>
#include <stdio.h>
#include <threads.h>

extern gconstpointer *global_data;

/*!
  \brief Attempts the resolve the function named within the widget. If 
  function_name is defined and the func ptr is null, we try and find it, if
  so store a ptr to it, else log the error. IF func found, run it and return
  the result of it.
  \param widget, widget containing the function name/ptr
  \param data, second arg to function that we call (if found )
  \returns result of function call or FALSE if func not found;
  */
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


/*!
  \brief initializes pointers to megatunix itself the core family plugin and
  the ECU specific plugin, then searches for and attempts to call the 
  "plugin_init()" function within each to initialize any datastructures, 
  handlers, threads within each plugin
  */
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
	module[MAIN] = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (!module[MAIN])
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugin_init()\n\tUnable to call g_module_open for MegaTunix itself, error: %s\n",g_module_error()));
	DATA_SET_FULL(global_data,"megatunix_module",(gpointer)module[0],g_module_close);

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
		module[COMMON] = g_module_open(libpath,G_MODULE_BIND_LAZY);
		if (!module[COMMON])
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugins_init()\n\tOpening Common library module error:\n\t%s\n",g_module_error()));
		g_free(libpath);
		DATA_SET_FULL(global_data,"common_module",(gpointer)module[COMMON],g_module_close);
	}
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
		module[ECU] = g_module_open(libpath,G_MODULE_BIND_LAZY);
		if (!module[ECU])
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": plugins_init()\n\tOpening ECU library module error:\n\t%s\n",g_module_error()));
		g_free(libpath);
		DATA_SET_FULL(global_data,"ecu_module",(gpointer)module[ECU],g_module_close);
	}

	/* Set pointer to error message function global data container is
	   passed to plugin(s) so they have access to get to global functions
	   by looking up the symbols
	 */
	DATA_SET(global_data,"error_msg_f",(gpointer)&error_msg);
	DATA_SET(global_data,"get_symbol_f",(gpointer)&get_symbol);

	/* Common module init */
	if (module[COMMON])
		if (g_module_symbol(module[COMMON],"plugin_init",(void *)&plugin_init))
			plugin_init(global_data);
	/* ECU Specific module init */
	if (module[ECU])

		if (g_module_symbol(module[ECU],"plugin_init",(void *)&plugin_init))
			plugin_init(global_data);

	if (get_symbol("common_gui_init",(void *)&common_gui_init))
		common_gui_init();

	/* Startup dispatcher thread */
	id =  g_thread_create(thread_dispatcher,
			NULL, /* Thread args */
			TRUE, /* Joinable */
			NULL); /*GError Pointer */
	if (id)
		DATA_SET(global_data,"thread_dispatcher_id",id);
}


/*!
  \brief Stops the thread_dispatcher thread, and lookups up "plugin_shutdown"
  within each plugin and calls them in turn to have them shutdown any resources
  or threads, in preparation for plugin unloading.
  */
G_MODULE_EXPORT void plugins_shutdown()
{
	GModule *module = NULL;
	GThread *id = NULL;
	void (*plugin_shutdown)(void);

	id = DATA_GET(global_data,"thread_dispatcher_id");
	if (id != NULL)
	{
		DATA_SET(global_data,"thread_dispatcher_exit",GINT_TO_POINTER(TRUE));
		g_thread_join(id);
		DATA_SET(global_data,"thread_dispatcher_id",NULL);
	}
	/* Shutdown ECU module */
	module = DATA_GET(global_data,"ecu_module");
	if (module)
	{
		if (g_module_symbol(module,"plugin_shutdown",(void *)&plugin_shutdown))
			plugin_shutdown();
		DATA_SET(global_data,"ecu_module",NULL);
	}
	/* Shutdown Common module */
	module = DATA_GET(global_data,"common_module");
	if (module)
	{
		if (g_module_symbol(module,"plugin_shutdown",(void *)&plugin_shutdown))
			plugin_shutdown();
		DATA_SET(global_data,"common_module",NULL);
	}
}


/*!
  \brief searches for a function name within megatunix, the core plugin and 
  the ecu specific plugin, if found, it sets the passed pointer to the 
  found function pointer as well as return TRUE
  \param name, the name of the function to find
  \param function_p, pointer to be filled with the address of the function
  \returns TRUE on success, false on failure
  */
G_MODULE_EXPORT gboolean get_symbol(const gchar *name, void **function_p)
{
	GModule *module[3] = {NULL,NULL,NULL};
	ModIndex i = MAIN;
	gboolean found = FALSE;
	extern gconstpointer *global_data;

	/* Megatunix itself */
	module[MAIN] = DATA_GET(global_data,"megatunix_module");
	/* Common library */
	module[COMMON] = DATA_GET(global_data,"common_module");
	/* ECU Specific library */
	module[ECU] = DATA_GET(global_data,"ecu_module");

	for (i=MAIN;i<NUM_MODULES;i++)
	{
		if (!module[i])
			printf("module %i is not found!\n",i);
		if ((module[i]) && (!found))
			if (g_module_symbol(module[i],name,function_p))
			{
				found = TRUE;
				/*printf("FOUND symbol %s (%p) in module %i\n",name,function_p,i);*/
			}
	}
	if (!found)
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": get_symbol()\n\tError finding symbol \"%s\" in any plugins\n",name));

	return found;
}
