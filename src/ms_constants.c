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
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "lookuptables.h"

void post_process(struct ms_raw_data_v1_and_v2 *in, struct ms_data_v1_and_v2 *out)
{
	out->secl = in->secl;
	out->squirt.value = in->squirt.value;
	out->engine.value = in->engine.value;
	out->baro = in->baro;
	out->map = in->map;
	out->mat_volt = in->mat*0.0197;
	out->clt_volt = in->clt*0.0197;
	out->tps_volt = in->tps*0.0197;
	out->mat = thermfactor[in->mat];
	out->clt = thermfactor[in->clt];
	out->tps = (in->tps/255.0)*100.0; /* Convert to percent of full scale */
	out->batt = in->batt * 0.0197 * 6;
	out->ego = in->ego * 0.0197;

	out->egocorr = in->egocorr;
	out->aircorr = in->aircorr;
	out->warmcorr = in->warmcorr;
	out->rpm = in->rpm * 100;
	out->pw = in->pw / 10.0;

	out->tpsaccel = in->tpsaccel;
	out->barocorr = in->barocorr;
	out->gammae = in->gammae;
	out->vecurr = in->vecurr;
	out->bspot1 = in->bspot1;
	out->bspot2 = in->bspot2;
	out->bspot3 = in->bspot3;
};

