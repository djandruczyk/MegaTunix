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
 \param user_data currently unused.
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

	if (name == NULL)
		return;
	if (g_strrstr((gchar *)name,"topframe"))
		return;
	if(!dynamic_widgets)
		dynamic_widgets = g_hash_table_new(g_str_hash,g_str_equal);
	fullname = g_strdup_printf("%s%s",prefix,name);
	if (!g_hash_table_lookup(dynamic_widgets,fullname))
		g_hash_table_insert(dynamic_widgets,g_strdup(fullname),(gpointer)widget);
	else
		dbg_func(g_strdup_printf(__FILE__": populate_master()\n\tKey %s  from file %s already exists in master table\n",name,cfg->filename),CRITICAL);
	g_free(prefix);
	g_free(fullname);
}


/*!
 \brief register_widget adds a widget to the master hashtable (dynamic_widgets)
 \see dynamic_widgets
 \param name (gchar *) String of widget name to store (any strings are allowed)
 \param widget (GtkWidget *) Pointer to widget to be stored by name
 \see deregister_widget
 */
void register_widget(gchar *name, GtkWidget * widget)
{
	if(!dynamic_widgets)
		dynamic_widgets = g_hash_table_new(g_str_hash,g_str_equal);
	if (g_hash_table_lookup(dynamic_widgets,name))
	{
		
		g_hash_table_replace(dynamic_widgets,g_strdup(name),(gpointer)widget);
		dbg_func(g_strdup_printf(__FILE__": register_widget()\n\tWidget named \"%s\" already exists in master table replacing it!\n",name),CRITICAL);
	}
	else
		g_hash_table_insert(dynamic_widgets,g_strdup(name),(gpointer)widget);
}


/*!
 \brief deregister_widget removes a widget to the master hashtable (dynamic_widgets)
 \see dynamic_widgets
 \param name (gchar *) String of widget name to remove (any strings are allowed)
 \returns TRUE on success removing, FALSE on failure removing 
 \see register_widget
 */
gboolean deregister_widget(gchar *name)
{
	return (g_hash_table_remove(dynamic_widgets,name));
}

/*!
 \brief get_raw_widget is a concenience function to return a pointer to
 the text entry on the raweditor for the widget at the provided page/offset
 We find this widget by looking for the "raw" flag set on the widget
 \param page (gint), page to search for this widget
 \param offset (gint) offset in above page that this widget resides
 \returns Pointer (GtkWidget *) to the widget requested.
 */
GtkWidget * get_raw_widget(gint page, gint offset)
{
	extern GList ***ve_widgets;
	gint len = 0;
	gint i = 0;
	GList *list = NULL;
	GtkWidget *widget = NULL;
	gboolean raw = FALSE;

	len = g_list_length(ve_widgets[page][offset]);
	list = g_list_first(ve_widgets[page][offset]);
	while (i < len)
	{
		raw = FALSE;
		widget = g_list_nth_data(list,i);
		raw = (gboolean)g_object_get_data(G_OBJECT(widget),"raw");
		if (raw)
			return widget;
		i++;
	}
	return NULL;
}
