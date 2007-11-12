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

#include <comms_gui.h>
#include <config.h>
#include <comms.h>
#include <dataio.h>
#include <defines.h>
#include <debugging.h>
#include <errno.h>
#include <fcntl.h>
#include <notifications.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <string.h>
#include <structures.h>
#include <termios.h>
#include <threads.h>
#include <unistd.h>
#ifdef __WIN32__
 #include <winserialio.h>
#endif


Serial_Params *serial_params;
gboolean connected = FALSE;
gboolean port_open = FALSE;
GStaticMutex serio_mutex = G_STATIC_MUTEX_INIT;
GAsyncQueue *serial_repair_queue = NULL;
extern gint dbg_lvl;

/*!
 \brief open_serial() called to open the serial port, updates textviews on the
 comms page on success/failure
 \param port_name (gchar *) name of the port to open
 */
gboolean open_serial(gchar * port_name)
{
	/* We are using DOS/Win32 style com port numbers instead of unix
	 * style as its easier to think of COM1 instead of /dev/ttyS0
	 * thus com1=/dev/ttyS0, com2=/dev/ttyS1 and so on 
	 */
	gint fd = -1;
	gchar *device = NULL;	/* temporary unix name of the serial port */
	gchar * err_text = NULL;

	g_static_mutex_lock(&serio_mutex);
	//printf("Opening serial port %s\n",port_name);
	device = g_strdup(port_name);
	/* Open Read/Write and NOT as the controlling TTY */
	/* Blocking mode... */
#ifdef __WIN32__
	fd = open(device, O_RDWR | O_NOCTTY | O_BINARY );
#else
	fd = open(device, O_RDWR | O_NOCTTY);
#endif
	if (fd > 0)
	{
		/* SUCCESS */
		/* NO Errors occurred opening the port */
		serial_params->port_name = g_strdup(port_name); 
		serial_params->open = TRUE;
		port_open = TRUE;
		serial_params->fd = fd;
		if (dbg_lvl & (SERIAL_RD|SERIAL_WR))
			dbg_func(g_strdup_printf(__FILE__" open_serial()\n\t%s Opened Successfully\n",device));
		thread_update_logbar("comms_view",NULL,g_strdup_printf("%s Opened Successfully\n",device),TRUE,FALSE);
		thread_update_widget(g_strdup("comms_serial_port_entry"),MTX_ENTRY,g_strdup(port_name));
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
		//printf("Error Opening \"%s\", Error Code: \"%s\"\n",device,g_strdup(err_text));
		if (dbg_lvl & (SERIAL_RD|SERIAL_WR|CRITICAL))
			dbg_func(g_strdup_printf(__FILE__": open_serial()\n\tError Opening \"%s\", Error Code: \"%s\"\n",device,err_text));
		thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup_printf("Error Opening \"%s\", Error Code: \"%s\"\n",device,err_text));

		thread_update_logbar("comms_view","warning",g_strdup_printf("Error Opening \"%s\", Error Code: %s \n",device,err_text),TRUE,FALSE);
	}

	g_free(device);
	//printf("open_serial returning\n");
	g_static_mutex_unlock(&serio_mutex);
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
void flush_serial(gint fd, gint type)
{
	g_static_mutex_lock(&serio_mutex);
#ifdef __WIN32__
	if (fd)
		win32_flush_serial(fd, type);
#else
	if ((serial_params) && (serial_params->fd))
		tcflush(serial_params->fd, type);
#endif	
	g_static_mutex_unlock(&serio_mutex);
}


/*!
 \brief toggle_serial_control_lines() is another wrapper that calls the 
 appropriate calls to toggle the hardware control lines.  This is an 
 experimental attempt to see if it resolves a serial over bluetooth connection
 loss problem
 */
/*
void toggle_serial_control_lines()
{
#ifdef __WIN32__
	win32_toggle_serial_control_lines();
#else
	struct termios oldtio;
	struct termios temptio;

	// Save current port settings //
	tcgetattr(serial_params->fd,&oldtio);
	memcpy(&temptio, &oldtio, sizeof(struct termios)); 

	serial_params->newtio.c_cflag &= ~(CLOCAL);
	serial_params->newtio.c_cflag |= (CRTSCTS);
	tcsetattr(serial_params->fd,TCSAFLUSH,&temptio);
	g_usleep (100000); // Wait 100 ms //
	//Set back
	tcsetattr(serial_params->fd,TCSAFLUSH,&oldtio);
#endif
}
*/

/*!
 \brief setup_serial_params() is another wrapper that calls the appropriate
 calls to initialize the serial port to the proper speed, bits, flow, parity
 etc..
 */
