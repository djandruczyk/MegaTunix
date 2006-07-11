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
#include <keybinder.h>
#include <keyparser.h>
#include <listmgmt.h>
#include <memory_gui.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>
#include <structures.h>
#include <tabloader.h>
#include <tag_loader.h>
#include <widgetmgmt.h>

gboolean tabs_loaded = FALSE;


/*!
 \brief load_gui_tabs() is called after interrogation completes successfully.
 It's purpose is to load all the glade files and datamaps as specified in the
 interrogation profile of the detected firmware. 
 */
gboolean load_gui_tabs(void)
{
	extern struct Firmware_Details * firmware;
	gint i = 0;
	ConfigFile *cfgfile = NULL;
	gchar * map_file = NULL;
	gchar * glade_file = NULL;
	gchar * tmpbuf = NULL;
	GladeXML *xml = NULL;
	gchar * tab_name = NULL;
	GtkWidget * label = NULL;
	GtkWidget *topframe = NULL;
	GHashTable *groups = NULL;
	struct BindGroup *bindgroup = NULL;
	extern GtkWidget * notebook;

	if (!firmware)
		return FALSE;
	if (!firmware->tab_list)
		return FALSE;
	if (!firmware->tab_confs)
		return FALSE;

	bindgroup = g_new0(struct BindGroup,1);

	while (firmware->tab_list[i])
	{
		glade_file = get_file(g_strconcat(GUI_DIR,"/",firmware->tab_list[i],NULL),g_strdup("glade"));
		map_file = get_file(g_strconcat(GUI_DIR,"/",firmware->tab_confs[i],NULL),g_strdup("datamap"));
		if (!g_file_test(glade_file,G_FILE_TEST_EXISTS))
		{
			dbg_func(g_strdup_printf(__FILE__": load_gui_tabs()\n\tGLADE FILE: \"%s.glade\" NOT FOUND\n",firmware->tab_list[i]),CRITICAL);
			update_logbar("interr_view","warning",g_strdup_printf("Glade File: "),FALSE,FALSE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.glade\"",firmware->tab_list[i]),FALSE,FALSE);
			update_logbar("interr_view","warning",g_strdup_printf("  is MISSING!\n"),FALSE,FALSE);
			i++;
			continue;
		}
		if (!g_file_test(map_file,G_FILE_TEST_EXISTS))
		{
			dbg_func(g_strdup_printf(__FILE__": load_gui_tabs()\n\tDATAMAP: \"%s.datamap\" NOT FOUND\n",firmware->tab_list[i]),CRITICAL);
			update_logbar("interr_view","warning",g_strdup_printf("Datamap File: "),FALSE,FALSE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.datamap\"",firmware->tab_confs[i]),FALSE,FALSE);
			update_logbar("interr_view","warning",g_strdup_printf("  is MISSING!\n"),FALSE,FALSE);
			i++;
			continue;
		}
		update_logbar("interr_view",NULL,g_strdup_printf("Load of tab: "),FALSE,FALSE);
		update_logbar("interr_view","info",g_strdup_printf("\"%s.glade\"",firmware->tab_list[i]),FALSE,FALSE);
		xml = glade_xml_new(glade_file,"topframe",NULL);
		cfgfile = cfg_open_file(map_file);
		if (cfgfile)
		{
			cfg_read_string(cfgfile,"global","tab_name",&tab_name);
			label = gtk_label_new_with_mnemonic(tab_name);
			topframe = glade_xml_get_widget(xml,"topframe");
			/* bind_data() is recursive and will take 
			 * care of all children
			 */
			groups = load_groups(cfgfile);
			bindgroup->cfgfile = cfgfile;
			bindgroup->groups = groups;
			bind_data(topframe,(gpointer)bindgroup);
			if (groups)
				g_hash_table_destroy(groups);

			populate_master(topframe,(gpointer)cfgfile);

			dbg_func(g_strdup_printf(__FILE__": load_gui_tabs()\n\t Tab %s successfully loaded...\n\n",tab_name),TABLOADER);
			g_free(tab_name);

			if (topframe == NULL)
			{
				dbg_func(g_strdup(__FILE__": load_gui_tabs()\n\t\"topframe\" not found in xml, ABORTING!!\n"),CRITICAL);
				return FALSE;
			}
			else
			{
				gtk_notebook_append_page(GTK_NOTEBOOK(notebook),topframe,label);
				glade_xml_signal_autoconnect(xml);
				gtk_widget_show_all(topframe);
			}
			if (cfg_read_string(cfgfile,"global","post_function",&tmpbuf))
			{
				run_post_function(tmpbuf);
				g_free(tmpbuf);
			}
			cfg_free(cfgfile);
			g_free(cfgfile);
		}
		else
		{
			update_logbar("interr_view","warning",g_strdup_printf("Datamap File: "),FALSE,FALSE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.datamap\"",firmware->tab_list[i]),FALSE,FALSE);
			update_logbar("interr_view","warning",g_strdup_printf(" Could not be processed!\n"),FALSE,FALSE);
		}
		g_free(map_file);
		g_free(glade_file);

		i++;

		/* Allow gui to update as it should.... */
		gdk_threads_enter();
		while (gtk_events_pending())
			gtk_main_iteration();
		gdk_threads_leave();

		update_logbar("interr_view",NULL,g_strdup_printf(" completed.\n"),FALSE,FALSE);

	}
	tabs_loaded = TRUE;
	dbg_func(g_strdup(__FILE__": load_gui_tabs()\n\t All is well, leaving...\n\n"),TABLOADER);
	g_free(bindgroup);
	return TRUE;
}



