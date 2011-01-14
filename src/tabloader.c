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
#include <widgetmgmt.h>

extern gconstpointer *global_data;


/*!
 \brief load_gui_tabs_pf() is called after interrogation completes successfully.
 It's purpose is to load all the glade files and datamaps as specified in the
 interrogation profile of the detected firmware. 
 */
G_MODULE_EXPORT gboolean load_gui_tabs_pf(void)
{
	gint i = 0;
	gint cur = 0;
	ConfigFile *cfgfile = NULL;
	gchar * map_file = NULL;
	gchar * glade_file = NULL;
	gchar * tmpbuf = NULL;
	GladeXML *xml = NULL;
	gchar * tab_name = NULL;
	gboolean tmpi = FALSE;
	GtkWidget *label = NULL;
	GtkWidget *topframe = NULL;
	GHashTable *groups = NULL;
	BindGroup *bindgroup = NULL;
	GtkWidget *child = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *item = NULL;
	extern GdkColor red;
	gboolean * hidden_list = NULL;
	Firmware_Details *firmware = NULL;
	CmdLineArgs *args = NULL;

	firmware = DATA_GET(global_data,"firmware");
	args = DATA_GET(global_data,"args");

	if (DATA_GET(global_data,"tabs_loaded"))
		return FALSE;
	if (!firmware)
		return FALSE;
	if (!firmware->tab_list)
		return FALSE;
	if (!firmware->tab_confs)
		return FALSE;
	if (args->inhibit_tabs)
		return FALSE;

	gdk_threads_enter();
	set_title(g_strdup(_("Loading Gui Tabs...")));
	bindgroup = g_new0(BindGroup,1);
	notebook = lookup_widget("toplevel_notebook");
	hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");

	while (firmware->tab_list[i])
	{
		glade_file = get_file(g_strconcat(GUI_DATA_DIR,PSEP,firmware->tab_list[i],NULL),g_strdup("glade"));
		map_file = get_file(g_strconcat(GUI_DATA_DIR,PSEP,firmware->tab_confs[i],NULL),g_strdup("datamap"));
		if (!g_file_test(glade_file,G_FILE_TEST_EXISTS))
		{
			dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": load_gui_tabs_pf()\n\tGLADE FILE: \"%s.glade\" NOT FOUND\n",firmware->tab_list[i]));
			update_logbar("interr_view","warning",_("Glade File: "),FALSE,FALSE,FALSE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.glade\"",firmware->tab_list[i]),FALSE,FALSE,TRUE);
			update_logbar("interr_view","warning",_("  is MISSING!\n"),FALSE,FALSE,FALSE);
			i++;
			continue;
		}
		if (!g_file_test(map_file,G_FILE_TEST_EXISTS))
		{
			dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": load_gui_tabs_pf()\n\tDATAMAP: \"%s.datamap\" NOT FOUND\n",firmware->tab_confs[i]));
			update_logbar("interr_view","warning",_("Datamap File: "),FALSE,FALSE,FALSE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.datamap\"",firmware->tab_confs[i]),FALSE,FALSE,TRUE);
			update_logbar("interr_view","warning",_("  is MISSING!\n"),FALSE,FALSE,FALSE);
			i++;
			continue;
		}
		update_logbar("interr_view",NULL,_("Load of tab: "),FALSE,FALSE,FALSE);
		update_logbar("interr_view","info", g_strdup_printf("\"%s.glade\"",firmware->tab_list[i]),FALSE,FALSE,TRUE);
		xml = glade_xml_new(glade_file,"topframe",NULL);
		cfgfile = cfg_open_file(map_file);
		if (cfgfile)
		{
			cfg_read_string(cfgfile,"global","tab_name",&tab_name);

			label = gtk_label_new(NULL);
			gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),_(tab_name));
			if (cfg_read_boolean(cfgfile,"global","ellipsize",&tmpi))
			{
				if (tmpi)
					gtk_label_set_ellipsize(GTK_LABEL(label),PANGO_ELLIPSIZE_END);
			}
			if (cfg_read_string(cfgfile,"global","bind_to_list",&tmpbuf))
			{
				OBJ_SET_FULL(label,"bind_to_list",g_strdup(tmpbuf),g_free);
				bind_to_lists(label,tmpbuf);
				g_free(tmpbuf);
			}
			gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
			topframe = glade_xml_get_widget(xml,"topframe");
			OBJ_SET_FULL(topframe,"glade_file",g_strdup(glade_file),g_free);
			OBJ_SET_FULL(label,"glade_file",g_strdup(glade_file),g_free);
			/* bind_data() is recursive and will take 
			 * care of all children
			 */
			groups = load_groups(cfgfile);
			bindgroup->cfgfile = cfgfile;
			bindgroup->groups = groups;
			bindgroup->map_file = g_strdup(map_file);
			bind_data(topframe,(gpointer)bindgroup);
			g_free(bindgroup->map_file);
			if (groups)
				g_hash_table_destroy(groups);
			groups = NULL;

			populate_master(topframe,(gpointer)cfgfile);

			dbg_func(TABLOADER,g_strdup_printf(__FILE__": load_gui_tabs_pf()\n\t Tab %s successfully loaded...\n\n",tab_name));
			g_free(tab_name);

			if (topframe == NULL)
			{
				dbg_func(TABLOADER|CRITICAL,g_strdup(__FILE__": load_gui_tabs_pf()\n\t\"topframe\" not found in xml, ABORTING!!\n"));
				set_title(g_strdup(_("ERROR Gui Tabs XML problem!!!")));
				gdk_threads_leave();
				return FALSE;
			}
			else
			{
				gtk_notebook_append_page(GTK_NOTEBOOK(notebook),topframe,label);
				gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook),topframe,TRUE);
				glade_xml_signal_autoconnect(xml);
				gtk_widget_show_all(topframe);
			}
			cur = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))-1;
			if (hidden_list[cur] == TRUE)
			{
				child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),cur);
				label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook),child);
				gtk_widget_hide(child);
				gtk_widget_hide(label);
				item = lookup_widget("show_tab_visibility_menuitem");
				gtk_widget_modify_text(GTK_BIN(item)->child,GTK_STATE_NORMAL,&red);

			}
			if (cfg_read_string(cfgfile,"global","post_functions",&tmpbuf))
			{
				run_post_functions(tmpbuf);
				g_free(tmpbuf);
			}
			cfg_free(cfgfile);
