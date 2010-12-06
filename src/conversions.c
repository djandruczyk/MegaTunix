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

#include <assert.h>
#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <dep_processor.h>
#include <enums.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <lookuptables.h>
#include <notifications.h>
#include <mtxmatheval.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <stdlib.h>
#include <tabloader.h>



/*!
 \brief convert_before_download() converts the value passed using the
 conversions bound to the widget
 \param widget (GtkWidget *) widget to extract the conversion info from
 \param value (gfloat *) the "real world" value from the tuning gui before
 translation to MS-units
 \returns the integere ms-units form after conversion
 */
G_MODULE_EXPORT gint convert_before_download(GtkWidget *widget, gfloat value)
{
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	gint return_value = 0;
	gint tmpi = 0;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	gint page = -1;
	gint offset = -1;
	DataSize size = MTX_U08;
	float lower = 0.0;
	float upper = 0.0;
	guint i = 0;
	GHashTable *hash = NULL;
	gchar *key_list = NULL;
	gchar *expr_list = NULL;
	gchar **keys = NULL;
	gchar **exprs = NULL;
	gint table_num = 0;
	gchar *tmpbuf = NULL;
	gchar * source_key = NULL;
	gchar * hash_key = NULL;
	gint *algorithm = NULL;
	GHashTable *sources_hash = NULL;
	extern gconstpointer *global_data;

	sources_hash = DATA_GET(global_data,"sources_hash");
	algorithm = DATA_GET(global_data,"algorithm");

	g_static_mutex_lock(&mutex);

	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");

	if (!OBJ_GET(widget,"size"))
		printf(__FILE__"%s %i %s %i\n",_(": convert_before_download, FATAL ERROR, size undefined for widget at page"),page,_("offset"),offset);

	size = (DataSize)OBJ_GET(widget,"size");

	if (OBJ_GET(widget,"raw_lower"))
		lower = (gfloat)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		lower = (gfloat)get_extreme_from_size(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		upper = (gfloat)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		upper = (gfloat)get_extreme_from_size(size,UPPER);

	if (OBJ_GET(widget,"multi_expr_keys"))
	{
		if (!OBJ_GET(widget,"dl_eval_hash"))
		{
			hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,evaluator_destroy);
			key_list = OBJ_GET(widget,"multi_expr_keys");
			expr_list = OBJ_GET(widget,"dl_conv_exprs");
			keys = g_strsplit(key_list,",",-1);
			exprs = g_strsplit(expr_list,",",-1);
			for (i=0;i<MIN(g_strv_length(keys),g_strv_length(exprs));i++)
			{
				evaluator = evaluator_create(exprs[i]);
				g_hash_table_insert(hash,g_strdup(keys[i]),evaluator);
			}
			g_strfreev(keys);
			g_strfreev(exprs);

			OBJ_SET(widget,"dl_eval_hash",hash);
		}
		hash = OBJ_GET(widget,"dl_eval_hash");
		source_key = OBJ_GET(widget,"source_key");
		if (!source_key)
			printf(_("big problem, source key is undefined!!\n"));
		hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
		tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
		if (tmpbuf)
			table_num = (gint)strtol(tmpbuf,NULL,10);
		if (table_num == -1)
		{
			if (!hash_key)
				evaluator = g_hash_table_lookup(hash,"DEFAULT");
			else
			{
				evaluator = g_hash_table_lookup(hash,(gchar *)hash_key);
				if (!evaluator)
					evaluator = g_hash_table_lookup(hash,"DEFAULT");
			}
		}
		else
		{
			switch (algorithm[table_num])
			{
				case SPEED_DENSITY:
					if (!hash_key)
						evaluator = g_hash_table_lookup(hash,"DEFAULT");
					else
					{
						evaluator = g_hash_table_lookup(hash,(gchar *)hash_key);
						if (!evaluator)
							evaluator = g_hash_table_lookup(hash,"DEFAULT");
					}
					break;
				case ALPHA_N:
					evaluator = g_hash_table_lookup(hash,"DEFAULT");
					break;
				case MAF:
					evaluator = g_hash_table_lookup(hash,"AFM_VOLTS");
					if (!evaluator)
						evaluator = g_hash_table_lookup(hash,"DEFAULT");
					break;
			}
		}
	}
	else
	{
		conv_expr = (gchar *)OBJ_GET(widget,"dl_conv_expr");
		evaluator = (void *)OBJ_GET(widget,"dl_evaluator");

		if ((conv_expr) && (!evaluator))
		{
			evaluator = evaluator_create(conv_expr);
			assert(evaluator);
			OBJ_SET(widget,"dl_evaluator",(gpointer)evaluator);
		}
	}
	if (!evaluator)
	{
		dbg_func(CONVERSIONS,g_strdup_printf(__FILE__": convert_before_dl()\n\tNO CONVERSION defined for page: %i, offset: %i, value %i\n",page, offset, (gint)value));
		if(value > upper)
		{
			dbg_func(CONVERSIONS|CRITICAL,g_strdup_printf(__FILE__": convert_before_download()\n\t WARNING value clamped at %f (no eval)!!\n",upper));
			value = upper;
		}
		if (value < lower)
		{
			dbg_func(CONVERSIONS|CRITICAL,g_strdup_printf(__FILE__": convert_before_download()\n\t WARNING value clamped at %f (no eval)!!\n",lower));
			value = lower;
		}
		return_value = (gint)value;
	}
	else
	{
		return_value = evaluator_evaluate_x(evaluator,value); 

		dbg_func(CONVERSIONS,g_strdup_printf(__FILE__": convert_before_dl():\n\tpage %i, offset %i, raw %.2f, sent %i\n",page, offset,value,return_value));

		if (return_value > upper)
		{
			dbg_func(CONVERSIONS|CRITICAL,g_strdup_printf(__FILE__": convert_before_download()\n\t WARNING value clamped at %f (ecu units, evaluated)!!\n",upper));
			return_value = upper;
		}
		if (return_value < lower)
		{
			dbg_func(CONVERSIONS|CRITICAL,g_strdup_printf(__FILE__": convert_before_download()\n\t WARNING value clamped at %f (ecu units, evaluated)!!\n",lower));
			return_value = lower;
		}
	}

	tmpi = return_value;
	 if (OBJ_GET(widget,"lookuptable"))
		return_value = (gint)reverse_lookup_obj(G_OBJECT(widget),tmpi);

	g_static_mutex_unlock(&mutex);
	return (return_value);
}


