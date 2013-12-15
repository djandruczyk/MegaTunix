/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/dataio.c
  \ingroup CoreMtx
  \brief Does the actual reading and writing to the ECU
  Provides wrappers that do the actual reading/writing to allow the 
  implementations to be changed with minimal disruption
  \author David Andruczyk
  */

#include <binlogger.h>
#include <dataio.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <firmware.h>
#include <notifications.h>
#include <plugin.h>
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
#include <netinet/in.h>
#include <sys/stat.h>
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
  \param total_wanted if set to -1, input is variable and we don't error out.
  otherwise error out if count doesn't match what is asked for
  \param buffer is the pointer to buffer to stick the data.
  \param reset_on_fail is a hacky flag that should be removed, (win32ism)
  \returns TRUE on success, FALSE on failure 
  */
G_MODULE_EXPORT gint read_data(gint total_wanted, guint8 **buffer, gboolean reset_on_fail)
{
	static GMutex *serio_mutex = NULL;
	static GMutex mutex;
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
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	ENTER();

	if (!serio_mutex)
		serio_mutex = (GMutex *)DATA_GET(global_data,"serio_mutex");

	g_mutex_lock(&mutex);

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
		MTXDBG(IO_PROCESS,_("Requesting %i bytes\n"),total_wanted-total_read);

		res = read_wrapper(serial_params->fd,
				ptr+total_read,
				total_wanted-total_read,&len);
		total_read += len;

		/* Increment bad read counter.... */
		if (!res) /* I/O Error Device disappearance or other */
		{
			MTXDBG((Dbg_Class)(IO_PROCESS|CRITICAL),_("I/O ERROR: \"%s\"\n"),(gchar *)g_strerror(errno));
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

		MTXDBG(IO_PROCESS,_("Read %i bytes, running total %i\n"),len,total_read);
	}
	g_mutex_unlock(serio_mutex);
	if ((bad_read) && (!ignore_errors))
	{
		MTXDBG((Dbg_Class)(IO_PROCESS|CRITICAL),_("Error reading from ECU\n"));

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
	g_mutex_unlock(&mutex);
	EXIT();
	return total_read;
}


/*!
  \brief write_data() physically sends the data to the ECU.
  \param message is the pointer to an Io_Message structure
  */
G_MODULE_EXPORT gboolean write_data(Io_Message *message)
{
	static GMutex *serio_mutex = NULL;
	static Serial_Params *serial_params = NULL;
	static Firmware_Details *firmware = NULL;
	static gfloat *factor = NULL;
	OutputData *output = (OutputData *)message->payload;
	gint res = 0;
	gchar * err_text = NULL;
	guint i = 0;
	gint j = 0;
	gint len = 0;
	guint8 * buffer = NULL;
	gint burst_len = 0;
	gboolean notifies = FALSE;
	gint notif_divisor = 32;
	WriteMode mode = MTX_CMD_WRITE;
	gboolean retval = TRUE;
	DBlock *block = NULL;
	static GMutex mutex;
	static void (*store_new_block)(gpointer) = NULL;
	static void (*set_ecu_data)(gpointer,gint *) = NULL;

	ENTER();

	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!serial_params)
		serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	if (!serio_mutex)
		serio_mutex = (GMutex *)DATA_GET(global_data,"serio_mutex");
	if (!factor)
		factor = (gfloat *)DATA_GET(global_data,"sleep_correction");
	if (!set_ecu_data)
		get_symbol("set_ecu_data",(void **)&set_ecu_data);
	if (!store_new_block)
		get_symbol("store_new_block",(void **)&store_new_block);

	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(serial_params,FALSE);
	g_return_val_if_fail(serio_mutex,FALSE);
	g_return_val_if_fail(factor,FALSE);
	g_return_val_if_fail(set_ecu_data,FALSE);
	g_return_val_if_fail(store_new_block,FALSE);

	g_mutex_lock(&mutex);
	g_mutex_lock(serio_mutex);

	if (output)
		mode = (WriteMode)(GINT)DATA_GET(output->data,"mode");

	if (DATA_GET(global_data,"offline"))
	{
		switch (mode)
		{
			case MTX_SIMPLE_WRITE:
				set_ecu_data(output->data,NULL);
				break;
			case MTX_CHUNK_WRITE:
				store_new_block(output->data);
				break;
			case MTX_CMD_WRITE:
				break;
		}
		g_mutex_unlock(serio_mutex);
		g_mutex_unlock(&mutex);
		EXIT();
		return TRUE;		/* can't write anything if offline */
	}
	if (!DATA_GET(global_data,"connected"))
	{
		g_mutex_unlock(serio_mutex);
		g_mutex_unlock(&mutex);
		EXIT();
		return FALSE;		/* can't write anything if disconnected */
	}

	/* For MS3 1.1 which uses a CRC32 wrapped serial stream, we can't easily
	 * do the old way as it had a bunch of fugly worarounds for ECU editions 
	 * that couldn't handle bursts and needed to be artificially throttled.
	 */
	if(DATA_GET(message->data,"burst_write"))
	{
		buffer = (guint8 *)DATA_GET(message->data,"burst_buffer");
		if (!buffer)
		{
			MTXDBG(CRITICAL|SERIAL_WR,_("MS3 CRC32 burst blob is not stored in the OutputData->data structure, ABORTING\n"));
			EXIT();
			return FALSE;
		}
		burst_len = (GINT)DATA_GET(message->data,"burst_len");
		QUIET_MTXDBG(SERIAL_WR,_("Writing MS3 burst write %i bytes\n"),burst_len);
		res = write_wrapper(serial_params->fd,buffer,burst_len, &len);
		/* Send write command */
		if (!res)
		{
			MTXDBG((Dbg_Class)(SERIAL_WR|CRITICAL),_("Error writing MS3 burst block ERROR \"%s\"!!!\n"),err_text);
			retval = FALSE;
		}
	}

	else
	{
		g_return_val_if_fail(message,FALSE);
		g_return_val_if_fail(message->sequence,FALSE);
		g_return_val_if_fail(message->sequence->len > 0,FALSE);
		
		for (i=0;i<message->sequence->len;i++)
		{
			block = g_array_index(message->sequence,DBlock *,i);
			/*	printf("Block pulled\n");*/
			if (block->type == ACTION)
			{
				/*		printf("Block type of ACTION!\n");*/
				if (block->action == SLEEP)
				{
					/*			printf("Sleeping for %i usec\n", block->arg);*/
					MTXDBG(SERIAL_WR,_("Sleeping for %i microseconds \n"),block->arg);
					g_usleep((*factor)*block->arg);
				}
			}
			else if (block->type == DATA)
			{
				/*		printf("Block type of DATA!\n");*/
				if (block->len > 100)
					notifies = TRUE;
				/* ICK bad form man, writing one byte at a time,  due to ECU's that can't take full wire speed without
				   dropping chars due to uber tiny buffers */
				for (j=0;j<block->len;j++)
				{
					/*printf("comms.c data[%i] is %i, block len is %i\n",j,block->data[j],block->len);*/
					if ((notifies) && ((j % notif_divisor) == 0))
						thread_update_widget("info_label",MTX_LABEL,g_strdup_printf(_("<b>Sending %i of %i bytes</b>"),j,block->len));
					QUIET_MTXDBG(SERIAL_WR,_("Writing argument %i byte %i of %i, \"%.2X\"\n"),i,j+1,block->len,block->data[j]);
					res = write_wrapper(serial_params->fd,&(block->data[j]),1, &len);
					/* Send write command */
					if (!res)
					{
						MTXDBG((Dbg_Class)(SERIAL_WR|CRITICAL),_("Error writing block offset %i, value \"%.2X\" ERROR \"%s\"!!!\n"),j,block->data[j],err_text);
						retval = FALSE;
					}
					if (firmware->capabilities & MS2)
						g_usleep((*factor)*firmware->interchardelay*1000);
				}
			}
		}
	}
	if (notifies)
	{
		thread_update_widget("info_label",MTX_LABEL,g_strdup("<b>Transfer Completed</b>"));
		g_timeout_add(2000,(GSourceFunc)reset_infolabel_wrapper,NULL);
	}
	/* If sucessfull update ecu_data as well, this way, current 
	 * and pending match, in the case of a failed write, the 
	 * update_write_status() function will catch it and rollback as needed
	 */
	if ((output) && (retval))
	{
		if (mode == MTX_SIMPLE_WRITE)
			set_ecu_data(output->data,NULL);
		else if (mode == MTX_CHUNK_WRITE)
			store_new_block(output->data);
	}

	g_mutex_unlock(serio_mutex);
	g_mutex_unlock(&mutex);
	EXIT();
	return retval;
}

