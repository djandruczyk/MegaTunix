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
  \file src/tabloader.c
  \ingroup CoreMtx
  \brief The  Tab loading/pre-rendering functions

  NOTE: This should be broken up into the plugins at some point to allow more
  flexibility in the gui layout of tabs and  not restrict things to 
  Glade+MTx Datamap .ini files
  \author David Andruczyk
  */

#include <args.h>
#include <config.h>
#include <configfile.h>
#include <combo_loader.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <glade/glade-parser.h>
#include <gui_handlers.h>
#include <init.h>
#include <keybinder.h>
#include <keyparser.h>
#include <listmgmt.h>
#include <notifications.h>
#include <plugin.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <tabloader.h>
#include <tag_loader.h>
#include <threads.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;
gboolean descend_tree(GladeWidgetInfo *info, ConfigFile *);

/*!
  \brief load_gui_tabs_pf() is called after interrogation completes 
  successfully. It's purpose is to load all the glade files and 
  datamaps as specified in the interrogation profile of the detected firmware. 
  */
G_MODULE_EXPORT gboolean load_gui_tabs_pf(void)
{
	gint i = 0;
	gint cur = 0;
	ConfigFile *cfgfile = NULL;
	gchar * map_file = NULL;
	gchar * glade_file = NULL;
	gchar * tmpbuf = NULL;
	gchar * tab_name = NULL;
	gchar * tab_ident = NULL;
	gboolean tmpi = FALSE;
	GtkWidget *label = NULL;
	GtkWidget *container = NULL;
	GtkWidget *child = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *item = NULL;
	TabInfo *tabinfo = NULL;
	GPtrArray *tabinfos = NULL;
	extern GdkColor red;
	gboolean * hidden_list = NULL;
	Firmware_Details *firmware = NULL;
	CmdLineArgs *args = NULL;
	gchar * pathstub = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	args = (CmdLineArgs *)DATA_GET(global_data,"args");

	if (DATA_GET(global_data,"tabs_loaded"))
	{
		EXIT();
		return FALSE;
	}
	if (!firmware)
	{
		EXIT();
		return FALSE;
	}
	if (!firmware->tab_list)
	{
		EXIT();
		return FALSE;
	}
	if (!firmware->tab_confs)
	{
		EXIT();
		return FALSE;
	}
	if (args->inhibit_tabs)
	{
		EXIT();
		return FALSE;
	}

	set_title(g_strdup(_("Loading Gui Tabs...")));
	notebook = lookup_widget("toplevel_notebook");
	hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");
	tabinfos = g_ptr_array_new();

	while (firmware->tab_list[i])
	{

		pathstub = g_build_filename(GUI_DATA_DIR,firmware->tab_list[i],NULL);
		glade_file = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"glade");
		g_free(pathstub);
		pathstub = g_build_filename(GUI_DATA_DIR,firmware->tab_confs[i],NULL);
		map_file = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,"datamap");
		g_free(pathstub);
		if (!g_file_test(glade_file,G_FILE_TEST_EXISTS))
		{
			MTXDBG(TABLOADER|CRITICAL,_("GLADE FILE: \"%s.glade\" NOT FOUND\n"),firmware->tab_list[i]);
			update_logbar("interr_view","warning",g_strdup(_("Glade File: ")),FALSE,FALSE,TRUE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.glade\"",firmware->tab_list[i]),FALSE,FALSE,TRUE);
			update_logbar("interr_view","warning",g_strdup(_("  is MISSING!\n")),FALSE,FALSE,TRUE);
			i++;
			continue;
		}
		if (!g_file_test(map_file,G_FILE_TEST_EXISTS))
		{
			MTXDBG(TABLOADER|CRITICAL,_("DATAMAP: \"%s.datamap\" NOT FOUND\n"),firmware->tab_confs[i]);
			update_logbar("interr_view","warning",g_strdup(_("Datamap File: ")),FALSE,FALSE,TRUE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.datamap\"",firmware->tab_confs[i]),FALSE,FALSE,TRUE);
			update_logbar("interr_view","warning",g_strdup(_("  is MISSING!\n")),FALSE,FALSE,TRUE);
			i++;
			continue;
		}
		cfgfile = cfg_open_file(map_file);
		if (cfgfile)
		{
			tabinfo = g_new0(TabInfo, 1);
			tabinfo->glade_file = g_strdup(glade_file);
			tabinfo->datamap_file = g_strdup(map_file);

			cfg_read_string(cfgfile,"global","tab_name",&tab_name);

			label = gtk_label_new(NULL);
			tabinfo->tab_label = label;
			gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),tab_name);
			if (cfg_read_boolean(cfgfile,"global","ellipsize",&tmpi))
			{
				if (tmpi)
				{
					OBJ_SET(label,"ellipsize_preferred",GINT_TO_POINTER(TRUE));
					if (DATA_GET(global_data,"ellipsize_tabs"))
						gtk_label_set_ellipsize(GTK_LABEL(label),PANGO_ELLIPSIZE_END);
				}
			}
			if (cfg_read_string(cfgfile,"global","bind_to_list",&tmpbuf))
			{
				OBJ_SET_FULL(label,"bind_to_list",g_strdup(tmpbuf),g_free);
				bind_to_lists(label,tmpbuf);
				g_free(tmpbuf);
				if (cfg_read_string(cfgfile,"global","match_type",&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					g_free(tmpbuf);
					OBJ_SET(label,"match_type",GINT_TO_POINTER(tmpi));
				}
			}
			gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
			container = gtk_vbox_new(1,0);
			if (cfg_read_string(cfgfile,"topframe","tab_ident",&tab_ident))
			{
				tmpi = translate_string(tab_ident);
				g_free(tab_ident);
				OBJ_SET(container,"tab_ident",GINT_TO_POINTER(tmpi));
			}
			g_free(tab_name);
			OBJ_SET_FULL(label,"glade_file",g_strdup(glade_file),cleanup);
			OBJ_SET_FULL(label,"datamap_file",g_strdup(map_file),cleanup);
			OBJ_SET(label,"not_rendered",GINT_TO_POINTER(TRUE));
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),container,label);
			gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook),container,TRUE);
			gtk_widget_show(container);
			cur = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))-1;
			tabinfo->page_num = cur;
			tabinfo->notebook = GTK_NOTEBOOK(notebook);
			if (hidden_list[cur] == TRUE)
			{
				child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),cur);
				label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook),child);
				gtk_widget_hide(child);
				gtk_widget_hide(label);
				item = lookup_widget("show_tab_visibility_menuitem");
				gtk_widget_modify_text(gtk_bin_get_child(GTK_BIN(item)),GTK_STATE_NORMAL,&red);
			}
			g_ptr_array_add(tabinfos,(gpointer)tabinfo);
		}
		cfg_free(cfgfile);
		g_free(map_file);
		g_free(glade_file);
		i++;

		if (!firmware)
			break;
	}
	preload_deps(tabinfos);
	DATA_SET_FULL(global_data,"tabinfos",tabinfos,dealloc_tabinfos);
	DATA_SET(global_data,"tabs_loaded",GINT_TO_POINTER(TRUE));
	MTXDBG(TABLOADER,_("All is well, leaving...\n\n"));
	set_title(g_strdup(_("Gui Tabs Loaded...")));
	gdk_flush();
	EXIT();
	return TRUE;
}