/*!
 \brief convert_after_upload() converts the ms-units data to the real world
 units for display on the GUI
 \param widget (GtkWidget *) to extract the conversion info from to perform
 the necessary math
 \returns the real world value for the GUI
 */
G_MODULE_EXPORT gfloat convert_after_upload(GtkWidget * widget)
{
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	gfloat return_value = 0.0;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	gint tmpi = 0;
	gint page = -1;
	gint offset = -1;
	gint canID = 0;
	DataSize size = 0;
	gboolean ul_complex = FALSE;
	guint i = 0;
	gint table_num = -1;
	GHashTable *hash = NULL;
	gchar *key_list = NULL;
	gchar *expr_list = NULL;
	gchar **keys = NULL;
	gchar **exprs = NULL;
	gchar * tmpbuf = NULL;
	gchar * source_key = NULL;
	gchar * hash_key = NULL;
	gint *algorithm = NULL;
	GHashTable *sources_hash = NULL;
	extern gconstpointer *global_data;

	sources_hash = DATA_GET(global_data,"sources_hash");
	algorithm = DATA_GET(global_data,"algorithm");

	g_static_mutex_lock(&mutex);

	ul_complex = (GBOOLEAN)OBJ_GET(widget,"ul_complex");
	if (ul_complex)
	{
		g_static_mutex_unlock(&mutex);
		/*printf("Complex upload conversion for widget %s\n",glade_get_widget_name(widget));*/
		return handle_complex_expr_obj(G_OBJECT(widget),NULL,UPLOAD);
	}

	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	canID = (GINT)OBJ_GET(widget,"canID");
	size = (DataSize)OBJ_GET(widget,"size");
	if (size == 0)
		printf(_("BIG PROBLEM, size undefined! at page %i, offset %i, widget %s\n"),page,offset,(gchar *)glade_get_widget_name(widget));

	if (OBJ_GET(widget,"multi_expr_keys"))
	{
		if (!OBJ_GET(widget,"ul_eval_hash"))
		{
			hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,evaluator_destroy);
			key_list = OBJ_GET(widget,"multi_expr_keys");
			expr_list = OBJ_GET(widget,"ul_conv_exprs");
			keys = g_strsplit(key_list,",",-1);
			exprs = g_strsplit(expr_list,",",-1);
			for (i=0;i<MIN(g_strv_length(keys),g_strv_length(exprs));i++)
			{
				evaluator = evaluator_create(exprs[i]);
				g_hash_table_insert(hash,g_strdup(keys[i]),evaluator);
			}
			g_strfreev(keys);
			g_strfreev(exprs);

			OBJ_SET(widget,"ul_eval_hash",hash);
		}
		hash = OBJ_GET(widget,"ul_eval_hash");
		source_key = OBJ_GET(widget,"source_key");
		if (!source_key)
			printf(_("big problem, source key is undefined!!\n"));
		hash_key = (gchar *)g_hash_table_lookup(sources_hash,source_key);
		tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
		if (tmpbuf)
			table_num = (gint)strtol(tmpbuf,NULL,10);
		if (table_num == -1)
		{
			if (!hash_key)
				evaluator = g_hash_table_lookup(hash,"DEFAULT");
			else
			{
				evaluator = g_hash_table_lookup(hash,hash_key);
				if (!evaluator)
					evaluator = g_hash_table_lookup(hash,"DEFAULT");
			}
		}
		else
		{
			switch (algorithm[table_num])
			{
				case SPEED_DENSITY:
					if (!hash_key)
						evaluator = g_hash_table_lookup(hash,"DEFAULT");
					else
					{
						evaluator = g_hash_table_lookup(hash,hash_key);
						if (!evaluator)
							evaluator = g_hash_table_lookup(hash,"DEFAULT");
					}
					break;
				case ALPHA_N:
					evaluator = g_hash_table_lookup(hash,"DEFAULT");
					break;
				case MAF:
					evaluator = g_hash_table_lookup(hash,"AFM_VOLTS");
					if (!evaluator)
						evaluator = g_hash_table_lookup(hash,"DEFAULT");
					break;
			}
		}
	}
	else
	{
		conv_expr = (gchar *)OBJ_GET(widget,"ul_conv_expr");
		evaluator = (void *)OBJ_GET(widget,"ul_evaluator");
		if ((conv_expr) && (!evaluator)) 	/* if no evaluator create one */
		{
			evaluator = evaluator_create(conv_expr);
			assert(evaluator);
			OBJ_SET(widget,"ul_evaluator",(gpointer)evaluator);
		}

	}
	if (OBJ_GET(widget,"lookuptable"))
		tmpi = lookup_data_obj(G_OBJECT(widget),get_ecu_data(canID,page,offset,size));
	else
	{
		/*printf("getting data at canid %i, page %i, offset %i, size %i\n",canID,page,offset,size);*/
		tmpi = get_ecu_data(canID,page,offset,size);
		/*printf("value is %i\n",tmpi);*/
	}


	if (!evaluator)
	{
		return_value = tmpi;
		dbg_func(CONVERSIONS,g_strdup_printf(__FILE__": convert_after_ul():\n\tNO CONVERSION defined for page: %i, offset: %i, value %f\n",page, offset, return_value));
		g_static_mutex_unlock(&mutex);
		return (return_value);		
	}
	/*return_value = evaluator_evaluate_x(evaluator,tmpi)+0.0001; */
	return_value = evaluator_evaluate_x(evaluator,tmpi);

	dbg_func(CONVERSIONS,g_strdup_printf(__FILE__": convert_after_ul()\n\t page %i,offset %i, raw %i, val %f\n",page,offset,tmpi,return_value));
	g_static_mutex_unlock(&mutex);
	return (return_value);
}


