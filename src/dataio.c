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
#include <glib/gprintf.h>
#include <ms_structures.h>
#include <post_process.h>
#include <pthread.h>
#include <serialio.h>
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
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gboolean bad_read = FALSE;
	unsigned char buf[255];
	unsigned char *ptr = buf;
	struct pollfd ufds;
	struct Raw_Runtime_Std *raw_runtime = NULL;
	extern unsigned char *ms_data;
	extern unsigned char *ms_data_last;
	extern struct Serial_Params *serial_params;
	extern struct Runtime_Common *runtime;

	//g_printf("handle_ms_data\n");
	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;

	/* different cases whether we're doing 
	 * realtime, VE/constants, or I/O test 
	 */
	switch (which_data)
	{
		case REALTIME_VARS:
			/* Data arrived,  drain buffer until we receive
			 * serial->params->rtvars_size, or readcount
			 * exceeded... 
			 */
			total_read = 0;
			total_wanted = serial_params->rtvars_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				//g_printf("requesting %i bytes, ",serial_params->rtvars_size-total_read);

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				//g_printf("read %i bytes, count %i\n",res,count);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					goto jumpout;
				}
			}
			if (bad_read)
			{
				g_fprintf(stderr,__FILE__":  Error reading Real-Time Variables \n");
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}

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

			/* Feed raw buffer over to post_process()
			 * as a void * and pass it a pointer to the new
			 * area for the parsed data...
			 */
			post_process((void *)buf,(void *)runtime);

			break;

		case VE_AND_CONSTANTS_1:
			total_read = 0;
			total_wanted = serial_params->table0_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
#ifdef DEBUG
				g_printf("requesting %i bytes, ",total_wanted-total_read);
#endif

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

#ifdef DEBUG
				g_printf("read %i bytes, running total: %i\n",res,total_read);
#endif
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					goto jumpout;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				g_fprintf(stderr,__FILE__":  Error reading VE/Constants for page 0\n");
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}
			/* Two copies, working copy and temp for 
			 * comparison against to know if we have 
			 * to burn stuff to flash.
			 */
			memcpy(ms_data,buf,
					sizeof(struct Ve_Const_Std));
			memcpy(ms_data_last,buf,
					sizeof(struct Ve_Const_Std));

			ms_ve_goodread_count++;
			break;

		case VE_AND_CONSTANTS_2:
			total_read = 0;
			total_wanted = serial_params->table1_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
#ifdef DEBUG
				g_printf("requesting %i bytes, ",total_wanted-total_read);
#endif

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

#ifdef DEBUG
				g_printf("read %i bytes, running total: %i\n",res,total_read);
#endif
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					goto jumpout;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				g_fprintf(stderr,__FILE__":  Error reading VE/Constants for table 2\n");
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}
			/* Two copies, working copy and temp for 
			 * comparison against to know if we have 
			 * to burn stuff to flash.
			 */
			memcpy(ms_data+MS_PAGE_SIZE,buf,
					sizeof(struct Ve_Const_DT_2));
			memcpy(ms_data_last+MS_PAGE_SIZE,buf,
					sizeof(struct Ve_Const_DT_2));

			ms_ve_goodread_count++;
			break;

		case IGNITION_VARS:
			total_read = 0;
			total_wanted = serial_params->ignvars_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					goto jumpout;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				g_fprintf(stderr,__FILE__":  Error reading VE/Constants for table 2\n");
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}
			/* Two copies, working copy and temp for 
			 * comparison against to know if we have 
			 * to burn stuff to flash.
			 */
			/* clear memory area */
			memset(ms_data+MS_PAGE_SIZE,0,MS_PAGE_SIZE);
			memset(ms_data_last+MS_PAGE_SIZE,0,MS_PAGE_SIZE);
			/* Copy new data into memory and backup blocks */
			memcpy(ms_data+MS_PAGE_SIZE,buf,
					sizeof(struct Ignition_Table));
			memcpy(ms_data_last+MS_PAGE_SIZE,buf,
					sizeof(struct Ignition_Table));

			ms_ve_goodread_count++;
			break;
		case RAW_MEMORY:
			g_printf("Not designed yet...\n");
			break;
		default:
			g_printf("handle_ms_data, improper case, contact author\n");
			break;
	}
	jumpout:

	//g_printf("leaving\n");
	return TRUE;
}
