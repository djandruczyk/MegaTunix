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

#include <assert.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <dep_processor.h>
#include <enums.h>
#include <keyparser.h>
#include <lookuptables.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <stdlib.h>
#include <structures.h>
#include <threads.h>
#include <user_outputs.h>

enum
{
	COL_OBJECT = 0,
	COL_NAME,
	COL_RANGE,
	COL_ENTRY,
	COL_EDITABLE,
	NUM_COLS
} ;

void build_model_and_view(GtkWidget * widget)
{
	extern gboolean rtvars_loaded;
	GtkWidget *view = NULL;
	GtkTreeModel *model = NULL;

	if (!rtvars_loaded)
	{
		dbg_func(__FILE__": build_model_and_view()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n",CRITICAL);
		return;
	}

	model = create_model ();

	g_object_set_data(G_OBJECT(model),"lim_offset",g_object_get_data(G_OBJECT(widget),"lim_offset"));
	g_object_set_data(G_OBJECT(model),"src_offset",g_object_get_data(G_OBJECT(widget),"src_offset"));
	g_object_set_data(G_OBJECT(model),"page",g_object_get_data(G_OBJECT(widget),"page"));
	g_object_set_data(G_OBJECT(model),"ign_parm",g_object_get_data(G_OBJECT(widget),"ign_parm"));

	view = gtk_tree_view_new_with_model(model);
	g_object_set_data(G_OBJECT(model),"view",(gpointer)view);
	gtk_container_add(GTK_CONTAINER(widget),view);
	g_object_unref(model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (view),COL_NAME);
	add_columns(GTK_TREE_VIEW(view), (gint)g_object_get_data(G_OBJECT(widget),"output_num"));

}

GtkTreeModel * create_model(void)
{
	GtkListStore  *model;
	GtkTreeIter    iter;
	gint i = 0;
	gchar * data = NULL;
	gchar * name = NULL;
	gint lower = 0;
	gint upper = 0;
	gchar * range = NULL;
	GObject * object = NULL;
	extern struct Rtv_Map *rtv_map;

	model = gtk_list_store_new (NUM_COLS, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

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
		range = g_strdup_printf("%i-%i",lower,upper);

		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				COL_OBJECT, object,
				COL_NAME, name,
				COL_RANGE, range,
				COL_ENTRY, g_strdup(""),
				COL_EDITABLE,TRUE,
				-1);
		g_free(range);
	}
	return GTK_TREE_MODEL(model);
}


void add_columns(GtkTreeView *view, gint output)
{
	GtkCellRenderer     *renderer;
	GtkTreeViewColumn *col;
	GtkTreeModel        *model = gtk_tree_view_get_model (view);

	/* --- Column #1, name --- */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set_data (G_OBJECT (renderer), "column", (gint *)COL_NAME);
	col = gtk_tree_view_column_new_with_attributes (
			g_strdup_printf("Out %i Variable",output),  
			renderer,
			"markup", COL_NAME,
			NULL);

	gtk_tree_view_column_set_sort_column_id (col, COL_NAME);
	gtk_tree_view_append_column (view, col);

	/* --- Column #2, range --- */

	renderer = gtk_cell_renderer_text_new ();
	g_object_set_data (G_OBJECT (renderer), "column", (gint *)COL_RANGE);
	col = gtk_tree_view_column_new_with_attributes (
			"Range",  
			renderer,
			"text", COL_RANGE,
			NULL);
	gtk_tree_view_append_column (view, col);

	/* --- Column #3, user choice --- */

	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect(renderer, "edited",
			G_CALLBACK(cell_edited),model);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *)COL_ENTRY);
	col = gtk_tree_view_column_new_with_attributes (
			"Value",  
			renderer,
			"text", COL_ENTRY,
			"editable",COL_EDITABLE,
			NULL);
	gtk_tree_view_append_column (view, col);

}

