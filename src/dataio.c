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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
#include <pthread.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"


static int lastcount=0;
int reset_count;
int just_starting;
       
void handle_ms_data(int which_data)
{
	int res = 0;
	unsigned char buf[255];
	char *ptr = buf;
	/* different cases whether we're doing 
	 * realtime, VE/constants, or I/O test 
	 */
	switch (which_data)
	{
		case REALTIME_VARS:
			res = read(serial_params.fd,ptr,255); 
			/* the number of bytes expected for raw data read */
			if (res != serial_params.raw_bytes) 

			{
				/* Serial I/O problem, resetting port 
				 * This problem can occur if the MS is 
				 * resetting due to power spikes, or other 
				 * problems with the serial interface, 
				 * perhaps even a very slow machine
				 * The problem is part of the data is lost, 
				 * and since we read in serial_params.raw_bytes
				 * blocks, the data is now offset can hoses 
				 * things all to hell.  The solution is to 
				 * close the port, re-open it and reinitialize 
				 * it, then continue.  We CAN do this here 
				 * because the serial I/O thread depends on 
				 * this function and blocks until we return.
				 */
				close_serial();
				open_serial(serial_params.comm_port);
				setup_serial_params();
				return;
			}
			raw = (struct ms_raw_data_v1_and_v2 *) buf;

			/* Test for MS reset */
			if (just_starting)
			{
				lastcount = raw->secl;
				just_starting = 0;
			}
			if ((lastcount - raw->secl > 1) \
					&& (lastcount - raw->secl != 255))
			{	/* Counter jump, megasquirt reset occurred */
				reset_count++;
			}
			lastcount = raw->secl;

			/* copy last round to out_last for checking */
			out_last = out;
			post_process(raw,&out);
			break;

		case VE_AND_CONSTANTS:
			/* Not written yet */
			printf("VEtable/constants came in\n");
			break;
	}

		
/* direct to console debugging output 
 *	printf("counter RAW:%i,\tActual %i\n", raw->secl,out.secl);
 *	printf("baro RAW:%i,\t\tActual %i\n", raw->baro,out.baro);
 *	printf("map RAW:%i,\t\tActual %i\n", raw->map,out.map);
 *	printf("mat RAW:%i,\t\tActual %.2f\n", raw->mat,out.mat);
 *	printf("clt RAW:%i,\t\tActual %.2f\n", raw->clt,out.clt);
 *	printf("tps RAW:%i,\t\tActual %.2f\n", raw->tps,out.tps);
 *	printf("batt RAW:%i,\t\tActual %.2f\n", raw->batt,out.batt);
 *	printf("ego RAW:%i,\t\tActual %.2f\n", raw->ego,out.ego);
 *	printf("egocorr RAW:%i,\t\tActual %i\n", raw->egocorr,out.egocorr);
 *	printf("aircorr RAW:%i,\tActual %i\n", raw->aircorr,out.aircorr);
 *	printf("warmcorr RAW:%i,\tActual %i\n", raw->warmcorr,out.warmcorr);
 *	printf("rpm RAW:%i,\t\tActual %i\n", raw->rpm,out.rpm);
 *	printf("pw RAW:%i,\t\tActual %.2f\n", raw->pw,out.pw);
 *	printf("tpsaccel RAW:%i,\t\tActual %i\n", raw->tpsaccel,out.tpsaccel);
 *	printf("barocorr RAW:%i,\tActual %i\n", raw->barocorr,out.barocorr);
 *	printf("gammae RAW:%i,\t\tActual %i\n", raw->gammae,out.gammae);
 *	printf("vecurr RAW:%i,\t\tActual %i\n", raw->vecurr,out.vecurr);
 *	printf("bspot1 RAW:%i,\t\tActual %i\n", raw->bspot1,out.bspot1);
 *	printf("bspot2 RAW:%i,\t\tActual %i\n", raw->bspot2,out.bspot2);
 *	printf("bspot3 RAW:%i,\t\tActual %i\n", raw->bspot3,out.bspot3);
 *	printf(" ENGINE BITS %i %i %i %i %i %i %i %i\n",\
 *			out.engine.bit.running,\
 *			out.engine.bit.crank,\
 *			out.engine.bit.startw,\
 *			out.engine.bit.warmup,\
 *			out.engine.bit.tpsaen,\
 *			out.engine.bit.tpsden,\
 *			out.engine.bit.mapaen,\
 *			out.engine.bit.reserved);
 *	if (out.engine.bit.running == 1)
 *		printf("Engine is running\n");
 *	if (out.engine.bit.crank == 1)
 *		printf("Engine is cranking\n");
 *	if (out.engine.bit.startw == 1)
 *		printf("ECU is in Startup warmup enrich (after-start)\n");
 *	if (out.engine.bit.warmup == 1)
 *		printf("ECU is in normal warmup enrich (cold temps)\n");
 *	if (out.engine.bit.tpsaen == 1)
 *		printf("Engine is getting an Accel Shot (TPSAEN)\n");
 *	if (out.engine.bit.tpsden == 1)
 *		printf("Engine is deceleration (Coasting, TPSDEN)\n");
 *	if (out.engine.bit.mapaen == 1)
 *		printf("ECU is MAP accel mode\n");
 *
 *	printf("iteration: %i\n",count);
 *	printf("Error Count: %i\n",serial_params.errcount);
 *	printf("Reset Count: %i\n",reset_count);
 */
}
