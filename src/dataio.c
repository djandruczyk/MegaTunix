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
#include <sys/poll.h>
#include <unistd.h>



static gint lastcount=0;
gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;
       
int handle_ms_data(InputData which_data)
{
	gint res = 0;
	gint total = 0;
	unsigned char buf[255];
	unsigned char *ptr = buf;
	struct pollfd ufds;
	struct Raw_Runtime_Std *raw_runtime = NULL;
	extern unsigned char *ms_data;
	extern unsigned char *ms_data_last;
	extern gboolean raw_reader_running;
	extern struct Serial_Params *serial_params;
	extern struct Runtime_Common *runtime;
	extern struct Runtime_Common *runtime_last;

	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;

	/* different cases whether we're doing 
	 * realtime, VE/constants, or I/O test 
	 */
	switch (which_data)
	{
		case REALTIME_VARS:
			while (poll(&ufds,1,serial_params->poll_timeout))
			{
				total += res = read(serial_params->fd,ptr+total,
						serial_params->rtvars_size); 
				//printf("Realtime read %i, total %i\n",res,total);
				if (total == serial_params->rtvars_size)
					break;
			}
			/* the number of bytes expected for raw data read */
			if (total > serial_params->rtvars_size) 
			{
				/* Serial I/O problem, resetting port 
				 * This problem can occur if the MS is 
				 * resetting due to power spikes, or other 
				 * problems with the serial interface, 
				 * perhaps even a very slow machine
				 * The problem is part of the data is lost, 
				 * and since we read in rtvars_size
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
				open_serial(serial_params->port_name);

				/* The raw_reader_running flag MUST be reset 
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
			else
			{

				raw_runtime = (struct Raw_Runtime_Std *)buf;
				/* Test for MS reset */
				if (just_starting)
				{
					lastcount = raw_runtime->secl;
					just_starting = FALSE;
				}
				/* Check for clock jump from the MS, a 
				 * jump in time from the MS clock indicates 
				 * a reset due to power and/or noise.
				 */
				if ((lastcount - raw_runtime->secl > 1) && \
					(lastcount - raw_runtime->secl != 255))
					ms_reset_count++;
				else
					ms_goodread_count++;

				lastcount = raw_runtime->secl;

				/* copy last round to runtime_last */
				memcpy(runtime_last,runtime,
						sizeof(struct Runtime_Common));

				/* Feed raw buffer over to post_process()
				 * as a void * and pass it a pointer to the new
				 * area for the parsed data...
				 */
				post_process((void *)buf,(void *)runtime);
			}

			break;

		case VE_AND_CONSTANTS_1:
			
			while (poll(&ufds,1,serial_params->poll_timeout) )
			{
				total += res = read(serial_params->fd,ptr+total,
						serial_params->table0_size); 
				//printf("polling VE/Const read %i, total %i\n",res,total);
			}
			/* the number of bytes expected for raw data read */
			if (total != serial_params->table0_size) 
			{
				fprintf(stderr,__FILE__":  Error reading VE/Constants for page 0\n");
				serial_params->errcount++;
			}
			else
			{
				/* Two copies, working copy and temp for 
				 * comparison against to know if we have 
				 * to burn stuff to flash.
				 */
				memcpy(ms_data,buf,
						sizeof(struct Ve_Const_Std));
				memcpy(ms_data_last,buf,
						sizeof(struct Ve_Const_Std));

				ms_ve_goodread_count++;
			}
			break;

		case VE_AND_CONSTANTS_2:
			while (poll(&ufds,1,serial_params->poll_timeout) )
			{
				total += res = read(serial_params->fd,ptr+total,
						serial_params->table1_size); 
			}
			/* the number of bytes expected for raw data read */
			if (total != serial_params->table1_size) 
			{
				fprintf(stderr,__FILE__":  Error reading VE/Constants for page 1\n");
				serial_params->errcount++;
			}
			else
			{
				/* Two copies, working copy and temp for 
				 * comparison against to know if we have 
				 * to burn stuff to flash.
				 */
				memcpy(ms_data+MS_PAGE_SIZE,buf,
						sizeof(struct Ve_Const_Std));
				memcpy(ms_data_last+MS_PAGE_SIZE,buf,
						sizeof(struct Ve_Const_Std));

				ms_ve_goodread_count++;
			}
			//printf("Dualtable not supported yet.. \n");
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
