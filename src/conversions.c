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
#include <listmgmt.h>
#include <notifications.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <rtv_processor.h>
#include <stdio.h>
#include <structures.h>
#include <tabloader.h>


gint convert_before_download(GtkWidget *widget, gfloat value)
{
	gint return_value = 0;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	gint page = -1;
	gint offset = -1;
	extern gint **ms_data;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	conv_expr = (gchar *)g_object_get_data(G_OBJECT(widget),"dl_conv_expr");
	evaluator = (void *)g_object_get_data(G_OBJECT(widget),"dl_evaluator");

	if (conv_expr == NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": convert_before_dl()\n\tNO CONVERSION defined for page: %i, offset: %i, value %i\n",page, offset, (gint)value),CONVERSIONS);
		ms_data[page][offset] = (gint)value;
		g_static_mutex_unlock(&mutex);
		return ((gint)value);		
	}
	if (!evaluator) 	/* if no evaluator create one */
	{
		evaluator = evaluator_create(conv_expr);
		assert(evaluator);
		g_object_set_data(G_OBJECT(widget),"dl_evaluator",(gpointer)evaluator);
	}
	return_value = evaluator_evaluate_x(evaluator,value)+0.001;

	dbg_func(g_strdup_printf(__FILE__": convert_before_dl():\n\tpage %i, offset %i, raw %.2f, sent %i\n",page, offset,value,return_value),CONVERSIONS);

	ms_data[page][offset] = return_value;

	g_static_mutex_unlock(&mutex);
	return (return_value);
}

gfloat convert_after_upload(GtkWidget * widget)
{
	gfloat return_value = 0.0;
	gint *ve_const_arr = NULL;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	extern gint **ms_data;
	gint page = -1;
	gint offset = -1;
	gboolean ul_complex = FALSE;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	ul_complex = (gboolean)g_object_get_data(G_OBJECT(widget),"ul_complex");
	if (ul_complex)
	{
		g_static_mutex_unlock(&mutex);
		return handle_complex_expr(G_OBJECT(widget),NULL,UPLOAD);
	}

	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	conv_expr = (gchar *)g_object_get_data(G_OBJECT(widget),"ul_conv_expr");
	evaluator = (void *)g_object_get_data(G_OBJECT(widget),"ul_evaluator");

	ve_const_arr = (gint *)ms_data[page];
	if (conv_expr == NULL)
	{
		return_value = ve_const_arr[offset];
		dbg_func(g_strdup_printf(__FILE__": convert_after_ul():\n\tNO CONVERSION defined for page: %i, offset: %i, value %f\n",page, offset, return_value),CONVERSIONS);
		g_static_mutex_unlock(&mutex);
		return (return_value);		
	}
	if (!evaluator) 	/* if no evaluator create one */
	{
		evaluator = evaluator_create(conv_expr);
		assert(evaluator);
		g_object_set_data(G_OBJECT(widget),"ul_evaluator",(gpointer)evaluator);
	}
	return_value = evaluator_evaluate_x(evaluator,ve_const_arr[offset])+0.001;

	dbg_func(g_strdup_printf(__FILE__": convert_after_ul()\n\t page %i,offset %i, raw %i, val %f\n",page,offset,ve_const_arr[offset],return_value),CONVERSIONS);
	g_static_mutex_unlock(&mutex);
	return (return_value);
}

void convert_temps(gpointer widget, gpointer units)
{
	gfloat upper = 0.0;
	gfloat value = 0.0;
	GtkAdjustment * adj = NULL;
	gchar *text = NULL;
	gchar *depend_on = NULL;
	gboolean state = FALSE;

	/* If this widgt depends on anything call check_dependancy which will
	 * return TRUE/FALSE.  True if what it depends on is in the matching
	 * state, FALSE otherwise...
	 */
	depend_on = (gchar *)g_object_get_data(G_OBJECT(widget),"depend_on");
	if (depend_on)
		state = check_dependancy(G_OBJECT(widget));

	if ((int)units == FAHRENHEIT)
	{
		if (GTK_IS_LABEL(widget))
		{
			if ((depend_on) && (state))	
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"alt_f_label");
			else
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"f_label");
			gtk_label_set_text(GTK_LABEL(widget),g_strdup(text));
		}

		if (GTK_IS_SPIN_BUTTON(widget))
		{
			adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(widget));
			upper = adj->upper;
			if (upper < 215) /* if so it was celsius, if not skip*/
			{
				value = adj->value;
				adj->upper = 215.0;
				adj->value = (value *(9.0/5.0))+32;

				gtk_adjustment_changed(adj);
				gtk_spin_button_set_value(
						GTK_SPIN_BUTTON(widget),
						adj->value);
			}
		}

	}
	else // CELSIUS Temp scale
	{
		if (GTK_IS_LABEL(widget))
		{
			if ((depend_on) && (state))	
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"alt_c_label");
			else
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"c_label");
			gtk_label_set_text(GTK_LABEL(widget),g_strdup(text));
		}

		if (GTK_IS_SPIN_BUTTON(widget))
		{
			adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(widget));
			upper = adj->upper;
			if (upper > 102) // if so it was fahren, if not skip
			{	
				value = adj->value;
				adj->upper = 101.6;
				adj->value = (value-32)*(5.0/9.0);
				gtk_adjustment_changed(adj);
				gtk_spin_button_set_value(
						GTK_SPIN_BUTTON(widget),
						adj->value);
			}
		}
	}

}

void reset_temps(gpointer type)
{
	/* Better way.. :) */
	g_list_foreach(get_list("temperature"),convert_temps,type);

}
