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
#include <lookuptables.h>
#include <ms_structures.h>
#include <post_process.h>

extern unsigned char *kpa_conversion;
extern gboolean fahrenheit;
extern gboolean dualtable;

void post_process(void *input, void *output)
{
	/* We can use the dualtable struct ptr only because
	 * all the ata is the same places in the structure.  We
	 * choose below which ones to take based on whether
	 * dualtable is set or not..
	 */
	struct Raw_Runtime_Dualtable *in = input;
	struct Raw_Runtime_Std *in_std = input;
	struct Runtime_Common *out = output;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_const_p0 = (struct Ve_Const_Std *) ms_data;
//	struct Ve_Const_Std *ve_const_p1 = (struct Ve_Const_Std *) ms_data+MS_PAGE_SIZE;

	gint divider = 0;
	gint nsquirts = 0;
	gfloat cycletime = 0.0;

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
	if (fahrenheit)
		out->clt = thermfactor[in->clt]-40;
	else
		out->clt = (short)((thermfactor[in->clt]-40-32)*(5.0/9.0));

	out->mat_volts = (float)in->mat * (5.0/255.0);
	out->mat_raw = in->mat;
	if (fahrenheit)
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
	out->rpm = in->rpm * 100;
	out->pw1 = (float)in->pw1 / 10.0;
	//out->dcycle1 = (float) out->pw1 / (1200.0 / (float) out->rpm);
	/* Hopefully correct dutycycle calc from Eric Fahlgren */
	nsquirts = (ve_const_p0->config11.bit.cylinders+1)/ve_const_p0->divider;
	if (ve_const_p0->alternate)
		divider = 2;
	else
		divider = 1;
	if (ve_const_p0->config11.bit.eng_type == 1)
		cycletime = 600.0 /(float) in->rpm;
	else
		cycletime = 1200.0 /(float) in->rpm;

	out->dcycle1 = 100.0 *nsquirts/divider* (float) out->pw1 / cycletime;
	out->tpsaccel = in->tpsaccel;
	out->barocorr = in->barocorr;
	out->gammae = in->gammae;
	out->vecurr1 = in->vecurr1;
	if (dualtable)
	{
/*
		out->vecurr2 = in->vecurr2;
		out->pw2 = (float)in->pw2 / 10.0;
		nsquirts = (ve_const_p1->config11.bit.cylinders+1)/ve_const_p1->divider;
		if (ve_const_p1->alternate)
			divider = 2;
		else
			divider = 1;
		if (ve_const_p1->config11.bit.eng_type == 1)
			cycletime = 600.0 /(float) in->rpm;
		else
			cycletime = 1200.0 /(float) in->rpm;

		//out->dcycle2 = (float) out->pw2 / (1200.0 / (float) out->rpm);
		out->dcycle2 = 100.0 *nsquirts/divider* (float) out->pw1 / cycletime;
		out->idleDC = in->idleDC;
*/
	}
	else
	{
		out->bspot1 = in_std->bspot1;
		out->bspot2 = in_std->bspot2;
		out->bspot3 = in_std->bspot3;
	}
}