void setup_serial_params(gint baudrate)
{
	guint baud = 0;
	if (serial_params->open == FALSE)
		return;
	//printf("setup_serial_params entered\n");
	g_static_mutex_lock(&serio_mutex);
#ifdef __WIN32__
	win32_setup_serial_params(baudrate);
#else
	/* Save serial port status */
	tcgetattr(serial_params->fd,&serial_params->oldtio);

	g_static_mutex_unlock(&serio_mutex);
	flush_serial(serial_params->fd, TCIOFLUSH);

	g_static_mutex_lock(&serio_mutex);

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
	cfsetispeed(&serial_params->newtio, baud);
	cfsetospeed(&serial_params->newtio, baud);

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
	//serial_params->newtio.c_cc[VTIME]    = 1;     /* 100ms timeout */
	serial_params->newtio.c_cc[VTIME]    = 1;     /* 100ms timeout */

	tcsetattr(serial_params->fd,TCSAFLUSH,&serial_params->newtio);

#endif
	g_static_mutex_unlock(&serio_mutex);
	return;
}


/*!
 \brief close_serial() closes the serial port, and sets several gui widgets
 to reflect the port closing (textview/connected indicator)
 */
void close_serial()
{
	g_static_mutex_lock(&serio_mutex);
	if (!serial_params)
	{
		g_static_mutex_unlock(&serio_mutex);
		return;
	}
	if (serial_params->open == FALSE)
	{
		g_static_mutex_unlock(&serio_mutex);
		return;
	}

	//printf("Closing serial port\n");
#ifndef __WIN32__
	tcsetattr(serial_params->fd,TCSAFLUSH,&serial_params->oldtio);
#endif
	close(serial_params->fd);
	serial_params->fd = -1;
	serial_params->open = FALSE;
	if (serial_params->port_name)
		g_free(serial_params->port_name);
	serial_params->port_name = NULL;
	connected = FALSE;
	port_open = FALSE;

	/* An Closing the comm port */
	if (dbg_lvl & (SERIAL_RD|SERIAL_WR))
		dbg_func(g_strdup(__FILE__": close_serial()\n\tCOM Port Closed\n"));
	thread_update_logbar("comms_view",NULL,g_strdup_printf("COM Port Closed\n"),TRUE,FALSE);
	g_static_mutex_unlock(&serio_mutex);
	return;
}


void *serial_repair_thread(gpointer data)
{
	/* We got sent here because of one of the following occurred:
	 * Serial port isn't opened yet (app just fired up)
	 * Serial I/O errors (missing data, or failures reading/writing)
	 *  - This includes things like pulling hte RS232 cable out of the ECU
	 * Serial port disappeared (i.e. device hot unplugged)
	 *  - This includes unplugging the USB side of a USB->Serial adapter
	 *    or going out of bluetooth range, for a BT serial device
	 *
	 * Thus we need to handle all possible conditions cleanly
	 */
	gboolean abort = FALSE;
	static gboolean serial_is_open = FALSE; // Assume never opened 
	extern gchar * potential_ports;
	extern volatile gboolean offline;
	gchar ** vector = NULL;
	gint i = 0;

	if (offline)
	{
		g_timeout_add(100,(GtkFunction)queue_function,g_strdup("kill_conn_warning"));
		g_thread_exit(0);
	}

	if (!serial_repair_queue)
		serial_repair_queue = g_async_queue_new();
	/* IF serial_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a cimple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (serial_is_open == TRUE)
	{
		i = 0;
		while (i <= 5)
		{
			if (comms_test())
			{
				g_thread_exit(0);
			}
			i++;
		}
		close_serial();
		serial_is_open = FALSE;
		/* Fall through */
	}
	vector = g_strsplit(potential_ports,",",-1);
	// App just started, no connection yet
	while ((!serial_is_open) && (!abort)) 	
	{
		for (i=0;i<g_strv_length(vector);i++)
		{
			/* Messagequeue used to exit immediately */
			if (g_async_queue_try_pop(serial_repair_queue))
			{
				g_timeout_add(100,(GtkFunction)queue_function,g_strdup("kill_conn_warning"));
				g_thread_exit(0);
			}
			if (!g_file_test(vector[i],G_FILE_TEST_EXISTS))
				continue;
			if (open_serial(vector[i]))
			{
				setup_serial_params(9600);
				if (!comms_test())
				{
					setup_serial_params(115200);
					if (!comms_test())
					{  
						close_serial();
						continue;
					}
					else
					{	/* We have a winner !!  Abort loop */
						serial_is_open = TRUE;
						break;
					}

				}
				else
				{	/* We have a winner !!  Abort loop */
					serial_is_open = TRUE;
					break;
				}
			}
		}
	}

	if (vector)
		g_strfreev(vector);
	g_thread_exit(0);
	return NULL;
}