/*!
  \brief load_gui_tabs_pf() is called after interrogation completes 
  successfully. It's purpose is to load all the glade files and datamaps 
  as specified in the interrogation profile of the detected firmware. 
  \param notebook is the pointer to the notebook the new tab should be placed
  \param page is the page number to load
  \returns TRUE on success, FALSE on failure
  */
G_MODULE_EXPORT gboolean load_actual_tab(GtkNotebook *notebook, gint page)
{
	ConfigFile *cfgfile = NULL;
	gchar * map_file = NULL;
	gchar * glade_file = NULL;
	gchar * tmpbuf = NULL;
	GladeXML *xml = NULL;
	GtkWidget *label = NULL;
	GtkWidget *topframe = NULL;
	GtkWidget *placeholder = NULL;
	GHashTable *groups = NULL;
	GList *tab_widgets = NULL;
	BindGroup *bindgroup = NULL;
	extern GdkColor red;

	ENTER();
	placeholder =  gtk_notebook_get_nth_page(notebook,page);
	label = gtk_notebook_get_tab_label(notebook,placeholder);

	glade_file = (gchar *)OBJ_GET(label,"glade_file");
	map_file = (gchar *)OBJ_GET(label,"datamap_file");
	xml = glade_xml_new(glade_file,"topframe",NULL);
	g_return_val_if_fail(xml,FALSE);
	thread_update_logbar("interr_view",NULL,g_strdup(_("Load of tab: ")),FALSE,FALSE);
	thread_update_logbar("interr_view","info", g_strdup_printf("\"%s\"",glade_file),FALSE,FALSE);
	thread_update_logbar("interr_view",NULL,g_strdup(_(" completed.\n")),FALSE,FALSE);
	thread_update_logbar("interr_view",NULL,g_strdup(_("Load of tabconf: ")),FALSE,FALSE);
	thread_update_logbar("interr_view","info", g_strdup_printf("\"%s\"",map_file),FALSE,FALSE);
	cfgfile = cfg_open_file(map_file);
	if (cfgfile)
	{
		topframe = glade_xml_get_widget(xml,"topframe");
		if (topframe == NULL)
		{
			MTXDBG(TABLOADER|CRITICAL,_("\"topframe\" not found in xml, ABORTING!!\n"));
			set_title(g_strdup(_("ERROR Gui Tab XML problem, \"topframe\" element not found!!!")));
			EXIT();
			return FALSE;
		}
		OBJ_SET_FULL(topframe,"glade_xml",(gpointer)xml,g_object_unref);
		// bind_data() is recursive and will take 
		// care of all children

		bindgroup = g_new0(BindGroup,1);
		groups = load_groups(cfgfile);
		bindgroup->cfgfile = cfgfile;
		bindgroup->groups = groups;
		bindgroup->map_file = g_strdup(map_file);
/*		tab_widgets = g_list_prepend(tab_widgets,topframe);
		OBJ_SET(topframe,"tab_widgets",tab_widgets);
		*/
		bindgroup->topframe = topframe;
		bind_data(topframe,(gpointer)bindgroup);
		g_free(bindgroup->map_file);
		if (groups)
			g_hash_table_destroy(groups);
		groups = NULL;
		/* Clear not_rendered flag */
		OBJ_SET(label,"not_rendered",NULL);

		populate_master(topframe,(gpointer)cfgfile);

		gtk_box_pack_start(GTK_BOX(placeholder),topframe,TRUE,TRUE,0);
		OBJ_SET(placeholder,"topframe",topframe);
		glade_xml_signal_autoconnect(xml);
		g_free(bindgroup);
		if (cfg_read_string(cfgfile,"global","post_functions",&tmpbuf))
		{
			run_post_functions(tmpbuf);
			g_free(tmpbuf);
		}
		cfg_free(cfgfile);
		gtk_widget_show(topframe);
		/*printf("Current length of tab_widgets is %i\n",g_list_length(OBJ_GET(topframe,"tab_widgets")));*/
		thread_update_logbar("interr_view",NULL,g_strdup(_(" completed.\n")),FALSE,FALSE);
	}
	update_groups_pf();
	update_sources_pf();
	/* Allow gui to update as it should.... */
	gdk_flush();
	EXIT();
	return TRUE;
}



