/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <lookuptables.h>
#include <ms_structures.h>
#include <post_process.h>

extern unsigned char *kpa_conversion;
gboolean invalid_divider_1 = FALSE;
gboolean invalid_divider_2 = FALSE;

void post_process_realtime_vars(void *input, void *output)
{
	/* We can use the dualtable struct ptr only because
	 * all the ata is the same places in the structure.  We
	 * choose below which ones to take based on whether
	 * dualtable is set or not..
	 */
	extern unsigned char *ms_data;
	extern unsigned int ecu_caps;
	extern gint temp_units;
	gint stroke = 0;
	gint cyls = 0;
	gfloat ign_int = 0.0;
	struct Raw_Runtime_Std *in = input;
	struct Raw_Runtime_Dualtable *in_dt = input;
//	struct Raw_Runtime_Enhanced *in_enh = input;
	struct Runtime_Common *out = output;
	struct Raw_Runtime_Ignition *ign_in = input;
	struct Ve_Const_Std *ve_const = NULL;
	struct Ve_Const_DT_1 *ve_const_dt1 = NULL;
	struct Ve_Const_DT_2 *ve_const_dt2 = NULL;
	ve_const = (struct Ve_Const_Std *) ms_data;
	if (ecu_caps & DUALTABLE)
	{
		ve_const_dt1 = (struct Ve_Const_DT_1 *) ms_data;
		ve_const_dt2 = (struct Ve_Const_DT_2 *) (ms_data+MS_PAGE_SIZE);
	}
	gint divider = 0;
	gint nsquirts = 0;
	gfloat cycletime = 0.0;
	gfloat possible_inj_time = 0.0;

	out->secl = in->secl;
	out->squirt.value = in->squirt.value;
	out->engine.value = in->engine.value;

	out->baro_volts = (float)in->baro * (5.0/255.0);
	out->baro_raw = in->baro;
	out->baro = kpa_conversion[in->baro];

	out->map_volts = (float)in->map * (5.0/255.0);
	out->map_raw = in->map;
	out->map = kpa_conversion[in->map];

	out->clt_volts = (float)in->clt * (5.0/255.0);
	out->clt_raw = in->clt;
	if (temp_units == FAHRENHEIT)
		out->clt = thermfactor[in->clt]-40;
	else
		out->clt = (short)((thermfactor[in->clt]-40-32)*(5.0/9.0));

	out->mat_volts = (float)in->mat * (5.0/255.0);
	out->mat_raw = in->mat;
	if (temp_units == FAHRENHEIT)
		out->mat = thermfactor[in->mat]-40;
	else
		out->mat = (short)((thermfactor[in->mat]-40-32)*(5.0/9.0));

	out->tps_volts = (float)in->tps * (5.0/255.0);
	out->tps_raw = in->tps;
	out->tps = ((float)in->tps/255.0)*100.0; /* Convert to percent of full scale */

	out->batt_volts = (float)in->batt * (5.0/255.0) * 6.0;
	out->batt_raw = in->batt;

	out->ego_volts = (float)in->ego * (5.0/255.0);
	out->ego_raw = in->ego;

	out->egocorr = in->egocorr;
	out->aircorr = in->aircorr;
	out->warmcorr = in->warmcorr;
	out->tpsaccel = in->tpsaccel;
	out->barocorr = in->barocorr;
	out->gammae = in->gammae;

	/* NON Spark variants.... */
	if (!(ecu_caps & (S_N_SPARK|S_N_EDIS)))
	{
		out->rpm = in->rpm * 100;
		out->bspot1 = in->bspot1;
		out->bspot2 = in->bspot2;
		out->bspot3 = in->bspot3;
	}
	else	 /* Spark variants,  SquirtnSpark and SquirtnEDIS */
	{
		/* Funky high resolution RPM output from MegaSquirtnSpark */
		cyls = ve_const->config11.bit.cylinders+1;
		if (ve_const->config11.bit.eng_type == 0)
			stroke = 4;
		else
			stroke = 2;
		out->ctimecommH = ign_in->ctimecommH;
		out->ctimecommL = ign_in->ctimecommL;
		ign_int = (((out->ctimecommH*256)+out->ctimecommL)/7.3728)*8.0;
		if (ign_in->rpm < 5)
			out->rpm = ign_in->rpm*100;
		else
			out->rpm = 1000000*60.0/(float)(ign_int*(stroke/cyls*2));
			if (out->rpm > 25500)
				out->rpm = 25500;

		out->sparkangle = (gint)((float)ign_in->sparkangle/2.84);
	}

	if (!(ecu_caps & DUALTABLE))
	{	/* Std B&G Code */
		if ((ve_const->divider == 0) || ((ve_const->config11.bit.cylinders+1)%ve_const->divider))
		{
			dbg_func(g_strdup_printf(__FILE__": Std Code, divider invalid, cyls %i, divider %i\n",ve_const->config11.bit.cylinders+1,ve_const->divider),CRITICAL);
			ve_const->divider = 1;
			invalid_divider_1 = TRUE;
		} 
		else
			invalid_divider_1 = FALSE;

		nsquirts = (ve_const->config11.bit.cylinders+1)/ve_const->divider;

		out->pw1 = (float)in->pw1 / 10.0;
		out->vecurr1 = in->vecurr1;
		if (ve_const->alternate)
			divider = 2;
		else
			divider = 1;
		if (ve_const->config11.bit.eng_type == 1)
			cycletime = 600.0 /(float) in->rpm;
		else
			cycletime = 1200.0 /(float) in->rpm;

		out->dcycle1 = 100.0 
			* (nsquirts/divider)
			* ((float) out->pw1 / cycletime);
	}
	else
	{	/* Dualtable code.... */
		out->pw1 = (float)in_dt->pw1 / 10.0;
		out->pw2 = (float)in_dt->pw2 / 10.0;
		out->vecurr1 = in_dt->vecurr1;
		out->vecurr2 = in_dt->vecurr2;
		out->idledc = in_dt->idledc;

		if (ve_const_dt1->config11.bit.eng_type == 1)
			cycletime = 600.0 /(float) in->rpm;
		else
			cycletime = 1200.0 /(float) in->rpm;

		/* Table 1 */
		if ((ve_const_dt1->divider == 0) || ((ve_const_dt1->config11.bit.cylinders+1)%ve_const_dt1->divider))
		{
			dbg_func(g_strdup_printf(__FILE__": Dualtable, Table1, divider invalid, cyls %i, divider %i\n",ve_const_dt1->config11.bit.cylinders+1,ve_const_dt1->divider),CRITICAL);
			ve_const_dt1->divider = 1;
			invalid_divider_1 = TRUE;
		} 
		else
			invalid_divider_1 = FALSE;

		nsquirts = (int) 0.00001 
				+ (float)(ve_const_dt1->config11.bit.cylinders+1)
				/ (float)ve_const_dt1->divider;

		possible_inj_time = (float)cycletime/(float)nsquirts;
		out->dcycle1 =  100.0 * (float) out->pw1 / possible_inj_time;

		/* Table 2 */
		if ((ve_const_dt2->divider == 0) || ((ve_const_dt2->config11.bit.cylinders+1)%ve_const_dt2->divider))
		{
			dbg_func(g_strdup_printf(__FILE__": Dualtable, Table2,  divider invalid, cyls %i, divider %i\n",ve_const_dt2->config11.bit.cylinders+1,ve_const_dt2->divider),CRITICAL);
			invalid_divider_2 = TRUE;
			ve_const_dt2->divider = 1;
		} 
		else
			invalid_divider_2 = FALSE;

		nsquirts = (int) 0.00001 
				+ (float)(ve_const_dt2->config11.bit.cylinders+1)
				/ (float)ve_const_dt2->divider;
		possible_inj_time = (float)cycletime/(float)nsquirts;
		out->dcycle2 =  100.0 * (float) out->pw2 / possible_inj_time;

	}
}


void post_process_raw_memory(void *input, gint offset)
{
	extern GArray * raw_memory;
	gint i = 0;
	GtkWidget *label = NULL;
	unsigned char *ptr = input;

//	dbg_func(g_strdup_printf(__FILE__": post_process_raw_memory(), not written yet, offset fed was %i\n",offset),CRITICAL);
	for (i=0;i<256;i++)
	{
		label = g_array_index(raw_memory, GtkWidget *, i+(256*offset));
		//gtk_label_set_text(GTK_LABEL(label),(const char *)g_strdup_printf("%.3i",ptr[i]));
		gtk_label_set_text(GTK_LABEL(label),(const char *)g_strdup_printf("%.2X",ptr[i]));
	}
				
}
