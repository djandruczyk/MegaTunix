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
 \brief populate_master() stores a pointer to all of the glade loaded widgets into a 
 master hashtable so that it can be recalled by name anywhere in the program.
 \param widget Widget pointer, name is derived from this pointer by
 a call to glade_get_widget_name
 \param user_data currently unused.
 */
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
		dbg_func(g_strdup_printf(__FILE__": populate_master()\n\tKey %s already exists in master table\n",name),CRITICAL);
}


/*!
 \brief register_widget adds a widget to the master hashtable (dynamic_widgets)
 \see dynamic_widgets
 \param name String of widget name to store (any strings are allowed)
 \param widget Pointer to widget to be stored by name
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
 \param name String of widget name to remove (any strings are allowed)
 \returns TRUE on success removing, FALSE on failure removing 
 \see register_widget
 */
gboolean deregister_widget(gchar *name)
{
	return (g_hash_table_remove(dynamic_widgets,name));
}