/*!
  \brief free's the data from the struct Group structure
  \param value is the pointer to the struct Group to be deallocated
  \see load_groups
  */
G_MODULE_EXPORT void group_free(gpointer value)
{
	Group *group = (Group *)value;
	gint i = 0;
	DataType keytype = MTX_INT;

	ENTER();
	for (i=0;i<group->num_keys;i++)
	{
		keytype = (DataType)translate_string(group->keys[i]);
		OBJ_SET(group->object,group->keys[i],NULL);
	}
	g_object_unref(group->object);
	g_strfreev(group->keys);
	g_free(group->keytypes);
	g_free(group);
	EXIT();
	return;
}

/*!
  \brief Called from the load_gui_tabs_pf function in order to
  load common settings for a group of controls.
  \param cfgfile is the pointer to the configuration file to read
  the group information from.
  \see group_free
  \see load_gui_tabs_pf
  \returns a GHashTable * to a newly created hashtable of the groups that were
  loaded. The groups are indexed in the hashtable by group name.
  */
G_MODULE_EXPORT GHashTable * load_groups(ConfigFile *cfgfile)
{
	gint x = 0;
	gint tmpi = 0;
	gchar * tmpbuf = NULL;
	gchar **groupnames = NULL;
	gchar *section = NULL;
	gint num_groups = 0;
	Group *group = NULL;
	GHashTable *groups = NULL;
	void (*load_dep_obj)(GObject *,ConfigFile *,const gchar *,const gchar *) = NULL;

	ENTER();
	if(cfg_read_string(cfgfile,"global","groups",&tmpbuf))
	{
		groupnames = parse_keys(tmpbuf,&num_groups,",");
		MTXDBG(TABLOADER,_("Number of groups to load settings for is %i\n"),num_groups);
		g_free(tmpbuf);
	}
	else
	{
		EXIT();
		return NULL;
	}


	groups = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,group_free);

	for (x=0;x<num_groups;x++)
	{
		/* Create structure and allocate ram for it */
		group = g_new0(Group, 1);
		section = g_strdup(groupnames[x]);
		if(cfg_read_string(cfgfile,section,"keys",&tmpbuf))
		{
			group->keys = parse_keys(tmpbuf,&group->num_keys,",");
			MTXDBG(TABLOADER,_("Number of keys for section %s is %i\n"),section,group->num_keys);
			g_free(tmpbuf);
		}
		else
		{
			MTXDBG(TABLOADER,_("\"keys\" key in section \"%s\" NOT found, aborting this group.\n"),section);
			g_free(group);
			g_free(section);
			continue;
		}

		group->object = (GObject *)g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref_sink(group->object);

		/* If this widget has a "depend_on" tag we need to 
		 * load the dependency information and store it for 
		 * use when needed...
		 */
		if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
		{
			if (get_symbol("load_dependencies_obj",(void **)&load_dep_obj))
				load_dep_obj(group->object,cfgfile,section,"depend_on");
			g_free(tmpbuf);
		}

		/* Adds on "default" options to any other groups */
		if (g_ascii_strcasecmp(section,"defaults") != 0)
			group->page = bind_group_data(cfgfile, group->object, groups, "defaults");

		if (cfg_read_int(cfgfile,section,"page",&tmpi))
			group->page = tmpi;

		/* Binds the rest of the settings, overriding any defaults */
		bind_keys(group->object,cfgfile,section,group->keys,group->num_keys);
		/* Store it in the hashtable... */
		g_hash_table_insert(groups,g_strdup(section),(gpointer)group);
		g_free(section);
	}
	g_strfreev(groupnames);
	if (group)
	{
		EXIT();
		return groups;
	}
	EXIT();
	return NULL;
}


/*!
  \brief bind_group_data() is called to bind data widget that is defined in
  a group. (saves from having to duplicate a large number of keys/values for 
  a big group of widgets) This function will set the necessary data on the 
  Gui object.
  \param cfg is the pointer to the config file object to read data from
  \param object is the widget to bind the data to
  \param groups is the hashtable that holds the group common data
  \param groupname is the textual name of the group to get the data for to
  be bound to the widget
  \returns the page of the group
  */
