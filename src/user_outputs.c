/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <keyparser.h>
#include <structures.h>
#include <user_outputs.h>

void populate_user_output_choices(void)
{
	extern gboolean rtvars_loaded;
	extern GHashTable *dynamic_widgets;
	GtkWidget *table = NULL;
	GtkWidget *button = NULL;
	GtkWidget *parent = NULL;

	if (!rtvars_loaded)
	{
		dbg_func(__FILE__": populate_dlog_choices()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n",CRITICAL);
		return;
	}
	parent = (GtkWidget *) g_hash_table_lookup(dynamic_widgets,"user_outputs_frame");
	if (!parent)
	{
		dbg_func(__FILE__": populate_user_output_choices()\n\t\"user_outputs_frame\" could NOT be located, critical error\n\n",CRITICAL);
		return;
	}
	/*
	table = gtk_table_new(2,2,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),10);
	gtk_container_add(GTK_CONTAINER(parent),table);

	button = gtk_button_new_with_label("User Output 1");
	g_signal_connect(button,"clicked",
			G_CALLBACK(show_user_output_choices),
			GINT_TO_POINTER(1));
	gtk_table_attach(GTK_TABLE(table),button,
			0,1,0,1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			0,0);
	
	gtk_widget_show_all(parent);
	*/


}


gboolean show_user_output_choices(GtkWidget *widget, gpointer data)
{
	extern struct Rtv_Map *rtv_map;
	gchar ** keys = NULL;
	gchar * key = NULL;
	gchar * dlog_name = NULL;
	gint num_keys = 0;
	gint i = 0;
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *label = NULL;
	GObject * object = NULL;

	menu = gtk_menu_new();
	while ((key = rtv_map->raw_list[i]) != NULL)
	{
		keys = parse_keys((rtv_map->raw_list[i]),&num_keys,";");
		if (num_keys > 1)
		{
			printf ("found multi-key list for offset %i\n",i);
			i++;
			g_strfreev(keys);
			continue;
		}

		printf("looking for \"%s\"\n",key);
		object = NULL;
		object = g_hash_table_lookup(rtv_map->rtv_hash,g_strdup(key));
		if (!object)
		{
			printf("object not found\n");
			i++;
			g_strfreev(keys);
			continue;
		}
		dlog_name = g_strdup(g_object_get_data(object,"dlog_gui_name"));
		item = gtk_menu_item_new();
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),dlog_name);
		gtk_container_add(GTK_CONTAINER(item),label);
		g_object_set_data(G_OBJECT(item),"object",object);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		g_strfreev(keys);
		i++;
	}
	printf("showing menu\n");
	gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL,0,gtk_get_current_event_time());

	return TRUE;
}