/*!
  \brief dump_output() dumps the newly read data to the console in HEX for
  debugging purposes
  \param total_read is the total bytes to print out
  \param buf is the pointer to data to write to console
  */
G_MODULE_EXPORT void dump_output(gint total_read, guchar *buf)
{
	guchar *p = NULL;
	gchar * tmpbuf = NULL;
	gint dbg_lvl = 0;
	dbg_lvl = (GINT)DATA_GET(global_data,"dbg_lvl");

	ENTER();

	p = buf;
	if (total_read > 0)
	{
		p = buf;
		if (dbg_lvl & SERIAL_RD)
		{
			MTXDBG(SERIAL_RD,_("Dumping output, enable IO_PROCESS debug to see the cmd's that were sent\n"));
			tmpbuf = g_strndup(((gchar *)buf),total_read);
			QUIET_MTXDBG(SERIAL_RD,_("Dumping Output string: \"%s\"\n"),tmpbuf);
			g_free(tmpbuf);
			QUIET_MTXDBG(SERIAL_RD,_("Data is in HEX!!\n"));
		}
		for (int j=0;j<total_read;j++)
		{
			QUIET_MTXDBG(SERIAL_RD,_("%.2X "), p[j]);
			if (!((j+1)%16))
				QUIET_MTXDBG(SERIAL_RD,_("\n"));
		}
		QUIET_MTXDBG(SERIAL_RD,_("\n\n"));
	}
	EXIT();
	return;
}