G_MODULE_EXPORT gint bind_group_data(ConfigFile *cfg, GObject *object, GHashTable *groups, const gchar *groupname)
{
	gint i = 0;
	gint tmpi = 0;
	Group *group = NULL;
	DataType keytype = MTX_STRING;

	ENTER();
	group = (Group *)g_hash_table_lookup(groups,groupname);
	if (!group)
	{
		MTXDBG(TABLOADER|CRITICAL,_("Group \"%s\" not found in file %s\n"),groupname,cfg->filename);
		EXIT();
		return -1;
	}
	/* Copy data from the group object to the */
	/* Grab hidden data if it exists */
	if (OBJ_GET(group->object, "dep_object"))
		OBJ_SET(object,"dep_object",OBJ_GET(group->object, "dep_object"));

	for (i=0;i<group->num_keys;i++)
	{
		keytype = (DataType)translate_string(group->keys[i]);
		switch((DataType)keytype)
		{
			case MTX_INT:
			case MTX_BOOL:
			case MTX_ENUM:
				tmpi = (GINT)OBJ_GET(group->object,group->keys[i]);
				OBJ_SET(object,group->keys[i],GINT_TO_POINTER(tmpi));
				if (strstr(group->keys[i], "temp_dep"))
				{
					OBJ_SET(object,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
				}
				break;
			case MTX_FLOAT:
				OBJ_SET_FULL(object,group->keys[i],g_memdup(OBJ_GET(group->object,group->keys[i]),sizeof(gfloat)),g_free);
				break;
			case MTX_STRING:
				OBJ_SET_FULL(object,group->keys[i],(gchar *)g_strdup((gchar *)OBJ_GET(group->object,group->keys[i])),g_free);
				if (OBJ_GET(object,"tooltip") != NULL)
					gtk_widget_set_tooltip_text((GtkWidget *)OBJ_GET(object,"self"),(gchar *)OBJ_GET(object,"tooltip"));
				if (OBJ_GET(group->object, "bind_to_list"))
					bind_to_lists((GtkWidget *)OBJ_GET(object,"self"),(gchar *)OBJ_GET(group->object, "bind_to_list"));
				break;
			default:
				break;
		}
	}
	EXIT();
	return group->page;
}


/*!
  \brief bind_to_lists() binds a widget to any number of string named lists.
  \param widget is the widget to bind to lists
  \param lists is the command seperated string list of lists to bind this
  widget into.
  */
G_MODULE_EXPORT void bind_to_lists(GtkWidget * widget, const gchar * lists)
{
	gint bind_num_keys = 0;
	gchar **tmpvector = NULL;
	GList *dest_list = NULL;
	gint i = 0;

	ENTER();
	if (!lists)
	{
		printf(_("Error, bind_to_lists(), lists is NULL\n"));
		EXIT();
		return;
	}
	tmpvector = parse_keys(lists,&bind_num_keys,",");

	/* This looks convoluted,  but it allows for an arbritrary 
	 * number of lists, that are indexed by a keyword.
	 * The get_list function looks the list up in a hashtable, if
	 * it isn't found (i.e. new list) it returns NULL which is OK
	 * as g_list_prepend() uses that to create a new list,  that
	 * returned list is used to store back into the hashtable so
	 * that the list is always stored and up to date...
	 */
	for (i=0;i<bind_num_keys;i++)
	{
		dest_list = get_list(tmpvector[i]);
		dest_list = g_list_prepend(dest_list,(gpointer)widget);

		store_list(tmpvector[i],dest_list);
	}
	g_strfreev(tmpvector);
	EXIT();
	return;
}


/*!
  \brief  Removed a pointer/widget from a list of lists
  \param lists is the comma separated list of list names
  \param data is the pointer to the item to remove from each list
  */
G_MODULE_EXPORT void remove_from_lists(const gchar * lists, gpointer data)
{
	gint i = 0;
	gint bind_num_keys = 0;
	gchar **tmpvector = NULL;
	GList *list = NULL;

	ENTER();
	if (!lists)
	{
		EXIT();
		return;
	}

	tmpvector = parse_keys(lists,&bind_num_keys,",");

	for (i=0;i<bind_num_keys;i++)
	{
		list = get_list(tmpvector[i]);
		list = g_list_remove(list,(gpointer)data);
		store_list(tmpvector[i],list);
	}
	g_strfreev(tmpvector);
	EXIT();
	return;
}

/*!
  \brief bind_data() is a recursive function that is called for every container
  widget in a glade frame and it's purpose is to search the datamap file passed
  for the widget names in the glade file and if it's fond in the datamap to
  load all the attribues listed and bind them to the object using GTK+'s
  object model.
  \param widget is the widget passed to load attributes on
  \param user_data is the pointer to a BingGroup structure.
  */
G_MODULE_EXPORT void bind_data(GtkWidget *widget, gpointer user_data)
{
	BindGroup *bindgroup = (BindGroup *)user_data;
	ConfigFile *cfgfile = bindgroup->cfgfile;
	GHashTable *groups = bindgroup->groups;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** keys = NULL;
	gint num_keys = 0;
	gint offset = 0;
	gint page = 0;
	gint index = 0;
	gchar * initializer = NULL;
	GdkColor color;
	gchar *size = NULL;
	gint count = 0;
	gint tmpi = 0;
	gboolean hidden = FALSE;
	gchar *ptr = NULL;
	gchar **vector = NULL;
	gchar **vec2 = NULL;
	void (*func)(void) = NULL;
	GList *list = NULL;
	GList *list2 = NULL;
	const gchar *name = NULL;
	gboolean indexed = FALSE;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets = NULL;
	GList *tab_widgets = NULL;
	void (*load_dep_obj)(GObject *, ConfigFile *,const gchar *,const gchar *) = NULL;

	ENTER();
	MTXDBG(TABLOADER,_("Entered"));
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(ecu_widgets);
	g_return_if_fail(firmware);

	if (!GTK_IS_WIDGET(widget))
	{
		EXIT();
		return;
	}

	if (GTK_IS_WIDGET(widget))
		if (GTK_IS_CONTAINER(widget))
			gtk_container_foreach(GTK_CONTAINER(widget),bind_data,user_data);
	name = gtk_widget_get_name(widget);
	if (!name)
	{
		EXIT();
		return;
	}

	if (NULL != (ptr = g_strrstr_len(name,strlen(name),"_of_")))
	{
		indexed = TRUE;
		ptr = g_strrstr_len(name,ptr-name,"_");
		tmpbuf = g_strdelimit(g_strdup(ptr),"_",' ');
		section = g_strndup(name,ptr-name);
		/*printf("(indexed) section is %s\n",section);*/
		gint result = sscanf(tmpbuf,"%d of %d",&index,&count);
		/*printf("sscanf result %i\n",result);*
		* printf("Found indexed value for \"%s\", index %i, count %i\n",tmpbuf,index,count);
		*/
		g_free(tmpbuf);
	}
	else
		section = g_strdup(name);

	if(cfg_read_string(cfgfile, section, "keys", &tmpbuf))
	{
		keys = parse_keys(tmpbuf,&num_keys,",");
		MTXDBG(TABLOADER,_("Number of keys for %s is %i\n"),section,num_keys);
		g_free(tmpbuf);
	}
	else 
	{
		g_free(section);
		EXIT();
		return;
	}

	page = -1;
	/* Store ptr to self in qdata, needed for bind_to_lists from groups*/
	OBJ_SET(widget,"self",widget);
	/* Bind the data in the "defaults" group per tab to EVERY var in that
	 * tab
	 */
	page = bind_group_data(cfgfile, G_OBJECT(widget), groups, "defaults");

	if(cfg_read_string(cfgfile, section, "group", &tmpbuf))
	{
		page = bind_group_data(cfgfile,G_OBJECT(widget),groups,tmpbuf);
		g_free(tmpbuf);
	}

	if ((!cfg_read_int(cfgfile, section, "page", &page)) && (page == -1))
	{
		MTXDBG(TABLOADER|CRITICAL,_("Object %s doesn't have a page assigned!!!!\n"),section);	

	}
	/* Bind widgets to lists if they have the bind_to_list flag set...
	 */
	tmpbuf = NULL;
	if (cfg_read_string(cfgfile, section, "bind_to_list", &tmpbuf))
	{
		bind_to_lists(widget, tmpbuf);
		g_free(tmpbuf);
	}
	if (cfg_read_boolean(cfgfile,section,"ellipsize",&tmpi))
	{
		if ((GTK_IS_LABEL(widget)) && (tmpi))
		{
			OBJ_SET(widget,"ellipsize_preferred",GINT_TO_POINTER(TRUE));
			if (DATA_GET(global_data,"ellipsize_tabs"))
				gtk_label_set_ellipsize(GTK_LABEL(widget),PANGO_ELLIPSIZE_END);
		}
	}

	/* Color selections */
	if (cfg_read_string(cfgfile, section, "active_fg", &tmpbuf))
	{
		gdk_color_parse(tmpbuf, &color);
		gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &color);
		g_free(tmpbuf);
	}
	if (cfg_read_string(cfgfile, section, "inactive_fg", &tmpbuf))
	{
		gdk_color_parse(tmpbuf, &color);
		gtk_widget_modify_fg(widget, GTK_STATE_INSENSITIVE, &color);
		g_free(tmpbuf);
	}

	/* If this widget has a "depend_on" tag we need to load the dependancy
	 * information  and store it for use when needed...
	 */
	if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
	{
		if (get_symbol("load_dependencies_obj",(void **)&load_dep_obj))
			load_dep_obj(G_OBJECT(widget),cfgfile,section,"depend_on");
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

	/* If this widget has "tooltip" set the tip on the widget */
	if (cfg_read_string(cfgfile,section,"tooltip",&tmpbuf))
	{
		gtk_widget_set_tooltip_text(widget,tmpbuf);
		g_free(tmpbuf);
	}

	/* If this widget (a label) has "set_label" we set the label on it
	 */
	if (cfg_read_string(cfgfile,section,"set_label",&tmpbuf))
	{
/*		printf("setting label on %s to \"%s\"\n",glade_get_widget_name(widget),tmpbuf);*/
		gtk_label_set_text(GTK_LABEL(widget),tmpbuf);
		g_free(tmpbuf);
	}

	/* If this widget is temp dependant, set the current units on it 
	 */
	if (cfg_read_string(cfgfile,section,"temp_dep",&tmpbuf))
	{
		OBJ_SET(widget,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
		g_free(tmpbuf);
	}

	/* If this widget has "register_as", register it with the supplied name
	 */
	if (cfg_read_string(cfgfile,section,"register_as",&tmpbuf))
	{
		register_widget(tmpbuf,widget);
		g_free(tmpbuf);
	}
	/* If this widget has visible_functions defined */
	if (cfg_read_string(cfgfile,section,"visible_functions",&tmpbuf))
	{
		vector = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
		for (guint i=0;i<g_strv_length(vector);i++)
		{
			vec2 = g_strsplit(vector[i],":",2);
			if (g_strv_length(vec2) != 2)
			{
				printf("ERROR in %s, visible_functions param is missing the framerate parameter (func:fps)\n",cfgfile->filename);
				g_strfreev(vec2);
				continue;
			}
			gint fps = (GINT)g_strtod(vec2[1],NULL);
			get_symbol(vec2[0],(void **)&func);
			if (func)
			{
				list = g_list_prepend(list,(gpointer)func);
				list2 = g_list_prepend(list2,GINT_TO_POINTER(fps));
			}
			g_strfreev(vec2);
		}
		g_strfreev(vector);
		OBJ_SET_FULL(widget,"func_list",list,g_list_free);
		OBJ_SET_FULL(widget,"func_fps_list",list2,g_list_free);
	}

	/* If this widget has "initializer" there's a global variable 
	 * with it's name on it 
	 */
	if (cfg_read_string(cfgfile,section,"initializer",&initializer))
	{
		gint widget_type = 0;
		if (!cfg_read_string(cfgfile,section,"widget_type",&tmpbuf))
			MTXDBG(TABLOADER|CRITICAL,_("Object %s has initializer, but no widget_type!!!!\n"),section);
		else
			widget_type = translate_string(tmpbuf);
		g_free(tmpbuf);
		switch (widget_type)
		{
			case MTX_RANGE:
				gtk_range_set_value(GTK_RANGE(widget),(GINT)DATA_GET(global_data,initializer));
				break;

			case MTX_SPINBUTTON:
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(GINT)DATA_GET(global_data,initializer));
				break;
			case MTX_ENTRY:
				gtk_entry_set_text(GTK_ENTRY(widget),(gchar *)DATA_GET(global_data,initializer));

			default:
				break;

		}
		g_free(initializer);
	}
	/* Hidden widgets have special handlers and should NOT be updated normally */
	cfg_read_boolean(cfgfile,section, "hidden", &hidden);
	offset = -1;
	cfg_read_int(cfgfile,section, "offset", &offset);
	if (offset >= 0 && indexed)
	{
		/*printf("indexed widget %s\n",name); */
		if (cfg_read_string(cfgfile, section, "size", &size))
		{
			offset += index * get_multiplier ((DataSize)translate_string (size));
			g_free(size);
		}
		else
		{
			if(OBJ_GET(widget, "size"))
			{
				offset += index * get_multiplier ((DataSize)(GINT)OBJ_GET(widget, "size"));
			}
			else
			{
				MTXDBG(TABLOADER|CRITICAL,_("Indexed Object %s has index and offset, but no size!!!!\n"),section);
				g_free(section);
				EXIT();
				return;
			}
		}
		/*printf("widget %s, offset %i\n",name,offset);*/
		OBJ_SET(widget,"offset",GINT_TO_POINTER(offset));
	}
	if (offset >= 0)
	{
		/* The way we do it now is to STORE widgets in LISTS for each
		 * offset, thus we can have multiple on screen controls bound
		 * to single data offset in the ECU
		 */
		if (page < 0)
		{
			MTXDBG(TABLOADER|CRITICAL,_("Attempting to append widget beyond bounds of Firmware Parameters,  there is a bug with this datamap widget %s, page %i, at offset %i...\n\n"),section,page,offset);
			g_free(section);
			EXIT();
			return;
		}
		if (page < firmware->total_pages)
		{
			if (offset >= firmware->page_params[page]->length)
				MTXDBG(TABLOADER|CRITICAL,_("Attempting to append widget beyond bounds of Firmware Parameters,  there is a bug with this datamap widget %s, at offset %i...\n\n"),section,offset);
			else if (!hidden)
			{

				tab_widgets = OBJ_GET(bindgroup->topframe,"tab_widgets");
				tab_widgets = g_list_prepend(tab_widgets, widget);
				OBJ_SET(bindgroup->topframe,"tab_widgets",tab_widgets);
				ecu_widgets[page][offset] = g_list_prepend(
						ecu_widgets[page][offset],
						(gpointer)widget);
			}
		}
		else
			MTXDBG(TABLOADER|CRITICAL,_("Attempting to append widget beyond bounds of Firmware Parameters, there is a bug with this datamap for widget %s, at page %i offset %i...\n\n"),section,page,offset);
	}

	/* If there is a "group" key in a section it means that it gets the
	 * rest of it's setting from the groupname listed.  This reduces
	 * redundant keys all throughout the file...
	 */
	bind_keys(G_OBJECT(widget), cfgfile, section, keys, num_keys);
	g_strfreev(keys);

	/* If this widget has the "choices" key (combobox)
	 */
	if (cfg_read_string(cfgfile,section,"choices",&tmpbuf))
	{
		combo_setup(G_OBJECT(widget),cfgfile,section);
		g_free(tmpbuf);
	}

	if (cfg_read_string(cfgfile,section,"post_functions_with_arg",&tmpbuf))
	{
		run_post_functions_with_arg(tmpbuf,widget);
		g_free(tmpbuf);
	}
	if (cfg_read_string(cfgfile,section,"post_functions",&tmpbuf))
	{
		run_post_functions(tmpbuf);
		g_free(tmpbuf);
	}
	if (cfg_read_boolean(cfgfile,section,"show_widget",&tmpi))
	{
		if (tmpi)
			gtk_widget_show(widget);
		else
			gtk_widget_hide(widget);
	}
	if (cfg_read_string(cfgfile,section,"set_tab_labels",&tmpbuf))
	{
		if (GTK_IS_NOTEBOOK(widget))
		{
			vector=g_strsplit(tmpbuf,",",-1);
			if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(widget)) == g_strv_length(vector))
			{
				for (int i=0;i<g_strv_length(vector);i++)
				{
					gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(widget),
							gtk_notebook_get_nth_page(GTK_NOTEBOOK(widget),i),
							vector[i]);
				}
			}
			g_strfreev(vector);
		}
		g_free(tmpbuf);
	}
	g_free(section);
	MTXDBG(TABLOADER,_("Leaving"));
	EXIT();
	return;
}