/*!
 \brief convert_temps() changes the values of controls based on the currently
 selected temperature scale.  IT works for labels, spinbuttons, etc...
 \param widget (gpointer) pointer to the widget that contains the necessary
 paramaters re temp (Alt label, etc)
 \param units (gpointer) the temp scale selected
 */
G_MODULE_EXPORT void convert_temps(gpointer widget, gpointer units)
{
	gconstpointer *dep_obj = NULL;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gfloat value = 0.0;
	gchar * text = NULL;
	GtkAdjustment * adj = NULL;
	gboolean state = FALSE;
	gint widget_temp = -1;
	extern GdkColor black;
	extern gconstpointer *global_data;

	/* If this widgt depends on anything call check_dependancy which will
	 * return TRUE/FALSE.  True if what it depends on is in the matching
	 * state, FALSE otherwise...
	 */
	if ((!widget) || (DATA_GET(global_data,"leaving")))
		return;
	dep_obj = (gconstpointer *)OBJ_GET(widget,"dep_object");
	widget_temp = (GINT)OBJ_GET(widget,"widget_temp");
	if (dep_obj)
		state = check_dependancies(dep_obj);

	if ((GINT)units == FAHRENHEIT) 
	{
		/*printf("fahr %s\n",glade_get_widget_name(widget));*/
		if (GTK_IS_LABEL(widget))
		{
			if ((dep_obj) && (state))	
				text = (gchar *)OBJ_GET(widget,"alt_f_label");
			else
				text = (gchar *)OBJ_GET(widget,"f_label");
			gtk_label_set_text(GTK_LABEL(widget),text);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));

		}
		if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp == CELSIUS))
		{

			adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(widget));
			upper = adj->upper;
			value = adj->value;
			lower = adj->lower;
			adj->value = ((value *(9.0/5.0))+32)+0.001;
			adj->upper = ((upper *(9.0/5.0))+32)+0.001;
			adj->lower = ((lower *(9.0/5.0))+32)+0.001;

			gtk_adjustment_changed(adj);
			/*
			gtk_spin_button_set_value(
					GTK_SPIN_BUTTON(widget),
					adj->value);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			*/
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			update_widget(widget,NULL);
		}
		if ((GTK_IS_ENTRY(widget)) && (widget_temp == CELSIUS))
		{
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			update_widget(widget,NULL);
		}
		if ((GTK_IS_RANGE(widget)) && (widget_temp == CELSIUS))
		{
			adj = (GtkAdjustment *) gtk_range_get_adjustment(
					GTK_RANGE(widget));
			upper = adj->upper;
			lower = adj->lower;
			value = adj->value;
			adj->value = ((value *(9.0/5.0))+32)+0.001;
			adj->lower = ((lower *(9.0/5.0))+32)+0.001;
			adj->upper = ((upper *(9.0/5.0))+32)+0.001;

			gtk_range_set_adjustment(GTK_RANGE(widget),adj);
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
		}
	}
	else
	{
		/*printf("cels %s\n",glade_get_widget_name(widget));*/
		if (GTK_IS_LABEL(widget))
		{
			if ((dep_obj) && (state))	
				text = (gchar *)OBJ_GET(widget,"alt_c_label");
			else
				text = (gchar *)OBJ_GET(widget,"c_label");
			gtk_label_set_text(GTK_LABEL(widget),text);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
		}

		if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp == FAHRENHEIT))
		{
			adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(widget));
			upper = adj->upper;
			lower = adj->lower;
			value = adj->value;
			adj->value = ((value-32)*(5.0/9.0))+0.001;
			adj->lower = ((lower-32)*(5.0/9.0))+0.001;
			adj->upper = ((upper-32)*(5.0/9.0))+0.001;
			gtk_adjustment_changed(adj);
			/*
			gtk_spin_button_set_value(
					GTK_SPIN_BUTTON(widget),
					adj->value);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			*/
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			update_widget(widget,NULL);
		}
		if ((GTK_IS_ENTRY(widget)) && (widget_temp == FAHRENHEIT))
		{
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
			update_widget(widget,NULL);
		}
		if ((GTK_IS_RANGE(widget)) && (widget_temp == FAHRENHEIT))
		{
			adj = (GtkAdjustment *) gtk_range_get_adjustment(
					GTK_RANGE(widget));
			upper = adj->upper;
			lower = adj->lower;
			value = adj->value;
			adj->value = ((value-32)*(5.0/9.0))+0.001;
			adj->lower = ((lower-32)*(5.0/9.0))+0.001;
			adj->upper = ((upper-32)*(5.0/9.0))+0.001;

			gtk_range_set_adjustment(GTK_RANGE(widget),adj);
			OBJ_SET(widget,"widget_temp",GINT_TO_POINTER(units));
		}
	}

}


/*!
 \brief reset_temps() calls the convert_temps function for each widget in
 the "temperature" list
 \param type (gpointer) the temp scale now selected
 */
G_MODULE_EXPORT void reset_temps(gpointer type)
{
	/* Better way.. :) */
	g_list_foreach(get_list("temperature"),convert_temps,type);

}
