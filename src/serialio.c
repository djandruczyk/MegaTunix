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

#ifndef CRTSCTS
#define CRTSCTS 0
#endif
#ifndef B115200
#define B115200 115200
#endif

#include <comms_gui.h>
#include <config.h>
#include <comms.h>
#include <defines.h>
#include <debugging.h>
#include <errno.h>
#include <fcntl.h>
#include <listmgmt.h>
#include <locking.h>
#include <notifications.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <string.h>
#ifndef __WIN32__
 #include <termios.h>
 #ifdef __PIS_SUPPORT__
  #include <linux/serial.h>
  #include <sys/ioctl.h>
 #endif
#endif
#include <threads.h>
#include <unistd.h>
#ifdef __WIN32__
 #include <winserialio.h>
#endif


gboolean port_open = FALSE;
extern gconstpointer *global_data;

/*!
 \brief open_serial() called to open the serial port, updates textviews on the
 comms page on success/failure
 \param port_name (gchar *) name of the port to open
 */
G_MODULE_EXPORT gboolean open_serial(gchar * port_name)
{
	/* We are using DOS/Win32 style com port numbers instead of unix
	 * style as its easier to think of COM1 instead of /dev/ttyS0
	 * thus com1=/dev/ttyS0, com2=/dev/ttyS1 and so on 
	 */
	static GMutex *serio_mutex = NULL;
	Serial_Params *serial_params = NULL;
	gint fd = -1;
	gchar * err_text = NULL;
	serial_params = DATA_GET(global_data,"serial_params");
	serio_mutex = DATA_GET(global_data,"serio_mutex");

	g_mutex_lock(serio_mutex);
	/*printf("Opening serial port %s\n",port_name);*/
	/* Open Read/Write and NOT as the controlling TTY */
	/* Blocking mode... */
#ifdef __WIN32__
	fd = open(port_name, O_RDWR | O_BINARY );
#else
	fd = open(port_name, O_RDWR | O_NOCTTY );
#endif
	if (fd > 0)
	{
		/* SUCCESS */
		/* NO Errors occurred opening the port */
		serial_params->port_name = g_strdup(port_name); 
		serial_params->open = TRUE;
		port_open = TRUE;
		serial_params->fd = fd;
		dbg_func(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" open_serial()\n\t%s Opened Successfully\n",port_name));
		thread_update_logbar("comms_view",NULL,g_strdup_printf(_("%s Opened Successfully\n"),port_name),FALSE,FALSE);

	}
	else
	{
		/* FAILURE */
		/* An Error occurred opening the port */
		port_open = FALSE;
		if (serial_params->port_name)
			g_free(serial_params->port_name);
		serial_params->port_name = NULL;
		serial_params->open = FALSE;
		serial_params->fd = -1;
		err_text = (gchar *)g_strerror(errno);
		/*printf("Error Opening \"%s\", Error Code: \"%s\"\n",port_name,g_strdup(err_text));*/
		dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL,g_strdup_printf(__FILE__": open_serial()\n\tError Opening \"%s\", Error Code: \"%s\"\n",port_name,err_text));
		thread_update_widget("titlebar",MTX_TITLE,g_strdup_printf(_("Error Opening \"%s\", Error Code: \"%s\""),port_name,err_text));

		thread_update_logbar("comms_view","warning",g_strdup_printf(_("Error Opening \"%s\", Error Code: %s \n"),port_name,err_text),FALSE,FALSE);
	}

	/*printf("open_serial returning\n");*/
	g_mutex_unlock(serio_mutex);
	return port_open;
}
	

/*!
 \brief flush_serial() is called whenever we want to flush the I/O port of any
 pending data. It's a wrapper to the tcflush command on unix and the 
 win32_serial_flush command in winserialio.c that does the equivalent 
 operation on windows.
 \param fd (gint) filedescriptor to flush
 \param type (gint) how to flush it (enumeration)
 */
G_MODULE_EXPORT void flush_serial(gint fd, FlushDirection type)
{
	GMutex *serio_mutex = NULL;
	Serial_Params *serial_params = NULL;
	serial_params = DATA_GET(global_data,"serial_params");
	serio_mutex = DATA_GET(global_data,"serio_mutex");
	if (serial_params->net_mode)
		return;

	g_mutex_lock(serio_mutex);
#ifdef __WIN32__
	if (fd)
		win32_flush_serial(fd, type);
#else
	if ((serial_params) && (serial_params->fd))
	{
		switch (type)
		{
			case INBOUND:
				tcflush(serial_params->fd, TCIFLUSH);
				break;
			case OUTBOUND:
				tcflush(serial_params->fd, TCOFLUSH);
				break;
			case BOTH:
				tcflush(serial_params->fd, TCIOFLUSH);
				break;
		}
	}
#endif	
	g_mutex_unlock(serio_mutex);
}


/*!
 \brief setup_serial_params() is another wrapper that calls the appropriate
 calls to initialize the serial port to the proper speed, bits, flow, parity
 etc..
 */