/*!
  \brief run_post_functions() is called to run a function AFTER tab loading.
  It'll search the exported symbols of MegaTunix for the function and if
  found execute it
  \param functions is the CSV list of functions to run
  */
G_MODULE_EXPORT void run_post_functions(const gchar * functions)
{
	ENTER();
	run_post_functions_with_arg(functions,NULL);
	EXIT();
	return;
}



/*!
  \brief run_post_functions_with_arg() is called to run a function AFTER 
  tab loading is complete. It'll search the exported symbols of MegaTunix 
  for the function and if found execute it with the passed widget as an
  argument.
  \param functions is the CSV list of functions to run
  \param widget is the pointer to widget to be passed to the function
  */
G_MODULE_EXPORT void run_post_functions_with_arg(const gchar * functions, GtkWidget *widget)
{
	void (*post_func_w_arg)(GtkWidget *) = NULL;
	void (*post_func)(void) = NULL;
	gchar ** vector = NULL;
	guint i = 0;
	ENTER();
	vector = g_strsplit(functions,",",-1);
	for (i=0;i<g_strv_length(vector);i++)
	{
		/* If widget defined, pass to post function */
		if (widget)
		{
			if (get_symbol(vector[i],(void **)&post_func_w_arg))
				post_func_w_arg(widget);
			else
				MTXDBG(TABLOADER|CRITICAL,_("Error finding symbol \"%s\", error:\n\t%s\n"),vector[i],g_module_error());
		}
		else /* If no widget find funct with no args.. */
		{
			if (get_symbol(vector[i],(void **)&post_func))
				post_func();
			else
				MTXDBG(TABLOADER|CRITICAL,_("Error finding symbol \"%s\", error:\n\t%s\n"),vector[i],g_module_error());
		}
	}
	g_strfreev(vector);
	EXIT();
	return;
}


