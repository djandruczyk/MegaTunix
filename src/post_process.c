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
#include <post_process.h>

extern unsigned char *kpa_conversion;
extern gboolean fahrenheit;

void post_process(struct raw_runtime_std *in, struct runtime_std *out)
{
	out->secl = in->secl;
	out->squirt.value = in->squirt.value;
	out->engine.value = in->engine.value;
	out->baro = kpa_conversion[in->baro];
	out->map = kpa_conversion[in->map];
	out->mat_volt = (float)in->mat*(5.0/255.0);
	out->clt_volt = (float)in->clt*(5.0/255.0);
	out->tps_volt = (float)in->tps*(5.0/255.0);
	out->mat = thermfactor[in->mat];
	out->clt = thermfactor[in->clt];
	out->tps = ((float)in->tps/255.0)*100.0; /* Convert to percent of full scale */
	out->batt = (float)in->batt * (5.0/255.0) * 6.0;
	out->ego = (float)in->ego * (5.0/255.0);

	out->egocorr = in->egocorr;
	out->aircorr = in->aircorr;
	out->warmcorr = in->warmcorr;
	out->rpm = in->rpm * 100;
	out->pw = in->pw / 10.0;
	out->dcycle = ((float)out->pw*2.0)/(6.0/(float)in->rpm);
	out->tpsaccel = in->tpsaccel;
	out->barocorr = in->barocorr;
	out->gammae = in->gammae;
	out->vecurr = in->vecurr;
	out->bspot1 = in->bspot1;
	out->bspot2 = in->bspot2;
	out->bspot3 = in->bspot3;
};

