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
#include <combo_mask.h>
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <dep_processor.h>
#include <enums.h>
#include <keyparser.h>
#include <gui_handlers.h>
#include <lookuptables.h>
#include <multi_expr_loader.h>
#include <mtxmatheval.h>
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

static GList *views = NULL;
extern gconstpointer *global_data;

G_MODULE_EXPORT gboolean force_view_recompute(gpointer data)
{
	guint i = 0;
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
G_MODULE_EXPORT void build_model_and_view(GtkWidget * widget)
{
	extern gboolean rtvars_loaded;
	GtkWidget *view = NULL;
	GtkTreeModel *model = NULL;


	if (!rtvars_loaded)
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": build_model_and_view()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
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
G_MODULE_EXPORT GtkTreeModel * create_model(void)
{
	GtkListStore  *model;
	GtkTreeIter    iter;
	gint i = 0;
	gchar * data = NULL;
	gchar * name = NULL;
	gint lower = 0;
	gint upper = 0;
	gchar * range = NULL;
	gconstpointer * object = NULL;
	extern Rtv_Map *rtv_map;

	model = gtk_list_store_new (NUM_COLS, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

	/* Append a row and fill in some data */
	while ((data = rtv_map->raw_list[i])!= NULL)
	{
		i++;
		object = NULL;
		name = NULL;
		object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,data);
		if (!object)
			continue;
		name = (gchar *) DATA_GET(object,"dlog_gui_name");
		if (!name)
			continue;
		if (DATA_GET(object,"real_lower"))
			lower = (gint)strtol(DATA_GET(object,"real_lower"),NULL,10);
		else
			lower = get_extreme_from_size((DataSize)DATA_GET(object,"size"),LOWER);
		if (DATA_GET(object,"real_upper"))
			upper = (gint)strtol(DATA_GET(object,"real_upper"),NULL,10);
		else
			upper = get_extreme_from_size((DataSize)DATA_GET(object,"size"),UPPER);
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
G_MODULE_EXPORT void add_columns(GtkTreeView *view, GtkWidget *widget)
{
	GtkCellRenderer     *renderer;
	GtkTreeViewColumn *col;
	GtkTreeModel        *model = gtk_tree_view_get_model (view);
	gchar * tmpbuf = NULL;
	gint output = -1;

	output = (GINT)OBJ_GET(widget,"output_num");
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
G_MODULE_EXPORT void cell_edited(GtkCellRendererText *cell, 
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
	gconstpointer *object = NULL;
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

	column = (GINT) OBJ_GET (cell, "column");
	canID = (GINT) OBJ_GET(model,"canID");
	page = (GINT) OBJ_GET(model,"page");
	size = (DataSize) OBJ_GET(model,"size");
	src_offset = (GINT) OBJ_GET(model,"src_offset");
	lim_offset = (GINT) OBJ_GET(model,"lim_offset");
	hys_offset = (GINT) OBJ_GET(model,"hys_offset");
	ulimit_offset = (GINT) OBJ_GET(model,"ulimit_offset");

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);

	rt_offset = (GINT) DATA_GET(object,"offset");
	precision = (GINT) DATA_GET(object,"precision");
	new = (gfloat)g_ascii_strtod(g_strdelimit((gchar *)new_text,",.",'.'),NULL);
	if (DATA_GET(object,"multi_expr_hash"))
	{
		hash = DATA_GET(object,"multi_expr_hash");
		key = (gchar *)DATA_GET(object,"source_key");
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

		if (DATA_GET(object,"real_lower"))
			lower = (gint)strtol(DATA_GET(object,"real_lower"),NULL,10);
		else
			lower = get_extreme_from_size((DataSize)DATA_GET(object,"size"),LOWER);
		if (DATA_GET(object,"real_upper"))
			upper = (gint)strtol(DATA_GET(object,"real_upper"),NULL,10);
		else
			upper = get_extreme_from_size((DataSize)DATA_GET(object,"size"),UPPER);
		evaluator = (void *)DATA_GET(object,"dl_evaluator");
		temp_dep = (GBOOLEAN)DATA_GET(object,"temp_dep");

		if (new < lower)
			new = lower;
		if (new > upper)
			new = upper;

			gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,g_strdup_printf("%1$.*2$f",new,precision), -1);

		if (!evaluator)
		{
			evaluator = evaluator_create(DATA_GET(object,"dl_conv_expr"));
			if (!evaluator)
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": cell_edited()\n\t Evaluator could NOT be created, expression is \"%s\"\n",(gchar *)DATA_GET(object,"dl_conv_expr")));
				DATA_SET_FULL(object,"dl_evaluator",(gpointer)evaluator,evaluator_destroy);
			}
		}
		/* First conver to fahrenheit temp scale if temp dependant */
		if (temp_dep)
		{
			if ((GINT)DATA_GET(global_data,"temp_units") == CELSIUS)
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
		if (DATA_GET(object,"lookuptable"))
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
	gdk_threads_add_timeout(500,(GSourceFunc)deferred_model_update,(GtkWidget *)view);
	return;

}


/*!
 \brief update_model_from_view() is called after a cell is cuccessfully edited
 and updates the treemodel associated with the view.
 \see cell_edited
 \param widget (GtkWidget *) pointer to the TreeView widget.
 */
G_MODULE_EXPORT void update_model_from_view(GtkWidget * widget)
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(widget));
	GtkTreeIter    iter;
	gconstpointer *object = NULL;
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
	temp_units = (GINT)DATA_GET(global_data,"temp_units");
	src_offset = (GINT)OBJ_GET(model,"src_offset");
	lim_offset = (GINT)OBJ_GET(model,"lim_offset");
	hys_offset = (GINT)OBJ_GET(model,"hys_offset");
	ulimit_offset = (GINT)OBJ_GET(model,"ulimit_offset");
	page = (GINT)OBJ_GET(model,"page");
	canID = (GINT)OBJ_GET(model,"canID");

	offset = get_ecu_data(canID,page,src_offset,MTX_U08);
	cur_val = get_ecu_data(canID,page,lim_offset,MTX_U08);
	hys_val = get_ecu_data(canID,page,hys_offset,MTX_U08);
	ulimit_val = get_ecu_data(canID,page,ulimit_offset,MTX_U08);

	looptest = TRUE;
	while (looptest)
	{
		gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);
		if (offset == (GINT)DATA_GET(object,"offset"))
		{
			precision =(GINT)DATA_GET(object,"precision");
			temp_dep =(GBOOLEAN)DATA_GET(object,"temp_dep");
			if (DATA_GET(object,"multi_expr_hash"))
			{
				hash = DATA_GET(object,"multi_expr_hash");
				key = (gchar *)DATA_GET(object,"source_key");
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
				evaluator =(void *)DATA_GET(object,"ul_evaluator");
				if (!evaluator) /* Not created yet */
				{
					expr = DATA_GET(object,"ul_conv_expr");
					if (expr == NULL)
					{
						dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_model_from_view()\n\t \"ul_conv_expr\" was NULL for control \"%s\", EXITING!\n",(gchar *)DATA_GET(object,"internal_name")));
						exit (-3);
					}
					evaluator = evaluator_create(expr);
					if (!evaluator)
					{
						dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_model_from_view()\n\t Creating of evaluator for function \"%s\" FAILED!!!\n\n",expr));
						DATA_SET_FULL(object,"ul_evaluator",evaluator,evaluator_destroy);
					}
					assert(evaluator);

				}
				else
					assert(evaluator);

				/* TEXT ENTRY part */
				if (DATA_GET(object,"lookuptable"))
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
					if (DATA_GET(object,"lookuptable"))
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
					if (DATA_GET(object,"lookuptable"))
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


			/*printf("offset matched for object %s\n",(gchar *)DATA_GET(object,"dlog_gui_name"));*/

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