/*!
 \brief group_free() free's the data from the struct Group structure
 \param value (gpointer) pointer to the struct Group to be deallocated
 \see load_groups
 */
void group_free(gpointer value)
{
	struct Group *group = value;
	gint i = 0;

	for (i=0;i<group->num_keys;i++)
	{
		g_object_set_data(group->object,group->keys[i],NULL);
	}
	g_strfreev(group->keys);
	g_free(group->keytypes);
	g_free(group);
}

/*!
 \brief load_groups() is called from the load_gui_tabs function in order to
 load common settings for a group of controls.
 \param cfgfile (ConfigFile *) the pointer to the configuration file to read
 the group information from.
 \see group_free
 \see load_gui_tabs
 \returns a GHashTable * to a newly created hashtable of the groups that were
 loaded. The groups are indexed in the hashtable by group name.
 */
GHashTable * load_groups(ConfigFile *cfgfile)
{
	gint x = 0;
	gint tmpi = 0;
	gchar * tmpbuf = NULL;
	gchar **groupnames = NULL;
	gchar *section = NULL;
	gint num_groups = 0;
	struct Group *group = NULL;
	GHashTable *groups = NULL;

	if(cfg_read_string(cfgfile,"global","groups",&tmpbuf))
	{
		groupnames = parse_keys(tmpbuf,&num_groups,",");
		dbg_func(g_strdup_printf(__FILE__": load_groups()\n\tNumber of groups to load settigns for is %i\n",num_groups),TABLOADER);
		g_free(tmpbuf);
	}
	else
		return NULL;

	groups = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,group_free);

	for (x=0;x<num_groups;x++)
	{
		/* Create structure and allocate ram for it */
		group = g_new0(struct Group, 1);
		section = g_strdup(groupnames[x]);
		if(cfg_read_string(cfgfile,section,"keys",&tmpbuf))
		{
			group->keys = parse_keys(tmpbuf,&group->num_keys,",");
			dbg_func(g_strdup_printf(__FILE__": load_groups()\n\tNumber of keys for section %s is %i\n",section,group->num_keys),TABLOADER);
			g_free(tmpbuf);
		}
		else
		{
			dbg_func(g_strdup_printf(__FILE__": load_groups()\n\t\"keys\" section NOT found, aborting this group %s\n",section),TABLOADER);
			continue;
		}
		if(cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
		{
			group->keytypes = parse_keytypes(tmpbuf,&group->num_keytypes,",");
			dbg_func(g_strdup_printf(__FILE__": load_groups()\n\tNumber of keytypes for section %s is %i\n",section,group->num_keytypes),TABLOADER);
			g_free(tmpbuf);
		}
		else
		{
			dbg_func(g_strdup_printf(__FILE__": load_groups()\n\t\"key_types\" section NOT found, aborting this group %s\n",section),TABLOADER);
			continue;
		}

		if (group->num_keytypes != group->num_keys)
		{
			dbg_func(g_strdup_printf(__FILE__": load_groups()\n\tNumber of keys (%i) and keytypes(%i) does\n\tNOT match for widget %s in file %s, CRITICAL!!!\n",group->num_keys,group->num_keytypes,section,cfgfile->filename),CRITICAL);
			g_strfreev(group->keys);
			g_free(group->keytypes);
			return NULL;

		}
		if (cfg_read_int(cfgfile,section,"page",&tmpi))
			group->page = tmpi;

		group->object = g_object_new(GTK_TYPE_INVISIBLE,NULL);
		// ATTEMPTED FIX FOR GLIB 2.10
		g_object_ref(group->object);
		gtk_object_sink(GTK_OBJECT(group->object));
		// ATTEMPTED FIX FOR GLIB 2.10

		bind_keys(group->object,cfgfile,section,group->keys,group->keytypes,group->num_keys);
		/* Store it in the hashtable... */
		g_hash_table_insert(groups,g_strdup(section),(gpointer)group);
		g_free(section);
	}
	g_strfreev(groupnames);
	return groups;
}


