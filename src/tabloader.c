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
#include <enums.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <stringmatch.h>
#include <structures.h>
#include <tabloader.h>

gboolean tabs_loaded = FALSE;
GHashTable *dynamic_widgets = NULL;
static GHashTable *lists_hash = NULL;

gboolean load_gui_tabs()
{
	extern struct Firmware_Details * firmware;
	gint i = 0;
	ConfigFile *cfgfile = NULL;
	gchar * map_file = NULL;
	gchar * glade_file = NULL;
	GladeXML *xml = NULL;
	GtkWidget *frame = NULL;
	gchar * tmpbuf = NULL;
	GtkWidget * label = NULL;
	GtkWidget *topframe = NULL;
	extern GtkWidget * notebook;

	if (!firmware->tab_list[i])
		return FALSE;

	while (firmware->tab_list[i])
	{
		glade_file = get_file(g_strconcat(GUI_DIR,"/",firmware->tab_list[i],".glade",NULL));
		map_file = get_file(g_strconcat(GUI_DIR,"/",firmware->tab_list[i],".datamap",NULL));
		if ((g_file_test(glade_file,G_FILE_TEST_EXISTS)) &&
				(g_file_test(map_file,G_FILE_TEST_EXISTS)))
		{
			xml = glade_xml_new(glade_file,"topframe",NULL);
			cfgfile = cfg_open_file(map_file);
			if (cfgfile)
			{
				cfg_read_string(cfgfile,"global","tab_name",&tmpbuf);
				label = gtk_label_new_with_mnemonic(g_strdup(tmpbuf));
				g_free(tmpbuf);
				topframe = glade_xml_get_widget(xml,"topframe");
				gtk_container_foreach(GTK_CONTAINER(topframe),bind_data,(gpointer)cfgfile);
				gtk_container_foreach(GTK_CONTAINER(topframe),populate_master,NULL);
				cfg_free(cfgfile);
			}
			else
				dbg_func(g_strdup_printf("DATAMAP: %s NOT FOUND\n",map_file),CRITICAL);

			g_free(map_file);
			g_free(glade_file);
			frame = glade_xml_get_widget(xml,"topframe");
			if (frame == NULL)
				dbg_func(__FILE__": load_gui_tabs() \"topframe\" not found in xml\n",CRITICAL);

			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
			gtk_widget_show_all(frame);
			glade_xml_signal_autoconnect(xml);
			g_free(xml);

		}
		else
			dbg_func(__FILE__": load_gui_tabs() .glade/.datamap file NOT FOUND!! \n",CRITICAL);
		i++;

	}
	tabs_loaded = TRUE;
	return TRUE;

}

