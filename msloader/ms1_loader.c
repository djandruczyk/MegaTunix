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
#include <defines.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <getfiles.h>
#include <ms1_loader.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef __WIN32__
 #include <termios.h>
#else
#include <winserialio.h>
#endif

#ifndef CRTSCTS
#define CRTSCTS 0
#endif


/* Globals */
#ifndef __WIN32__
struct termios oldtio;
struct termios newtio;
#endif

gint setup_port(gchar * port_name, gint baud)
{
	gint fd = 0;
	gint _baud = 0;
#ifdef __WIN32__
	fd = open(port_name, O_RDWR | O_BINARY );
#else
	fd = open(port_name, O_RDWR | O_NOCTTY);
#endif
	if (fd > 0)
	{

#ifdef __WIN32__

		win32_setup_serial_params(fd, baud);
#else

		/* Save serial port status */
		tcgetattr(fd,&oldtio);
		flush_serial(fd, TCIOFLUSH);

		memset(&newtio, 0, sizeof(newtio));
		/* 
		 * BAUDRATE: Set bps rate. You could also use cfsetispeed and 
		 * cfsetospeed
		 * CRTSCTS : output hardware flow control (only used if the cable has
		 * all necessary lines. See sect. 7 of Serial-HOWTO)
		 * CS8     : 8n1 (8bit,no parity,1 stopbit)
		 * CLOCAL  : local connection, no modem contol
		 * CREAD   : enable receiving characters
		 */

		/* Set baud (posix way) */

		if (baud == 9600)
			_baud = B9600;
		else if (baud == 115200)
			_baud = B115200;
		else
			printf("INVALID BAUD RATE %i\n",baud);
		cfsetispeed(&newtio, _baud);
		cfsetospeed(&newtio, _baud);

		/* Mask and set to 8N1 mode... */
		newtio.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
		/* Set additional flags, note |= syntax.. */
		/* Enable receiver, ignore modem ctrls lines, use 8 bits */
		newtio.c_cflag |= CLOCAL | CREAD | CS8;

		/* RAW Input */
		/* Ignore signals, enable canonical, etc */
		newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

		/* Disable software flow control */
		newtio.c_iflag &= ~(IXON | IXOFF );

		/* Set raw output */
		newtio.c_oflag &= ~OPOST;
		/* 
		 *            initialize all control characters 
		 *                       default values can be found in /usr/include/termios.h, and are given
		 *                                  in the comments, but we don't need them here
		 *                                           */
		newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
		newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
		newtio.c_cc[VERASE]   = 0;     /* del */
		newtio.c_cc[VKILL]    = 0;     /* @ */
		newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
		newtio.c_cc[VEOL]     = 0;     /* '\0' */
		newtio.c_cc[VMIN]     = 0;
		newtio.c_cc[VTIME]    = 1;     /* 100ms timeout */

		tcsetattr(fd,TCSAFLUSH,&newtio);

#endif
	}
	return fd;

}


void close_port(gint fd)
{
#ifndef __WIN32__
	tcsetattr(fd,TCSAFLUSH,&oldtio);
#endif
	close(fd);
	return;
}


void flush_serial(gint fd, FlushDirection type)
{
#ifdef __WIN32__
	if (fd)
		win32_flush_serial(fd, type); 
#else
	if (fd)
	{
		switch (type)
		{
			case INBOUND:
				tcflush(fd, TCIFLUSH);
				break;
			case OUTBOUND:
				tcflush(fd, TCOFLUSH);
				break;
			case BOTH:
				tcflush(fd, TCIOFLUSH);
				break;
		}
	}
#endif
}

EcuState detect_ecu(gint fd)
{
	gint res = 0;
	gint size = 1024;
	guchar buf[1024];
	guchar *ptr = buf;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gchar  *message = NULL;

	/* Probe for response 
	 * First check for signature (running state)
	 * If that fails, see if we are in bootloader mode already
	 */

	res = write (fd,"S",1);
	flush_serial(fd,BOTH);
	if (res != 1)
		output("Failure sending signature request!\n",FALSE);
	g_usleep(300000); /* 300ms timeout */
	total_read = 0;
	total_wanted = size;
	zerocount = 0;
	while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
	{
		total_read += res = read(fd,
				ptr+total_read,
				total_wanted-total_read);

		/* If we get nothing back (i.e. timeout, assume done)*/
		if (res <= 0)
			zerocount++;

		if (zerocount > 1)
			break;
	}
	if (total_read > 0)
	{
		message = g_strndup(((gchar *)buf),total_read);
		/* Check for "what" or "Boot" */
		if (g_strrstr_len(message,total_read, "what"))
		{
			g_free(message);
			return IN_BOOTLOADER;
		}
		else if (g_strrstr_len(message,total_read, "Boot"))
		{
			g_free(message);
			return IN_BOOTLOADER;
		}
		else	
		{
			output(g_strdup_printf("ECU signature: \"%s\"\n",message),TRUE);
			g_free(message);
			return  LIVE_MODE;
		}
	}

	return NOT_LISTENING;
}


