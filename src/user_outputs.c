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
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <dep_processor.h>
#include <enums.h>
#include <keyparser.h>
#include <lookuptables.h>
#include <multi_expr_loader.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <rtv_map_loader.h>
#include <stdlib.h>
#include <threads.h>
#include <user_outputs.h>

enum
{
	COL_OBJECT = 0,
	COL_NAME,
	COL_RANGE,
	COL_ENTRY,
	COL_HYS,
	COL_ULIMIT,
	COL_MODE,
	COL_EDITABLE,
	NUM_COLS
} ;

extern gint dbg_lvl;
static GList *views = NULL;
extern GObject *global_data;

gboolean force_view_recompute()
{
	gint i = 0;
	for (i=0;i<g_list_length(views);i++)
		update_model_from_view(g_list_nth_data(views,i));
	return FALSE;
}
/*!
 \brief build_model_and_view() is called to create the model and view for
 a preexisting textview. Currently used to generate the User controlled outputs
 lists for MSnS and MSnEDIS firmwares
 \param widget (GtkWidget *) The textview that this model and view is to be
 created for.
 */
EXPORT void build_model_and_view(GtkWidget * widget)
{
	extern gboolean rtvars_loaded;
	GtkWidget *view = NULL;
	GtkTreeModel *model = NULL;


	if (!rtvars_loaded)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": build_model_and_view()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}

	model = create_model ();

	OBJ_SET(model,"lim_offset",OBJ_GET(widget,"lim_offset"));
	OBJ_SET(model,"src_offset",OBJ_GET(widget,"src_offset"));
	OBJ_SET(model,"hys_offset",OBJ_GET(widget,"hys_offset"));
	OBJ_SET(model,"ulimit_offset",OBJ_GET(widget,"ulimit_offset"));
	OBJ_SET(model,"page",OBJ_GET(widget,"page"));
	OBJ_SET(model,"canID",OBJ_GET(widget,"canID"));

	view = gtk_tree_view_new_with_model(model);
	views = g_list_append(views,view);
	OBJ_SET(model,"view",(gpointer)view);
	gtk_container_add(GTK_CONTAINER(widget),view);
	g_object_unref(model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (view),COL_NAME);
	add_columns(GTK_TREE_VIEW(view), widget);

	update_model_from_view((GtkWidget *)view);

}


/*!
 \brief create_model() Creates a TreeModel used by the user outputs treeviews
 \returns a pointer to a newly created GtkTreeModel.
 */
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
	extern Rtv_Map *rtv_map;

	model = gtk_list_store_new (NUM_COLS, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

	/* Append a row and fill in some data */
	while ((data = rtv_map->raw_list[i])!= NULL)
	{
		i++;
		object = NULL;
		name = NULL;
		object = (GObject *)g_hash_table_lookup(rtv_map->rtv_hash,data);
		if (!object)
			continue;
		name = (gchar *) OBJ_GET(object,"dlog_gui_name");
		if (!name)
			continue;
		lower = (gint) OBJ_GET(object,"lower_limit"); 
		upper = (gint) OBJ_GET(object,"upper_limit"); 
		if ((!OBJ_GET(object,"lower_limit")) && (!OBJ_GET(object,"upper_limit")))
			range = g_strdup_printf("Varies");
		else
			range = g_strdup_printf("%i-%i",lower,upper);

		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				COL_OBJECT, object,
				COL_NAME, name,
				COL_RANGE, range,
				COL_ENTRY, NULL,
				COL_HYS, NULL,
				COL_ULIMIT,NULL,
				COL_MODE,GTK_CELL_RENDERER_MODE_EDITABLE,
				COL_EDITABLE,TRUE,
				-1);
		g_free(range);
	}
	return GTK_TREE_MODEL(model);
}


/*!
 \brief add_columns() creates the column fields for a treeview.
 \param view (GtkTreeView *) pointer to the Treeview
 \param widget (Gtkwidget) Widget passed to extract the needed info from
 */
