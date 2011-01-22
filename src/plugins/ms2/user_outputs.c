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
#include <enums.h>
#include <keyparser.h>
#include <ms2_plugin.h>
#include <rtv_map_loader.h>
#include <stdlib.h>
#include <user_outputs.h>

extern gconstpointer *global_data;


/*! \brief sets up and populates the MS2-Extra combo for output choice
 */
G_MODULE_EXPORT void ms2_output_combo_setup(GtkWidget *widget)
{
	gint i = 0;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * toecu_conv = NULL;
	gchar * fromecu_conv = NULL;
	gchar * range = NULL;
	gint bitval = 0;
	gint width = 0;
	DataSize size = MTX_U08;
	gint precision = 0;
	gconstpointer * object = NULL;
	gchar * data = NULL;
	gchar * name = NULL;
	gchar *regex = NULL;
	GString *string = NULL;
	GtkWidget *entry = NULL;
	GtkEntryCompletion *completion = NULL;
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	Rtv_Map *rtv_map = NULL;

	rtv_map = DATA_GET(global_data,"rtv_map");

	if (!rtv_map)
		return;
	/* Create the store for the combo, with severla hidden values
	*/
	store = gtk_list_store_new(UO_COMBO_COLS,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_UCHAR);
	/* Iterate across valid variables */
	string = g_string_sized_new(32);

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
		toecu_conv = DATA_GET(object,"toecu_conv_expr");
		fromecu_conv = DATA_GET(object,"fromecu_conv_expr");
		precision = (GINT) DATA_GET(object,"precision");
		bitval = (GINT) DATA_GET(object,"offset");
		if ((!lower) && (!upper))
			range = g_strdup_printf("Valid Range: undefined");
		else
			range = g_strdup_printf("Valid Range: %s <-> %s",lower,upper);

		string = g_string_append(string,name);
		if (rtv_map->raw_list[i])
			string = g_string_append(string,"|");

		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,
				UO_CHOICE_COL,name,
				UO_BITVAL_COL,bitval,
				UO_TOECU_CONV_COL,toecu_conv,
				UO_FROMECU_CONV_COL,fromecu_conv,
				UO_LOWER_COL,lower,
				UO_UPPER_COL,upper,
				UO_RANGE_COL,range,
				UO_SIZE_COL,size,
				UO_PRECISION_COL,precision,
				-1);
		g_free(range);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(store));
	if (GTK_IS_COMBO_BOX_ENTRY(widget))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget),UO_CHOICE_COL);
		entry = mask_entry_new_with_mask_f(string->str);
		g_string_free(string,TRUE);
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


