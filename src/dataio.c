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
#include <errno.h>
#include <ms_structures.h>
#include <post_process.h>
#include <rtv_processor.h>
#include <serialio.h>
#include <string.h>
#include <structures.h>
#include <threads.h>
#include <unistd.h>


static gint lastcount=0;
gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;
extern gint dbg_lvl;
extern GStaticMutex serio_mutex;


/*!
 \brief handle_ecu_data() reads in the data from the ECU, checks to make sure
 enough arrived, copies it to thedestination buffer and returns;
 \param handler (InputHandler enumeration) determines a courseof action
 \param message (Io_Message *) extra data for some handlers...
 \returns TRUE on success, FALSE on failure 
 */
gboolean handle_ecu_data(InputHandler handler, Io_Message * message)
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
	gchar *err_text = NULL;
	gchar *tmpbuf = NULL;
	extern gint **ms_data;
	extern gint **ms_data_last;
	extern Serial_Params *serial_params;
	extern Firmware_Details *firmware;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);
	if (dbg_lvl & IO_PROCESS)
		dbg_func(g_strdup("\n"__FILE__": handle_ecu_data()\tENTERED...\n\n"));

	/* different cases whether we're doing 
	 * realtime, VE/constants, or I/O test 
	 */
	switch (handler)
	{
		case NULL_HANDLER:
			/* This is hte case where we don't expect any data
			 * back, like a reboot command (X)
			 * so jsut flush IO and return nicely
			 */
			total_read = 0;
			total_wanted = 1024;
			zerocount = 0;
			g_static_mutex_lock(&serio_mutex);
			while (total_read < total_wanted )
			{
				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tNULL_HANDLER requesting %i bytes\n",total_wanted-total_read));

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				/* Increment bad read counter.... */
				if (res < 0) /* ERROR condition */
					break;
				if (res == 0) /* No data condition */
					zerocount++;

				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tNULL_HANDLER read %i bytes, running total %i\n",res,total_read));
				if (zerocount > 1)  /* 2 bad reads, abort */
					break;
			}
			g_static_mutex_unlock(&serio_mutex);
			flush_serial(serial_params->fd, TCIOFLUSH);
			break;
		case C_TEST:
			/* Check_ecu_comms equivalent....
			 * REALLY REALLY overkill just to read 1 byte, but 
			 * done this way for consistency sake with all the 
			 * other handlers....
			 */
			total_read = 0;
			total_wanted = 1;
			zerocount = 0;

			g_static_mutex_lock(&serio_mutex);
			while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
			{
				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tC_TEST requesting %i bytes\n",total_wanted-total_read));

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				/* Increment bad read counter.... */
				if (res < 0) /* I/O Error */
				{
					err_text = (gchar *)g_strerror(errno);
					if (dbg_lvl & (IO_PROCESS|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__"\tC_TEST I/O ERROR: \"%s\"\n",err_text));
					bad_read = TRUE;
					break;
				}
				if (res == 0)
					zerocount++;

				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tC_TEST read %i bytes, running total %i\n",res,total_read));
				if (zerocount > 1)  /* 2 bad reads, abort */
				{
					bad_read = TRUE;
					break;
				}
			}
			g_static_mutex_unlock(&serio_mutex);
			if (bad_read)
			{
				if (dbg_lvl & (IO_PROCESS|CRITICAL))
					dbg_func(g_strdup(__FILE__": handle_ecu_data()\n\tError reading ECU Clock (C_TEST)\n"));
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
				goto jumpout;
			}
			dump_output(total_read,buf);
			break;
		case GET_ERROR:
			/* A reboot command is sent and this grabs whatever
			 * string it happens to send back as long as it's 
			 * shorter than 1024 chars...
			 */
			total_read=0;
			total_wanted=1024;
			zerocount=0;
			g_static_mutex_lock(&serio_mutex);
			while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
			{
				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tGET_ERROR requesting %i bytes\n",total_wanted-total_read));

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res < 0) /* ERROR */
				{
					err_text = (gchar *)g_strerror(errno);
					if (dbg_lvl & (IO_PROCESS|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__"\tGET_ERROR I/O ERROR: \"%s\"\n",err_text));
					break;
				}
				if (res == 0) /* No Data avail */
					zerocount++;

				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tGET_ERROR read %i bytes, running total %i\n",res,total_read));
				if (zerocount > 1)  // 2 bad reads, abort
					break;
			}
			g_static_mutex_unlock(&serio_mutex);
			dump_output(total_read,buf);
			flush_serial(serial_params->fd, TCIOFLUSH);
			if (total_read <= 1)
			{
				thread_update_logbar("error_status_view",NULL,g_strdup("No ECU Errors were reported....\n"),FALSE,FALSE);
				break;
			}

			if (g_utf8_validate(((gchar *)buf)+1,total_read-1,NULL))
			{
				thread_update_logbar("error_status_view",NULL,g_strndup(((gchar *)buf)+1,total_read-1),FALSE,FALSE);
				if (dbg_lvl & (IO_PROCESS|SERIAL_RD))
				{
					tmpbuf = g_strndup(((gchar *)buf)+1,total_read-1);
					dbg_func(g_strdup_printf(__FILE__"\tECU  ERROR string: \"%s\"\n",tmpbuf));
					g_free(tmpbuf);
				}

			}
			else
			{
				thread_update_logbar("error_status_view",NULL,g_strdup("The data that came back was jibberish, try rebooting again.\n"),FALSE,FALSE);
				if (dbg_lvl & (IO_PROCESS|SERIAL_RD))
				{
					tmpbuf = g_strndup(((gchar *)buf)+1,total_read-1);
					dbg_func(g_strdup_printf(__FILE__"\tECU  ERROR string: \"%s\"\n",tmpbuf));
					g_free(tmpbuf);
				}
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

			g_static_mutex_lock(&serio_mutex);
			while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
			{
				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tRT_VARS requesting %i bytes\n",total_wanted-total_read));

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res < 0) /* ERROR */
				{
					err_text = (gchar *)g_strerror(errno);
					if (dbg_lvl & (IO_PROCESS|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__"\tRT_VARS I/O ERROR: \"%s\"\n",err_text));
					bad_read = TRUE;
					break;
				}
				if (res == 0) /* No Data avail */
					zerocount++;

				if (dbg_lvl & (IO_PROCESS))
					dbg_func(g_strdup_printf(__FILE__"\tRT_VARS read %i bytes, running total %i\n",res,total_read));
				if (zerocount > 1)  // 2 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			g_static_mutex_unlock(&serio_mutex);
			if (bad_read)
			{
				if (dbg_lvl & (IO_PROCESS|CRITICAL))
					dbg_func(g_strdup(__FILE__": handle_ecu_data()\n\tError reading Real-Time Variables \n"));
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
				goto jumpout;
			}

			ptr = buf;
			/* Test for MS reset */
			if (just_starting)
			{
				lastcount = ptr[0];
				just_starting = FALSE;
			}
			/* Check for clock jump from the MS, a 
			 * jump in time from the MS clock indicates 
			 * a reset due to power and/or noise.
			 */
			if ((lastcount - ptr[0] > 1) && \
					(lastcount - ptr[0] != 255))
			{
				ms_reset_count++;
				gdk_beep();
			}
			else
				ms_goodread_count++;

			lastcount = ptr[0];

			/* Feed raw buffer over to post_process()
			 * as a void * and pass it a pointer to the new
			 * area for the parsed data...
			 */
			dump_output(total_read,buf);
			process_rt_vars((void *)buf);
			break;

		case VE_BLOCK:
			total_read = 0;
			total_wanted = firmware->page_params[message->page]->length;
			zerocount = 0;

			g_static_mutex_lock(&serio_mutex);
			while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
			{
				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tVE_BLOCK, page %i, requesting %i bytes\n",message->page,total_wanted-total_read));

				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res < 0) /* ERROR */
				{
					err_text = (gchar *)g_strerror(errno);
					if (dbg_lvl & (IO_PROCESS|CRITICAL))
						dbg_func(g_strdup_printf(__FILE__"\tVE_BLOCK I/O ERROR: \"%s\"\n",err_text));
					bad_read = TRUE;
					break;
				}
				if (res == 0) /* NO data avail */
					zerocount++;

				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tVE_BLOCK read %i bytes, running total: %i\n",res,total_read));
				if (zerocount > 1)  // 2 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			g_static_mutex_unlock(&serio_mutex);
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				if (dbg_lvl & (IO_PROCESS|CRITICAL))
					dbg_func(g_strdup_printf(__FILE__": handle_ecu_data()\n\tError reading VE-Block Constants for page %i\n",message->page));
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

			g_static_mutex_lock(&serio_mutex);
			while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
			{
				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tRAW_MEMORY_DUMP requesting %i bytes\n",total_wanted-total_read));
				total_read += res = read(serial_params->fd,
						ptr+total_read,
						total_wanted-total_read);

				// Increment bad read counter....
				if (res < 0) /* ERROR condition */
				{
					if (dbg_lvl & (IO_PROCESS|CRITICAL))
						err_text = (gchar *)g_strerror(errno);
					dbg_func(g_strdup_printf(__FILE__"\tRAW_MEMORY_DUMP I/O ERROR: \"%s\"\n",err_text));
					bad_read = TRUE;
					break;
				}
				if (res == 0) /* NO data condition */
					zerocount++;

				if (dbg_lvl & IO_PROCESS)
					dbg_func(g_strdup_printf(__FILE__"\tread %i bytes, running total: %i\n",res,total_read));
				if (zerocount > 1)  // 2 bad reads, abort
				{
					bad_read = TRUE;
					break;
				}
			}
			g_static_mutex_unlock(&serio_mutex);
			/* the number of bytes expected for raw data read */
			if (bad_read)
			{
				if (dbg_lvl & (IO_PROCESS|CRITICAL))
					dbg_func(g_strdup(__FILE__"handle_ecu_data()\n\tError reading Raw Memory Block\n"));
				flush_serial(serial_params->fd, TCIOFLUSH);
				serial_params->errcount++;
				state = FALSE;
				goto jumpout;
			}
			post_process_raw_memory((void *)buf, message->offset);
			dump_output(total_read,buf);
			break;
		default:
			if (dbg_lvl & (IO_PROCESS|CRITICAL))
				dbg_func(g_strdup(__FILE__": handle_ecu_data()\n\timproper case, contact author!\n"));
			state = FALSE;
			break;
	}
