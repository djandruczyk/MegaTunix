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

extern struct Conversion_Chart *conversions;
extern struct DynamicLabels labels;
extern struct DynamicAdjustments adjustments;
extern struct DynamicSpinners spinners;
extern GtkWidget *ve_widgets[];


void read_conversions(void)
{
	gint i = 0;
	gint dl_type;
	struct Conversion_Chart *conv_chart=NULL;

	conv_chart = conversions;

	for (i=0;i<2*MS_PAGE_SIZE;i++)
	{
		if (GTK_IS_OBJECT(ve_widgets[i]))
		{
			dl_type = (gint)g_object_get_data(
					G_OBJECT(ve_widgets[i]),
					"dl_type");
			if (dl_type == IMMEDIATE)
			{
				conv_chart->conv_type[i] = (gint)
					g_object_get_data(G_OBJECT
							(ve_widgets[i]),
							"conv_type");
				conv_chart->conv_factor[i] = (gfloat)((gint)
						g_object_get_data(G_OBJECT
							(ve_widgets[i]),
							"conv_factor_x100"))
					/100.0;
			}

		}
		if (i == 90)	/* Required fuel special case */
		{
			conv_chart->conv_type[i] = NOTHING;
			conv_chart->conv_factor[i] = 1.0;
		}
#ifdef DEBUG
		printf("BASE Offset, %i, conv_type %i, conv_factor %f\n",i,conv_chart->conv_type[i],conv_chart->conv_factor[i]);
#endif
	}
	return;
}

gint convert_before_download(gint offset, gfloat value)
{
	gint return_value = 0;
	gint tmp_val = (gint)(value+0.001);
	gfloat factor;
	unsigned char *ve_const_arr; 
	struct Conversion_Chart *conv_chart;
	extern unsigned char *ms_data;

	conv_chart = conversions;
	ve_const_arr = (unsigned char *)ms_data;

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

gfloat convert_after_upload(gint offset)
{
	gfloat return_value = 0.0;
	gfloat factor = 0.0;
	unsigned char *ve_const_arr;
	struct Conversion_Chart *conv_chart;
	extern unsigned char *ms_data;

	conv_chart = conversions;
	ve_const_arr = (unsigned char *)ms_data;

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
	gchar * string;
	extern unsigned int ecu_caps;
	extern const gchar * F_warmup_labels[];
	extern const gchar * C_warmup_labels[];
	switch ((gint)type)
	{
		case FAHRENHEIT:
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_lowtemp_lab),
					"-40\302\260 F.");
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_hightemp_lab),
					"170\302\260 F.");
			gtk_label_set_text(
					GTK_LABEL(labels.warmup_lab),
					"Engine Temp in Degrees Fahrenheit");
			gtk_label_set_text(
					GTK_LABEL(labels.ego_temp_lab),
					"EGO Active Temp(\302\260 F.)");
			if (ecu_caps & (DUALTABLE|IAC_PWM|IAC_STEPPER))
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Temp (\302\260 F.)");
			else
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Cutoff Temp (\302\260 F.)");
			if (ecu_caps & (S_N_SPARK|S_N_EDIS))
			{
				gtk_label_set_text(
						GTK_LABEL(labels.cooling_fan_temp_lab),
						"Cooling Fan Turn-On Temp (\302\260 F.)");
			}
			gtk_label_set_text(
					GTK_LABEL(labels.slow_idle_temp_lab),
					"Slow Idle Temp (\302\260 F.)");
			gtk_label_set_text(
					GTK_LABEL(labels.warmwiz_clt_lab),
					"Coolant (\302\260 F.)");
/*			gtk_label_set_text(
					GTK_LABEL(labels.runtime_clt_lab),
					"Coolant (\302\260 F.)");
			gtk_label_set_text(
					GTK_LABEL(labels.runtime_mat_lab),
					"MAT (\302\260 F.)");
*/
			for (i=0;i<10;i++)
			{
				string = g_strdup_printf("%s\302\260",
						F_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmup_bins_lab[i]),
						string);
				g_free(string);
				string = g_strdup_printf("%s\302\260 F.",
						F_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmwizard_lab[i]),
						string);
				g_free(string);
			}
			/* Cooling Fan Temp  (MSnEDIS/Spark ONLY) */
			/* Fast Idle Temp */
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
							spinners.fast_idle_temp_spin),
						(value*(9.0/5.0))+32);
			}
			/* Slow Idle Temp */
			upper = adjustments.slow_idle_temp_adj->upper;
			if (upper < 215) /* if so it was celsius, if not skip*/
			{	
				value = adjustments.slow_idle_temp_adj->value;
				adjustments.slow_idle_temp_adj->upper=215.0;
				adjustments.slow_idle_temp_adj->value=
					(value *(9.0/5.0))+32;
				gtk_adjustment_changed(
						adjustments.slow_idle_temp_adj);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
							spinners.slow_idle_temp_spin),
						(value*(9.0/5.0))+32);
			}
			/* EGO Activation Temp */
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
					"-40\302\260 C.");
			gtk_label_set_text(
					GTK_LABEL(labels.cr_pulse_hightemp_lab),
					"77\302\260 C.");
			gtk_label_set_text(
					GTK_LABEL(labels.warmup_lab),
					"Engine Temp in Degrees Celsius");
			gtk_label_set_text(
					GTK_LABEL(labels.ego_temp_lab),
					"EGO Active Temp(\302\260 C.)");
			if (ecu_caps & (DUALTABLE|IAC_PWM|IAC_STEPPER))
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Temp (\302\260 C.)");
			else
				gtk_label_set_text(
						GTK_LABEL(labels.fast_idle_temp_lab),
						"Fast Idle Cutoff Temp (\302\260 C.)");
			if (ecu_caps & (S_N_SPARK|S_N_EDIS))
			{
				gtk_label_set_text(
						GTK_LABEL(labels.cooling_fan_temp_lab),
						"Cooling Fan Turn-On Temp (\302\260 C.)");
			}
			gtk_label_set_text(
					GTK_LABEL(labels.slow_idle_temp_lab),
					"Slow Idle Temp (\302\260 C.)");
			gtk_label_set_text(
					GTK_LABEL(labels.warmwiz_clt_lab),
					"Coolant (\302\260 C.)");
/*			gtk_label_set_text(
					GTK_LABEL(labels.runtime_clt_lab),
					"Coolant (\302\260 C.)");
			gtk_label_set_text(
					GTK_LABEL(labels.runtime_mat_lab),
					"MAT (\302\260 C.)");
*/
			for (i=0;i<10;i++)
			{
				string = g_strdup_printf("%s\302\260",
						C_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmup_bins_lab[i]),
						string);
				g_free(string);
				string = g_strdup_printf("%s\302\260 C.",
						C_warmup_labels[i]);
				gtk_label_set_text(
						GTK_LABEL(labels.warmwizard_lab[i]),
						string);
				g_free(string);
			}

			/* Fast Idle Temp */
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
							spinners.fast_idle_temp_spin),
						(value-32)*(5.0/9.0));
			}
			/* Slow Idle Temp */
			upper = adjustments.slow_idle_temp_adj->upper;
			if (upper > 102) /* if so it was fahren, if not skip*/
			{
				value = adjustments.slow_idle_temp_adj->value;
				adjustments.slow_idle_temp_adj->upper=101.6;
				adjustments.slow_idle_temp_adj->value=
					(value-32)*(5.0/9.0);
				gtk_adjustment_changed(
						adjustments.slow_idle_temp_adj);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
							spinners.slow_idle_temp_spin),
						(value-32)*(5.0/9.0));
			}
			/* EGO Activation Temp */
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
