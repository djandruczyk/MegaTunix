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
#include <sys/types.h>
#include <sys/time.h>
#include <threads.h>
#include <unistd.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif


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
gint read_data(gint total_wanted, void **buffer, gboolean reset_on_fail)
{
	gint res = 0;
	gint total_read = 0;
	gint zerocount = 0;
	gboolean bad_read = FALSE;
	guchar buf[4096];
	guchar *ptr = buf;
	gboolean ignore_errors = FALSE;
	extern gboolean connected;
	extern Serial_Params *serial_params;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	static gint failcount = 0;
	static gboolean reset = FALSE;

	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() before lock reentrant mutex\n"));
	g_static_mutex_lock(&mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() after lock reentrant mutex\n"));

	dbg_func(IO_PROCESS,g_strdup("\n"__FILE__": read_data()\tENTERED...\n\n"));

	total_read = 0;
	zerocount = 0;
	if (total_wanted == -1)
	{
		ignore_errors = TRUE;
		total_wanted = 1024;
	}
	/* Werid windows issue.  Occasional "short" reads,  but nothing else
	 * comes in for some reason. So if that happens, double what's read
	 * next time and throw it away to get things back in sync. 
	 * Ugly hack,  but couldn't find out why it did it.  might be due to
	 * excess latency in my test VM
	 */
#ifdef __WIN32__
	if (reset)
		total_wanted *= 2;
#endif

	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() before lock serio_mutex\n"));
	g_static_mutex_lock(&serio_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() after lock serio_mutex\n"));
	while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
	{
		dbg_func(IO_PROCESS,g_strdup_printf(__FILE__"\t requesting %i bytes\n",total_wanted-total_read));

		total_read += res = read_wrapper(serial_params->fd,
				ptr+total_read,
				total_wanted-total_read);

		/* Increment bad read counter.... */
		if (res < 0) /* I/O Error Device disappearance or other */
		{
			dbg_func(IO_PROCESS|CRITICAL,g_strdup_printf(__FILE__"\tI/O ERROR: \"%s\"\n",(gchar *)g_strerror(errno)));
			bad_read = TRUE;
			connected = FALSE;
			break;
		}
		if (res == 0)  /* Short read! */
		{
			bad_read = TRUE;
			break;
		}

		dbg_func(IO_PROCESS,g_strdup_printf(__FILE__"\tread %i bytes, running total %i\n",res,total_read));
	}
	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() before UNlock serio_mutex\n"));
	g_static_mutex_unlock(&serio_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() after UNlock serio_mutex\n"));
	if ((bad_read) && (!ignore_errors))
	{
		dbg_func(IO_PROCESS|CRITICAL,g_strdup(__FILE__": read_data()\n\tError reading from ECU\n"));

		serial_params->errcount++;
		if ((reset_on_fail) && (!reset))
			reset = TRUE;
		else
			reset = FALSE;
		failcount++;
		/* Excessive failures triggers port recheck */
		if (failcount > 10)
			connected = FALSE;
	}
	else
	{
		failcount = 0;
		reset = FALSE;
	}

	if (buffer)
		*buffer = g_memdup(buf,total_read);
	dump_output(total_read,buf);
	dbg_func(IO_PROCESS,g_strdup("\n"__FILE__": read_data\tLEAVING...\n\n"));
	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() before UNlock reentrant mutex\n"));
	g_static_mutex_unlock(&mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": read_data() after UNlock reentrant mutex\n"));
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
			dbg_func(SERIAL_RD,g_strdup_printf(__FILE__": dataio.c()\n\tDumping output, enable IO_PROCESS debug to see the cmd's that were sent\n"));
			tmpbuf = g_strndup(((gchar *)buf),total_read);
			dbg_func(SERIAL_RD,g_strdup_printf(__FILE__": dataio.c()\n\tDumping Output string: \"%s\"\n",tmpbuf));
			g_free(tmpbuf);
			dbg_func(SERIAL_RD,g_strdup_printf("Data is in HEX!!\n"));
		}
		for (j=0;j<total_read;j++)
		{
			dbg_func(SERIAL_RD,g_strdup_printf("%.2x ", p[j]));
			if (!((j+1)%8))
				dbg_func(SERIAL_RD,g_strdup("\n"));
		}
		dbg_func(SERIAL_RD,g_strdup("\n\n"));
	}

}


gint read_wrapper(gint fd, void * buf, size_t count)
{
	extern Serial_Params * serial_params;
	gint res = 0;
	fd_set rd;
	extern GObject *global_data;
	struct timeval timeout = {0,OBJ_GET(global_data, "read_timeout") == NULL ? 500000:(gint)OBJ_GET(global_data, "read_timeout")*1000};
	FD_ZERO(&rd);
	FD_SET(fd,&rd);

	/* Network mode requires select to see if data is ready, otherwise
	 * connection will block.  Serial is configured with timeout if no
	 * data is avail,  hence we simulate that with select.. Setting this
	 * timeout around 500ms seems to give us ok function to new zealand,
	 * but may require tweaking for slow wireless links.
	 */
	if (serial_params->net_mode)
	{
		res = select(fd+1,&rd,NULL,NULL,&timeout);
		if (res < 0) /* Error, socket close, abort */
			return 0;
		if (res > 0) /* Data Arrived! */
			res = recv(fd,buf,count,0);
	}
	else
		res = read(fd,buf,count);
	if (res < 0)
		res = 0;
	return res;
}

gint write_wrapper(gint fd, const void *buf, size_t count)
{
	extern Serial_Params * serial_params;
	gint res = 0;

	if (serial_params->net_mode)
		res = send(fd,buf,count,0);
	else
		res = write(fd,buf,count);
	if (res < 0)
	{
		printf("Write error!\n");
		res = 0;
	}
	return res;
}
