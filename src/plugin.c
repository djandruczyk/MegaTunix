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
  \file src/plugin.c
  \ingroup CoreMtx
  \brief Functions specific to  plugin init/shutdown as well as symbol lookup
  within plugins for dereferencing addresses of functions
  \authors David Andruczyk
  */

#include <debugging.h>
#include <defines.h>
#include <notifications.h>
#include <plugin.h>
#include <stdio.h>
#include <threads.h>

extern gconstpointer *global_data;


/*!
  \brief initializes pointers to megatunix itself the core family plugin and
  the ECU specific plugin, then searches for and attempts to call the 
  "plugin_init()" function within each to initialize any datastructures, 
  handlers, threads within each plugin
  */
G_MODULE_EXPORT void plugins_init()
{
	GModule *module[NUM_MODULES] = {NULL,NULL,NULL};
	GThread *id = NULL;
#ifdef __WIN32__
	gchar * libname = NULL;
#endif 
	gchar * libpath = NULL;
	void (*plugin_init)(gconstpointer *);
	void (*common_gui_init)(void) = NULL;

	ENTER();
	/* MegaTunix itself */
	module[MAIN] = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (module[MAIN] == NULL)
		MTXDBG(CRITICAL,_("Unable to call g_module_open for MegaTunix itself, This usually means you forgot \"sudo make install\", error: %s\n"),g_module_error());
	DATA_SET_FULL(global_data,"megatunix_module",(gpointer)module[MAIN],g_module_close);

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
		if (module[COMMON] == NULL)
			MTXDBG(CRITICAL,_("Opening Common library module error, This usually means you forgot \"sudo make install; sudo ldconfig\" :\n\t%s\n"),g_module_error());
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
		if (module[ECU] == NULL)
			MTXDBG(CRITICAL,_("Opening ECU library module error, This usually means you forgot \"sudo make install; sudo ldconfig\" :\n\t%s\n"),g_module_error());
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
		if (g_module_symbol(module[COMMON],"plugin_init",(void **)&plugin_init))
			plugin_init(global_data);
	/* ECU Specific module init */
	if (module[ECU])

		if (g_module_symbol(module[ECU],"plugin_init",(void **)&plugin_init))
			plugin_init(global_data);

	if (get_symbol("common_gui_init",(void **)&common_gui_init))
		common_gui_init();

	/* Startup dispatcher thread */
	id =  g_thread_new("Dispatcher Thread",
			thread_dispatcher,
			NULL); /* Thread args */
	if (id)
		DATA_SET(global_data,"thread_dispatcher_id",id);
	EXIT();
	return;
}


/*!
  \brief Stops the thread_dispatcher thread, and lookups up "plugin_shutdown"
  within each plugin and calls them in turn to have them shutdown any resources
  or threads, in preparation for plugin unloading.
  */
G_MODULE_EXPORT void plugins_shutdown()
{
	GModule *module = NULL;
	void (*plugin_shutdown)(void);

	ENTER();
	/* Shutdown ECU module */
	module = (GModule *)DATA_GET(global_data,"ecu_module");
	if (module)
	{
		if (g_module_symbol(module,"plugin_shutdown",(void **)&plugin_shutdown))
			plugin_shutdown();
		DATA_SET(global_data,"ecu_module",NULL);
	}
	/* Shutdown Common module */
	module = (GModule *)DATA_GET(global_data,"common_module");
	if (module)
	{
		if (g_module_symbol(module,"plugin_shutdown",(void **)&plugin_shutdown))
			plugin_shutdown();
		DATA_SET(global_data,"common_module",NULL);
	}
	EXIT();
	return;
}


/*!
  \brief searches for a function name within megatunix, the core plugin and 
  the ecu specific plugin, if found, it sets the passed pointer to the 
  found function pointer as well as return TRUE
  \param name is the name of the function to find
  \param function_p is the pointer to be filled with the address of the function
  \returns TRUE on success, false on failure
  */
G_MODULE_EXPORT gboolean get_symbol(const gchar *name, void **function_p)
{
	GModule *module[NUM_MODULES] = {NULL,NULL,NULL};
	gint i = (gint)MAIN;
	gboolean found = FALSE;
	extern gconstpointer *global_data;

	ENTER();
	/* Megatunix itself */
	module[MAIN] = (GModule *)DATA_GET(global_data,"megatunix_module");
	/* Common library */
	module[COMMON] = (GModule *)DATA_GET(global_data,"common_module");
	/* ECU Specific library */
	module[ECU] = (GModule *)DATA_GET(global_data,"ecu_module");

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
		MTXDBG(CRITICAL,_("\nCRITICAL ERROR: Failed to find symbol/function\n\"%s\" in MegaTunix core or any plugins, returning FALSE, expect a crash\n"),name);
	EXIT();
	return found;
}
