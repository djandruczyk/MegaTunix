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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>


static gint lastcount=0;
gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;
extern struct raw_runtime_std *raw_runtime;
extern struct runtime_std *runtime;
extern struct runtime_std *runtime_last;
extern struct ve_const_std *ve_constants;
extern struct ve_const_std *ve_const_tmp;
//char * test_ptr;
       
int handle_ms_data(int which_data)
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
			res = read(serial_params.fd,ptr,serial_params.raw_bytes*2); 
			/* the number of bytes expected for raw data read */
			if (res > serial_params.raw_bytes) 
			{
				/* Serial I/O problem, resetting port 
				 * This problem can occur if the MS is 
				 * resetting due to power spikes, or other 
				 * problems with the serial interface, 
				 * perhaps even a very slow machine
				 * The problem is part of the data is lost, 
				 * and since we read in serial_params.raw_bytes
				 * blocks, the data is now offset, the damn 
				 * problem is that there is no formatting to 
				 * the datastream which is now out of sync by
				 * an unknown amount which tends to hose 
				 * things all to hell.  The solution is to 
				 * close the port, re-open it and reinitialize 
				 * it, then continue.  We CAN do this here 
				 * because the serial I/O thread depends on 
				 * this function and blocks until we return.
				 */
				printf("warning serial data read error\n");
				close_serial();
				usleep(100000);
				open_serial(serial_params.comm_port);
				setup_serial_params();
				return FALSE;
			}

			/* copy data out of temp buffer into struct for 
			 * use elsewhere (datalogging)
			 */
			memcpy(raw_runtime,buf,
					sizeof(struct raw_runtime_std));

			/* Test for MS reset */
			if (just_starting)
			{
				lastcount = raw_runtime->secl;
				just_starting = 0;
			}
			/* Check for clock jump from the MS, a jump in time
			 * from the MS clock indicates a reset due to power
			 * and/or noise.
			 */
			if ((lastcount - raw_runtime->secl > 1) \
					&& (lastcount - raw_runtime->secl != 255))
			{
				ms_reset_count++;
			}
			else
			{
                        	ms_goodread_count++;
			}
			lastcount = raw_runtime->secl;

			/* copy last round to runtime_last for checking */
			memcpy(runtime_last,runtime,
					sizeof(struct runtime_std));
			post_process(raw_runtime,runtime);

			break;

		case VE_AND_CONSTANTS:
			res = read(serial_params.fd,ptr,serial_params.veconst_size); 
			/* the number of bytes expected for raw data read */
			if (res != serial_params.veconst_size) 
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
//				printf("data read was not right size (%i)\n",res);
//				close_serial();
//				open_serial(serial_params.comm_port);
//				setup_serial_params();
//				return FALSE;
			}
			/* Two copies, working copy and temp for comparison
			 * against to know if we have to burn stuff to flash
			 */
			memcpy(ve_constants,buf,sizeof(struct ve_const_std));
			memcpy(ve_const_tmp,buf,sizeof(struct ve_const_std));
			//test_ptr = (char *)ve_constants;
			//printf("cr_pulse via array manip at -40 = %i\n",test_ptr[64]);
                        ms_ve_goodread_count++;
			break;
	}

	return TRUE;
}
