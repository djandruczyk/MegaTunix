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
#include <debugging.h>
#include <enums.h>
#include <ms_structures.h>
#include <post_process.h>
#include <pthread.h>
#include <serialio.h>
#include <string.h>
#include <structures.h>
#include <unistd.h>



static gint lastcount=0;
gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;
       
void handle_ms_data(InputHandler handler, gint page)
{
	gint res = 0;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gboolean bad_read = FALSE;
	unsigned char buf[2048];
	unsigned char *ptr = buf;
	struct Raw_Runtime_Std *raw_runtime = NULL;
	extern unsigned char *ms_data[MAX_SUPPORTED_PAGES];
	extern unsigned char *ms_data_last[MAX_SUPPORTED_PAGES];
	extern struct Serial_Params *serial_params;
	extern struct Firmware_Details *firmware;
	extern struct Runtime_Common *runtime;

	dbg_func(__FILE__": handle_ms_data()\n",IO_PROCESS);

	/* different cases whether we're doing 
	 * realtime, VE/constants, or I/O test 
	 */
	switch (handler)
	{
		case C_TEST:
			/* Check_ecu_comms equivalent....
			 * REALLY REALLY overkill just to read 1 byte, but 
			 * done this way for consistency sake with all the 
			 * other handlers....
			 */
			total_read = 0;
			total_wanted = 1;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				dbg_func(g_strdup_printf(__FILE__": C_TEST requesting %i bytes, ",total_wanted-total_read),IO_PROCESS);

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				dbg_func(g_strdup_printf("\tC_TEST read %i bytes, running total %i\n",res,total_read),IO_PROCESS);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			if (bad_read)
			{
				dbg_func(__FILE__":  Error reading MS Clock (C_TEST)\n",CRITICAL);
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}
			break;
			
		case REALTIME_VARS:
			/* Data arrived,  drain buffer until we receive
			 * serial->params->rtvars_size, or readcount
			 * exceeded... 
			 */
			total_read = 0;
			total_wanted = firmware->rtvars_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				dbg_func(g_strdup_printf(__FILE__": RT_VARS requesting %i bytes, ",total_wanted-total_read),IO_PROCESS);

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				dbg_func(g_strdup_printf("\tRT_VARS read %i bytes, running total %i\n",res,total_read),IO_PROCESS);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			if (bad_read)
			{
				dbg_func(__FILE__":  Error reading Real-Time Variables \n",CRITICAL);
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
			post_process_realtime_vars((void *)buf,(void *)runtime);

			break;

		case VE_BLOCK:
			total_read = 0;
			total_wanted = firmware->page_params[page]->size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				dbg_func(g_strdup_printf(__FILE__": VE_BLOCK, page %i, requesting %i bytes, ",page,total_wanted-total_read),IO_PROCESS);

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				dbg_func(g_strdup_printf("read %i bytes, running total: %i\n",res,total_read),IO_PROCESS);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				dbg_func(g_strdup_printf(__FILE__":  Error reading VE-BlockConstants for page %i\n",page),CRITICAL);
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}
			/* Two copies, working copy and temp for 
			 * comparison against to know if we have 
			 * to burn stuff to flash.
			 */
			memcpy(ms_data[page],buf,total_wanted);
			memcpy(ms_data_last[page],buf,total_wanted);
			ms_ve_goodread_count++;
			break;

		case RAW_MEMORY_DUMP:
			total_read = 0;
			total_wanted = firmware->memblock_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				dbg_func(g_strdup_printf(__FILE__": RAW_MEMORY_DUMP requesting %i bytes, ",total_wanted-total_read),IO_PROCESS);
				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				dbg_func(g_strdup_printf("read %i bytes, running total: %i\n",res,total_read),IO_PROCESS);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				dbg_func(__FILE__": Error reading Raw Memory Block\n",CRITICAL);
				tcflush(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				goto jumpout;
			}
			post_process_raw_memory((void *)buf, page);
			break;
		default:
			dbg_func("handle_ms_data, improper case, contact author\n",CRITICAL);
			break;
	}
	jumpout:

	return;
}