/*!
  \brief Pre-loads the dependencies related to a tab/group of tabs
  \param data is the pointer to a GPtrArray list of tab infos, used to traverse 
  each tab config file to preload the needed interdependancy structures
  */
gboolean preload_deps(gpointer data)
{
	Firmware_Details *firmware = NULL;
	GPtrArray *array = (GPtrArray *)data;
	TabInfo *tabinfo = NULL;
	GladeInterface *iface = NULL;
	GladeWidgetInfo *info = NULL;
	ConfigFile *cfgfile = NULL;
	guint i = 0;	
	guint j = 0;	

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,FALSE);

	for (i=0;i<array->len;i++)
	{
		tabinfo = (TabInfo *)g_ptr_array_index(array,i);
		iface = glade_parser_parse_file(tabinfo->glade_file,NULL);
		cfgfile = cfg_open_file(tabinfo->datamap_file);
		if ((!cfgfile) || (!iface))
			continue;
		for(j=0;j<iface->n_toplevels;j++)
		{
			info = iface->toplevels[j];
			descend_tree(info,cfgfile);
		}
		glade_interface_destroy(iface);
		cfg_free(cfgfile);
	}
	io_cmd(firmware->get_all_command,NULL);
	EXIT();
	return FALSE; /* Make it not run again... */
}