void update_ms2_user_outputs(GtkWidget *widget)
{
	GtkTreeModel *model = NULL;
	DataSize size = MTX_U08;
	GtkTreeIter iter;
	gint tmpi = 0;
	gint t_bitval = 0;
	gboolean valid = FALSE;
	gchar * toecu_conv = NULL;
	gchar * fromecu_conv = NULL;
	gchar *lower = NULL;
	gchar *upper = NULL;
	gchar *range = NULL;
	gchar *tmpbuf = NULL;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	gint i = 0;
	gint precision = 0;
	void *eval = NULL;
	GtkWidget *tmpwidget = NULL;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model),&iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,UO_BITVAL_COL,&t_bitval,-1);
		if (tmpi == t_bitval)
		{

			/* Get the rest of the data from the combo */
			gtk_tree_model_get(model,&iter,UO_SIZE_COL,&size,UO_LOWER_COL,&lower,UO_UPPER_COL,&upper,UO_RANGE_COL,&range,UO_PRECISION_COL,&precision,UO_TOECU_CONV_COL,&toecu_conv,UO_FROMECU_CONV_COL,&fromecu_conv,-1);
			tmpbuf = (gchar *)OBJ_GET(widget,"range_label");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_LABEL(tmpwidget))
				gtk_label_set_text(GTK_LABEL(tmpwidget),range);
			tmpbuf = (gchar *)OBJ_GET(widget,"thresh_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				OBJ_SET(tmpwidget,"dl_evaluator",NULL);
				if (toecu_conv)
				{
					eval = evaluator_create_f(toecu_conv);
					OBJ_SET_FULL(tmpwidget,"dl_evaluator",eval,evaluator_destroy_f);
					if (upper)
					{
						tmpf2 = g_ascii_strtod(upper,NULL);
						tmpf = evaluator_evaluate_x_f(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_upper");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET_FULL(tmpwidget,"raw_upper",g_strdup_printf("%f",tmpf),g_free);
						/*printf("update_widget thresh has dl conv expr and upper limit of %f\n",tmpf);*/
					}
					if (lower)
					{
						tmpf2 = g_ascii_strtod(lower,NULL);
						tmpf = evaluator_evaluate_x_f(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_lower");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET_FULL(tmpwidget,"raw_lower",g_strdup_printf("%f",tmpf),g_free);
						/*printf("update_widget thresh has dl conv expr and lower limit of %f\n",tmpf);*/
					}
				}
				else
					OBJ_SET(tmpwidget,"raw_upper",upper);

				OBJ_SET(tmpwidget,"ul_evaluator",NULL);
				if (fromecu_conv)
				{
					eval = evaluator_create_f(fromecu_conv);
					OBJ_SET_FULL(tmpwidget,"ul_evaluator",eval,evaluator_destroy_f);
				}
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"toecu_conv_expr",toecu_conv);
				OBJ_SET(tmpwidget,"fromecu_conv_expr",fromecu_conv);
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				/*printf ("update widgets setting thresh widget to size '%i', toecu_conv '%s' fromecu_conv '%s' precision '%i'\n",size,toecu_conv,fromecu_conv,precision);*/
				update_widget_f(tmpwidget,NULL);
			}
			tmpbuf = (gchar *)OBJ_GET(widget,"hyst_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				OBJ_SET(tmpwidget,"dl_evaluator",NULL);
				if (toecu_conv)
				{
					eval = evaluator_create_f(toecu_conv);
					OBJ_SET_FULL(tmpwidget,"dl_evaluator",eval,evaluator_destroy_f);
					if (upper)
					{
						tmpf2 = g_ascii_strtod(upper,NULL);
						tmpf = evaluator_evaluate_x_f(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_upper");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET_FULL(tmpwidget,"raw_upper",g_strdup_printf("%f",tmpf),g_free);
						/*printf("update_widget hyst has dl conv expr and upper limit of %f\n",tmpf);*/
					}
					if (lower)
					{
						tmpf2 = g_ascii_strtod(lower,NULL);
						tmpf = evaluator_evaluate_x_f(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_lower");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET_FULL(tmpwidget,"raw_lower",g_strdup_printf("%f",tmpf),g_free);
						/*printf("update_widget hyst has dl conv expr and lower limit of %f\n",tmpf);*/
					}
				}
				else
					OBJ_SET(tmpwidget,"raw_upper",upper);

				OBJ_SET(tmpwidget,"ul_evaluator",NULL);
				if (fromecu_conv)
				{
					eval = evaluator_create_f(fromecu_conv);
					OBJ_SET_FULL(tmpwidget,"ul_evaluator",eval,evaluator_destroy_f);
				}
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"toecu_conv_expr",toecu_conv);
				OBJ_SET(tmpwidget,"fromecu_conv_expr",fromecu_conv);
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				/*printf ("update widgets setting hyst widget to size '%i', toecu_conv '%s' fromecu_conv '%s' precision '%i'\n",size,toecu_conv,fromecu_conv,precision);*/
				update_widget_f(tmpwidget,NULL);
			}
			g_free(lower);
			g_free(upper);
			g_free(range);
			g_free(toecu_conv);
			g_free(fromecu_conv);
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		i++;
	}
}