#ifndef DEBUG
			g_object_unref(xml);
#endif
			update_logbar("interr_view",NULL,_(" completed.\n"),FALSE,FALSE,FALSE);
		}
		else
		{
			update_logbar("interr_view","warning",_("\nDatamap File: "),FALSE,FALSE,FALSE);
			update_logbar("interr_view","info",g_strdup_printf("\"%s.datamap\"",firmware->tab_list[i]),FALSE,FALSE,TRUE);
			update_logbar("interr_view","warning",_(" Could not be processed!\n"),FALSE,FALSE,FALSE);
		}
		g_free(map_file);
		g_free(glade_file);

		i++;

		if (!firmware)
			break;

		/* Allow gui to update as it should.... */
		while (gtk_events_pending())
		{
			if (DATA_GET(global_data,"leaving"))
			{
				gdk_threads_leave();
				return FALSE;
			}
			gtk_main_iteration();
		}



	}
	update_logbar("interr_view","warning",_("Tab Loading Complete!"),FALSE,FALSE,FALSE);
	DATA_SET(global_data,"tabs_loaded",GINT_TO_POINTER(TRUE));
	dbg_func(TABLOADER,g_strdup(__FILE__": load_gui_tabs_pf()\n\t All is well, leaving...\n\n"));
	g_free(bindgroup);
	set_title(g_strdup(_("Gui Tabs Loaded...")));
	gdk_threads_leave();
	return TRUE;
}



/*!
 \brief group_free() free's the data from the struct Group structure
 \param value (gpointer) pointer to the struct Group to be deallocated
 \see load_groups
 */
G_MODULE_EXPORT void group_free(gpointer value)
{
	Group *group = value;
	gint i = 0;
	DataType keytype = MTX_INT;

	for (i=0;i<group->num_keys;i++)
	{
		keytype = translate_string(group->keys[i]);
		OBJ_SET(group->object,group->keys[i],NULL);
	}
	g_object_unref(group->object);
	g_strfreev(group->keys);
	g_free(group);
}