/*!
  \brief Wrapper function that does a nonblocking select()/read loop .
  \param fd is the serial port filedescriptor
  \param buf is the pointer to where to stick the data
  \param count is how many bytes to read
  \param len is the pointer to length read
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean read_wrapper(gint fd, guint8 * buf, size_t count, gint *len)
{
	gint res = 0;
	fd_set rd;
	struct timeval timeout;
	Serial_Params *serial_params = NULL;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	ENTER();

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
		{
			EXIT();
			return FALSE;
		}
		if (res > 0) /* Data Arrived! */
			*len = recv(fd,(void *)buf,count,0);
		EXIT();
		return TRUE;
	}
	else
		res = read(fd,buf,count);
	log_inbound_data(buf,res);

	if (res < 0)
	{
		EXIT();
		return FALSE;
	}
	else
		*len = res;
	EXIT();
	return TRUE;
}


/*!
  \brief wrapper for writing data that handles serial and network modes
  \param fd is the serial port filedescriptor
  \param buf is the pointer to where to pull the data from
  \param count is how many bytes to write
  \param len is the pointer to length to write
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean write_wrapper(gint fd, const guint8 *buf, size_t count, gint *len)
{
	gint res = 0;
	GError *error = NULL;
	Serial_Params *serial_params = NULL;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	ENTER();

	log_outbound_data(buf,count);

	/*      printf("write_wrapper\n"); */
	if (serial_params->net_mode)
	{
		/*              printf("net mode write\n"); */
#if GTK_MINOR_VERSION >= 18
		res = g_socket_send(serial_params->socket,(const gchar *)buf,(gsize)count,NULL,&error);
		if (res == -1)
		{
			MTXDBG(CRITICAL|SERIAL_WR,_("g_socket_send_error \"%s\"\n\n"),error->message);
			g_error_free(error);
		}
#else
		res = send(fd,buf,count,MSG_NOSIGNAL);
#endif
	}
	else
	{
		/*              printf("normal write %i bytes\n",count); */
		res = write(fd,buf,count);
		/*              printf("result of write is %i\n",res); */
	}
	if (len)
		*len = res;
	if (res < 0)
	{
		printf(_("Write error! \"%s\"\n"),strerror(errno));
		EXIT();
		return FALSE;
	}
	EXIT();
	return TRUE;
}