/*!
  \brief descends into a GladeWidgetInfo tree looking for special case 
  widgets to handle
  \param info is the pointer to a GladeWidgetInfo structure
  \param cfgfile is the pointer to the corresponding datamap file
  \returns TRUE, unless eat end of the tree
  */
gboolean descend_tree(GladeWidgetInfo *info,ConfigFile *cfgfile)
{
	static GHashTable *widget_2_tab_hash = NULL;
	static ConfigFile *last_cfgfile = NULL;
	static gchar * prefix = NULL;
	gchar *groups = NULL;
	gchar *bitvals = NULL;
	gchar *source_key = NULL;
	gchar *source_values = NULL;
	gint bitval = 0;
	gint bitmask = 0;
	gint offset = 0;
	gint page = 0;
	/*gint canID = 0;*/
	DataSize size = MTX_U08;
	GObject *object = NULL;
	GList *list = NULL;
	guint i = 0;

	ENTER();
	if (!widget_2_tab_hash)
	{
		widget_2_tab_hash = (GHashTable *)DATA_GET(global_data,"widget_2_tab_hash");
		g_return_val_if_fail(widget_2_tab_hash,FALSE);
	}
	/*
	   if (!info->parent)
	   printf("%s is a TOPLEVEL\n",info->name);
	   else if (info->n_children == 0)
	   {
	   printf("%s is a BOTTOM WIDGET\n",info->name);
	   EXIT();
	   return FALSE;
	   }
	   else
	   printf("%s\n",info->name);

	   printf("widget %s has %i children\n",info->name,info->n_children);
	 */
	for (i=0;i<info->n_children;i++)
		descend_tree(info->children[i].child,cfgfile);

	if (last_cfgfile != cfgfile)
	{
		if (prefix)
			cleanup(prefix);
		if(!cfg_read_string(cfgfile,"global","id_prefix", &prefix))
			prefix = NULL;
		last_cfgfile = cfgfile;
	}
	if (cfg_find_section(cfgfile,info->name)) // This widget exists 
	{
		if (prefix)
			g_hash_table_insert(widget_2_tab_hash,g_strdup_printf("%s%s",prefix,info->name),g_strdup(cfgfile->filename));
		else
			g_hash_table_insert(widget_2_tab_hash,g_strdup(info->name),g_strdup(cfgfile->filename));
	}

	if (cfg_read_string(cfgfile,info->name,"source_values",&source_values))
	{
		if (!cfg_read_string(cfgfile,info->name,"source_key",&source_key))
		{
			MTXDBG(TABLOADER|CRITICAL,_("%s needs source_key\n"),info->name);
			EXIT();
			return TRUE;
		}
		if (!cfg_read_int(cfgfile,info->name,"offset",&offset))
		{
			MTXDBG(TABLOADER|CRITICAL,_("%s needs offset\n"),info->name);
			EXIT();
			return TRUE;
		}
		if (!cfg_read_int(cfgfile,info->name,"bitmask",&bitmask))
		{
			MTXDBG(TABLOADER|CRITICAL,_("%s needs bitmask\n"),info->name);
			EXIT();
			return TRUE;
		}
		if (!cfg_read_string(cfgfile,info->name,"bitvals",&bitvals))
		{
			if (!cfg_read_int(cfgfile,info->name,"bitval",&bitval))
			{
				MTXDBG(TABLOADER|CRITICAL,_("%s needs bitvals or bitval\n"),info->name);
				EXIT();
				return TRUE;
			}
		}
		if (!cfg_read_int(cfgfile,info->name,"page",&page))
		{
			if (!cfg_read_int(cfgfile,"defaults","page",&page))
			{
				MTXDBG(TABLOADER|CRITICAL,_("%s has no page defined!\n"),info->name);
				EXIT();
				return TRUE;
			}
		}
		object = (GObject *)g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref_sink(object);
		/*OBJ_SET(object,"canID",GINT_TO_POINTER(canID));*/
		OBJ_SET(object,"page",GINT_TO_POINTER(page));
		OBJ_SET(object,"offset",GINT_TO_POINTER(offset));
		OBJ_SET(object,"size",GINT_TO_POINTER(size));
		OBJ_SET(object,"bitmask",GINT_TO_POINTER(bitmask));
		if (bitvals)
			OBJ_SET_FULL(object,"bitvals",g_strdup(bitvals),g_free);
		else
			OBJ_SET(object,"bitval",GINT_TO_POINTER(bitval));
		OBJ_SET_FULL(object,"source_key",g_strdup(source_key),g_free);
		OBJ_SET_FULL(object,"source_values",g_strdup(source_values),g_free);
		list = (GList *)DATA_GET(global_data,"source_list");
		list = g_list_prepend(list,object);
		DATA_SET(global_data,"source_list",(gpointer)list);
		cleanup(groups);
		cleanup(bitvals);
		cleanup(source_key);
		cleanup(source_values);

	}
	if (cfg_read_string(cfgfile,info->name,"toggle_groups",&groups))
	{
		if (!cfg_read_int(cfgfile,info->name,"offset",&offset))
		{
			MTXDBG(TABLOADER|CRITICAL,_("%s needs offset\n"),info->name);
			EXIT();
			return TRUE;
		}
		if (!cfg_read_int(cfgfile,info->name,"bitmask",&bitmask))
		{
			MTXDBG(TABLOADER|CRITICAL,_("%s needs bitmask\n"),info->name);
			EXIT();
			return TRUE;
		}
		if (!cfg_read_string(cfgfile,info->name,"bitvals",&bitvals))
		{
			if (!cfg_read_int(cfgfile,info->name,"bitval",&bitval))
			{
				MTXDBG(TABLOADER|CRITICAL,_("%s needs bitvals or bitval\n"),info->name);
				EXIT();
				return TRUE;
			}
		}
		if (!cfg_read_int(cfgfile,info->name,"page",&page))
		{
			if (!cfg_read_int(cfgfile,"defaults","page",&page))
			{
				MTXDBG(TABLOADER|CRITICAL,_("%s has no page defined!\n"),info->name);
				EXIT();
				return TRUE;
			}
		}
		/*
		   if (!cfg_read_int(cfgfile,info->name,"canID",&canID))
		   {
		   if (!cfg_read_int(cfgfile,"defaults","canID",&canID))
		   {
		   printf("%s has no canID defined!\n",info->name);
		   EXIT();
		   return TRUE;
		   }
		   }
		 */
		object = (GObject *)g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref_sink(object);
		/*OBJ_SET(object,"canID",GINT_TO_POINTER(canID));*/
		OBJ_SET(object,"page",GINT_TO_POINTER(page));
		OBJ_SET(object,"offset",GINT_TO_POINTER(offset));
		OBJ_SET(object,"size",GINT_TO_POINTER(size));
		OBJ_SET(object,"bitmask",GINT_TO_POINTER(bitmask));
		if (bitvals)
			OBJ_SET_FULL(object,"bitvals",g_strdup(bitvals),g_free);
		else
			OBJ_SET(object,"bitval",GINT_TO_POINTER(bitval));
		OBJ_SET_FULL(object,"toggle_groups",g_strdup(groups),g_free);
		list = (GList *)DATA_GET(global_data,"toggle_group_list");
		list = g_list_prepend(list,object);
		DATA_SET(global_data,"toggle_group_list",(gpointer)list);
		cleanup(groups);
		cleanup(bitvals);
		cleanup(source_key);
		cleanup(source_values);
	}
	EXIT();
	return TRUE;
}