/*!
 \brief load_groups() is called from the load_gui_tabs_pf function in order to
 load common settings for a group of controls.
 \param cfgfile (ConfigFile *) the pointer to the configuration file to read
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

	if(cfg_read_string(cfgfile,"global","groups",&tmpbuf))
	{
		groupnames = parse_keys(tmpbuf,&num_groups,",");
		dbg_func(TABLOADER,g_strdup_printf(__FILE__": load_groups()\n\tNumber of groups to load settings for is %i\n",num_groups));
		g_free(tmpbuf);
	}
	else
		return NULL;


	groups = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,group_free);

	for (x=0;x<num_groups;x++)
	{
		/* Create structure and allocate ram for it */
		group = g_new0(Group, 1);
		section = g_strdup(groupnames[x]);
		if(cfg_read_string(cfgfile,section,"keys",&tmpbuf))
		{
			group->keys = parse_keys(tmpbuf,&group->num_keys,",");
			dbg_func(TABLOADER,g_strdup_printf(__FILE__": load_groups()\n\tNumber of keys for section %s is %i\n",section,group->num_keys));
			g_free(tmpbuf);
		}
		else
		{
			dbg_func(TABLOADER,g_strdup_printf(__FILE__": load_groups()\n\t\"keys\" key in section \"%s\" NOT found, aborting this group.\n",section));
			g_free(group);
			g_free(section);
			continue;
		}

		group->object = g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref_sink(group->object);

		/* If this widget has a "depend_on" tag we need to 
		 * load the dependency information and store it for 
		 * use when needed...
		 */
		if (cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
		{
			if (get_symbol("load_dependancies_obj",(void*)&load_dep_obj))
				load_dep_obj(group->object,cfgfile,section,"depend_on");
			g_free(tmpbuf);
		}

		/* Adds on "default" options to any other groups */
		if (g_strcasecmp(section,"defaults") != 0)
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
		return groups;
	else
		return NULL;
}


/*!
 \brief bind_group_data() is called to bind data widget that is defined in
 a group. (saves from having to duplicate a large number of keys/values for 
 a big group of widgets) This function will set the necessary data on the 
 Gui object.
 \param cfgfile
 \param object (GObject *) the widget to bind the data to
 \param groups (GHashTable *) the hashtable that holds the  group common data
 \param groupname (gchar *) textual name of the group to get the data for to
 be bound to the widget
 \returns the page of the group
 */