jumpout:
	if (dbg_lvl & IO_PROCESS)
		dbg_func(g_strdup("\n"__FILE__": handle_ecu_data\tLEAVING...\n\n"));
	g_static_mutex_unlock(&mutex);
	return state;
}


/*!
 \brief dump_output() dumps the newly read data to the console in HEX for
 debugging purposes
 \param total_read (gint) total bytesto printout
 \param buf (guchar *) pointer to data to write to console
 */
void dump_output(gint total_read, guchar *buf)
{
	guchar *p = NULL;
	gchar * tmpbuf = NULL;
	gint j = 0;

	p = buf;
	if (total_read > 0)
	{
		p = buf;
		if (dbg_lvl & SERIAL_RD)
		{
			dbg_func(g_strdup_printf(__FILE__": dataio.c()\n\tDumping output, enable IO_PROCESS debug to see the cmd's that were sent\n"));
			dbg_func(g_strdup_printf("Data is in HEX!!\n"));
			tmpbuf = g_strndup(((gchar *)buf),total_read);
			dbg_func(g_strdup_printf(__FILE__": dataio.c()\n\tDumping Output string: \"%s\"\n",tmpbuf));
			g_free(tmpbuf);
		}
		for (j=0;j<total_read;j++)
		{
			if (dbg_lvl & SERIAL_RD)
				dbg_func(g_strdup_printf("%.2x ", p[j]));
			if (!((j+1)%8))
			{
				if (dbg_lvl & SERIAL_RD)
					dbg_func(g_strdup("\n"));
			}
		}
		if (dbg_lvl & SERIAL_RD)
			dbg_func(g_strdup("\n\n"));
	}

}