G_MODULE_EXPORT gboolean deferred_model_update(GtkWidget * widget)
{
	update_model_from_view(widget);
	return FALSE;
}


/*! \brief sets up and populates the MS2-Extra combo for output choice
 */
G_MODULE_EXPORT void ms2_output_combo_setup(GtkWidget *widget)
{
	gint i = 0;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * dl_conv = NULL;
	gchar * ul_conv = NULL;
	gchar * range = NULL;
	gint bitval = 0;
	gint width = 0;
	DataSize size = MTX_U08;
	gint precision = 0;
	gconstpointer * object = NULL;
	gchar * data = NULL;
	gchar * name = NULL;
	gchar *regex = NULL;
	GtkWidget *entry = NULL;
	GtkEntryCompletion *completion = NULL;
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	extern Rtv_Map *rtv_map;


	if (!rtv_map)
		return;
	/* Create the store for the combo, with severla hidden values
	*/
	store = gtk_list_store_new(UO_COMBO_COLS,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_UCHAR);
	/* Iterate across valid variables */
	while ((data = rtv_map->raw_list[i])!= NULL)
	{
		i++;
		object = NULL;
		name = NULL;
		object = (gconstpointer *)g_hash_table_lookup(rtv_map->rtv_hash,data);
		if (!object)
		{
			printf("couldn't find Raw RTV var with int name %s\n",data);
			continue;
		}
		name = (gchar *) DATA_GET(object,"dlog_gui_name");
		if (!name)
			continue;
		size = (DataSize)DATA_GET(object,"size");
		lower = DATA_GET(object,"real_lower");
		upper = DATA_GET(object,"real_upper");
		dl_conv = DATA_GET(object,"dl_conv_expr");
		ul_conv = DATA_GET(object,"ul_conv_expr");
		precision = (GINT) DATA_GET(object,"precision");
		bitval = (GINT) DATA_GET(object,"offset");
		if ((!lower) && (!upper))
			range = g_strdup_printf("Valid Range: undefined");
		else
			range = g_strdup_printf("Valid Range: %s <-> %s",lower,upper);

		regex = g_strconcat(name,NULL);
		if (rtv_map->raw_list[i])
			regex = g_strconcat("|",NULL);
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,
				UO_CHOICE_COL,name,
				UO_BITVAL_COL,bitval,
				UO_DL_CONV_COL,dl_conv,
				UO_UL_CONV_COL,ul_conv,
				UO_LOWER_COL,lower,
				UO_UPPER_COL,upper,
				UO_RANGE_COL,range,
				UO_SIZE_COL,size,
				UO_PRECISION_COL,precision,
				-1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(store));
	if (GTK_IS_COMBO_BOX_ENTRY(widget))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget),UO_CHOICE_COL);
		entry = mask_entry_new_with_mask(regex);
		/* Nasty hack, but otherwise the entry is an obnoxious size.. */
		if ((width = (GINT)OBJ_GET((GtkWidget *)widget,"max_chars")) > 0)
			gtk_entry_set_width_chars(GTK_ENTRY(entry),width);
		else
			gtk_entry_set_width_chars(GTK_ENTRY(entry),12);

		gtk_widget_set_size_request(GTK_WIDGET(widget),-1,(3*(GINT)DATA_GET(global_data,"font_size")));

		gtk_container_remove (GTK_CONTAINER (widget), GTK_BIN (widget)->child);
		gtk_container_add (GTK_CONTAINER (widget), entry);

		completion = gtk_entry_completion_new();
		gtk_entry_set_completion(GTK_ENTRY(entry),completion);
		gtk_entry_completion_set_model(completion,GTK_TREE_MODEL(store));
		gtk_entry_completion_set_text_column(completion,UO_CHOICE_COL);
		gtk_entry_completion_set_inline_completion(completion,TRUE);
		gtk_entry_completion_set_inline_selection(completion,TRUE);
		gtk_entry_completion_set_popup_single_match(completion,FALSE);
		OBJ_SET(widget,"arrow-size",GINT_TO_POINTER(1));
	}
	g_free(regex);

}
