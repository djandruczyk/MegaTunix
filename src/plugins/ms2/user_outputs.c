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
	gboolean temp_dep = FALSE;
	gchar *raw_lower_str = NULL;
	gchar *raw_upper_str = NULL;
	gchar *tempc_range = NULL;
	gchar *tempf_range = NULL;
	gchar *tempk_range = NULL;
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
	store = gtk_list_store_new(UO_COMBO_COLS,
			G_TYPE_STRING,	/* Choice */
			G_TYPE_UCHAR,	/* BITval */
			G_TYPE_POINTER,	/* FromECU Multiplier (gfloat *) */
			G_TYPE_POINTER,	/* FromECU Adder (gfloat *) */
			G_TYPE_STRING,	/* Raw Lower clamp limit */
			G_TYPE_STRING,	/* Raw Upper clamp limit */
			G_TYPE_STRING,	/* Range widget string (non temp ctrls) */
			G_TYPE_STRING,	/* Range widget string (Celsius) */
			G_TYPE_STRING,	/* Range widget string (Fahrenheit) */
			G_TYPE_STRING,	/* Range widget string (Kelvin) */
			G_TYPE_INT,		/* Size enumeration (_U08_, _U16_, etc.) */
			G_TYPE_UCHAR,	/* Precision (floating point precision) */
			G_TYPE_BOOLEAN);/* Temp dependent flag */

	/* Iterate across valid variables */
	for (i=0;i<rtv_map->rtv_list->len;i++)
	{
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

		temp_dep = (GBOOLEAN)DATA_GET(object,"temp_dep");
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
			raw_lower = get_extreme_from_size_f(size,LOWER);
		if (DATA_GET(object,"real_upper"))
		{
			upper = (gchar *)DATA_GET(object,"real_upper");
			real_upper = g_strtod(upper,NULL);
			raw_upper = calc_value_f(real_upper,multiplier,adder,TOECU);
		}
		else
			raw_upper = get_extreme_from_size_f(size,UPPER);

		range = g_strdup_printf("Valid Range: %.1f <-> %.1f",real_lower,real_upper);
		if (temp_dep)
		{
			tempc_range = g_strdup_printf("Valid Range: %.1f\302\260C <-> %.1f\302\260C",f_to_c_f(real_lower),f_to_c_f(real_upper));
			tempf_range = g_strdup_printf("Valid Range: %.1f\302\260F <-> %.1f\302\260F",real_lower,real_upper);
			tempk_range = g_strdup_printf("Valid Range: %.1f\302\260K <-> %.1f\302\260K",f_to_k_f(real_lower),f_to_k_f(real_upper));
		}
		else
		{
			tempc_range = g_strdup(range);
			tempf_range = g_strdup(range);
			tempk_range = g_strdup(range);
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
				UO_RANGE_TEMPC_COL,tempc_range,
				UO_RANGE_TEMPF_COL,tempf_range,
				UO_RANGE_TEMPK_COL,tempk_range,
				UO_SIZE_COL,size,
				UO_PRECISION_COL,precision,
				UO_TEMP_DEP_COL,temp_dep,
				-1);
		g_free(raw_lower_str);
		g_free(raw_upper_str);
		g_free(range);
		g_free(tempc_range);
		g_free(tempf_range);
		g_free(tempk_range);
	}
	if (GTK_IS_COMBO_BOX_ENTRY(widget))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget),UO_CHOICE_COL);
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
	gchar *tempc_range = NULL;
	gchar *tempf_range = NULL;
	gchar *tempk_range = NULL;
	gchar *tmpbuf = NULL;
	gchar *tmpstr = NULL;
	gint bitmask = 0;
	gint bitshift = 0;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	gfloat value = 0.0;
	gboolean temp_dep = FALSE;
	gint i = 0;
	gint precision = 0;
	void *eval = NULL;
	GtkWidget *tmpwidget = NULL;

	ENTER();
	/*printf("update_ms2_user_outputs widget %s %p\n",glade_get_widget_name(widget),(void *)widget);*/
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
			gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,
					UO_TEMP_DEP_COL,&temp_dep,
					UO_SIZE_COL,&size,
					UO_RAW_LOWER_COL,&lower,
					UO_RAW_UPPER_COL,&upper,
					UO_RANGE_COL,&range,
					UO_RANGE_TEMPC_COL,&tempc_range,
					UO_RANGE_TEMPF_COL,&tempf_range,
					UO_RANGE_TEMPK_COL,&tempk_range,
					UO_PRECISION_COL,&precision,
					UO_FROMECU_MULT_COL,&multiplier,
					UO_FROMECU_ADD_COL,&adder,-1);
			/*
			if (temp_dep)
			{
				printf("THIS WIDGET IS TEMP DEPENDENT\n");
				printf("raw_lower %s, raw_upper %s in treemodel\n",lower,upper);
				if (multiplier)
					printf("multiplier %f\n",*multiplier);
				if (adder)
					printf("adder %f\n",*adder);
				printf("size %i, precision %i, temp_dep %i\n",(gint)size,precision,(gint)temp_dep);
			}
			*/
			tmpbuf = (gchar *)OBJ_GET(widget,"range_label");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_LABEL(tmpwidget))
			{
				if (temp_dep)
				{
					OBJ_SET(tmpwidget,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
					OBJ_SET(tmpwidget,"c_label",tempc_range);
					OBJ_SET(tmpwidget,"f_label",tempf_range);
					OBJ_SET(tmpwidget,"k_label",tempk_range);
					bind_to_lists_f(tmpwidget,"temperature");
					gtk_label_set_text(GTK_LABEL(tmpwidget),tempf_range);
					convert_temps_f(tmpwidget,DATA_GET(global_data,"mtx_temp_units"));
				}
				else
				{
					remove_from_lists_f("temperature",tmpwidget);
					OBJ_SET(tmpwidget,"widget_temp",NULL);
					OBJ_SET(tmpwidget,"temp_dep",NULL);
					/*OBJ_SET(tmpwidget,"c_label",NULL);
					  OBJ_SET(tmpwidget,"f_label",NULL);
					  OBJ_SET(tmpwidget,"k_label",NULL);
					  */
					gtk_label_set_text(GTK_LABEL(tmpwidget),range);
				}
			}
			tmpbuf = (gchar *)OBJ_GET(widget,"thresh_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				if (temp_dep)
				{
					OBJ_SET(tmpwidget,"widget_temp",DATA_GET(global_data,"mtx_temp_units"));
					OBJ_SET(tmpwidget,"temp_dep",GINT_TO_POINTER(temp_dep));
					bind_to_lists_f(tmpwidget,"temperature");
				}
				else
				{
					OBJ_SET(tmpwidget,"widget_temp",NULL);
					OBJ_SET(tmpwidget,"temp_dep",NULL);
					remove_from_lists_f("temperature",tmpwidget);
				}
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
				//convert_temps_f(tmpwidget,DATA_GET(global_data,"mtx_temp_units"));
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

