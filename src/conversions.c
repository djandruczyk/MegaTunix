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

#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <notifications.h>
#include <stdio.h>
#include <structures.h>
#include <tabloader.h>

/* Conversions.c
 * 
 * conv_Type has one of 4 possibles, CONV_ADD,CONV_SUB,CONV_MULT and CONV_DIV, 
 * for addition subtraction, multiplication and division respectivlely.  
 * These mathematical operaands use the conv_factor and perform the 
 * requested Op on the value from the gui BEFORE downloading to the MS.  
 * On upload, the converse operation is performed (if CONV_ADD used on 
 * downlaod,  CONV_SUB will be used on upload), likewise for multiplication 
 * and division.
 * 
 */

extern struct DynamicLabels labels;
extern struct DynamicAdjustments adjustments;
extern struct DynamicSpinners spinners;

gint convert_before_download(GtkWidget *widget, gfloat value)
{
	gint return_value = 0;
	extern unsigned char *ms_data[MAX_SUPPORTED_PAGES];
	gint tmp_val = (gint)(value+0.001);
	gint page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	gint offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	gfloat factor = (gfloat)((gint)g_object_get_data(G_OBJECT(widget),"conv_factor_x100"))/100.0;
	Conversions conv_type = (Conversions)g_object_get_data(G_OBJECT(widget),"conv_type");


	switch (conv_type)
	{
		case (CONV_ADD):
			return_value = tmp_val + factor;
			break;
		case (CONV_SUB):
			return_value = tmp_val - factor;
			break;
		case (CONV_MULT):
			return_value = (gint)((value*factor) + 0.001);
			break;
		case (CONV_DIV):
			return_value = (gint)((value/factor) + 0.001);
			break;
		case (CONV_NOTHING):
			return_value = tmp_val;
			break;
		default:
			dbg_func(g_strdup_printf(__FILE__": convert_before_dl(): NO CONVERSION defined, BUG!!! offset: %i\n",offset),DL_CONV);
			break;
	}
	dbg_func(g_strdup_printf(__FILE__": convert_before_dl(): offset %i, raw %.2f, sent %i, page %i,\n",offset,value,return_value,page),DL_CONV);

	/* Store value in veconst_arr pointer (to structure) 
	 * (accessing it via array syntax as it's friggin easier).... 
	 *
	 * Ignition variables are stored in the second page,  but referenced
	 * by an offset from the beginning of their page.
	 */

	ms_data[page][offset] = return_value; 
	return (return_value);
}

gfloat convert_after_upload(GtkWidget * widget)
{
	gfloat return_value = 0.0;
	unsigned char *ve_const_arr;
	extern unsigned char *ms_data[MAX_SUPPORTED_PAGES];
	gint page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	gint offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	gfloat factor = (gfloat)((gint)g_object_get_data(G_OBJECT(widget),"conv_factor_x100"))/100.0;
	Conversions conv_type = (Conversions)g_object_get_data(G_OBJECT(widget),"conv_type");

	ve_const_arr = (unsigned char *)ms_data[page];

	/* Since this is the upload we actually do the CONVERSE mathematical 
	 * operation since the algorithm was designed for the download side, 
	 * On upload we need to "un-convert" from MS values to Gui friendly
	 * versions....
	 */
	switch (conv_type)
	{
		case (CONV_ADD):
			return_value = ve_const_arr[offset] - factor;
			break;
		case (CONV_SUB):
			return_value = ve_const_arr[offset] + factor;
			break;
		case (CONV_MULT):
			return_value = (gfloat)ve_const_arr[offset] / factor;
			break;
		case (CONV_DIV):
			return_value = (gfloat)ve_const_arr[offset] * factor;
			break;
		case (CONV_NOTHING):
			return_value = ve_const_arr[offset];
			break;
		default:
			dbg_func(g_strdup_printf(__FILE__": convert_after_ul() NO CONVERSION defined, index %i BUG!!!\n",offset),UL_CONV);
			break;

	}
	dbg_func(g_strdup_printf(__FILE__": convert_after_ul(),offset %i, raw %i, val %f, page %i\n",offset,ve_const_arr[offset],return_value,page),UL_CONV);
	return (return_value);
}

void convert_temps(gpointer widget, gpointer units)
{
	gfloat upper = 0.0;
	gfloat value = 0.0;
	GtkAdjustment * adj = NULL;
	gchar *text = NULL;

	if ((int)units == FAHRENHEIT)
	{
		if (GTK_IS_LABEL(widget))
		{
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
