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
#include <serialio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif


/* Externs */
extern gconstpointer *global_data;

/* Cause OS-X sucks.... */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/*!
 \brief read_data() reads in the data from the ECU, checks to make sure
 enough arrived, copies it to thedestination buffer and returns;
 \param total_wanted, if set to -1, input is variabel and we don't error out.
 otherwise error out if count doesn't match what is asked for
 \param buffer, pointer to buffer to stick the data.
 \returns TRUE on success, FALSE on failure 
 */
G_MODULE_EXPORT gint read_data(gint total_wanted, void **buffer, gboolean reset_on_fail)
{
	static GMutex *serio_mutex = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	static gint failcount = 0;
	static gboolean reset = FALSE;
	gboolean res = 0;
	gint total_read = 0;
	gint zerocount = 0;
	gint len = 0;
	gboolean bad_read = FALSE;
	guchar buf[4096];
	guchar *ptr = buf;
	gboolean ignore_errors = FALSE;
	Serial_Params *serial_params = NULL;;
	serial_params = DATA_GET(global_data,"serial_params");

	if (!serio_mutex)
		serio_mutex = DATA_GET(global_data,"serio_mutex");

	g_static_mutex_lock(&mutex);

	dbg_func_f(IO_PROCESS,g_strdup("\n"__FILE__": read_data()\tENTERED...\n\n"));

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

	g_mutex_lock(serio_mutex);
	while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
	{
		dbg_func_f(IO_PROCESS,g_strdup_printf(__FILE__"\t requesting %i bytes\n",total_wanted-total_read));

		res = read_wrapper(serial_params->fd,
				ptr+total_read,
				total_wanted-total_read,&len);
		total_read += len;

		/* Increment bad read counter.... */
		if (!res) /* I/O Error Device disappearance or other */
		{
			dbg_func_f(IO_PROCESS|CRITICAL,g_strdup_printf(__FILE__"\tI/O ERROR: \"%s\"\n",(gchar *)g_strerror(errno)));
			bad_read = TRUE;
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			break;
		}
		if (len == 0) /* Short read!*/
			zerocount++;
		if ((len == 0) && (zerocount > 3))  /* Too many Short reads! */
		{
			bad_read = TRUE;
			break;
		}

		dbg_func_f(IO_PROCESS,g_strdup_printf(__FILE__"\tread %i bytes, running total %i\n",res,total_read));
	}
	g_mutex_unlock(serio_mutex);
	if ((bad_read) && (!ignore_errors))
	{
		dbg_func_f(IO_PROCESS|CRITICAL,g_strdup(__FILE__": read_data()\n\tError reading from ECU\n"));

		serial_params->errcount++;
		if ((reset_on_fail) && (!reset))
			reset = TRUE;
		else
			reset = FALSE;
		failcount++;
		/* Excessive failures triggers port recheck */
		if (failcount > 10)
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	}
	else
	{
		failcount = 0;
		reset = FALSE;
	}

	if (buffer)
		*buffer = g_memdup(buf,total_read);
	dump_output(total_read,buf);
	dbg_func_f(IO_PROCESS,g_strdup("\n"__FILE__": read_data\tLEAVING...\n\n"));
	g_static_mutex_unlock(&mutex);
	return total_read;
}


/*!
 \brief dump_output() dumps the newly read data to the console in HEX for
 debugging purposes
 \param total_read (gint) total bytesto printout
 \param buf (guchar *) pointer to data to write to console
 */
G_MODULE_EXPORT void dump_output(gint total_read, guchar *buf)
{
	guchar *p = NULL;
	gchar * tmpbuf = NULL;
	gint j = 0;
	gint dbg_lvl = 0;
	dbg_lvl = (gint)DATA_GET(global_data,"dbg_lvl");

	p = buf;
	if (total_read > 0)
	{
		p = buf;
		if (dbg_lvl & SERIAL_RD)
		{
			dbg_func_f(SERIAL_RD,g_strdup_printf(__FILE__": dataio.c()\n\tDumping output, enable IO_PROCESS debug to see the cmd's that were sent\n"));
			tmpbuf = g_strndup(((gchar *)buf),total_read);
			dbg_func_f(SERIAL_RD,g_strdup_printf(__FILE__": dataio.c()\n\tDumping Output string: \"%s\"\n",tmpbuf));
			g_free(tmpbuf);
			dbg_func_f(SERIAL_RD,g_strdup_printf("Data is in HEX!!\n"));
		}
		for (j=0;j<total_read;j++)
		{
			dbg_func_f(SERIAL_RD,g_strdup_printf("%.2x ", p[j]));
			if (!((j+1)%8))
				dbg_func_f(SERIAL_RD,g_strdup("\n"));
		}
		dbg_func_f(SERIAL_RD,g_strdup("\n\n"));
	}

}


G_MODULE_EXPORT gboolean read_wrapper(gint fd, void * buf, size_t count, gint *len)
{
	gint res = 0;
	fd_set rd;
	struct timeval timeout;
	Serial_Params *serial_params = NULL;
	serial_params = DATA_GET(global_data,"serial_params");

	FD_ZERO(&rd);
	FD_SET(fd,&rd);

	*len = 0;
	timeout.tv_sec = 0;
	timeout.tv_usec = DATA_GET(global_data, "read_timeout") == NULL ? 500000:(GINT)DATA_GET(global_data, "read_timeout")*1000;
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
			return FALSE;
		if (res > 0) /* Data Arrived! */
			*len = recv(fd,buf,count,0);
		return TRUE;
	}
	else
		res = read(fd,buf,count);
	if (res < 0)
		return FALSE;
	else
		*len = res;
	return TRUE;
}

G_MODULE_EXPORT gboolean write_wrapper(gint fd, const void *buf, size_t count, gint *len)
{
	gint res = 0;
	GError *error = NULL;
	Serial_Params *serial_params = NULL;
	serial_params = DATA_GET(global_data,"serial_params");

/*	printf("write_wrapper\n"); */
	if (serial_params->net_mode)
	{
/*		printf("net mode write\n"); */
#if GTK_MINOR_VERSION >= 18
		res = g_socket_send(serial_params->socket,buf,(gsize)count,NULL,&error);
#else
		res = send(fd,buf,count,MSG_NOSIGNAL);
#endif
		if (res == -1)
		{
			dbg_func_f(CRITICAL|SERIAL_WR,g_strdup_printf("\n"__FILE__": write_wrapper()\n\tg_socket_send_error \"%s\"\n\n",error->message));
			g_error_free(error);
		}
	}
	else
	{
/*		printf("normal write %i bytes\n",count); */
		res = write(fd,buf,count);
/*		printf("result of write is %i\n",res); */
	}
	*len = res;
	if (res < 0)
	{
		printf(_("Write error! \"%s\"\n"),strerror(errno));
		return FALSE;
	}
	return TRUE;
}
