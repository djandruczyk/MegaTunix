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

extern struct Conversion_Chart *std_conversions[];
extern struct DynamicLabels labels;
extern struct DynamicAdjustments adjustments;
extern struct DynamicSpinners spinners;
GList *temp_dep = NULL;
static gchar * conv_text[] = 
{
	"Add",
	"Subtract",
	"Multiply",
	"Divide",
	"No Conversion"
};

void store_conversions(void)
{
	gint i = 0;
	gint x = 0;
	gint dl_type;
	struct Conversion_Chart *conv_chart=NULL;
	extern GtkWidget *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];


	for (x=0;x<MAX_SUPPORTED_PAGES;x++)
	{
		conv_chart = std_conversions[x];
		for (i=0;i<2*MS_PAGE_SIZE;i++)
		{
			if (GTK_IS_OBJECT(ve_widgets[x][i]))
			{
				dl_type = (gint)g_object_get_data(
						G_OBJECT(ve_widgets[x][i]),
						"dl_type");
				if (dl_type == IMMEDIATE)
				{
					conv_chart->conv_type[i] = (gint)g_object_get_data(G_OBJECT(ve_widgets[x][i]),"conv_type");
					conv_chart->conv_factor[i] = (gfloat)((gint)g_object_get_data(G_OBJECT(ve_widgets[x][i]),"conv_factor_x100"))/100.0;
				}

			}
			if ((i == 90) && ((x == 0)||(x==1)))	/* Required fuel special case */
			{
				conv_chart->conv_type[i] = CONV_NOTHING;
				conv_chart->conv_factor[i] = 1.0;
			}

			dbg_func(g_strdup_printf(__FILE__": load_conversions() PAGE: %i,BASE Offset, %i, conv_type %s, conv_factor %f\n",x,i,conv_text[conv_chart->conv_type[i]],conv_chart->conv_factor[i]),DL_CONV);
		}
	}

	return;
}

gint convert_before_download(gint page, gint offset, gfloat value)
{
	gint return_value = 0;
	gint tmp_val = (gint)(value+0.001);
	gfloat factor;
	unsigned char *ve_const_arr; 
	struct Conversion_Chart *conv_chart;
	extern unsigned char *ms_data[MAX_SUPPORTED_PAGES];

	conv_chart = std_conversions[page];
	ve_const_arr = (unsigned char *)ms_data[page];

	factor = conv_chart->conv_factor[offset];

	switch ((Conversions)conv_chart->conv_type[offset])
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

	ve_const_arr[offset] = return_value; 
	return (return_value);
}

gfloat convert_after_upload(gint page, gint offset)
{
	gfloat return_value = 0.0;
	gfloat factor = 0.0;
	unsigned char *ve_const_arr;
	struct Conversion_Chart *conv_chart;
	extern unsigned char *ms_data[MAX_SUPPORTED_PAGES];

	conv_chart = std_conversions[page];
	ve_const_arr = (unsigned char *)ms_data[page];

	factor = conv_chart->conv_factor[offset];

	/* Since this is the upload we actually do the CONVERSE mathematical 
	 * operation since the algorithm was designed for the download side, 
	 * On upload we need to "un-convert" from MS values to Gui friendly
	 * versions....
	 */
	switch ((Conversions)conv_chart->conv_type[offset])
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
	gchar *text = NULL;
	gchar * newtext = NULL;
	gchar **array = NULL;
	gfloat upper, value;
	GtkAdjustment * adj;

	if ((int)units == FAHRENHEIT)
	{
		if (GTK_IS_LABEL(widget))
		{
			text = g_strdup(gtk_label_get_text(GTK_LABEL(widget)));
			array = g_strsplit(text,"C.",0);
			if (array[1] == NULL)// If null, means not needed
			{
				g_free(text);
				g_strfreev(array);
				return;
			}
			newtext = g_strdup_printf("%sF.%s",array[0],array[1]);	
			gtk_label_set_text(GTK_LABEL(widget),newtext);
			g_free(text);
			g_strfreev(array);
			g_free(newtext);
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
			text = g_strdup(gtk_label_get_text(GTK_LABEL(widget)));
			array = g_strsplit(text,"F.",0);
			if (array[1] == NULL)// If null, means not needed
			{
				g_free(text);
				g_strfreev(array);
				return;
			}
			newtext = g_strdup_printf("%sC.%s",array[0],array[1]);	
			gtk_label_set_text(GTK_LABEL(widget),newtext);
			g_free(text);
			g_strfreev(array);
			g_free(newtext);
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
	gint i;
	gchar * string;
	extern gint ecu_caps;
	extern const gchar * F_warmup_labels[];
	extern const gchar * C_warmup_labels[];

	/* Better way.. :) */
	g_list_foreach(temp_dep,convert_temps,type);

	switch ((gint)type)
	{
		case FAHRENHEIT:
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_hightemp_lab),
					"170 \302\260F.");
			gtk_label_set_text(
					GTK_LABEL(labels.ww_cr_pulse_hightemp_lab),
					"Pulsewidth at 170 \302\260F.");
			if (ecu_caps & (DUALTABLE|IAC_PWM|IAC_STEPPER))
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Temp (\302\260F.)");
			else
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Cutoff Temp (\302\260F.)");
			for (i=0;i<10;i++)
			{
				string = g_strdup_printf("%s \302\260",
						F_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmup_bins_lab[i]),
						string);
				g_free(string);
				string = g_strdup_printf("%s \302\260F.",
						F_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmwizard_lab[i]),
						string);
				g_free(string);
			}
			break;

		case CELSIUS:
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_hightemp_lab),
					"77 \302\260C.");
			gtk_label_set_text(
					GTK_LABEL(labels.ww_cr_pulse_hightemp_lab),
					"Pulsewidth at 77 \302\260C.");
			if (ecu_caps & (DUALTABLE|IAC_PWM|IAC_STEPPER))
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Temp (\302\260C.)");
			else
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Cutoff Temp (\302\260C.)");
			for (i=0;i<10;i++)
			{
				string = g_strdup_printf("%s \302\260",
						C_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmup_bins_lab[i]),
						string);
				g_free(string);
				string = g_strdup_printf("%s \302\260C.",
						C_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmwizard_lab[i]),
						string);
				g_free(string);
			}
			break;

	}
	return;	
}