void get_ecu_signature(gint fd)
{
	gint res = 0;
	gint size = 1024;
	guchar buf[1024];
	guchar *ptr = buf;
	gint total_read = 0;
	gint total_wanted = 0;
	gint zerocount = 0;
	gchar  *message = NULL;
	gint attempt = 0;

	/* Probe for response 
	 * First check for signature (running state)
	 * If that fails, see if we are in bootloader mode already
	 */

sig_check:
	if (attempt > 2)
	{
		output("Max attempts reached, sorry\n",FALSE);
		return;
	}
	res = write (fd,"S",1);
	flush_serial(fd,OUTBOUND);
	if (res != 1)
		output("Failure sending signature request!\n",FALSE);
	g_usleep(300000); /* 300ms timeout */
	total_read = 0;
	total_wanted = size;
	zerocount = 0;
	while ((total_read < total_wanted ) && (total_wanted-total_read) > 0 )
	{
		total_read += res = read(fd,
				ptr+total_read,
				total_wanted-total_read);

		/* If we get nothing back (i.e. timeout, assume done)*/
		if (res <= 0)
			zerocount++;

		if (zerocount > 1)
			break;
	}
	if (total_read > 0)
	{
		message = g_strndup(((gchar *)buf),total_read);
		/* Check for "what" or "Boot" */
		if (g_strrstr_len(message,total_read, "Vector"))
		{
			g_free(message);
			output("ECU has corrupted firmware!\n",FALSE);
			return;
		}
		else if (g_strrstr_len(message,total_read, "what"))
		{
			g_free(message);
			attempt++;
			output("ECU is in bootloader mode, attempting reboot\n",FALSE);
			res = write (fd,"X",1);
			flush_serial(fd,BOTH);
			g_usleep(500000);
			goto sig_check;
		}
		else if (g_strrstr_len(message,total_read, "Boot"))
		{
			g_free(message);
			output("ECU is in bootloader mode, attempting reboot\n",FALSE);
			attempt++;
			res = write (fd,"X",1);
			flush_serial(fd,BOTH);
			g_usleep(500000);
			goto sig_check;
		}
		else	
		{
			output(g_strdup_printf("Detected signature: \"%s\"\n",message),TRUE);
			g_free(message);
			return;
		}
	}
	return;
}


gboolean jump_to_bootloader(gint fd)
{
	gint res = 0;

	flush_serial(fd,OUTBOUND);
	res = write (fd,"!!",2);
	g_usleep(100000); /* 100ms timeout  */
	if (res != 2)
	{
		output("Error trying to get \"Boot>\" Prompt,\n",FALSE);
		return FALSE;
	}

	return TRUE;
}


gboolean prepare_for_upload(gint fd)
{
	gint res = 0;
	gchar buf[1024];
	gchar * message = NULL;

	res = write(fd,"W",1);
	if (res != 1)
	{
		output("Error trying to initiate ECU wipe\n",FALSE);
		return FALSE;
	}
	flush_serial(fd,OUTBOUND);
	g_usleep(1000000); /* 1000ms timeout for flash to erase */
	res = read(fd,&buf,1024);
	message = g_strndup(((gchar *)buf),res);
	if (g_strrstr_len(buf,res,"Complete"))
	{
		g_free(message);
		output("ECU Wipe complete\n",FALSE);
		res = write(fd,"U",1);
		if (res != 1)
		{
			output("Error trying to initiate ECU upgrade\n",FALSE);
			return FALSE;
		}
		flush_serial(fd,OUTBOUND);
		g_usleep(2000000); /* 2000ms timeout for flash to erase */
		res = read(fd,&buf,1024);
		if (g_strrstr_len(buf,res,"waiting"))
		{
			output("Ready to update ECU firmware\n",FALSE);
			return TRUE;
		}
		else
		{
			message = g_strndup(buf,res);
			output(g_strdup_printf("ECU returned \"%s\"\n",message),TRUE);	
			g_free(message);
			output("Error getting \"ready to update\" message from ECU\n",FALSE);
			return FALSE;
		}
	}
	else
	{
		output(g_strdup_printf("Error wiping ECU, result \"%s\"\n",message),TRUE);
		g_free(message);
		return FALSE;
	}
}


void upload_firmware(gint fd, gint file_fd)
{
	gint res = 0;
	gchar buf[128];
	gint chunk = 128;
	gint i = 0;
	GTimeVal last;
	GTimeVal now;
	GTimeVal begin;
	gfloat elapsed = 0.0;
	gint rate = 0;

	g_get_current_time(&begin);
	g_get_current_time(&now);
	res = read(file_fd,buf,chunk);
	while (res > 0)
	{
		last = now;
		g_get_current_time(&now);
		i+=res;
		write (fd,buf,chunk);
		elapsed = now.tv_usec - last.tv_usec;
		if (elapsed < 0)
			elapsed += 1000000;
		rate = (chunk*1000000)/elapsed;
		output(g_strdup_printf("%6i bytes written, %i bytes/sec.\n",i,rate),TRUE);
		res = read(file_fd,buf,chunk);
	}
	flush_serial(fd,BOTH);
	g_get_current_time(&now);
	output(g_strdup_printf("Upload completed in %li Seconds\n",now.tv_sec-begin.tv_sec),TRUE);

	return;
}


void reboot_ecu(gint fd)
{
	gint res = 0;

	output("Sleeping 3 Seconds\n",FALSE);
	g_usleep(3000000);
	res = write (fd,"X",1);
	flush_serial(fd,OUTBOUND);
	if (res != 1)
		output("Error trying to Reboot ECU\n",FALSE);

	return ;
}