void cell_edited(GtkCellRendererText *cell, 
		const gchar * path_string,
		const gchar * new_text,
		gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *)data;
	GtkTreeView *view = (GtkTreeView *)g_object_get_data(G_OBJECT(model),"view");
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;
	gboolean temp_dep;
	extern gint temp_units;
	gint lower = 0;
	gint upper = 0;
	gfloat new = 0;
	gint column = 0;
	GObject *object = NULL;
	gint src_offset = -1;
	gint lim_offset = -1;
	gint rt_offset = -1;
	gfloat x = 0.0;
	gfloat tmpf = 0.0;
	void * evaluator = NULL;
	gchar * table = NULL;
	gchar * alt_table = NULL;
	gchar * lookuptable = NULL;
	gint * lookup = NULL;
	gint result = 0;
	gint page = 0;
	gboolean ign_parm = TRUE;
	gboolean state = FALSE;
	gboolean is_float = FALSE;
	extern GHashTable *lookuptables;
	extern gint ** ms_data;

	column = (gint) g_object_get_data (G_OBJECT (cell), "column");
	page = (gint) g_object_get_data(G_OBJECT(model),"page");
	ign_parm = (gboolean) g_object_get_data(G_OBJECT(model),"ign_parm");
	src_offset = (gint) g_object_get_data(G_OBJECT(model),"src_offset");
	lim_offset = (gint) g_object_get_data(G_OBJECT(model),"lim_offset");

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);

	lower = (gint) g_object_get_data(G_OBJECT(object),"lower_limit");
	upper = (gint) g_object_get_data(G_OBJECT(object),"upper_limit");
	rt_offset = (gint) g_object_get_data(G_OBJECT(object),"offset");
	evaluator = (void *)g_object_get_data(G_OBJECT(object),"dl_evaluator");
	temp_dep = (gboolean)g_object_get_data(G_OBJECT(object),"temp_dep");
	is_float = (gboolean)g_object_get_data(G_OBJECT(object),"is_float");
	lookuptable = (gchar *)g_object_get_data(G_OBJECT(object),"lookuptable");
	new = (gfloat)g_ascii_strtod(new_text,NULL);
	if (new < lower)
		new = lower;
	if (new > upper)
		new = upper;

	if (is_float)
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
				g_strdup_printf("%.2f",new), -1);
	else
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
				g_strdup_printf("%.i",(gint)new), -1);
	
	if (!evaluator)
	{
		evaluator = evaluator_create(g_object_get_data(G_OBJECT(object),"dl_conv_expr"));
		if (!evaluator)
			dbg_func(g_strdup_printf(__FILE__": cell_edited()\n\t Evaluator could NOT be created, expression is \"%s\"\n",(gchar *)g_object_get_data(G_OBJECT(object),"dl_conv_expr")),CRITICAL);
		g_object_set_data(object,"dl_evaluator",(gpointer)evaluator);
	}
	// First conver to fahrenheit temp scale if temp dependant 
	if (temp_dep)
	{
		if (temp_units == CELSIUS)
			x = (new*9.0/5.0)+32;
		else
			x = new;
	}
	else
		x = new;
	// Then evaluate it in reverse....
	tmpf = evaluator_evaluate_x(evaluator,x);
	// Then if it used a lookuptable,  reverse map it if possible to 
	// determine the ADC reading we need to send to ECU
	if (lookuptable)
	{

		table = (gchar *)g_object_get_data(G_OBJECT(object),"lookuptable");
		alt_table = (gchar *)g_object_get_data(G_OBJECT(object),"alt_lookuptable");
		if (g_object_get_data(object,"depend_on"))
			state = check_dependancy(object);
		if (state)
			lookup = (gint *)g_hash_table_lookup(lookuptables,alt_table);
		else
			lookup = (gint *)g_hash_table_lookup(lookuptables,table);

		result = reverse_lookup((gint)tmpf,lookup);
	}
	else
		result = (gint)tmpf;

	ms_data[page][src_offset] = rt_offset;
	ms_data[page][lim_offset] = result;
	write_ve_const(page,src_offset,rt_offset,ign_parm);
	write_ve_const(page,lim_offset,result,ign_parm);
	update_model_from_view((GtkWidget *)view);
	
}

void update_model_from_view(GtkWidget * widget)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(widget));
	GtkTreeIter    iter;
	GObject *object = NULL;
	gint src_offset = 0;
	gint lim_offset = 0;
	gint page = 0;
	gint offset = 0;
	gint cur_val = 0;
	gint x = 0;
	gfloat tmpf = 0.0;
	gfloat result = 0.0;
	gchar * expr = NULL;
	gboolean is_float = FALSE;
	gboolean temp_dep = FALSE;
	gboolean looptest = FALSE;
	void * evaluator = NULL;
	extern gint ** ms_data;
	extern gint temp_units;


	if (!gtk_tree_model_get_iter_first(model,&iter))
		return;
	src_offset = (gint)g_object_get_data(G_OBJECT(model),"src_offset");
	lim_offset = (gint)g_object_get_data(G_OBJECT(model),"lim_offset");
	page = (gint)g_object_get_data(G_OBJECT(model),"page");
	offset = ms_data[page][src_offset];
	cur_val = ms_data[page][lim_offset];
	//printf("offset from secl we're looking for is %i\n",offset);

	looptest = TRUE;
	while (looptest)
	{
		gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);
		if (offset == (gint)g_object_get_data(object,"offset"))
		{
			is_float =(gboolean)g_object_get_data(object,"is_float");
			temp_dep =(gboolean)g_object_get_data(object,"temp_dep");
			evaluator =(void *)g_object_get_data(object,"ul_evaluator");
			if (!evaluator) /* Not created yet */
			{
				expr = g_object_get_data(object,"ul_conv_expr");
				if (expr == NULL)
				{
					dbg_func(g_strdup_printf(__FILE__": update_model_from_view()\n\t \"ul_conv_expr\" was NULL for control \"%s\", EXITING!\n",(gchar *)g_object_get_data(object,"internal_name")),CRITICAL);
					exit (-3);
				}
				evaluator = evaluator_create(expr);
				if (!evaluator)
					dbg_func(g_strdup_printf(__FILE__": update_model_from_view()\n\t Creating of evaluator for function \"%s\" FAILED!!!\n\n",expr),CRITICAL);
				assert(evaluator);
				g_object_set_data(object,"ul_evaluator",evaluator);

			}
			else
				assert(evaluator);

			if (g_object_get_data(object,"lookuptable"))
				x = lookup_data(object,cur_val);
			else
				x = cur_val;
			tmpf = evaluator_evaluate_x(evaluator,x);
			if (temp_dep)
			{
				if (temp_units == CELSIUS)
					result = (tmpf-32)*(5.0/9.0);
				else
					result = tmpf;
			}
			else
				result = tmpf;

			if (is_float)
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ENTRY,g_strdup_printf("%.2f",result), -1);
			else
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ENTRY,g_strdup_printf("%.i",(gint)result), -1);

//			printf("offset matched for object %s\n",(gchar *)g_object_get_data(object,"dlog_gui_name"));

		}
		else
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ENTRY,g_strdup(""), -1);
		looptest = gtk_tree_model_iter_next(model,&iter);
	}



}
