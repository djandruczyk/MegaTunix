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
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <dep_loader.h>
#include <enums.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <gmodule.h>
#include <keyparser.h>
#include <listmgmt.h>
#include <memory_gui.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>
#include <structures.h>
#include <tabloader.h>
#include <tag_loader.h>
#include <widgetmgmt.h>

gboolean tabs_loaded = FALSE;

gboolean load_gui_tabs()
{
	extern struct Firmware_Details * firmware;
	gint i = 0;
	ConfigFile *cfgfile = NULL;
	gchar * map_file = NULL;
	gchar * glade_file = NULL;
	gchar * tmpbuf = NULL;
	GladeXML *xml = NULL;
	GtkWidget *frame = NULL;
	gchar * tab_name = NULL;
	GtkWidget * label = NULL;
	GtkWidget *topframe = NULL;
	extern GtkWidget * notebook;

	if (!firmware)
		return FALSE;
	if (!firmware->tab_list)
		return FALSE;
	if (!firmware->tab_confs)
		return FALSE;


	while (firmware->tab_list[i])
	{
		glade_file = get_file(g_strconcat(GUI_DIR,"/",firmware->tab_list[i],".glade",NULL));
		map_file = get_file(g_strconcat(GUI_DIR,"/",firmware->tab_confs[i],".datamap",NULL));
		if ((g_file_test(glade_file,G_FILE_TEST_EXISTS)) &&
				(g_file_test(map_file,G_FILE_TEST_EXISTS)))
		{
			xml = glade_xml_new(glade_file,"topframe",NULL);
			cfgfile = cfg_open_file(map_file);
			if (cfgfile)
			{
				cfg_read_string(cfgfile,"global","tab_name",&tab_name);
				label = gtk_label_new_with_mnemonic(g_strdup(tab_name));
				topframe = glade_xml_get_widget(xml,"topframe");
				/* bind_data() is recursive and will take 
				 * care of all children
				 */
				bind_data(topframe,(gpointer)cfgfile);
				populate_master(topframe,(gpointer)cfgfile);

				dbg_func(g_strdup_printf(__FILE__": load_gui_tabs()\n\t Tab %s successfully loaded...\n\n",tab_name),TABLOADER);
				g_free(tab_name);

				frame = glade_xml_get_widget(xml,"topframe");
				if (frame == NULL)
				{
					dbg_func(__FILE__": load_gui_tabs()\n\t\"topframe\" not found in xml, ABORTING!!\n",CRITICAL);
					return FALSE;
				}
				else
				{
					gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
					glade_xml_signal_autoconnect(xml);
					gtk_widget_show_all(frame);
				}
				if (cfg_read_string(cfgfile,"global","post_function",&tmpbuf))
				{
					run_post_function(tmpbuf);
					g_free(tmpbuf);
				}
				cfg_free(cfgfile);
			}
			g_free(xml);
			g_free(map_file);
			g_free(glade_file);

		}
		else
		{
			dbg_func(g_strdup_printf(__FILE__": load_gui_tabs()\n\tDATAMAP: \"%s.datamap\" NOT FOUND\n",firmware->tab_list[i]),CRITICAL);
			dbg_func(g_strdup_printf(__FILE__": load_gui_tabs()\n\tGLADE FILE: \"%s.glade\" NOT FOUND\n",firmware->tab_list[i]),CRITICAL);
			g_free(map_file);
			g_free(glade_file);
		}
		i++;

	}
	tabs_loaded = TRUE;
	dbg_func(__FILE__": load_gui_tabs()\n\t All is well, leaving...\n\n",TABLOADER);
	return TRUE;

}


