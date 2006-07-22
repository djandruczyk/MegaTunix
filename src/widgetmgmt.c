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
#include <widgetmgmt.h>
#include <debugging.h>
#include <dep_loader.h>
#include <enums.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <gmodule.h>
#include <keyparser.h>
#include <memory_gui.h>
#include <rtv_map_loader.h>
#include <string.h>
#include <stringmatch.h>
#include <structures.h>
#include <tabloader.h>
#include <tag_loader.h>

GHashTable *dynamic_widgets = NULL;

/*!
 \brief populate_master() stores a pointer to all of the glade loaded 
 widgets into a master hashtable so that it can be recalled by name 
 anywhere in the program.
 \param widget a (GtkWidget *) pointer, name is derived from this pointer by
 a call to glade_get_widget_name
 \param user_data (gpointer) is currently unused.
 */
void populate_master(GtkWidget *widget, gpointer user_data)
{
	gchar *name = NULL;
	gchar *fullname = NULL;
	gchar *prefix = NULL;
	ConfigFile *cfg = (ConfigFile *) user_data;
	/* Populates a big master hashtable of all dynamic widgets so that 
	 * various functions can do a lookup for the widgets name and get it's
	 * GtkWidget * for manipulation.  We do NOT insert the topframe
	 * widgets from the XML tree as if more than 1 tab loads there will 
	 * be a clash, and there's no need to store the top frame widget 
	 * anyways...
	 */
	if (GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),populate_master,user_data);
	if (!cfg_read_string(cfg,"global","id_prefix",&prefix))
		prefix = g_strdup("");

	name = (char *)glade_get_widget_name(widget);
	//printf("name of widget stored is %s\n",name);

	if (name == NULL)
		return;
	if (g_strrstr((gchar *)name,"topframe"))
	{
		g_free(name);
		return;
	}
	if(!dynamic_widgets)
		dynamic_widgets = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	fullname = g_strdup_printf("%s%s",prefix,name);
	if (!g_hash_table_lookup(dynamic_widgets,fullname))
		g_hash_table_insert(dynamic_widgets,g_strdup(fullname),(gpointer)widget);
	else
		dbg_func(g_strdup_printf(__FILE__": populate_master()\n\tKey %s  from file %s already exists in master table\n",name,cfg->filename),CRITICAL);
	g_free(prefix);
	g_free(fullname);
	g_free(name);
}


/*!
 \brief register_widget() adds a widget to the master hashtable (dynamic_widgets)
 \see dynamic_widgets
 \param name (gchar *) Names of widget to store (any strings are allowed)
 \param widget (GtkWidget *) Pointer to the widget to be stored by name.
 \see deregister_widget
 */
void register_widget(gchar *name, GtkWidget * widget)
{
	if(!dynamic_widgets)
		dynamic_widgets = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	if (g_hash_table_lookup(dynamic_widgets,name))
	{
		
		g_hash_table_replace(dynamic_widgets,g_strdup(name),(gpointer)widget);
		dbg_func(g_strdup_printf(__FILE__": register_widget()\n\tWidget named \"%s\" already exists in master table replacing it!\n",name),CRITICAL);
	}
	else
		g_hash_table_insert(dynamic_widgets,g_strdup(name),(gpointer)widget);
}


/*!
 \brief deregister_widget() removes a widget from the master hashtable (dynamic_widgets)
 \see dynamic_widgets
 \param name (gchar *) Name of widget to remove (any strings are allowed)
 \returns TRUE on success removing, FALSE on failure removing 
 \see register_widget
 */
gboolean deregister_widget(gchar *name)
{
	return (g_hash_table_remove(dynamic_widgets,name));
}


/*!
 \brief get_State() returns either TRUE or false based on the encoded value 
 passed across as a string.  The string is split up using g_strsplit, the 
 values are check for true/false and hte appropriate value is returned
 \param string (gchar *) string to parse and dissect
 \param index (int) which one we want to check
 \returns the decoded state from the string
 */
gboolean get_state(gchar *string, gint index)
{
	gchar **tmpbuf = NULL;
	gboolean state = FALSE;

	tmpbuf = g_strsplit(string,",",0);
	if (g_ascii_strcasecmp(g_ascii_strup(tmpbuf[index],-1),"ENABLED") == 0)
		state = TRUE;
	if (g_ascii_strcasecmp(g_ascii_strup(tmpbuf[index],-1),"DISABLED") == 0)
		state =  FALSE;
	g_strfreev(tmpbuf);
	return state;
}


void alter_widget_state(gpointer key, gpointer data)
{
	GtkWidget * widget = key;
	gchar * tmpbuf;
	gchar ** groups;
	gint num_groups;
	gint i = 0;
	gpointer value;
	gboolean state;
	extern GHashTable *widget_group_states;

	tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"bind_to_list");
	groups = parse_keys(tmpbuf,&num_groups,",");
	state = TRUE;
	//printf("setting state for %s in groups \"%s\" to:",(gchar *) g_object_get_data(G_OBJECT(widget),"name"),tmpbuf);
	for (i=0;i<num_groups;i++)
	{
		value = g_hash_table_lookup(widget_group_states,groups[i]);
		if ((gboolean)value == FALSE)
		{
			state = FALSE;
			break;
		}
	}
	g_strfreev(groups);
	//printf("%i\n",state);
	gtk_widget_set_sensitive(GTK_WIDGET(widget),state);
}
