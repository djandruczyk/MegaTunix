/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/ms2/user_outputs.c
  \ingroup MS2Plugin,Plugins
  \brief MS2 Specific user outputs functionality
  \author David Andruczyk
  */

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#endif

#include <ms2_plugin.h>
#include <rtv_map_loader.h>
#include <user_outputs.h>

extern gconstpointer *global_data;


/*!
  \brief sets up and populates the MS2-Extra combo for output choice
  \param widget is the pointer to the combo to initialize
  */
G_MODULE_EXPORT void ms2_output_combo_setup(GtkWidget *widget)
{
	guint i = 0;
	GtkWidget *parent = NULL;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gfloat * multiplier = NULL;
	gfloat * adder = NULL;
	gfloat * testmult = NULL;
	gchar * range = NULL;
	gint bitval = 0;
	gint width = 0;
	DataSize size = MTX_U08;
	gint precision = 0;
	gconstpointer * object = NULL;
	gint raw_lower = 0;
	gint raw_upper = 0;
	gboolean freelower = FALSE;
	gboolean freeupper = FALSE;
	gchar *raw_lower_str = NULL;
	gchar *raw_upper_str = NULL;
	gfloat real_lower = 0.0;
	gfloat real_upper = 0.0;
	gfloat tmpf = 0.0;
	gchar * name = NULL;
	gchar *internal_names = NULL;
	GtkWidget *entry = NULL;
	GtkEntryCompletion *completion = NULL;
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	ENTER();
	Rtv_Map *rtv_map = NULL;

	rtv_map = (Rtv_Map *)DATA_GET(global_data,"rtv_map");

	if (!rtv_map)
	{
		EXIT();
		return;
	}
	/* Create the store for the combo, with severla hidden values
	 */
	store = gtk_list_store_new(UO_COMBO_COLS,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_POINTER,G_TYPE_POINTER,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_UCHAR,G_TYPE_UCHAR);

	/* Iterate across valid variables */
	for (i=0;i<rtv_map->rtv_list->len;i++)
	{
		freelower = FALSE;
		freeupper = FALSE;
		object = NULL;
		name = NULL;
		object = (gconstpointer *)g_ptr_array_index(rtv_map->rtv_list,i);
		if (!object)
			continue;
		name = (gchar *) DATA_GET(object,"dlog_gui_name");
		if (!name)
			continue;
		if (DATA_GET(object,"fromecu_complex"))
			continue;
		if (DATA_GET(object,"special"))
			continue;
		internal_names = (gchar *) DATA_GET(object,"internal_names");
		if (!find_in_list(rtv_map->raw_list,internal_names))
			continue;

		size = (DataSize)(GINT)DATA_GET(object,"size");
		multiplier = (gfloat *)DATA_GET(object,"fromecu_mult");
		adder = (gfloat *)DATA_GET(object,"fromecu_add");
		precision = (GINT) DATA_GET(object,"precision");
		bitval = (GINT) DATA_GET(object,"offset");
		if (DATA_GET(object,"real_lower"))
		{
			lower = (gchar *)DATA_GET(object,"real_lower");
			real_lower = g_strtod(lower,NULL);
			raw_lower = calc_value_f(real_lower,multiplier,adder,TOECU);
		}
		else
		{
			raw_lower = get_extreme_from_size_f(size,LOWER);
			tmpf = calc_value_f(raw_lower,multiplier,adder,FROMECU);
			lower = g_strdup_printf("%i",(gint)tmpf);
			freelower = TRUE;
		}
		if (DATA_GET(object,"real_upper"))
		{
			upper = (gchar *)DATA_GET(object,"real_upper");
			real_upper = g_strtod(upper,NULL);
			raw_upper = calc_value_f(real_upper,multiplier,adder,TOECU);
		}
		else
		{
			raw_upper = get_extreme_from_size_f(size,UPPER);
			tmpf = calc_value_f(raw_upper,multiplier,adder,FROMECU);
			upper = g_strdup_printf("%i",(gint)tmpf);
			freeupper = TRUE;
		}
		range = g_strdup_printf("Valid Range: %s <-> %s",lower,upper);
		if (freelower)
			g_free(lower);
		if (freeupper)
			g_free(upper);
		if ((multiplier) && (adder))
		{
			raw_lower = (GINT)((real_lower - (*adder))/(*multiplier));
			raw_upper = (GINT)((real_upper - (*adder))/(*multiplier));
		}
		else if (multiplier)
		{
			raw_lower = (GINT)(real_lower/(*multiplier));
			raw_upper = (GINT)(real_upper/(*multiplier));
		}
		else
		{
			raw_lower = (GINT)real_lower;
			raw_upper = (GINT)real_upper;
		}
		raw_lower_str = g_strdup_printf("%i",raw_lower);
		raw_upper_str = g_strdup_printf("%i",raw_upper);

		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,
				UO_CHOICE_COL,name,
				UO_BITVAL_COL,bitval,
				UO_FROMECU_MULT_COL,multiplier,
				UO_FROMECU_ADD_COL,adder,
				UO_RAW_LOWER_COL,raw_lower_str,
				UO_RAW_UPPER_COL,raw_upper_str,
				UO_RANGE_COL,range,
				UO_SIZE_COL,size,
				UO_PRECISION_COL,precision,
				-1);
		g_free(raw_lower_str);
		g_free(raw_upper_str);
		g_free(range);
	}
