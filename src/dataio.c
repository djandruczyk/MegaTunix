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
#include <dataio.h>
#include <defines.h>
#include <enums.h>
#include <globals.h>
#include <post_process.h>
#include <pthread.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <unistd.h>



static gint lastcount=0;
gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;
extern gboolean raw_reader_running;
extern struct Serial_Params *serial_params;
extern struct Raw_Runtime_Std *raw_runtime;
extern struct Runtime_Common *runtime;
extern struct Runtime_Common *runtime_last;
extern struct Ve_Const_Std *ve_const_p0;
extern struct Ve_Const_Std *ve_const_p1;
extern struct Ve_Const_Std *ve_const_p0_tmp;
extern struct Ve_Const_Std *ve_const_p1_tmp;
       
int handle_ms_data(InputData which_data)
{
	int res = 0;
	unsigned char buf[255];
	unsigned char *ptr = buf;

	/* different cases whether we're doing 
	 * realtime, VE/constants, or I/O test 
	 */
	switch (which_data)
	{
		case REALTIME_VARS:
			res = read(serial_params->fd,ptr,serial_params->raw_bytes*2); 
			/* the number of bytes expected for raw data read */
			if (res > serial_params->raw_bytes) 
			{
				/* Serial I/O problem, resetting port 
				 * This problem can occur if the MS is 
				 * resetting due to power spikes, or other 
				 * problems with the serial interface, 
				 * perhaps even a very slow machine
				 * The problem is part of the data is lost, 
				 * and since we read in serial_params->raw_bytes
				 * blocks, the data is now offset, the damn 
				 * problem is that there is no formatting so 
				 * the datastream which is now out of sync by
				 * an unknown amount which tends to hose 
				 * things all to hell.  The solution is to 
				 * close the port, re-open it and reinitialize 
				 * it, then continue.  We CAN do this here 
				 * because the serial I/O thread depends on 
				 * this function and blocks until we return.
				 */
				//printf("warning serial data read error (%i bytes)\n",res);
				close_serial();
				open_serial(serial_params->comm_port);

				/* The raw_reader_running flag  MUST be reset 
				 * to prevent a deadlock in check_ecu_comms 
				 * which will attempt to kill the thread that 
				 * we are executing under. By flipping this 
				 * flag we prevent the thread kill 
				 * (and deadlock) and just make sure to 
				 * set it back to normal afterwards....
				 */
				raw_reader_running = FALSE;
				setup_serial_params();
				raw_reader_running = TRUE;
				return FALSE;
			}

			/* copy data out of temp buffer into struct for 
			 * use elsewhere (datalogging)
			 */
			memcpy(raw_runtime,buf,
					sizeof(struct Raw_Runtime_Std));

			/* Test for MS reset */
			if (just_starting)
			{
				lastcount = raw_runtime->secl;
				just_starting  = FALSE;
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
					sizeof(struct Runtime_Common));

			post_process((void *)raw_runtime,(void *)runtime);

			break;

		case VE_AND_CONSTANTS_1:
			res = read(serial_params->fd,ptr,serial_params->veconst_size); 
			/* the number of bytes expected for raw data read */
			if (res != serial_params->veconst_size) 
			{
				/* Serial I/O problem, resetting port 
				 * This problem can occur if the MS is 
				 * resetting due to power spikes, or other 
				 * problems with the serial interface, 
				 * perhaps even a very slow machine
				 * The problem is part of the data is lost, 
				 * and since we read in serial_params->raw_bytes
				 * blocks, the data is now offset can hoses 
				 * things all to hell.  The solution is to 
				 * close the port, re-open it and reinitialize 
				 * it, then continue.  We CAN do this here 
				 * because the serial I/O thread depends on 
				 * this function and blocks until we return.
				 */
			}
			/* Two copies, working copy and temp for comparison
			 * against to know if we have to burn stuff to flash
			 */
			memcpy(ve_const_p0,buf,sizeof(struct Ve_Const_Std));
			memcpy(ve_const_p0_tmp,buf,sizeof(struct Ve_Const_Std));

			ms_ve_goodread_count++;
			break;

		case VE_AND_CONSTANTS_2:
			printf("Dualtable not supported yet.. \n");
			break;

		case IGNITION_VARS:
			printf("handling of read ignition data isn't handled yet\n");
			break;
		case RAW_MEMORY:
			printf("Not designed yet...\n");
			break;
		default:
			printf("handle_ms_data, improper case, contact author\n");
			break;
	}

	return TRUE;
}
