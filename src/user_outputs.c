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

enum
{
	COL_NAME = 0,
	COL_LOWER,
	COL_UPPER,
	COL_OBJECT,
	NUM_COLS
} ;

void populate_user_output_choices(void)
{
	extern gboolean rtvars_loaded;
	extern GHashTable *dynamic_widgets;
	GtkWidget *table = NULL;
	GtkWidget *parent = NULL;
	GtkWidget *view = NULL;
	GtkWidget *sw = NULL;

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

	table = gtk_table_new(2,2,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),10);
	gtk_container_add(GTK_CONTAINER(parent),table);

	view = create_view();
	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(sw,100,50);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(sw),view);
	gtk_table_attach(GTK_TABLE(table),sw,
			0,1,0,1,
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
			0,0);

	gtk_widget_show_all(parent);


}

GtkTreeModel * create_model(void)
{
	GtkListStore  *store;
	GtkTreeIter    iter;
	gint i = 0;
	gchar * data = NULL;
	gchar * name = NULL;
	gint lower = 0;
	gint upper = 0;
	GObject * object = NULL;
	extern struct Rtv_Map *rtv_map;

	store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_POINTER);

	/* Append a row and fill in some data */
	while ((data = rtv_map->raw_list[i])!= NULL)
	{
		i++;
		object = NULL;
		name = NULL;
		object = (GObject *)g_hash_table_lookup(rtv_map->rtv_hash,data);
		if (!object)
			continue;
		name = (gchar *) g_object_get_data(object,"dlog_gui_name");
		if (!name)
			continue;
		lower = (gint) g_object_get_data(object,"lower_limit"); 
		upper = (gint) g_object_get_data(object,"upper_limit"); 

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				COL_NAME, name,
				COL_LOWER, lower,
				COL_UPPER, upper,
				COL_OBJECT, object,
				-1);
	}
	return GTK_TREE_MODEL(store);
}


GtkWidget * create_view(void)
{
	GtkTreeViewColumn   *col;
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model;
	GtkWidget           *view;

	view = gtk_tree_view_new ();

	/* --- Column #1 --- */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
			-1,      
			"Variable",  
			renderer,
			"markup", 0,
			NULL);

	/* --- Column #2 --- */

	col = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
			-1,      
			"Lower",  
			renderer,
			"text", COL_LOWER,
			NULL);

	/* --- Column #3 --- */

	col = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
			-1,      
			"Upper",  
			renderer,
			"text", COL_UPPER,
			NULL);

	model = create_model ();

	gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

	g_object_unref (model); /* destroy model automatically with view */

	return view;
}
