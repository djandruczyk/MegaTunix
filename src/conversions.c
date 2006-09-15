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



/*!
 \brief convert_before_download() converts the value passed using the
 conversions bound to the widget
 \param widget (GtkWidget *) widget to extract the conversion info from
 \param value (gfloat *) the "real world" value from the tuning gui before
 translation to MS-units
 \returns the integere ms-units form after conversion
 */
gint convert_before_download(GtkWidget *widget, gfloat value)
{
	gint return_value = 0;
	gchar * conv_expr = NULL;
	void *evaluator = NULL;
	gint page = -1;
	gint offset = -1;
	gint lower = -1;
	gint upper = -1;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (NULL == g_object_get_data(G_OBJECT(widget),"raw_lower"))
		lower = 0; // BAD assumption
	else
		lower = (gint)g_object_get_data(G_OBJECT(widget),"raw_lower");

	if (NULL == g_object_get_data(G_OBJECT(widget),"raw_upper"))
		upper = 255; // BAD assumption
	else
		upper = (gint)g_object_get_data(G_OBJECT(widget),"raw_upper");

	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	conv_expr = (gchar *)g_object_get_data(G_OBJECT(widget),"dl_conv_expr");
	evaluator = (void *)g_object_get_data(G_OBJECT(widget),"dl_evaluator");

	if (conv_expr == NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": convert_before_dl()\n\tNO CONVERSION defined for page: %i, offset: %i, value %i\n",page, offset, (gint)value),CONVERSIONS);
		if(value > upper)
		{
			dbg_func(g_strdup(__FILE__": convert_before_download()\n\t WARNING value clamped at 255 (no eval)!!\n"),CRITICAL);
			value = upper;
		}
		if (value < lower)
		{
			dbg_func(g_strdup(__FILE__": convert_before_download()\n\t WARNING value clamped at 0 (no eval)!!\n"),CRITICAL);
			value = lower;
		}
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

	if (return_value > upper)
	{
		dbg_func(g_strdup(__FILE__": convert_before_download()\n\t WARNING value clamped at 255 (evaluated)!!\n"),CRITICAL);
		return_value = upper;
	}
	if (return_value < lower)
	{
		dbg_func(g_strdup(__FILE__": convert_before_download()\n\t WARNING value clamped at 0 (evaluated)!!\n"),CRITICAL);
		return_value = lower;
	}


	g_static_mutex_unlock(&mutex);
	return (return_value);
}


/*!
 \brief convert_after_upload() converts the ms-units dat to the real world
 units for display on the GUI
 \param widget (GtkWidget *) to extract the conversion info from to perform
 the necessary math
 \returns the real world value for the GUI
 */
