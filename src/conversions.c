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
#include <enums.h>
#include <globals.h>
#include <stdio.h>
#include <structures.h>

/* Conversions.c
 * 
 * conv_Type has one of 4 possibles, ADD,SUB,MULT and DIV, for addition
 * subtraction, multiplication and division respectivlely.  These mathematical
 * operaands use the conv_factor and perform the requested Op on the value from
 * the gui BEFORE downloading to the MS.  On upload, the converse operation
 * is performed (if ADD used on downlaod, SUB will be used on upload), likewise
 * for multiplication and division.
 * 
 * As of 09/02/2003 (sept 2nd 2003), The external file has been dropped in 
 * favor of coding the conversions internally.  This makes it easier for the
 * user to install as they don't have to worry about a conversions table to 
 * put in the right place to be loaded...
 */

extern struct Conversion_Chart * page0_conversions;
extern struct Conversion_Chart * page1_conversions;
extern struct DynamicLabels labels;
extern struct DynamicAdjustments adjustments;
extern struct DynamicSpinners spinners;
extern struct Ve_Const_Std *ve_const_p0;
extern struct Ve_Const_Std *ve_const_p1;
extern struct Ve_Widgets *page0_widgets;
extern struct Ve_Widgets *page1_widgets;


void read_conversions(void)
{
	gint i = 0;
	gint j = 0;
	gint dl_type;
	struct Conversion_Chart *conv_chart=NULL;
	struct Ve_Widgets *widget_list = NULL;

	for (j=0;j<2;j++)
	{
		if (j == 0)
		{
			conv_chart = page0_conversions;
			widget_list = page0_widgets;
		}
		else
		{
			conv_chart = page1_conversions;
			widget_list = page1_widgets;
		}

		for (i=0;i<MS_PAGE_SIZE;i++)
		{
			if (GTK_IS_OBJECT(widget_list->widget[i]))
			{
				dl_type = (gint)g_object_get_data(
						G_OBJECT(widget_list->widget[i]),
						"dl_type");
				if (dl_type == IMMEDIATE)
				{
					conv_chart->conv_type[i] = 
						(gint)g_object_get_data(
									G_OBJECT(
										widget_list->widget[i]),
									"conv_type");
					conv_chart->conv_factor[i] = 
						(gfloat)((gint)
							 g_object_get_data(
								 G_OBJECT(
									 widget_list->widget[i]),
								 "conv_factor_x100"))
						/100.0;
				}

			}
			if (i == 90)	/* Required fuel special case */
			{
				conv_chart->conv_type[i] = MULT;
				conv_chart->conv_factor[i] = 10.0;
			}
#ifdef DEBUG
			printf("Page %i Offset, %i, conv_type %i, conv_factor %f\n",j,i,conv_chart->conv_type[i],conv_chart->conv_factor[i]);
#endif
		}
	}
	return;
}

gint convert_before_download(gint offset, gfloat value, gint page)
{
	gint return_value = 0;
	gint tmp_val = (gint)(value+0.001);
	gfloat factor;
	unsigned char *ve_const_arr; 
	struct Conversion_Chart *conv_chart;

	if (page == 0)
	{	/* Assign pointers to point at Page 0 conversions */
		conv_chart = page0_conversions;
		ve_const_arr = (unsigned char *)ve_const_p0;
	}
	else
	{	/* Assign pointers to point at Page 1 conversions */
		conv_chart = page1_conversions;
		ve_const_arr = (unsigned char *)ve_const_p1;
	}

	factor = conv_chart->conv_factor[offset];

	switch ((Conversions)conv_chart->conv_type[offset])
	{
		case (ADD):
			return_value = tmp_val + factor;
			break;
		case (SUB):
			return_value = tmp_val - factor;
			break;
		case (MULT):
			return_value = (gint)((value*factor) + 0.001);
			break;
		case (DIV):
			return_value = (gint)((value/factor) + 0.001);
			break;
		case (NOTHING):
			return_value = tmp_val;
			break;
		default:
			printf("Convert_before_download() NO CONVERSION defined, BUG!!!\b\b\n");
			break;
	}
	/* Store value in veconst_arr pointer (to structure) 
	 * (accessing it via array syntax as it's friggin easier).... 
	 */
	ve_const_arr[offset] = return_value; 
	return (return_value);
}

gfloat convert_after_upload(gint offset, gint page)
{
	gfloat return_value = 0.0;
	gfloat factor = 0.0;
	unsigned char *ve_const_arr;
	struct Conversion_Chart *conv_chart;

	if (page == 0)
	{	/* Assign pointers to point at Page 0 conversions */
		conv_chart = page0_conversions;
		ve_const_arr = (unsigned char *)ve_const_p0;
	}
	else
	{	/* Assign pointers to point at Page 1 conversions */
		conv_chart = page1_conversions;
		ve_const_arr = (unsigned char *)ve_const_p1;
	}

	factor = conv_chart->conv_factor[offset];

	/* Since this is the upload we actually do the CONVERSE mathematical 
	 * operation since the algorithm was designed for the download side, 
	 * On upload we need to "un-convert" from MS values to Gui friendly
	 * versions....
	 */
	switch ((Conversions)conv_chart->conv_type[offset])
	{
		case (ADD):
			return_value = ve_const_arr[offset] - factor;
			break;
		case (SUB):
			return_value = ve_const_arr[offset] + factor;
			break;
		case (MULT):
			return_value = (gfloat)ve_const_arr[offset] / factor;
			break;
		case (DIV):
			return_value = (gfloat)ve_const_arr[offset] * factor;
			break;
		case (NOTHING):
			return_value = ve_const_arr[offset];
			break;
		default:
			printf("Convert_after_upload() NO CONVERSION defined, BUG!!!\b\b\n");
			break;

	}
	return (return_value);
}

