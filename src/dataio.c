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
#include <rtv_processor.h>
#include <serialio.h>
#include <string.h>
#include <structures.h>
#include <unistd.h>


static gint lastcount=0;
gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;

/// Returns true on success or false for failure....
gboolean handle_ms_data(InputHandler handler, void * msg)
{
	gint res = 0;
	gint i = 0;
	gboolean state = TRUE;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gboolean bad_read = FALSE;
	guchar buf[2048];
	guchar *ptr = buf;
	struct Raw_Runtime_Std *raw_runtime = NULL;
	struct Io_Message *message = (struct Io_Message *)msg;
	extern gint **ms_data;
	extern gint **ms_data_last;
	extern struct Serial_Params *serial_params;
	extern struct Firmware_Details *firmware;

	dbg_func("\n"__FILE__": handle_ms_data()\tENTERED...\n\n",IO_PROCESS);

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
				dbg_func(g_strdup_printf("\tC_TEST requesting %i bytes\n",total_wanted-total_read),IO_PROCESS);

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
				dbg_func(__FILE__": handle_ms_data()\n\tError reading ECU Clock (C_TEST)\n",CRITICAL);
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
				goto jumpout;
			}
			dump_output(total_read,buf);
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
				dbg_func(g_strdup_printf("\tRT_VARS requesting %i bytes\n",total_wanted-total_read),IO_PROCESS);

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
				dbg_func(__FILE__": handle_ms_data()\n\tError reading Real-Time Variables \n",CRITICAL);
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
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
			dump_output(total_read,buf);
			process_rt_vars((void *)buf);
			break;

		case VE_BLOCK:
			total_read = 0;
			total_wanted = firmware->page_params[message->page]->size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				dbg_func(g_strdup_printf("\tVE_BLOCK, page %i, requesting %i bytes\n",message->page,total_wanted-total_read),IO_PROCESS);

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				dbg_func(g_strdup_printf("\tVE_BLOCK read %i bytes, running total: %i\n",res,total_read),IO_PROCESS);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				dbg_func(g_strdup_printf(__FILE__"handle_ms_data()\n\tError reading VE-Block Constants for page %i\n",message->page),CRITICAL);
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
				goto jumpout;
			}
			/* Two copies, working copy and temp for 
			 * comparison against to know if we have 
			 * to burn stuff to flash.
			 */
			for (i=0;i<total_read;i++)
				ms_data[message->page][i] = buf[i];
			memcpy(ms_data_last[message->page],ms_data[message->page],total_read*sizeof(gint));
			ms_ve_goodread_count++;
			dump_output(total_read,buf);
			break;

		case RAW_MEMORY_DUMP:
			total_read = 0;
			total_wanted = firmware->memblock_size;
			zerocount = 0;

			while (total_read < total_wanted )
			{
				dbg_func(g_strdup_printf("\tRAW_MEMORY_DUMP requesting %i bytes\n",total_wanted-total_read),IO_PROCESS);
				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res == 0)
					zerocount++;

				dbg_func(g_strdup_printf("\tread %i bytes, running total: %i\n",res,total_read),IO_PROCESS);
				if (zerocount >= 5)  // 3 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				dbg_func(__FILE__"handle_ms_data()\n\tError reading Raw Memory Block\n",CRITICAL);
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
				goto jumpout;
			}
			post_process_raw_memory((void *)buf, message->offset);
			dump_output(total_read,buf);
			break;
		default:
			dbg_func(__FILE__": handle_ms_data()\n\timproper case, contact author!\n",CRITICAL);
			state = FALSE;
			break;
	}
jumpout:

	dbg_func("\n"__FILE__": handle_ms_data\tLEAVING...\n\n",IO_PROCESS);
	return state;
}

void dump_output(gint total_read, guchar *buf)
{
	guchar *p = NULL;
	gint j = 0;

	p = buf;
	if (total_read > 0)
	{
		dbg_func(g_strdup_printf(__FILE__": dataio.c()\n\tDumping output, eable IO_PROCESS debug to see the cmd's sent\n"),SERIAL_RD);
		p = buf;
		for (j=0;j<total_read;j++)
		{
			dbg_func(g_strdup_printf("0x%.2x ", p[j]),SERIAL_RD);
			if (!((j+1)%8))
				dbg_func(g_strdup_printf("\n"),SERIAL_RD);
		}
		dbg_func("\n\n",SERIAL_RD);
	}

}