/*!
 \brief bind_group_data() is called to bind data widget that is defined in
 a group. (saves from having to duplicate a large number of keys.values for 
 a big group of widgets) This function will set the necessary data on the 
 Gui object.
 \param widget (GtkWidget *) the widget to bind the data to
 \param groups (GHashTable *) the hashtable that holds the  group common data
 \param groupname (gchar *) textual name of the group to get the data for to
 be bound to the widget
 \returns the page of the group
 */
gint bind_group_data(GtkWidget *widget, GHashTable *groups, gchar *groupname)
{
	gint i = 0;
	gint tmpi = 0;
	struct Group *group = NULL;
	extern GtkTooltips *tip;

	group = g_hash_table_lookup(groups,groupname);
	if (!group)
	{
		dbg_func(g_strdup_printf(__FILE__": bind_group_data()\n\t group \"%s\" not found in hashtable\n",groupname),CRITICAL);
		return -1;
	}
	/* Copy data from the group object to the */
	for (i=0;i<group->num_keys;i++)
	{
		switch((DataType)group->keytypes[i])
		{
			case MTX_INT:
			case MTX_BOOL:
			case MTX_ENUM:
				tmpi = (gint)g_object_get_data(group->object,group->keys[i]);
				g_object_set_data(G_OBJECT(widget),group->keys[i],GINT_TO_POINTER(tmpi));
				break;
			case MTX_STRING:
				g_object_set_data(G_OBJECT(widget),group->keys[i],g_strdup(g_object_get_data(group->object,group->keys[i])));
				if (g_object_get_data(G_OBJECT(widget),"tooltip") != NULL)
					gtk_tooltips_set_tip(tip,widget,(gchar *)g_object_get_data(G_OBJECT(widget),"tooltip"),NULL);
				if (g_object_get_data(G_OBJECT(group->object), "bind_to_list"))
					bind_to_lists(widget,(gchar *)g_object_get_data(G_OBJECT(group->object), "bind_to_list"));
				break;
		}
	}
	return group->page;
}


/*!
 \brief bind_to_lists() binds a widget to any number of string named lists.
 \param widget (GtkWidget *) widget to bind to lists
 \param lists (gchar *) command seperated string list of lists to bind this
 widget into.
 \returns void
 */
void bind_to_lists(GtkWidget * widget, gchar * lists)
{
	gint bind_num_keys = 0;
	gchar **tmpvector = NULL;
	gint i = 0;

	tmpvector = parse_keys(lists,&bind_num_keys,",");

	/* This looks convoluted,  but it allows for an arbritrary 
	 * number of lists, that are indexed by a keyword.
	 * The get_list function looks the list up in a hashtable, if
	 * it isn't found (i.e. new list) it returns NULL which is OK
	 * as g_list_append() uses that to create a new list,  that
	 * returned list is used to store back into the hashtable so
	 * that the list is always stored and up to date...
	 */
	for (i=0;i<bind_num_keys;i++)
		store_list(tmpvector[i],g_list_append(get_list(tmpvector[i]),(gpointer)widget));
	g_strfreev(tmpvector);
}

/*!
 \brief bind_data() is a recursive function that is called for every container
 widget in a glade frame and it's purpose is to search the datamap file passed
 for the widget names in the glade file and if it's fond in the datamap to
 load all the attribues listed and bind them to the object using GTK+'s
 object model.
 \param widget (GtkWidget *) widget passed to load attributes on
 \param user_data (gpointer) pointer to a BingGroup structure.
 */