void populate_master(GtkWidget *widget, gpointer user_data)
{
	gchar *name = NULL;
	/* Populates a big master hashtable of all dynamic widgets so that 
	 * various functions can do a lookup for the widgets name and get it's
	 * GtkWidget * for manipulation.  We do NOT insert the topframe
	 * widgets from the XML tree as if more than 1 tab loads there will 
	 * be a clash, and there's no need to store the top frame widget 
	 * anyways...
	 */
	if (GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),populate_master,user_data);
	name = (char *)glade_get_widget_name(widget);
	if (name == NULL)
		return;
	if (g_strrstr((gchar *)name,"topframe"))
		return;
	if(!dynamic_widgets)
		dynamic_widgets = g_hash_table_new(g_str_hash,g_str_equal);
	if (!g_hash_table_lookup(dynamic_widgets,name))
		g_hash_table_insert(dynamic_widgets,g_strdup(name),(gpointer)widget);
	else
		dbg_func(g_strdup_printf(__FILE__": populate_master(), key %s already exists in master table\n",name),CRITICAL);
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
	extern GList *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];

	if (GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),bind_data,user_data);
	section = (char *)glade_get_widget_name(widget);
	if (section == NULL)
		return;
	if(cfg_read_string(cfgfile,section,"keys",&tmpbuf))
	{
		keys = parse_keys(tmpbuf,&num_keys);
		dbg_func(g_strdup_printf(__FILE__": bind_data() number_keys for %s is %i\n",section,num_keys),TABLOADER);
		g_free(tmpbuf);
	}
	else
		return;

	if(cfg_read_string(cfgfile,section,"key_types",&tmpbuf))
	{
		keytypes = parse_keytypes(tmpbuf, &num_keytypes);
		g_free(tmpbuf);
	}

	if (num_keytypes != num_keys)
	{
		dbg_func(g_strdup_printf(__FILE__": Number of keys (%i) and keytypes(%i) does NOT match for widget %s, CRITICAL!!!\n",num_keys,num_keytypes,section),CRITICAL);
		g_strfreev(keys);
		g_free(keytypes);
		return;
	}
	page = -1;
	if (!cfg_read_int(cfgfile,section,"page",&page))
		dbg_func(g_strdup_printf(__FILE__": bind_data(), Object %s doesn't have a page assigned!!!!\n",section),CRITICAL);	

	/* Bind widgets to lists if thy have the bind_to_list flag set...
	 */
	if (cfg_read_string(cfgfile,section,"bind_to_list",&tmpbuf))
	{
		bind_keys = parse_keys(tmpbuf,&bind_num_keys);
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
					dbg_func(g_strdup_printf(__FILE__": bind_data() binding INT \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_INT: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_ENUM:
				if (cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					tmpi = translate_string(tmpbuf);
					dbg_func(g_strdup_printf(__FILE__": bind_data() binding ENUM \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
					g_free(tmpbuf);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_ENUM: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_BOOL:
				if (cfg_read_boolean(cfgfile,section,keys[i],&tmpi))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_data() binding BOOL \"%s\",\"%i\" to widget \"%s\"\n",keys[i],tmpi,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							GINT_TO_POINTER(tmpi));	
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_BOOL: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;
			case MTX_STRING:
				if(cfg_read_string(cfgfile,section,keys[i],&tmpbuf))
				{
					dbg_func(g_strdup_printf(__FILE__": bind_data() binding STRING key:\"%s\" value:\"%s\" to widget \"%s\"\n",keys[i],tmpbuf,section),TABLOADER);
					g_object_set_data(G_OBJECT(widget),
							g_strdup(keys[i]),
							g_strdup(tmpbuf));
					g_free(tmpbuf);
				}
				else
					dbg_func(g_strdup_printf(__FILE__": bind_data(), MTX_STRING: read of key \"%s\" from section \"%s\" failed\n",keys[i],section),CRITICAL);
				break;

		}
	}
	g_free(keytypes);
	g_strfreev(keys);
}

GList * get_list(gchar * key)
{
	if (!lists_hash)
		lists_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	return (GList *)g_hash_table_lookup(lists_hash,key);
}

void store_list(gchar * key, GList * list)
{
	g_hash_table_insert(lists_hash,g_strdup(key),(gpointer)list);
	return;
}

gchar ** parse_keys(gchar * string, gint * count)
{
	gchar **result = NULL;	
	gint i = 0;
	result = g_strsplit(string,",",0);
	while (result[i])
		i++;
	*count = i;	
	return result;
}

gint * parse_keytypes(gchar * string, gint * count)
{
	gchar **vector = NULL;	
	gint *keytypes = NULL;
	gint i = 0;
	gint ct = 0;
	vector = g_strsplit(string,",",0);
	while (vector[ct])
		ct++;

	keytypes = (gint *)g_malloc0(ct*sizeof(gint));	
	while (vector[i])
	{
		dbg_func(g_strdup_printf(__FILE__": parse_keytypes() trying to translate %s\n",vector[i]),TABLOADER);
		keytypes[i] = translate_string(vector[i]);
		dbg_func(g_strdup_printf(__FILE__": parse_keytypes() translated value of %s is %i\n",vector[i],keytypes[i]),TABLOADER);
		i++;
	}
	g_strfreev(vector);
	*count = i;	
	return keytypes;

}