G_MODULE_EXPORT void setup_serial_params(void)
{
	GMutex *serio_mutex = NULL;
	gint baudrate = 0;	
	Serial_Params *serial_params = NULL;
	serial_params = DATA_GET(global_data,"serial_params");
	serio_mutex = DATA_GET(global_data,"serio_mutex");
	baudrate = (GINT)DATA_GET(global_data,"ecu_baud");
#ifndef __WIN32__
	speed_t baud = B9600;
#endif
	if (serial_params->open == FALSE)
		return;
	/*printf("setup_serial_params entered\n");*/
	g_mutex_lock(serio_mutex);
#ifdef __WIN32__
	win32_setup_serial_params(serial_params->fd, baudrate);
#else
	/* Save serial port status */
	tcgetattr(serial_params->fd,&serial_params->oldtio);

	g_mutex_unlock(serio_mutex);
	flush_serial(serial_params->fd, TCIOFLUSH);

	g_mutex_lock(serio_mutex);

	/* Sets up serial port for the modes we want to use. 
	 * NOTE: Original serial tio params are stored and restored 
	 * in the open_serial() and close_serial() functions.
	 */ 

	/*clear struct for new settings*/
	memset(&serial_params->newtio, 0, sizeof(serial_params->newtio)); 
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

	if (baudrate == 9600)
		baud = B9600;
	else if (baudrate == 115200)
		baud = B115200;
	else if (baudrate == 8192)
		baud = B38400;

	cfsetspeed(&serial_params->newtio, baud);

	/* Mask and set to 8N1 mode... */
	serial_params->newtio.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
	/* Set additional flags, note |= syntax.. */
	/* Enable receiver, ignore modem ctrls lines, use 8 bits */
	serial_params->newtio.c_cflag |= CLOCAL | CREAD | CS8;

	/* RAW Input */
	/* Ignore signals, enable canonical, etc */
	serial_params->newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* Disable software flow control */
	serial_params->newtio.c_iflag &= ~(IXON | IXOFF );

	/* Set raw output */
	serial_params->newtio.c_oflag &= ~OPOST;

	/* 
	   initialize all control characters 
	   default values can be found in /usr/include/termios.h, and are given
	   in the comments, but we don't need them here
	 */
	serial_params->newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	serial_params->newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	serial_params->newtio.c_cc[VERASE]   = 0;     /* del */
	serial_params->newtio.c_cc[VKILL]    = 0;     /* @ */
	serial_params->newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
	serial_params->newtio.c_cc[VEOL]     = 0;     /* '\0' */
	serial_params->newtio.c_cc[VMIN]     = 0;     
	serial_params->newtio.c_cc[VTIME]    = 1;     /* 100ms timeout */

#ifdef __PIS_SUPPORT__
	if (ioctl(serial_params->fd, TIOCGSERIAL, &serial_params->oldctl) != 0)
		dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL, g_strdup_printf(__FILE__": setup_serial_params()\tError getting ioctl\n"));
	else
	{
		dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL, g_strdup_printf(__FILE__": setup_serial_params()\tget ioctl OK\n"));
		// copy ioctl structure
		memcpy(&serial_params->newctl, &serial_params->oldctl, sizeof(serial_params->newctl));

		// and if we are PIS 8192 make a custom divisor
		if (baudrate == 8192)
		{
			if (serial_params->newctl.baud_base < 115200)
				serial_params->newctl.baud_base = 115200;

			serial_params->newctl.custom_divisor = 14;
			serial_params->newctl.flags |= (ASYNC_SPD_MASK & ASYNC_SPD_CUST);

			if (ioctl(serial_params->fd, TIOCSSERIAL, &serial_params->newctl) != 0)
				dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL, g_strdup_printf(__FILE__": setup_serial_params()\tError setting ioctl\n"));
			else
				dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL, g_strdup_printf(__FILE__": setup_serial_params()\tset ioctl OK\n"));
		}
	}

#endif

	tcsetattr(serial_params->fd, TCSAFLUSH, &serial_params->newtio);

#endif
	g_mutex_unlock(serio_mutex);
	return;
}


/*!
 \brief close_serial() closes the serial port, and sets several gui widgets
 to reflect the port closing (textview/connected indicator)
 */
G_MODULE_EXPORT void close_serial(void)
{
	GMutex *serio_mutex = NULL;
	Serial_Params *serial_params = NULL;
	serial_params = DATA_GET(global_data,"serial_params");
	serio_mutex = DATA_GET(global_data,"serio_mutex");
	if (!serial_params)
		return;
	if (serial_params->open == FALSE)
		return;

	g_mutex_lock(serio_mutex);

	/*printf("Closing serial port\n");*/
#ifndef __WIN32__
 #ifdef __PIS_SUPPORT
	if (ioctl(serial_params->fd, TIOCSSERIAL, &serial_params->oldctl) != 0)
		dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL, g_strdup_printf(__FILE__": close_serial()\tError restoring ioctl\n"));
	else
		dbg_func(SERIAL_RD|SERIAL_WR|CRITICAL, g_strdup_printf(__FILE__": close_serial()\tioctl restored OK\n"));

 #endif
	tcsetattr(serial_params->fd,TCSAFLUSH,&serial_params->oldtio);
#endif
	close(serial_params->fd);
	serial_params->fd = -1;
	serial_params->open = FALSE;
	if (serial_params->port_name)
		g_free(serial_params->port_name);
	serial_params->port_name = NULL;
	DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	port_open = FALSE;

	/* An Closing the comm port */
	dbg_func(SERIAL_RD|SERIAL_WR,g_strdup(__FILE__": close_serial()\n\tSerial Port Closed\n"));
	if (!DATA_GET(global_data,"leaving"))
		thread_update_logbar("comms_view",NULL,g_strdup_printf(_("Serial Port Closed\n")),FALSE,FALSE);
	g_mutex_unlock(serio_mutex);
	return;
}


