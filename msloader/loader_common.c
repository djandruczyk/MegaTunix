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

/*!
  \file msloader/loader_common.c
  \ingroup Loader
  \brief Common loader code among all supported ECU's
  \author David Andruczyk
  */

#ifndef B115200
#define B115200 115200
#endif

#include <errno.h>
#include <fcntl.h>
#include <glib/gstdio.h>
#include <loader_common.h>
#include <hc08_loader.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __WIN32__
 #include <winserialio.h>
 #include <winsock2.h>
#else
 #include <sys/select.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <unistd.h>
 #include <termios.h>
#endif

#ifndef CRTSCTS
#define CRTSCTS 0
#endif
#define POLL_ATTEMPTS 4

/* Globals */
#ifndef __WIN32__
static struct termios oldtio;
static struct termios newtio;
#endif
static gchar * serial_lockfile = NULL;

gint open_port(gchar * port_name)
{
	gint fd = 0;
#ifdef __WIN32__
	fd = open(port_name, O_RDWR | O_BINARY );
#else
	/* Open Nonblocking */
	/*fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);*/
	fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
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
	/* These are ZERO becasue we are using select() */
	newtio.c_cc[VMIN]     = 0;     /* No min chars requirement */
	newtio.c_cc[VTIME]    = 0;     /* 0ms timeout */

	tcsetattr(fd,TCSANOW,&newtio);
#endif
	return 0;
}


void close_port(gint fd)
{
#ifdef __WIN32__
	WSACleanup();
#else
	tcsetattr(fd,TCSANOW,&oldtio);
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
		if ((g_ascii_strcasecmp(vector[i],"") == 0) || (g_ascii_strcasecmp(vector[i],"dev") == 0))
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


gint read_wrapper(gint fd, gchar *buf, gint requested)
{
#ifdef __WIN32__
	/* Windows can't do select() on anything but network sockets which
	   is completely braindead, and thus yoou must do overlapped IO
	   or uglier busylooped IO.  I'm lazy and hate windows and the 
	   overlapped IO stuff is ugly as hell, so I used the busylooped method
	   instead...
	   */
	gint received = 0;
        gint read_pos = 0;
	gboolean timeout = FALSE;
	gint tries = 0;
	gint wanted = requested;
	gint total = 0;

	while (!timeout)
	{
		tries++;
		read_pos = requested - wanted;
		received = read(fd, &buf[read_pos], wanted);
		printf("Try %i, requested %i, got %i\n",tries,wanted,received);
		g_usleep(10000); /* 10ms rest */
		if (received == -1)
		{
			output((gchar *)"Serial I/O Error, read failure\n",FALSE);
			return -1;
		}
		total += received;
		wanted -= received;
		if (tries > requested)
			timeout = TRUE;
		if (total == requested)
		{
			printf("got what was requested, returning\n");
			return total;
		}
	}
	printf("timeout, returning only %i of %i bytes\n",total,requested);
	return total;
	   
#else	/* Linux/OS-X where sane I/O lives */
	fd_set readfds;
	fd_set errfds;
	struct timeval t;
	gint attempts = 0;
	gint read_pos = 0;
	gint wanted = requested;
	gint received = 0;
	gint res = 0;
	gint total = 0;

	while (wanted > 0)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&errfds);
		t.tv_sec = 1;
		t.tv_usec = 0;
		FD_SET(fd,&readfds);
		FD_SET(fd,&errfds);
		res = select (fd+1, &readfds,NULL,&errfds, &t);
		if (res == -1)
		{
			output((gchar *)"ERROR, select() failure!\n",FALSE);
			return -1;
		}
		if (res == 0) /* Timeout */
		{
			output(g_strdup_printf("select() (read) timeout, attempts %i!\n",attempts),TRUE);
			attempts++;
		}
		/* OK we have an error condition waiting for us, read it */
		if (FD_ISSET(fd,&errfds))
		{
			output((gchar *)"select() (read) ERROR !\n",FALSE);
			attempts++;
		}
		if (attempts > POLL_ATTEMPTS)
			return total;
		/* OK we have something waiting for us, read it */
		if (FD_ISSET(fd,&readfds))
		{
			/*printf("data avail!\n");*/
			read_pos = requested - wanted;
			received = read(fd, &buf[read_pos], wanted);
			if (received <= 0)
			{
				output((gchar *)"Serial I/O Error, read failure\n",FALSE);
				return -1;
			}
			total += received;
			/*printf("got %i bytes\n",received);*/
			wanted -= received;
		}
	}
	return total;
#endif
}


gint write_wrapper(gint fd, guchar *buf, gint total)
{
#ifdef __WIN32__
	return write(fd,buf,total);
#else	/* Linux/OS-X where sane IO lives */
	fd_set writefds;
	fd_set errfds;
	struct timeval t;
	gint attempts = 0;
	gint write_pos = 0;
	gint to_send = total;
	gint received = 0;
	gint res = 0;
	gint sent = 0;
	gint count = 0;

	while (to_send > 0)
	{
		FD_ZERO(&writefds);
		FD_ZERO(&errfds);
		t.tv_sec = 0;
		t.tv_usec = 200000;
		FD_SET(fd,&writefds);
		FD_SET(fd,&errfds);
		res = select (fd+1, NULL,&writefds,&errfds, &t);
		if (res == -1)
		{
			output((gchar *)"ERROR, select() failure!\n",FALSE);
			return -1;
		}
		if (res == 0) /* Timeout */
		{
			/*printf("select() (write) timeout!\n");*/
			attempts++;
		}
		/* Error condition set? */
		if (FD_ISSET(fd,&errfds))
		{
			output((gchar *)"select() (write) ERROR !\n",FALSE);
			attempts++;
		}
		if (attempts > POLL_ATTEMPTS)
			return sent;
		/* OK we can now write, write it */
		if (FD_ISSET(fd,&writefds))
		{
			/*printf("data avail!\n");*/
			write_pos = total - to_send;
			count = write(fd, &buf[write_pos], to_send);
			if (count <= 0)
			{
				output((gchar *)"Serial I/O Error, write failure\n",FALSE);
				return -1;
			}
			sent += count;
			/*printf("got %i bytes\n",received);*/
			to_send -= count;
		}
	}
	return sent;
#endif
}