void bind_data(GtkWidget *widget, gpointer user_data)
{
	ConfigFile *cfgfile = (ConfigFile *)user_data;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** bind_keys = NULL;
	gint bind_num_keys = 0;
	gchar ** keys = NULL;
	gint * keytypes = NULL;
	gint num_keys = 0;
	gint num_keytypes = 0;
	gint i = 0;
	gint offset = 0;
	gint page = 0;
	gint tmpi = 0;
	extern GList ***ve_widgets;

	if (GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),bind_data,user_data);
	section = (char *)glade_get_widget_name(widget);
	if (section == NULL)
		return;
	if(cfg_read_string(cfgfile,section,"keys",&tmpbuf))
	{
		keys = parse_keys(tmpbuf,&num_keys,",");
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tNumber_keys for %s is %i\n",section,num_keys),TABLOADER);
		g_free(tmpbuf);
	}
	else
		return;

	if(cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
	{
		keytypes = parse_keytypes(tmpbuf, &num_keytypes,",");
		g_free(tmpbuf);
	}

	if (num_keytypes != num_keys)
	{
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tNumber of keys (%i) and keytypes(%i) does\n\tNOT match for widget %s, CRITICAL!!!\n",num_keys,num_keytypes,section),CRITICAL);
		g_strfreev(keys);
		g_free(keytypes);
		return;
	}
	page = -1;
	if (!cfg_read_int(cfgfile,section,"page",&page))
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tObject %s doesn't have a page assigned!!!!\n",section),CRITICAL);	

	/* Bind widgets to lists if thy have the bind_to_list flag set...
	*/
	if (cfg_read_string(cfgfile,section,"bind_to_list",&tmpbuf))
	{
		bind_keys = parse_keys(tmpbuf,&bind_num_keys,",");
		g_free(tmpbuf);
		/* This looks convoluted,  but it allows for an arbritrary 
		 * number of lists, that are indexed by a keyword.
		 * The get_list function looks the list up in a hashtable, if
		 * it isn't found (i.e. new list) it returns NULL which is OK
		 * as g_list_append() uses that to create a new list,  that
		 * returned list is used to store back into the hashtable so
		 * that the list is always stored and up to date...
		 */
		for (i=0;i<bind_num_keys;i++)
			store_list(bind_keys[i],g_list_append(get_list(bind_keys[i]),(gpointer)widget));
		g_strfreev(bind_keys);
	}
	/* If this widget has a "depend_on" tag we need to load the dependancy
	 * information  and store it for use when needed...
	 */
	if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
	{
		load_dependancy(G_OBJECT(widget),cfgfile,section);
		g_free(tmpbuf);
	}

	/* If this widget (a textview) has "create_tags" we call a special
	 * handler jsut for that..
	 */
	if (cfg_read_string(cfgfile,section,"create_tags",&tmpbuf))
	{
		load_tags(G_OBJECT(widget),cfgfile,section);
		g_free(tmpbuf);
	}

	/* If this widget (a label) has "set_lanel" we set the label on it
	 */
	if (cfg_read_string(cfgfile,section,"set_label",&tmpbuf))
	{
		gtk_label_set_text(GTK_LABEL(widget),tmpbuf);
		g_free(tmpbuf);
	}

	offset = -1;
	cfg_read_int(cfgfile,section,"offset",&offset);
	if (offset >= 0)
	{
		/* The way we do it now is to STORE widgets in LISTS for each
		 * offset, thus we can have multiple on screen controls bound
		 * to single data offset in the ECU
		 */
		ve_widgets[page][offset] = g_list_append(
				ve_widgets[page][offset],
				(gpointer)widget);
	}

	for (i=0;i<num_keys;i++)
	{
		switch((DataType)keytypes[i])
		{
			case MTX_INT:
				if (cfg_read_int(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tbinding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tMTX_INT: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_ENUM:
				if (cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tbinding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
					g_free(tmpbuf);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tMTX_ENUM: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_BOOL:
				if (cfg_read_boolean(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tbinding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
					if (strstr(keys[i],"ul_complex"))
						load_complex_params(G_OBJECT(widget),cfgfile,section);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tMTX_BOOL: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tbinding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[i],tmpbuf,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							g_strdup(tmpbuf));
					g_free(tmpbuf);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tMTX_STRING: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;

		}
	}
	if (cfg_read_string(cfgfile,section,"post_function_with_arg",&tmpbuf))
	{
		run_post_function_with_arg(tmpbuf,widget);
		g_free(tmpbuf);
	}
	if (cfg_read_string(cfgfile,section,"post_function",&tmpbuf))
	{
		run_post_function(tmpbuf);
		g_free(tmpbuf);
	}
	g_free(keytypes);
	g_strfreev(keys);
	dbg_func(__FILE__": bind_data()\n\t All is well, leaving...\n\n",TABLOADER);
}

void run_post_function(gchar * function_name)
{
	void (*function)(void);
	GModule *module = NULL;

	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (!module)
		dbg_func(g_strdup_printf(__FILE__": run_post_function()\n\tUnable to call g_module_open, error: %s\n",g_module_error()),CRITICAL);
	if (!g_module_symbol(module,function_name,(void *)&function))
	{
		dbg_func(g_strdup_printf(__FILE__": run_post_function()\n\tError finding symbol \"%s\", error:\n\t%s\n",function_name,g_module_error()),CRITICAL);
		if (!g_module_close(module))
			dbg_func(g_strdup_printf(__FILE__": run_post_function()\n\t Failure calling \"g_module_close()\", error %s\n",g_module_error()),CRITICAL);
	}
	else
	{
		function();
		if (!g_module_close(module))
			dbg_func(g_strdup_printf(__FILE__": run_post_function()\n\t Failure calling \"g_module_close()\", error %s\n",g_module_error()),CRITICAL);
	}
}



void run_post_function_with_arg(gchar * function_name, GtkWidget *widget)
{
	void (*function)(GtkWidget *);
	GModule *module = NULL;

	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	if (!module)
		dbg_func(g_strdup_printf(__FILE__": run_post_function_with_arg()\n\tUnable to call g_module_open, error: %s\n",g_module_error()),CRITICAL);
	if (!g_module_symbol(module,function_name,(void *)&function))
	{
		dbg_func(g_strdup_printf(__FILE__": run_post_function_with_arg()\n\tError finding symbol \"%s\", error:\n\t%s\n",function_name,g_module_error()),CRITICAL);
		if (!g_module_close(module))
			dbg_func(g_strdup_printf(__FILE__": run_post_function_with_arg()\n\t Failure calling \"g_module_close()\", error %s\n",g_module_error()),CRITICAL);
	}
	else
	{
		function(widget);
		if (!g_module_close(module))
			dbg_func(g_strdup_printf(__FILE__": run_post_function_with_arg()\n\t Failure calling \"g_module_close()\", error %s\n",g_module_error()),CRITICAL);
	}
}



