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
#include <enums.h>
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
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];
	gint page = -1;
	gint offset = -1;

	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	conv_expr = (gchar *)g_object_get_data(G_OBJECT(widget),"dl_conv_expr");
	evaluator = (void *)g_object_get_data(G_OBJECT(widget),"dl_evaluator");

	if (conv_expr == NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": convert_before_dl(): NO CONVERSION defined for page: %i, offset: %i\n",page, offset),DL_CONV);
		ms_data[page][offset] = (gint)value; 
		return ((gint)value);		
	}
	if (!evaluator) 	/* if no evaluator create one */
	{
		evaluator = evaluator_create(conv_expr);
		assert(evaluator);
		g_object_set_data(G_OBJECT(widget),"dl_evaluator",(gpointer)evaluator);
	}
	return_value = evaluator_evaluate_x(evaluator,value)+0.001;

	dbg_func(g_strdup_printf(__FILE__": convert_before_dl(): page %i, offset %i, raw %.2f, sent %i\n",page, offset,value,return_value),DL_CONV);

	ms_data[page][offset] = return_value; 
	return (return_value);
}

gfloat convert_after_upload(GtkWidget * widget)
{
	gfloat return_value = 0.0;
	guchar *ve_const_arr;
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];
	gchar * conv_expr;
	void *evaluator = NULL;
	gint page = -1;
	gint offset = -1;
	gboolean ul_complex = FALSE;

	ul_complex = (gboolean)g_object_get_data(G_OBJECT(widget),"ul_complex");
	if (ul_complex)
		return handle_complex_expr(G_OBJECT(widget),NULL);

	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	conv_expr = (gchar *)g_object_get_data(G_OBJECT(widget),"ul_conv_expr");
	evaluator = (void *)g_object_get_data(G_OBJECT(widget),"ul_evaluator");

	ve_const_arr = (guchar *)ms_data[page];
	if (conv_expr == NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": convert_after_ul(): NO CONVERSION defined for page: %i, offset: %i\n",page, offset),UL_CONV);

		return_value = ve_const_arr[offset];
		return (return_value);		
	}
	if (!evaluator) 	/* if no evaluator create one */
	{
		evaluator = evaluator_create(conv_expr);
		assert(evaluator);
		g_object_set_data(G_OBJECT(widget),"ul_evaluator",(gpointer)evaluator);
	}
	return_value = evaluator_evaluate_x(evaluator,ve_const_arr[offset])+0.001;

	dbg_func(g_strdup_printf(__FILE__": convert_after_ul(),offset %i, raw %i, val %f, page %i\n",offset,ve_const_arr[offset],return_value,page),UL_CONV);
	return (return_value);
}

void convert_temps(gpointer widget, gpointer units)
{
	gfloat upper = 0.0;
	gfloat value = 0.0;
	GtkAdjustment * adj = NULL;
	gchar *text = NULL;
	gchar *alt_depend_on = NULL;
	gboolean state = FALSE;
	extern GHashTable *dynamic_widgets;

	/* If widget has the "alt_depend_on" data bound to it (A widget name)
	 * then get state of that widget (toggle button only so far) so that
	 * we can use the right labels if needed...
	 */
	alt_depend_on = (gchar *)g_object_get_data(G_OBJECT(widget),"alt_depend_on");
	if (alt_depend_on)
		state = (gboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_hash_table_lookup(dynamic_widgets,alt_depend_on)));

	if ((int)units == FAHRENHEIT)
	{
		if (GTK_IS_LABEL(widget))
		{
			if ((alt_depend_on) && (state))	
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
			if ((alt_depend_on) && (state))	
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
