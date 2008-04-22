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
#include <datamgmt.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <firmware.h>
#include <post_process.h>
#include <rtv_processor.h>
#include <serialio.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>


gint ms_reset_count;
gint ms_goodread_count;
gint ms_ve_goodread_count;
gint just_starting;
extern gint dbg_lvl;
extern GStaticMutex serio_mutex;
extern GObject *global_data;


/*!
 \brief read_data() reads in the data from the ECU, checks to make sure
 enough arrived, copies it to thedestination buffer and returns;
 \param total_wanted, if set to -1, input is variabel and we don't error out.
 otherwise error out if count doesn't match what is asked for
 \param buffer, pointer to buffer to stick the data.
 \returns TRUE on success, FALSE on failure 
 */
gint read_data(gint total_wanted, void **buffer)
{
	gint res = 0;
	gint total_read = 0;
	gint zerocount = 0;
	gboolean bad_read = FALSE;
	guchar buf[4096];
	guchar *ptr = buf;
	gchar *err_text = NULL;
	gboolean ignore_errors = FALSE;
	extern gboolean connected;
	extern Serial_Params *serial_params;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);
	if (dbg_lvl & IO_PROCESS)
		dbg_func(g_strdup("\n"__FILE__": read_data()\tENTERED...\n\n"));

	total_read = 0;
	zerocount = 0;
	if (total_wanted == -1)
	{
		ignore_errors = TRUE;
		total_wanted = 1024;
	}

	g_static_mutex_lock(&serio_mutex);
	while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
	{
		if (dbg_lvl & IO_PROCESS)
			dbg_func(g_strdup_printf(__FILE__"\t requesting %i bytes\n",total_wanted-total_read));

		total_read += res = read(serial_params->fd,
				ptr+total_read,
				total_wanted-total_read);

		/* Increment bad read counter.... */
		if (res < 0) /* I/O Error */
		{
			err_text = (gchar *)g_strerror(errno);
			if (dbg_lvl & (IO_PROCESS|CRITICAL))
				dbg_func(g_strdup_printf(__FILE__"\tI/O ERROR: \"%s\"\n",err_text));
			bad_read = TRUE;
			break;
		}
		if (res == 0)
			zerocount++;

		if (dbg_lvl & IO_PROCESS)
			dbg_func(g_strdup_printf(__FILE__"\tread %i bytes, running total %i\n",res,total_read));
		if (zerocount > 1)  /* 2 bad reads, abort */
		{
			bad_read = TRUE;
			break;
		}
	}
	g_static_mutex_unlock(&serio_mutex);
	if ((bad_read) && (!ignore_errors))
	{
		if (dbg_lvl & (IO_PROCESS|CRITICAL))
			dbg_func(g_strdup(__FILE__": read_data()\n\tError reading from ECU\n"));
		if (dbg_lvl & (SERIAL_WR|SERIAL_RD|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": read_data()\n\tError reading data: %s\n",g_strerror(errno)));
		flush_serial(serial_params->fd, TCIOFLUSH);
		serial_params->errcount++;
		connected = FALSE;
	}

	if (buffer)
		*buffer = g_memdup(buf,total_read);
	dump_output(total_read,buf);
	if (dbg_lvl & IO_PROCESS)
		dbg_func(g_strdup("\n"__FILE__": read_data\tLEAVING...\n\n"));
	g_static_mutex_unlock(&mutex);
	return total_read;
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
			tmpbuf = g_strndup(((gchar *)buf),total_read);
			dbg_func(g_strdup_printf(__FILE__": dataio.c()\n\tDumping Output string: \"%s\"\n",tmpbuf));
			g_free(tmpbuf);
			dbg_func(g_strdup_printf("Data is in HEX!!\n"));
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