/*#if GTK_MINOR_VERSION < 24 */
	if (GTK_IS_COMBO_BOX_ENTRY(widget))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget),UO_CHOICE_COL);
/*
#else
	if (GTK_IS_COMBO_BOX(widget))
	{
		gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(widget),UO_CHOICE_COL);
#endif
*/
		gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(store));
		g_object_unref(store);
		entry = gtk_bin_get_child(GTK_BIN(widget));
		/* Nasty hack, but otherwise the entry is an obnoxious size.. */
		if ((width = (GINT)OBJ_GET((GtkWidget *)widget,"max_chars")) > 0)
			gtk_entry_set_width_chars(GTK_ENTRY(entry),width);
		else
			gtk_entry_set_width_chars(GTK_ENTRY(entry),12);

		gtk_widget_set_size_request(GTK_WIDGET(widget),-1,(3*(GINT)DATA_GET(global_data,"font_size")));

//		gtk_container_remove (GTK_CONTAINER (widget), gtk_bin_get_child(GTK_BIN(widget)));
//		gtk_container_add (GTK_CONTAINER (widget), entry);

		completion = gtk_entry_completion_new();
		gtk_entry_set_completion(GTK_ENTRY(entry),completion);
		g_object_unref(completion);
		gtk_entry_completion_set_model(completion,GTK_TREE_MODEL(store));
		gtk_entry_completion_set_text_column(completion,UO_CHOICE_COL);
		gtk_entry_completion_set_inline_completion(completion,TRUE);
		gtk_entry_completion_set_inline_selection(completion,TRUE);
		gtk_entry_completion_set_popup_single_match(completion,FALSE);
		OBJ_SET(widget,"arrow-size",GINT_TO_POINTER(1));
	}
	EXIT();
	return;
}


/*!
  \brief updates the ms2 user_outputs controls which are a bit special purpose
  \param widget is the pointer to the widget to update
  */
G_MODULE_EXPORT void update_ms2_user_outputs(GtkWidget *widget)
{
	GtkTreeModel *model = NULL;
	DataSize size = MTX_U08;
	GtkTreeIter iter;
	gint tmpi = 0;
	gint t_bitval = 0;
	gboolean valid = FALSE;
	gfloat * multiplier = NULL;
	gfloat * adder = NULL;
	gchar *lower = NULL;
	gchar *upper = NULL;
	gchar *range = NULL;
	gchar *tmpbuf = NULL;
	gint bitmask = 0;
	gint bitshift = 0;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	gfloat value = 0.0;
	gint i = 0;
	gint precision = 0;
	void *eval = NULL;
	GtkWidget *tmpwidget = NULL;

	ENTER();
	get_essential_bits_f(widget, NULL, NULL, NULL, NULL, &bitmask, &bitshift);

	value = convert_after_upload_f(widget);
	tmpi = ((GINT)value & bitmask) >> bitshift;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model),&iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,UO_BITVAL_COL,&t_bitval,-1);
		if (tmpi == t_bitval)
		{
			/* Get the rest of the data from the combo */
			gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,UO_SIZE_COL,&size,UO_RAW_LOWER_COL,&lower,UO_RAW_UPPER_COL,&upper,UO_RANGE_COL,&range,UO_PRECISION_COL,&precision,UO_FROMECU_MULT_COL,&multiplier,UO_FROMECU_ADD_COL,&adder,-1);
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
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				OBJ_SET(tmpwidget,"raw_lower",lower);
				OBJ_SET(tmpwidget,"raw_upper",upper);
				if (multiplier)
					OBJ_SET(tmpwidget,"fromecu_mult",multiplier);
				else
					OBJ_SET(tmpwidget,"fromecu_mult",NULL);
				if (adder)
					OBJ_SET(tmpwidget,"fromecu_add",adder);
				else
					OBJ_SET(tmpwidget,"fromecu_add",NULL);
				update_widget_f(tmpwidget,NULL);
			}
			tmpbuf = (gchar *)OBJ_GET(widget,"hyst_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				OBJ_SET(tmpwidget,"raw_lower",lower);
				OBJ_SET(tmpwidget,"raw_upper",upper);
				if (multiplier)
					OBJ_SET(tmpwidget,"fromecu_mult",multiplier);
				else
					OBJ_SET(tmpwidget,"fromecu_mult",NULL);
				if (adder)
					OBJ_SET(tmpwidget,"fromecu_add",adder);
				else
					OBJ_SET(tmpwidget,"fromecu_add",NULL);
				update_widget_f(tmpwidget,NULL);
			}
			g_free(range);
			g_free(lower);
			g_free(upper);
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		i++;
	}
	EXIT();
	return;
}

/*!
 * \brief finds searching elements of vector for a match from the list 
 * of potentials.
 * \param vectore,  vector of strings to check against
 * \param names, command separatend list of strings tosearch for in vector
 *\reutns true if match made, false  otherwise
 * */
gboolean find_in_list(gchar **vector, gchar *names)
{
	gboolean retval = FALSE;
	gchar ** potentials = g_strsplit(names,",",-1);
	ENTER();
	for (int i=0;i<g_strv_length(potentials);i++)
	{
		for (int j=0;j<g_strv_length(vector);j++)
			if (0 == g_ascii_strcasecmp(potentials[i],vector[j]))
				retval = TRUE;
	}
	g_strfreev(potentials);
	EXIT();
	return retval;
}