void reset_temps(gpointer type)
{
	gint i;
	gfloat value;
	gfloat upper;
	extern const gchar * F_warmup_labels[];
	extern const gchar * C_warmup_labels[];
	switch ((gint)type)
	{
		case FAHRENHEIT:
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_lowtemp_lab),
					"-40 Deg. F");
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_hightemp_lab),
					"170 Deg. F");
			gtk_label_set_text(
					GTK_LABEL(labels.warmup_lab),
					"Engine Temp in Degrees Fahrenheit");
			gtk_label_set_text(
					GTK_LABEL(labels.ego_temp_lab),
					"Coolant Temp\nActivation(Deg F.)");
			gtk_label_set_text(
					GTK_LABEL(labels.fastidletemp_lab),
					"Fast Idle Threshold\n(Degrees F.)");
			gtk_label_set_text(
					GTK_LABEL(labels.runtime_clt_lab),
					"Coolant (F)");
			gtk_label_set_text(
					GTK_LABEL(labels.runtime_mat_lab),
					"MAT (F)");
			for (i=0;i<10;i++)
				gtk_label_set_text(
						GTK_LABEL(labels.warmup_bins_lab[i]),
						F_warmup_labels[i]);
			upper = adjustments.fast_idle_temp_adj->upper;
			if (upper < 215) /* if so it was celsius, if not skip*/
			{	
				value = adjustments.fast_idle_temp_adj->value;
				adjustments.fast_idle_temp_adj->upper=215.0;
				adjustments.fast_idle_temp_adj->value=
					(value *(9.0/5.0))+32;
				gtk_adjustment_changed(
						adjustments.fast_idle_temp_adj);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
							spinners.fast_idle_thresh_spin),
						(value*(9.0/5.0))+32);
			}
			upper = adjustments.ego_temp_adj->upper;
			if (upper < 215) /* if so it was celsius, if not skip*/
			{	
				value = adjustments.ego_temp_adj->value;
				adjustments.ego_temp_adj->upper=215.0;
				adjustments.ego_temp_adj->value=
					(value *(9.0/5.0))+32;
				gtk_adjustment_changed(
						adjustments.ego_temp_adj);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
							spinners.ego_temp_active_spin),
						(value*(9.0/5.0))+32);
			}
			break;

		case CELSIUS:
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_lowtemp_lab),
					"-40 Deg. C");
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_hightemp_lab),
					"77 Deg. C");
			gtk_label_set_text(
					GTK_LABEL(labels.warmup_lab),
					"Engine Temp in Degrees Celsius");
			gtk_label_set_text(
					GTK_LABEL(labels.ego_temp_lab),
					"Coolant Temp\nActivation(Deg C.)");
			gtk_label_set_text(
					GTK_LABEL(labels.fastidletemp_lab),
					"Fast Idle Threshold\n(Degrees C.)");
			gtk_label_set_text(
					GTK_LABEL(labels.runtime_clt_lab),
					"Coolant (C)");
			gtk_label_set_text(
					GTK_LABEL(labels.runtime_mat_lab),
					"MAT (C)");
			for (i=0;i<10;i++)
				gtk_label_set_text(
						GTK_LABEL(labels.warmup_bins_lab[i]),
						C_warmup_labels[i]);

			upper = adjustments.fast_idle_temp_adj->upper;
			if (upper > 102) /* if so it was fahren, if not skip*/
			{
				value = adjustments.fast_idle_temp_adj->value;
				adjustments.fast_idle_temp_adj->upper=101.6;
				adjustments.fast_idle_temp_adj->value=
					(value-32)*(5.0/9.0);
				gtk_adjustment_changed(
						adjustments.fast_idle_temp_adj);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
							spinners.fast_idle_thresh_spin),
						(value-32)*(5.0/9.0));
			}
			upper = adjustments.ego_temp_adj->upper;
			if (upper > 102) /* if so it was fahren, if not skip*/
			{	
				value = adjustments.ego_temp_adj->value;
				adjustments.ego_temp_adj->upper=101.6;
				adjustments.ego_temp_adj->value=
					(value-32)*(5.0/9.0);
				gtk_adjustment_changed(
						adjustments.ego_temp_adj);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
							spinners.ego_temp_active_spin),
						(value-32)*(5.0/9.0));
			}
			break;
	}
	return;	
}
