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
#include <glade/glade.h>
#include <stringmatch.h>
#include <structures.h>
#include <tabloader.h>
#include <glade-private.h>

gboolean tabs_loaded = FALSE;
GHashTable *dynamic_widgets = NULL;

void check_ve_widgets();
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
	extern GtkWidget * notebook;
	
	if (!firmware->tab_list[i])
		return FALSE;

	
	while (firmware->tab_list[i])
	{
		//printf("should load %s\n",firmware->tab_list[i]);
		glade_file = g_strconcat(DATA_DIR,"/",GUI_DIR,"/",
				firmware->tab_list[i],".glade",NULL);
		map_file = g_strconcat(DATA_DIR,"/",GUI_DIR,"/",
				firmware->tab_list[i],".datamap",NULL);
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
				g_hash_table_foreach(xml->priv->name_hash,bind_data,(gpointer)cfgfile);
				g_hash_table_foreach(xml->priv->name_hash,populate_master,NULL);
				cfg_free(cfgfile);
			}
			else
				dbg_func(g_strdup_printf("DATAMAP: %s NOT FOUND\n",map_file),CRITICAL);
			
			g_free(map_file);
			g_free(glade_file);
			frame = glade_xml_get_widget(xml,"topframe");
			if (frame == NULL)
				printf("frame not found in xml\n");
			
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);
			gtk_widget_show_all(frame);
			glade_xml_signal_autoconnect(xml);
			
		}
		else
			printf("file NOT found \n");
		i++;
		
	}
	tabs_loaded = TRUE;
	return TRUE;

}

void populate_master(gpointer name, gpointer value,gpointer user_data)
{
	if(!dynamic_widgets)
		dynamic_widgets = g_hash_table_new(g_str_hash,g_str_equal);
	g_hash_table_insert(dynamic_widgets,g_strdup(name),value);
}

void bind_data(gpointer widget_name, gpointer value, gpointer user_data)
{
	ConfigFile *cfgfile = (ConfigFile *)user_data;
	gchar * tmpbuf = NULL;
	GtkWidget *widget = (GtkWidget *) value;
	extern GtkWidget *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];
	gchar * section = g_strdup((gchar *)widget_name);
	gchar ** keys = NULL;
	gint keytypes[50];	/* bad idea to be fixed size!! */
	gint num_keys = 0;
	gint num_keytypes = 0;
	gint i = 0;
	gint tmpi = 0;
	gint offset = 0;
	gint page = 0;

	if(!cfg_read_string(cfgfile,section,"keys",&tmpbuf))
		return;

	keys = parse_keys(tmpbuf,&num_keys);
	dbg_func(g_strdup_printf(__FILE__": bind_data() number_keys for %s is %i\n",section,num_keys),TABLOADER);

	g_free(tmpbuf);
	cfg_read_string(cfgfile,section,"key_types",&tmpbuf);
	parse_keytypes(tmpbuf,keytypes, &num_keytypes);

	if (num_keytypes != num_keys)
	{
		dbg_func(g_strdup_printf(__FILE__": Number of keys (%i) and keytypes(%i) does NOT match for widget %s, CRITICAL!!!\n",num_keys,num_keytypes,section),CRITICAL);
		g_strfreev(keys);
		return;
	}
	offset = -1;
	page = -1;
	cfg_read_int(cfgfile,section,"offset",&offset);
	if (!cfg_read_int(cfgfile,section,"page",&page))
		dbg_func(g_strdup_printf(__FILE__": bind_data(), Object %s doesn't have a page assigned!!!!\n",section),CRITICAL);	
	if (offset >= 0)
	{
		if (GTK_IS_OBJECT(ve_widgets[page][offset]))
			dbg_func(g_strdup_printf(__FILE__": bind_data() CRITICAL ERROR, Attempted overwrite of widget pointer at page %i, offset %i with info from widget defiend as %s\n",page,offset,section),CRITICAL);
		else
			ve_widgets[page][offset] = widget;
	}
	for (i=0;i<num_keys;i++)
	{
		switch((DataTypes)keytypes[i])
		{
			case INT:
				cfg_read_int(cfgfile,section,keys[i],&tmpi);
				g_object_set_data(G_OBJECT(widget),
						g_strdup(keys[i]),
						GINT_TO_POINTER(tmpi));	
				dbg_func(g_strdup_printf(__FILE__": bind_data() binding INT %s,%i to widget %s\n",keys[i],tmpi,section),TABLOADER);
				break;
			case ENUM:
				cfg_read_string(cfgfile,section,keys[i],&tmpbuf);
				tmpi = translate_string(tmpbuf);
				g_free(tmpbuf);
				g_object_set_data(G_OBJECT(widget),
						g_strdup(keys[i]),
						GINT_TO_POINTER(tmpi));	
				dbg_func(g_strdup_printf(__FILE__": bind_data() binding STRING %s,%i to widget %s\n",keys[i],tmpi,section),TABLOADER);
				break;
			case BOOL:
				cfg_read_boolean(cfgfile,section,keys[i],&tmpi);
				g_object_set_data(G_OBJECT(widget),
						g_strdup(keys[i]),
						GINT_TO_POINTER(tmpi));	
				dbg_func(g_strdup_printf(__FILE__": bind_data() binding BOOL %s,%i to widget %s\n",keys[i],tmpi,section),TABLOADER);
				break;
			case STRING:
				cfg_read_string(cfgfile,section,keys[i],&tmpbuf);
				g_object_set_data(G_OBJECT(widget),
						g_strdup(keys[i]),
						g_strdup(tmpbuf));
				break;
		
		}
	}

}

gchar ** parse_keys(gchar * string, gint * count)
{
	gchar **result;	
	gint i = 0;
	result = g_strsplit(string,",",0);
	while (result[i])
		i++;
	*count = i;	
	return result;
}

void parse_keytypes(gchar * string, gint keytypes[], gint * count)
{
	gchar **vector;	
	gint i = 0;
	vector = g_strsplit(string,",",0);
	while (vector[i])
	{
		dbg_func(g_strdup_printf(__FILE__": parse_keytypes() trying to translate %s\n",vector[i]),TABLOADER);
		keytypes[i] = translate_string(vector[i]);
		dbg_func(g_strdup_printf(__FILE__": parse_keytypes() translated value of %s is %i\n",vector[i],keytypes[i]),TABLOADER);
		i++;
	}
	g_strfreev(vector);
	*count = i;	
		
}
void check_ve_widgets()
{
	extern GtkWidget * ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];
	gint x = 0;
	gint y = 0;

	for (x=0;x<2;x++)
	{
		for(y=0;y<MS_PAGE_SIZE;y++)
		{
			if (GTK_IS_OBJECT(ve_widgets[x][y]))
				printf("widget at %i,%i\n",x,y);
			else
				printf("NO widget at %i,%i\n",x,y);
		}
	}
}
