/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef B115200
#define B115200 115200
#endif

#include <config.h>
#include <defines.h>
#include <errno.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <getfiles.h>
#include <loader_common.h>
#include <ms1_loader.h>
#include <serialio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __WIN32__
#include <winserialio.h>
#else
#include <termios.h>
#endif

#ifndef CRTSCTS
#define CRTSCTS 0
#endif


/* Globals */
#ifndef __WIN32__
struct termios oldtio;
struct termios newtio;
#endif
static gchar * serial_lockfile = NULL;

gint open_port(gchar * port_name)
{
	gint fd = 0;
#ifdef __WIN32__
	fd = open(port_name, O_RDWR | O_BINARY );
#else
	fd = open(port_name, O_RDWR | O_NOCTTY );
#endif
	return fd;
}

gint setup_port(gint fd, gint baud)
{
#ifdef __WIN32__

	win32_setup_serial_params(fd, baud, 8, NONE, 1);
#else
	gint _baud = 0;

	/* Save serial port status */
	tcgetattr(fd,&oldtio);
	flush_serial(fd, BOTH);

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
	newtio.c_cflag &= ~( PARENB | CSTOPB | CSIZE);
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
	 initialize all control characters 
	 default values can be found in /usr/include/termios.h, and are given
	 in the comments, but we don't need them here
	 *                                           */
	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VMIN]     = 0;
	newtio.c_cc[VTIME]    = 2;     /* 100ms timeout */
	/* MS2 quirk */

	tcsetattr(fd,TCSANOW,&newtio);

#endif
	return 0;

}


void close_port(gint fd)
{
#ifndef __WIN32__
	tcsetattr(fd,TCSAFLUSH,&oldtio);
	flush_serial(fd,BOTH);
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

gboolean get_ecu_signature(gint fd)
{
	gint res = 0;
	gint size = 128;
	guchar buf[128];
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

	flush_serial(fd,BOTH);
sig_check:
	if (attempt > 1)
		return FALSE;
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
			return FALSE;
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
			return TRUE;
		}
	}
	return FALSE;
}


FirmwareType detect_firmware(gchar * filename)
{
	GIOChannel *chan = NULL;
	GError *error = NULL;
	gchar * buf = NULL;
	gsize len = 0;
	FirmwareType type = MS1;

	if (NULL != g_strrstr(filename,"bootstrap.s19"))
		return MS2;

	chan = g_io_channel_new_file(filename,"r",&error);
	while (G_IO_STATUS_NORMAL == g_io_channel_read_line(chan, &buf,&len,NULL,&error))
	{
		if (g_strrstr(buf,"S2"))
		{	
			type = MS2;
			g_free(buf);
			g_io_channel_shutdown(chan,FALSE,&error);
			return type;
		}
		g_free(buf);
	}
	g_io_channel_shutdown(chan, FALSE,&error);
	return type;
}


void unlock_port()
{
#ifndef __WIN32__
/*	printf("told to unlock serial,  path \"%s\"\n",fname); */
	if (serial_lockfile)
	{
		if (g_file_test(serial_lockfile,G_FILE_TEST_IS_REGULAR))
		{
			g_remove(serial_lockfile);
			g_free(serial_lockfile);
			serial_lockfile = NULL;
		}
	}
#endif
}


gboolean lock_port(gchar * name)
{
#ifndef __WIN32__
	gchar *tmpbuf = NULL;
	gchar *lock = NULL;
	gchar **vector = NULL;
	gchar *contents = NULL;
	GError *err = NULL;
	guint i = 0;
	gint pid = 0;

	/* If no /proc (i.e. os-X), just fake it and return */
	if (!g_file_test("/proc",G_FILE_TEST_IS_DIR))
		return TRUE;

	lock = g_strdup_printf("/var/lock/LCK..");
	vector = g_strsplit(name,PSEP,-1);
	for (i=0;i<g_strv_length(vector);i++)
	{
		if ((g_strcasecmp(vector[i],"") == 0) || (g_strcasecmp(vector[i],"dev") == 0))
			continue;
		lock = g_strconcat(lock,vector[i],NULL);
	}
	g_strfreev(vector);
	if (g_file_test(lock,G_FILE_TEST_IS_REGULAR))
	{
/*		printf("found existing lock!\n");*/
		if(g_file_get_contents(lock,&contents,NULL,&err))
		{
/*			printf("read existing lock\n");*/
			vector = g_strsplit(g_strchug(contents)," ", -1);
/*			printf("lock had %i fields\n",g_strv_length(vector));*/
			pid = (gint)g_ascii_strtoull(vector[0],NULL,10);
/*			printf("pid in lock \"%i\"\n",pid);*/
			g_free(contents);
			g_strfreev(vector);
			tmpbuf = g_strdup_printf("/proc/%i",pid);
			if (g_file_test(tmpbuf,G_FILE_TEST_IS_DIR))
			{
/*				printf("process active\n");*/
				g_free(tmpbuf);
				g_free(lock);
				return FALSE;
			}
			else
				g_remove(lock);
			g_free(tmpbuf);
		}
		
	}
	contents = g_strdup_printf("     %i",getpid());
	if(g_file_set_contents(lock,contents,-1,&err))
	{
		serial_lockfile = g_strdup(lock);
		g_free(contents);
		g_free(lock);
		return TRUE;
	}
	else
		printf("Error setting serial lock %s\n",(gchar *)strerror(errno));
	g_free(contents);
#endif	
	return TRUE;
}