/*!
  \brief loads a tab when its clicked upon
  \param datamap is the datamap that goes with this tab
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean handle_dependant_tab_load(gchar * datamap)
{
	GPtrArray *tabinfos = NULL;
	guint i = 0;
	TabInfo *tabinfo = NULL;

	ENTER();
	g_return_val_if_fail(datamap,FALSE);
	tabinfos = (GPtrArray *)DATA_GET(global_data,"tabinfos");
	g_return_val_if_fail(tabinfos,FALSE);
	for (i=0;i<tabinfos->len;i++)
	{
		tabinfo = (TabInfo *)g_ptr_array_index(tabinfos,i);
		if (!tabinfo)
			continue;
		if (g_ascii_strcasecmp(tabinfo->datamap_file,datamap) == 0)
		{
			g_signal_handlers_block_by_func (G_OBJECT (tabinfo->notebook),
					(gpointer)notebook_page_changed,
					NULL);
			set_title(g_strdup(_("Rendering Tab...")));
			load_actual_tab(tabinfo->notebook,tabinfo->page_num);
			g_signal_handlers_unblock_by_func (G_OBJECT (tabinfo->notebook),
					(gpointer)notebook_page_changed,
					NULL);
			set_title(g_strdup(_("Tab Loaded")));
			EXIT();
			return TRUE;
		}
	}
	EXIT();
	return FALSE;
}