void add_columns(GtkTreeView *view, GtkWidget *widget)
{
	GtkCellRenderer     *renderer;
	GtkTreeViewColumn *col;
	GtkTreeModel        *model = gtk_tree_view_get_model (view);
	gchar * tmpbuf = NULL;
	gint output = -1;

	output = (gint)OBJ_GET(widget,"output_num");
	/* --- Column #1, name --- */
	renderer = gtk_cell_renderer_text_new ();
	OBJ_SET(renderer, "column", (gint *)COL_NAME);
	tmpbuf = g_strdup_printf("Output %i Var.",output);
	col = gtk_tree_view_column_new_with_attributes (
			tmpbuf,  
			renderer,
			"markup", COL_NAME,
			NULL);
	g_free(tmpbuf);

	gtk_tree_view_column_set_sort_column_id (col, COL_NAME);
	gtk_tree_view_append_column (view, col);

	/* --- Column #2, range --- */

	renderer = gtk_cell_renderer_text_new ();
	OBJ_SET(renderer, "column", (gint *)COL_RANGE);
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
	OBJ_SET(renderer, "column", (gint *)COL_ENTRY);
	col = gtk_tree_view_column_new_with_attributes (
			"Value",  
			renderer,
			"text", COL_ENTRY,
			"mode",COL_MODE,
			"editable",COL_EDITABLE,
			NULL);
	gtk_tree_view_append_column (view, col);

	/* --- Column #4 , Hysteresis value --- */

	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect(renderer, "edited",
			G_CALLBACK(cell_edited),model);
	OBJ_SET(renderer, "column", (gint *)COL_HYS);
	col = gtk_tree_view_column_new_with_attributes (
			"Off Hysteresis",  
			renderer,
			"text", COL_HYS,
			"editable",COL_EDITABLE,
			NULL);
	gtk_tree_view_append_column (view, col);
	/* If no hysteresis params, this column should NOT show */
	if (OBJ_GET(widget,"hys_offset") == NULL)
		gtk_tree_view_column_set_visible(col,FALSE);


	/* --- Column #5, user choice --- */

	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect(renderer, "edited",
			G_CALLBACK(cell_edited),model);
	OBJ_SET(renderer, "column", (gint *)COL_ULIMIT);
	col = gtk_tree_view_column_new_with_attributes (
			"Upper Limit",  
			renderer,
			"text", COL_ULIMIT,
			"editable",COL_EDITABLE,
			NULL);
	gtk_tree_view_append_column (view, col);
	/* If no upperlimit params, this column should NOT show */
	if (OBJ_GET(widget,"ulimit_offset") == NULL)
		gtk_tree_view_column_set_visible(col,FALSE);

}


/*!
 \brief cell_edited() is called whenever an editable cell on the user outputs
 treeviews are modified. This function will vheck and verify the user input
 is valid, and process it and send it to the ECU
 \param cell (GtkCellRendererText *)  pointer to the cell that was edited
 \param path_string (const gchar *) tree_path for the treeview (see GTK+ docs)
 \param new_text (const gchar *) new text thatwas entered into the cell
 \param data (gpointer) pointer to the GtkTreeModel
 */
void cell_edited(GtkCellRendererText *cell, 
		const gchar * path_string,
		const gchar * new_text,
		gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *)data;
	GtkTreeView *view = (GtkTreeView *)OBJ_GET(model,"view");
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;
	gboolean temp_dep;
	gint lower = 0;
	gint upper = 0;
	gfloat new = 0;
	gint column = 0;
	GObject *object = NULL;
	gint src_offset = -1;
	gint lim_offset = -1;
	gint rt_offset = -1;
	gint hys_offset = -1;
	gint ulimit_offset = -1;
	gint precision = 0;
	gfloat x = 0.0;
	gfloat tmpf = 0.0;
	void * evaluator = NULL;
	gint result = 0;
	gint canID = 0;
	gint page = 0;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	DataSize size = 0;
	MultiExpr *multi = NULL;
	GHashTable *hash = NULL;
	extern GHashTable *sources_hash;

	column = (gint) OBJ_GET (cell, "column");
	canID = (gint) OBJ_GET(model,"canID");
	page = (gint) OBJ_GET(model,"page");
	size = (DataSize) OBJ_GET(model,"size");
	src_offset = (gint) OBJ_GET(model,"src_offset");
	lim_offset = (gint) OBJ_GET(model,"lim_offset");
	hys_offset = (gint) OBJ_GET(model,"hys_offset");
	ulimit_offset = (gint) OBJ_GET(model,"ulimit_offset");

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);

	rt_offset = (gint) OBJ_GET(object,"offset");
	precision = (gint)OBJ_GET(object,"precision");
	new = (gfloat)strtod(new_text,NULL);
	if (OBJ_GET(object,"multi_expr_hash"))
	{
		hash = OBJ_GET(object,"multi_expr_hash");
		key = (gchar *)OBJ_GET(object,"source_key");
		if (key)
		{
			hash_key = (gchar *)g_hash_table_lookup(sources_hash,key);
			if (hash_key)
				multi = (MultiExpr *)g_hash_table_lookup(hash,hash_key);
			else
				multi = (MultiExpr *)g_hash_table_lookup(hash,"DEFAULT");

		}
		else
			multi = (MultiExpr *)g_hash_table_lookup(hash,"DEFAULT");
		if (!multi)
			return;
		if (new < multi->lower_limit)
			new = multi->lower_limit;
		if (new > multi->upper_limit)
			new = multi->upper_limit;

			gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,g_strdup_printf("%1$.*2$f",new,precision), -1);

		/* Then evaluate it in reverse.... */
		tmpf = evaluator_evaluate_x(multi->dl_eval,new);
		/* Then if it used a lookuptable, reverse map it if possible 
		 * to determine the ADC reading we need to send to ECU
		 */
		if (multi->lookuptable)
			result = direct_reverse_lookup(multi->lookuptable,(gint)tmpf);
		else
			result = (gint)tmpf;

	}
	else
	{

		lower = (gint) OBJ_GET(object,"lower_limit");
		upper = (gint) OBJ_GET(object,"upper_limit");
		evaluator = (void *)OBJ_GET(object,"dl_evaluator");
		temp_dep = (gboolean)OBJ_GET(object,"temp_dep");

		if (new < lower)
			new = lower;
		if (new > upper)
			new = upper;

			gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,g_strdup_printf("%1$.*2$f",new,precision), -1);

		if (!evaluator)
		{
			evaluator = evaluator_create(OBJ_GET(object,"dl_conv_expr"));
			if (!evaluator)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": cell_edited()\n\t Evaluator could NOT be created, expression is \"%s\"\n",(gchar *)OBJ_GET(object,"dl_conv_expr")));
				OBJ_SET(object,"dl_evaluator",(gpointer)evaluator);
			}
		}
		/* First conver to fahrenheit temp scale if temp dependant */
		if (temp_dep)
		{
			if ((gint)OBJ_GET(global_data,"temp_units") == CELSIUS)
				x = (new*9.0/5.0)+32;
			else
				x = new;
		}
		else
			x = new;
		/* Then evaluate it in reverse.... */
		tmpf = evaluator_evaluate_x(evaluator,x);
		/* Then if it used a lookuptable,  reverse map it if possible
		 * to determine the ADC reading we need to send to ECU
		 */
		if (OBJ_GET(object,"lookuptable"))
			result = reverse_lookup(object,(gint)tmpf);
		else
			result = (gint)tmpf;
	}

	switch (column)
	{
		case COL_HYS:
			send_to_ecu(canID, page, hys_offset, MTX_U08, result, TRUE);
			break;
		case COL_ULIMIT:
			send_to_ecu(canID, page, ulimit_offset, MTX_U08, result, TRUE);
			break;
		case COL_ENTRY:
			send_to_ecu(canID, page, src_offset, MTX_U08, rt_offset, TRUE);
			send_to_ecu(canID, page, lim_offset, MTX_U08, result, TRUE);
			break;
	}
	g_timeout_add(500,(GtkFunction)deferred_model_update,(GtkWidget *)view);
	return;

}