void bind_data(GtkWidget *widget, gpointer user_data)
{
	struct BindGroup *bindgroup = user_data;
	ConfigFile *cfgfile = bindgroup->cfgfile;
	GHashTable *groups = bindgroup->groups;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** keys = NULL;
	gint * keytypes = NULL;
	gint num_keys = 0;
	gint num_keytypes = 0;
	gint offset = 0;
	gint page = 0;
	GdkColor color;
	extern GtkTooltips *tip;
	extern GList ***ve_widgets;
	extern struct Firmware_Details *firmware;


	if (GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),bind_data,user_data);
	section = (char *)glade_get_widget_name(widget);
	if (section == NULL)
		return;
	if(cfg_read_string(cfgfile,section,"keys",&tmpbuf))
	{
		keys = parse_keys(tmpbuf,&num_keys,",");
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tNumber of keys for %s is %i\n",section,num_keys),TABLOADER);
		g_free(tmpbuf);
	}
	else
		return;

	if(cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
	{
		keytypes = parse_keytypes(tmpbuf, &num_keytypes,",");
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tNumberk of keytypes for %s is %i\n",section,num_keys),TABLOADER);
		g_free(tmpbuf);
	}
	else
		return;

	if (num_keytypes != num_keys)
	{
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tNumber of keys (%i) and keytypes(%i) does\n\tNOT match for widget %s, CRITICAL!!!\n",num_keys,num_keytypes,section),CRITICAL);
		g_strfreev(keys);
		g_free(keytypes);
		return;
	}
	page = -1;
	if (cfg_read_string(cfgfile,section,"group",&tmpbuf))
	{
		page = bind_group_data(widget,groups,tmpbuf);
		g_free(tmpbuf);
	}

	if ((!cfg_read_int(cfgfile,section,"page",&page)) && (page == -1))
		dbg_func(g_strdup_printf(__FILE__": bind_data()\n\tObject %s doesn't have a page assigned!!!!\n",section),CRITICAL);	

	/* Bind widgets to lists if thy have the bind_to_list flag set...
	*/
	if (cfg_read_string(cfgfile,section,"bind_to_list",&tmpbuf))
	{
		bind_to_lists(widget,tmpbuf);
		g_free(tmpbuf);
	}

	/* Color selections */
	if (cfg_read_string(cfgfile,section,"active_fg",&tmpbuf))
	{
		gdk_color_parse(tmpbuf,&color);
		gtk_widget_modify_fg(widget,GTK_STATE_NORMAL,&color);
		g_free(tmpbuf);
	}
	if (cfg_read_string(cfgfile,section,"inactive_fg",&tmpbuf))
	{
		gdk_color_parse(tmpbuf,&color);
		gtk_widget_modify_fg(widget,GTK_STATE_INSENSITIVE,&color);
		g_free(tmpbuf);
	}

	/* If this widget has a "depend_on" tag we need to load the dependancy
	 * information  and store it for use when needed...
	 */
	if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
	{
		load_dependancies(G_OBJECT(widget),cfgfile,section);
		g_free(tmpbuf);
	}

	/* If this widget (a textview) has "create_tags" we call a special
	 * handler just for that..
	 */
	if (cfg_read_string(cfgfile,section,"create_tags",&tmpbuf))
	{
		load_tags(G_OBJECT(widget),cfgfile,section);
		g_free(tmpbuf);
	}

	/* If this widget  has "tooltip" set the tip on the widget */
	if (cfg_read_string(cfgfile,section,"tooltip",&tmpbuf))
	{
		gtk_tooltips_set_tip(tip,widget,tmpbuf,NULL);
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
		if (page < firmware->total_pages)
		{
			if (offset > firmware->page_params[page]->length)
			{
				dbg_func(g_strdup_printf(__FILE__": bind_data()\n\t Attempting to append widget beyond bounds of Firmware Parameters,  ther eis a bug with this datamap widgt %s, at offset %i...\n\n",section,offset),CRITICAL);
			}
			else
				ve_widgets[page][offset] = g_list_append(
						ve_widgets[page][offset],
						(gpointer)widget);
		}
		else
			dbg_func(g_strdup_printf(__FILE__": bind_data()\n\t Attempting to append widget beyond bounds of Firmware Parameters, there is a bug with this datamap for widget %s, at page %i offset %i...\n\n",section,page,offset),CRITICAL);


	}
	/* If there is a "group" key in a section it means that it gets the
	 * rest of it's setting from the groupname listed.  This reduces
	 * redundant keys all throughout the file...
	 */

	bind_keys(G_OBJECT(widget),cfgfile,section,keys,keytypes,num_keys);


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
	dbg_func(g_strdup(__FILE__": bind_data()\n\t All is well, leaving...\n\n"),TABLOADER);
}


/*!
 \brief run_post_function() is called to run a function AFTER tab loading.
 It'll search the exported symbols of MegaTunix for the function and if
 found execute it
 \param function_name (gchar *) textual name of the function to run.
 */
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



/*!
 \brief run_post_function_with_arg() is called to run a function AFTER 
 tab loading is complete. It'll search the exported symbols of MegaTunix 
 for the function and if found execute it with the passed widget as an
 argument.
 \param function_name (gchar *) textual name of the function to run.
 \param widget (GtkWidget *) pointer to widget to be passed to the function
 */
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