gfloat convert_after_upload(GtkWidget * widget)
{
	gfloat return_value = 0.0;
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

	if (conv_expr == NULL)
	{
		return_value = ms_data[page][offset];
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
	return_value = evaluator_evaluate_x(evaluator,ms_data[page][offset])+0.001;

	dbg_func(g_strdup_printf(__FILE__": convert_after_ul()\n\t page %i,offset %i, raw %i, val %f\n",page,offset,ms_data[page][offset],return_value),CONVERSIONS);
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
void convert_temps(gpointer widget, gpointer units)
{
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gfloat value = 0.0;
	GtkAdjustment * adj = NULL;
	gchar *text = NULL;
	gchar *depend_on = NULL;
	gboolean state = FALSE;
	gint widget_temp = -1;
	extern GdkColor black;

	/* If this widgt depends on anything call check_dependancy which will
	 * return TRUE/FALSE.  True if what it depends on is in the matching
	 * state, FALSE otherwise...
	 */
	depend_on = (gchar *)g_object_get_data(G_OBJECT(widget),"depend_on");
	widget_temp = (gint)g_object_get_data(G_OBJECT(widget),"widget_temp");
	if (depend_on)
		state = check_dependancies(G_OBJECT(widget));


	if ((int)units == FAHRENHEIT) 
	{
		if (GTK_IS_LABEL(widget))
		{
			if ((depend_on) && (state))	
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"alt_f_label");
			else
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"f_label");
			gtk_label_set_text(GTK_LABEL(widget),text);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			g_object_set_data(G_OBJECT(widget),"widget_temp",GINT_TO_POINTER(units));

		}

		if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp == CELSIUS))
		{

			printf("fahrenheit mode,  spinner was previously celsius,  adjusting\n");
			adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(widget));
			upper = adj->upper;
			value = adj->value;
			lower = adj->lower;
			adj->value = (value *(9.0/5.0))+32;
			adj->upper = (upper *(9.0/5.0))+32;
			adj->lower = (lower *(9.0/5.0))+32;

			gtk_adjustment_changed(adj);
			gtk_spin_button_set_value(
					GTK_SPIN_BUTTON(widget),
					adj->value);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			g_object_set_data(G_OBJECT(widget),"widget_temp",GINT_TO_POINTER(units));
		}
		if ((GTK_IS_RANGE(widget)) && (widget_temp == CELSIUS))
		{
			adj = (GtkAdjustment *) gtk_range_get_adjustment(
					GTK_RANGE(widget));
			upper = adj->upper;
			lower = adj->lower;
			value = adj->value;
			adj->value = (value *(9.0/5.0))+32;
			adj->lower = (lower *(9.0/5.0))+32;
			adj->upper = (upper *(9.0/5.0))+32;

			gtk_range_set_adjustment(GTK_RANGE(widget),adj);
			g_object_set_data(G_OBJECT(widget),"widget_temp",GINT_TO_POINTER(units));
		}
	}
	else
	{
		if (GTK_IS_LABEL(widget))
		{
			if ((depend_on) && (state))	
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"alt_c_label");
			else
				text = (gchar *)g_object_get_data(G_OBJECT(widget),"c_label");
			gtk_label_set_text(GTK_LABEL(widget),text);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			g_object_set_data(G_OBJECT(widget),"widget_temp",GINT_TO_POINTER(units));
		}

		if ((GTK_IS_SPIN_BUTTON(widget)) && (widget_temp == FAHRENHEIT))
		{
			printf("celsius mode,  spinner was previously fahrenheit,  adjusting\n");
			adj = (GtkAdjustment *) gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(widget));
			upper = adj->upper;
			lower = adj->lower;
			value = adj->value;
			adj->value = (value-32)*(5.0/9.0);
			adj->lower = (lower-32)*(5.0/9.0);
			adj->upper = (upper-32)*(5.0/9.0);
			gtk_adjustment_changed(adj);
			gtk_spin_button_set_value(
					GTK_SPIN_BUTTON(widget),
					adj->value);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			g_object_set_data(G_OBJECT(widget),"widget_temp",GINT_TO_POINTER(units));
		}
		if ((GTK_IS_RANGE(widget)) && (widget_temp == FAHRENHEIT))
		{
			adj = (GtkAdjustment *) gtk_range_get_adjustment(
					GTK_RANGE(widget));
			upper = adj->upper;
			lower = adj->lower;
			value = adj->value;
			adj->value = (value-32)*(5.0/9.0);
			adj->lower = (lower-32)*(5.0/9.0);
			adj->upper = (upper-32)*(5.0/9.0);

			gtk_range_set_adjustment(GTK_RANGE(widget),adj);
			g_object_set_data(G_OBJECT(widget),"widget_temp",GINT_TO_POINTER(units));
		}
	}

}


/*!
 \brief reset_temps() calls the convert_temps function for each widget in
 the "temperature" list
 \param type (gpointer) the temp scale now selected
 */
void reset_temps(gpointer type)
{
	/* Better way.. :) */
	g_list_foreach(get_list("temperature"),convert_temps,type);

}