G_MODULE_EXPORT gint bind_group_data(ConfigFile *cfg, GObject *object, GHashTable *groups, gchar *groupname)
{
	gint i = 0;
	gint tmpi = 0;
	Group *group = NULL;
	DataType keytype = MTX_STRING;

	group = g_hash_table_lookup(groups,groupname);
	if (!group)
	{
		dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_group_data()\n\t group \"%s\" not found in file %s\n",groupname,cfg->filename));
		return -1;
	}
	/* Copy data from the group object to the */
	/* Grab hidden data if it exists */
	if (OBJ_GET(group->object, "dep_object"))
		OBJ_SET(object,"dep_object",OBJ_GET(group->object, "dep_object"));

	for (i=0;i<group->num_keys;i++)
	{
		keytype = translate_string(group->keys[i]);
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
			case MTX_STRING:
				OBJ_SET_FULL(object,group->keys[i],g_strdup(OBJ_GET(group->object,group->keys[i])),g_free);
				if (OBJ_GET(object,"tooltip") != NULL)
					gtk_widget_set_tooltip_text(OBJ_GET(object,"self"),(gchar *)_(OBJ_GET(object,"tooltip")));
				if (OBJ_GET(group->object, "bind_to_list"))
					bind_to_lists(OBJ_GET(object,"self"),(gchar *)OBJ_GET(group->object, "bind_to_list"));
				break;
			default:
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
G_MODULE_EXPORT void bind_to_lists(GtkWidget * widget, gchar * lists)
{
	gint bind_num_keys = 0;
	gchar **tmpvector = NULL;
	GList *dest_list = NULL;
	gint i = 0;

	if (!lists)
	{
		printf(_("Error, bind_to_lists(), lists is NULL\n"));
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
}


G_MODULE_EXPORT void remove_from_lists(gchar * lists, gpointer data)
{
	gint i = 0;
	gint bind_num_keys = 0;
	gchar **tmpvector = NULL;
	GList *list = NULL;

	if (!lists)
		return;

	tmpvector = parse_keys(lists,&bind_num_keys,",");

	for (i=0;i<bind_num_keys;i++)
	{
		list = get_list(tmpvector[i]);
		list = g_list_remove(list,(gpointer)data);
		store_list(tmpvector[i],list);
	}
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
G_MODULE_EXPORT void bind_data(GtkWidget *widget, gpointer user_data)
{
	BindGroup *bindgroup = user_data;
	ConfigFile *cfgfile = bindgroup->cfgfile;
	GHashTable *groups = bindgroup->groups;
	gchar * tmpbuf = NULL;
	gchar * section = NULL;
	gchar ** keys = NULL;
	gint table_num = 0;
	gint num_keys = 0;
	gint offset = 0;
	gint page = 0;
	gint widget_type = 0;
	gchar * initializer = NULL;
	GdkColor color;
	gchar *size = NULL;
	gint index = 0;
	gint count = 0;
	gint result = 0;
	gint tmpi = 0;
	gchar *ptr = NULL;
	gboolean indexed = FALSE;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets = NULL;
	void (*load_dep_obj)(GObject *, ConfigFile *,const gchar *,const gchar *) = NULL;

	ecu_widgets = DATA_GET(global_data,"ecu_widgets");
	firmware = DATA_GET(global_data,"firmware");

	if (!widget)
		return;

	if (GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),bind_data,user_data);
	if (widget->name == NULL)
		return;

	if (NULL != (ptr = g_strrstr_len(widget->name,strlen(widget->name),"_of_")))
	{
		indexed = TRUE;
		ptr = g_strrstr_len(widget->name,ptr-widget->name,"_");
		tmpbuf = g_strdelimit(g_strdup(ptr),"_",' ');
		section = g_strndup(widget->name,ptr-widget->name);
		/*printf("(indexed) section is %s\n",section);*/
		result = sscanf(tmpbuf,"%d of %d",&index,&count);
		/*printf("sscanf result %i\n",result);
		  printf("Found indexed value for \"%s\", index %i, count %i\n",tmpbuf,index,count); */
		g_free(tmpbuf);
	}
	else
		section = g_strdup(widget->name);

	if(cfg_read_string(cfgfile, section, "keys", &tmpbuf))
	{
		keys = parse_keys(tmpbuf,&num_keys,",");
		dbg_func(TABLOADER,g_strdup_printf(__FILE__": bind_data()\n\tNumber of keys for %s is %i\n",section,num_keys));
		g_free(tmpbuf);
	}
	else 
	{
		g_free(section);
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
		dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_data()\n\tObject %s doesn't have a page assigned!!!!\n",section));	

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
		if (tmpi)
			gtk_label_set_ellipsize(GTK_LABEL(widget),PANGO_ELLIPSIZE_END);
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
		if (get_symbol("load_dependancies_obj",(void*)&load_dep_obj))
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

	/* If this widget  has "tooltip" set the tip on the widget */
	if (cfg_read_string(cfgfile,section,"tooltip",&tmpbuf))
	{
		gtk_widget_set_tooltip_text(widget,_(tmpbuf));
		g_free(tmpbuf);
	}

	/* If this widget (a label) has "set_label" we set the label on it
	*/
	if (cfg_read_string(cfgfile,section,"set_label",&tmpbuf))
	{
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

	/* If this widget has "initializer" there's a global variable 
	 * with it's name on it 
	 */
	if (cfg_read_string(cfgfile,section,"initializer",&initializer))
	{
		if (!cfg_read_string(cfgfile,section,"widget_type",&tmpbuf))
			dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_data()\n\tObject %s has initializer, but no widget_type!!!!\n",section));	
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
	offset = -1;
	cfg_read_int(cfgfile,section, "offset", &offset);
	if (offset >= 0 && indexed)
	{
		/*printf("indexed widget %s\n",widget->name); */
		if (cfg_read_string(cfgfile, section, "size", &size))
		{

			offset += index * get_multiplier (translate_string (size));
			g_free(size);
		}
		else
		{
			if(OBJ_GET(widget, "size"))
			{
				offset += index * get_multiplier ((GINT) OBJ_GET(widget, "size"));
			}
			else
			{
				dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_data()\n\tIndexed Object %s has index and offset, but no size!!!!\n",section));     
				g_free(section);
				return;
			}
		}
		/*printf("widget %s, offset %i\n",widget->name,offset);*/
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
			dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_data()\n\t Attempting to append widget beyond bounds of Firmware Parameters,  there is a bug with this datamap widget %s, page %i, at offset %i...\n\n",section,page,offset));
			g_free(section);
			return;
		}
		if (page < firmware->total_pages)
		{
			if (offset >= firmware->page_params[page]->length)
				dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_data()\n\t Attempting to append widget beyond bounds of Firmware Parameters,  there is a bug with this datamap widget %s, at offset %i...\n\n",section,offset));
			else
			{
				/*printf("adding widget %s to ecu_widgets[%i][%i]\n",glade_get_widget_name(widget),page,offset);*/
				ecu_widgets[page][offset] = g_list_prepend(
						ecu_widgets[page][offset],
						(gpointer)widget);
			}
		}
		else
			dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": bind_data()\n\t Attempting to append widget beyond bounds of Firmware Parameters, there is a bug with this datamap for widget %s, at page %i offset %i...\n\n",section,page,offset));

	}
	/* If there is a "group" key in a section it means that it gets the
	 * rest of it's setting from the groupname listed.  This reduces
	 * redundant keys all throughout the file...
	 */

	bind_keys(G_OBJECT(widget), cfgfile, section, keys, num_keys);

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
	g_free(section);
	g_strfreev(keys);
	if (GTK_IS_ENTRY(widget))
	{
		if (NULL != (tmpbuf = OBJ_GET(widget,"table_num")))
		{
			table_num = (gint)strtol(tmpbuf,NULL,10);
			page = (gint)OBJ_GET(widget,"page");
			offset = (gint)OBJ_GET(widget,"offset");
			if ((page == firmware->table_params[table_num]->z_page) && ((offset >= firmware->table_params[table_num]->z_base) && (offset < firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount)))
				g_array_append_val(firmware->table_params[table_num]->table,widget);
		}
	}
	dbg_func(TABLOADER,g_strdup(__FILE__": bind_data()\n\t All is well, leaving...\n\n"));
}


/*!
 \brief run_post_functions() is called to run a function AFTER tab loading.
 It'll search the exported symbols of MegaTunix for the function and if
 found execute it
 \param functions (gchar *) CSV list of functions to run
 */
G_MODULE_EXPORT void run_post_functions(const gchar * functions)
{
	run_post_functions_with_arg(functions,NULL);
}



/*!
 \brief run_post_functions_with_arg() is called to run a function AFTER 
 tab loading is complete. It'll search the exported symbols of MegaTunix 
 for the function and if found execute it with the passed widget as an
 argument.
 \param functions (gchar *) CSV list of functions to run
 \param widget (GtkWidget *) pointer to widget to be passed to the function
 */
G_MODULE_EXPORT void run_post_functions_with_arg(const gchar * functions, GtkWidget *widget)
{
	void (*f_widget)(GtkWidget *) = NULL;
	void (*f_void)(void) = NULL;
	gchar ** vector = NULL;
	guint i = 0;
	vector = g_strsplit(functions,",",-1);
	for (i=0;i<g_strv_length(vector);i++)
	{
		/* If widget defined, pass to post function */
		if (widget)
		{
			if (get_symbol(vector[i],(void *)&f_widget))
				f_widget(widget);
			else
				dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": run_post_functions_with_arg()\n\tError finding symbol \"%s\", error:\n\t%s\n",vector[i],g_module_error()));
		}
		else /* If no widget find funct with no args.. */
		{
			if (get_symbol(vector[i],(void *)&f_void))
				f_void();
			else
				dbg_func(TABLOADER|CRITICAL,g_strdup_printf(__FILE__": run_post_functions_with_arg()\n\tError finding symbol \"%s\", error:\n\t%s\n",vector[i],g_module_error()));
		}
	}
	g_strfreev(vector);
}