/*!
 \brief update_model_from_view() is called after a cell is cuccessfully edited
 and updates the treemodel associated with the view.
 \see cell_edited
 \param widget (GtkWidget *) pointer to the TreeView widget.
 */
void update_model_from_view(GtkWidget * widget)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(widget));
	GtkTreeIter    iter;
	GObject *object = NULL;
	GtkTreePath *treepath = NULL;
	gint src_offset = 0;
	gint lim_offset = 0;
	gint hys_offset = -1;
	gint ulimit_offset = -1;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint cur_val = 0;
	gint hys_val = 0;
	gint ulimit_val = 0;
	gint x = 0;
	gfloat tmpf = 0.0;
	gfloat result = 0.0;
	gchar * expr = NULL;
	gint precision = 0;
	gboolean temp_dep = FALSE;
	gboolean looptest = FALSE;
	void * evaluator = NULL;
	gint temp_units;
	gchar * tmpbuf = NULL;
	gchar * key = NULL;
	gchar * hash_key = NULL;
	GHashTable *hash = NULL;
	MultiExpr * multi = NULL;
	extern GHashTable *sources_hash;

	if (!gtk_tree_model_get_iter_first(model,&iter))
		return;
	temp_units = (gint)OBJ_GET(global_data,"temp_units");
	src_offset = (gint)OBJ_GET(model,"src_offset");
	lim_offset = (gint)OBJ_GET(model,"lim_offset");
	hys_offset = (gint)OBJ_GET(model,"hys_offset");
	ulimit_offset = (gint)OBJ_GET(model,"ulimit_offset");
	page = (gint)OBJ_GET(model,"page");
	canID = (gint)OBJ_GET(model,"canID");

	offset = get_ecu_data(canID,page,src_offset,MTX_U08);
	cur_val = get_ecu_data(canID,page,lim_offset,MTX_U08);
	hys_val = get_ecu_data(canID,page,hys_offset,MTX_U08);
	ulimit_val = get_ecu_data(canID,page,ulimit_offset,MTX_U08);

	looptest = TRUE;
	while (looptest)
	{
		gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);
		if (offset == (gint)OBJ_GET(object,"offset"))
		{
			precision =(gint)OBJ_GET(object,"precision");
			temp_dep =(gboolean)OBJ_GET(object,"temp_dep");
			if (OBJ_GET(object,"multi_expr_hash"))
			{
				hash = OBJ_GET(object,"multi_expr_hash");
				key = (gchar *)OBJ_GET(object,"source_key");
				if (key)
				{
					hash_key = (gchar *)g_hash_table_lookup(sources_hash,key);
					if (hash_key)
						multi = (MultiExpr *)g_hash_table_lookup(hash,hash_key);
					else
						multi = (MultiExpr *)g_hash_table_lookup(hash,"DEFAULT");
				}
				else
					multi = (MultiExpr *)g_hash_table_lookup(hash,"DEFAULT");
				if (!multi)
					continue;

				evaluator = multi->ul_eval;
				assert(evaluator);

				/* TEXT ENTRY part */
				if (multi->lookuptable)
					x = direct_lookup_data(multi->lookuptable,cur_val);
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
				tmpbuf =  g_strdup_printf("%1$.*2$f",result,precision);

				gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ENTRY,tmpbuf, -1);
				g_free(tmpbuf);

				/* HYSTERESIS VALUE */
				if (OBJ_GET(model,"hys_offset") != NULL)
				{
					if (multi->lookuptable)
						x = direct_lookup_data(multi->lookuptable,hys_val);
					else
						x = hys_val;
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

					tmpbuf =  g_strdup_printf("%1$.*2$f",result,precision);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_HYS,tmpbuf, -1);
					g_free(tmpbuf);

				}
				/* UPPER LIMIT VALUE */
				if (OBJ_GET(model,"ulimit_offset") != NULL)
				{
					if (multi->lookuptable)
						x = direct_lookup_data(multi->lookuptable,ulimit_val);
					else
						x = ulimit_val;
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

					tmpbuf =  g_strdup_printf("%1$.*2$f",result,precision);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ULIMIT,tmpbuf, -1);
					g_free(tmpbuf);
				}
			}

			else /* Non multi-expression variable */
			{
				evaluator =(void *)OBJ_GET(object,"ul_evaluator");
				if (!evaluator) /* Not created yet */
				{
					expr = OBJ_GET(object,"ul_conv_expr");
					if (expr == NULL)
					{
						if (dbg_lvl & CRITICAL)
							dbg_func(g_strdup_printf(__FILE__": update_model_from_view()\n\t \"ul_conv_expr\" was NULL for control \"%s\", EXITING!\n",(gchar *)OBJ_GET(object,"internal_name")));
						exit (-3);
					}
					evaluator = evaluator_create(expr);
					if (!evaluator)
					{
						if (dbg_lvl & CRITICAL)
							dbg_func(g_strdup_printf(__FILE__": update_model_from_view()\n\t Creating of evaluator for function \"%s\" FAILED!!!\n\n",expr));
						OBJ_SET(object,"ul_evaluator",evaluator);
					}
					assert(evaluator);

				}
				else
					assert(evaluator);

				/* TEXT ENTRY part */
				if (OBJ_GET(object,"lookuptable"))
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
				tmpbuf =  g_strdup_printf("%1$.*2$f",result,precision);

				gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ENTRY,tmpbuf, -1);
				g_free(tmpbuf);

				/* HYSTERESIS VALUE */
				if (OBJ_GET(model,"hys_offset") != NULL)
				{
					if (OBJ_GET(object,"lookuptable"))
						x = lookup_data(object,hys_val);
					else
						x = hys_val;
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

					tmpbuf =  g_strdup_printf("%1$.*2$f",result,precision);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_HYS,tmpbuf, -1);
					g_free(tmpbuf);

				}
				/* UPPER LIMIT VALUE */
				if (OBJ_GET(model,"ulimit_offset") != NULL)
				{
					if (OBJ_GET(object,"lookuptable"))
						x = lookup_data(object,ulimit_val);
					else
						x = ulimit_val;
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

					tmpbuf =  g_strdup_printf("%1$.*2$f",result,precision);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ULIMIT,tmpbuf, -1);
					g_free(tmpbuf);
				}
			}

			/* Scroll the treeview s othe current one is in view */
			treepath = gtk_tree_model_get_path (GTK_TREE_MODEL(model),&iter);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget),treepath,NULL,TRUE,1.0,0.0);
			gtk_tree_path_free(treepath);


			/*printf("offset matched for object %s\n",(gchar *)OBJ_GET(object,"dlog_gui_name"));*/

		}
		else
		{
			tmpbuf = g_strdup("");
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ENTRY,tmpbuf, -1);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_HYS,tmpbuf, -1);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, COL_ULIMIT,tmpbuf, -1);
			g_free(tmpbuf);
		}
		looptest = gtk_tree_model_iter_next(model,&iter);
	}
}

gboolean deferred_model_update(GtkWidget * widget)
{
	update_model_from_view(widget);
	return FALSE;
}
