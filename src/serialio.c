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

#include <comms_gui.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <errno.h>
#include <fcntl.h>
#include <glib/gprintf.h>
#include <notifications.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <string.h>
#include <structures.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <termios.h>
#include <threads.h>
#include <unistd.h>


extern gboolean raw_reader_running;
extern GtkWidget *comms_view;
extern struct DynamicMisc misc;
struct Serial_Params *serial_params;
gboolean connected;
static gboolean burn_needed = FALSE;
       
void open_serial(gchar * port_name)
{
	/* We are using DOS/Win32 style com port numbers instead of unix
	 * style as its easier to think of COM1 instead of /dev/ttyS0
	 * thus com1=/dev/ttyS0, com2=/dev/ttyS1 and so on 
	 */
	gint fd = -1;
	gchar *tmpbuf;
	gchar *device;	/* temporary unix name of the serial port */
	serial_params->port_name = g_strdup(port_name); 

	device = g_strdup(port_name);
	/* Open Read/Write and NOT as the controlling TTY in nonblock mode */
	//fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	/* Blocking mode... */
	fd = open(device, O_RDWR | O_NOCTTY );
	if (fd >= 0)
	{
		/* SUCCESS */
		/* NO Errors occurred opening the port */
		serial_params->open = TRUE;
		serial_params->fd = fd;
		/* Save serial port status */
		tcgetattr(serial_params->fd,&serial_params->oldtio);
		tmpbuf = g_strdup_printf("%s Opened Successfully\n",device);
		dbg_func(tmpbuf,SERIAL_GEN);
		update_logbar(comms_view,NULL,tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
	}
	else
	{
		/* FAILURE */
		/* An Error occurred opening the port */
		serial_params->open = FALSE;
		tmpbuf = g_strdup_printf("Error Opening %s Error Code: %s\n",
				device,g_strerror(errno));
		dbg_func(tmpbuf,CRITICAL);
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
	}

	g_free(device);
	return;
}
	
int setup_serial_params()
{
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
	cfsetispeed(&serial_params->newtio, BAUDRATE);
	cfsetospeed(&serial_params->newtio, BAUDRATE);

	/* Set additional flags, note |= syntax.. */
	serial_params->newtio.c_cflag |= CLOCAL | CREAD;
	/* Mask and set to 8N1 mode... */
	serial_params->newtio.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	serial_params->newtio.c_cflag |= CS8;

	/* RAW Input */
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
	serial_params->newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	serial_params->newtio.c_cc[VTIME]    = 1;     /* inter-character timer unused */

	tcsetattr(serial_params->fd,TCSAFLUSH,&serial_params->newtio);

	/* No hurt in checking to see if the MS is present, if it is
	 * It'll update the serial status log, and set the "Connected" flag
	 * which is visible in the Runtime screen 
	 */
	check_ecu_comms(NULL,NULL);

	return 0;
}

void close_serial()
{
	gchar *tmpbuf;

	tcsetattr(serial_params->fd,TCSAFLUSH,&serial_params->oldtio);
	close(serial_params->fd);
	serial_params->open = FALSE;
	connected = FALSE;
	gtk_widget_set_sensitive(misc.status[CONNECTED],
			connected);
	gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
			connected);

	tmpbuf = g_strdup_printf("COM Port Closed\n");
	/* An Closing the comm port */
	dbg_func(tmpbuf,SERIAL_GEN);
	update_logbar(comms_view,NULL,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
}

gboolean check_ecu_comms(GtkWidget *widget, gpointer data)
{
	gint res;
	struct pollfd ufds;
	gint restart_reader = FALSE;
	gchar *tmpbuf;
	gchar buff[1024];
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	/* If port isn't opened no sense trying here... */
	if(serial_params->open)
	{
		/* If realtime reader thread is running shut it down... */
		if (raw_reader_running)
		{
			restart_reader = TRUE;
			stop_serial_thread(); /* stops realtime read */
		}

		ufds.fd = serial_params->fd;
		ufds.events = POLLIN;
		/* save state */
		tcflush(serial_params->fd, TCIOFLUSH);

		/* request one batch of realtime vars */
		while (write(serial_params->fd,"A",1) != 1)
		{
			usleep(1000);
			dbg_func(__FILE__": Error writing \"A\" to the ecu in check_ecu_comms()\n",CRITICAL);
		}
		dbg_func(__FILE__": check_ecu_comms() Requesting one batch of realtime vars (\"A\" cmd)\n",SERIAL_RD);
			
		res = poll (&ufds,1,serial_params->poll_timeout*5);
		if (res)
		{
			/* Command succeeded,  but we still need to drain the
			 * buffer...
			 */
			while (poll(&ufds,1,25))
                        {
                                read(serial_params->fd,buff,64);
                        }

			tmpbuf = g_strdup_printf("ECU Comms Test Successfull\n");
			/* COMMS test succeeded */
			dbg_func(tmpbuf,SERIAL_RD);
			update_logbar(comms_view,NULL,tmpbuf,TRUE,FALSE);
			g_free(tmpbuf);
			connected = TRUE;
			gtk_widget_set_sensitive(misc.status[CONNECTED],
					connected);
			gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
					connected);
			
		}
		else
		{
			tmpbuf = g_strdup_printf("I/O with MegaSquirt Timeout\n");
			/* An I/O Error occurred with the MegaSquirt ECU */
			dbg_func(tmpbuf,CRITICAL);
			update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
			g_free(tmpbuf);
			connected = FALSE;
			gtk_widget_set_sensitive(misc.status[CONNECTED],
					connected);
			gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
					connected);

		}

		tcflush(serial_params->fd, TCIOFLUSH);

		if (restart_reader)
			start_serial_thread();
	}
	else
	{
		tmpbuf = g_strdup_printf("Serial Port NOT Opened, Can NOT Test ECU Communications\n");
		/* Serial port not opened, can't test */
		dbg_func(tmpbuf,CRITICAL);
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
	}
	g_static_mutex_unlock(&mutex);
	return (connected);

}

void read_ve_const()
{
	gboolean restart_reader = FALSE;
	struct pollfd ufds;
	int res = 0;
	extern unsigned int ecu_caps;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		no_ms_connection();
		g_static_mutex_unlock(&mutex);
		return;		/* can't do anything if not connected */
	}
	if (raw_reader_running)
	{
		restart_reader = TRUE;
		stop_serial_thread(); /* stops realtime read */
	}

	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;

	/* Flush serial port... */
	tcflush(serial_params->fd, TCIOFLUSH);

	if (ecu_caps & DUALTABLE)
		set_ms_page(0);
	res = write(serial_params->fd,"V",1);
	res = poll (&ufds,1,serial_params->poll_timeout);
	if (res == 0)	/* Error */
	{
		dbg_func(__FILE__": failure reading VE-Table\n",CRITICAL);
		serial_params->errcount++;
		connected = FALSE;
	}
	else		/* Data arrived */
	{
		connected = TRUE;
		dbg_func(__FILE__": reading VE-Table(0)\n",SERIAL_RD);
		res = handle_ms_data(VE_AND_CONSTANTS_1);

	}
	if (ecu_caps & DUALTABLE)
	{
		set_ms_page(1);
		res = write(serial_params->fd,"V",1);
		res = poll (&ufds,1,serial_params->poll_timeout);
		if (res == 0)	// Error 
		{
			dbg_func(__FILE__": failure reading VE-Table(1)\n",CRITICAL);
			serial_params->errcount++;
			connected = FALSE;
		}
		else		// Data arrived 
		{
			connected = TRUE;
			dbg_func(__FILE__": reading VE-Table(1)\n",SERIAL_RD);
			res = handle_ms_data(VE_AND_CONSTANTS_2);

		}
		set_ms_page(0);
	}
	if (ecu_caps & (S_N_SPARK|S_N_EDIS))
	{
		res = write(serial_params->fd,"I",1);
		res = poll (&ufds,1,serial_params->poll_timeout);
		if (res == 0)	// Error 
		{
			dbg_func(__FILE__": failure reading Spark-Table\n",CRITICAL);
			serial_params->errcount++;
			connected = FALSE;
		}
		else		// Data arrived 
		{
			connected = TRUE;
			dbg_func(__FILE__": reading Spark-Table(1)\n",SERIAL_RD);
			res = handle_ms_data(IGNITION_VARS);
		}
	}

	gtk_widget_set_sensitive(misc.status[CONNECTED],
			connected);
	gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
			connected);

	tcflush(serial_params->fd, TCIOFLUSH);

	if (restart_reader)
		start_serial_thread();

	g_static_mutex_unlock(&mutex);
	return;
}

void set_ms_page(gint ms_page)
{
	gint res = 0;
	gchar buf;
	gchar *tmpbuf;

	if ((ms_page > 1) || (ms_page < 0))
	{
		tmpbuf = g_strdup_printf(__FILE__": page choice %i is out of range(0,1)\n",ms_page);
		dbg_func(tmpbuf,CRITICAL);
		g_free(tmpbuf);
	}
	
	buf = ms_page & 0x01;
	res = write(serial_params->fd,"P",1);
	if (res != 1)
		dbg_func(__FILE__": FAILURE sending \"P\" (change page) command to ECU \n",CRITICAL);
	res = write(serial_params->fd,&buf,1);
	if (res != 1)
	{
		tmpbuf = g_strdup_printf(__FILE__": FAILURE changing page on ECU to %i\n",ms_page);
		dbg_func(tmpbuf,CRITICAL);
		g_free(tmpbuf);
	}
}

void write_ve_const(gint value, gint offset, gboolean ign_var)
{
	gint highbyte = 0;
	gint lowbyte = 0;
	gboolean twopart = 0;
	gboolean restart_reader = FALSE;
	gint res = 0;
	gint count = 0;
	gchar * tmpbuf;
	char lbuff[3] = {0, 0, 0};
	extern unsigned char *ms_data;
	extern unsigned char *ms_data_last;
	extern unsigned int ecu_caps;
	gchar * write_cmd = NULL;;

	if (!connected)
	{
		no_ms_connection();
		return;		/* can't write anything if disconnected */
	}

	tmpbuf = g_strdup_printf("MS Serial Write, Value %i, Mem Offset %i\n",value,offset);
	dbg_func(tmpbuf,SERIAL_WR);
	g_free(tmpbuf);

	/* If realtime reader thread is running shut it down... */
	if (raw_reader_running)
	{
		restart_reader = TRUE;
		stop_serial_thread(); // stops realtime read 
	}
	if (value > 255)
	{
		//	g_printf("large value, %i, offset %i\n",value,offset);
		highbyte = (value & 0xff00) >> 8;
		lowbyte = value & 0x00ff;
		twopart = TRUE;
	}
	if (value < 0)
	{
		dbg_func(__FILE__": WARNING!!, value sent is below 0\n",CRITICAL);
		return;
	}

	/* Handles variants and dualtable... */
	if (offset > MS_PAGE_SIZE)
	{
		offset -= MS_PAGE_SIZE;
		if (ecu_caps & DUALTABLE)
			set_ms_page(1);
		else
		{
			tmpbuf = g_strdup_printf(__FILE__": High offset (%i), but no DT flag\n",offset+MS_PAGE_SIZE);
			dbg_func(tmpbuf,CRITICAL);
			g_free(tmpbuf);
		}
	
	}
	/* NOT high offset, but if using DT switch page back to 0 */
	else if (ecu_caps & DUALTABLE)
		set_ms_page(0);

	if (ign_var)
		write_cmd = g_strdup("J");
	else
		write_cmd = g_strdup("W");

	lbuff[0]=offset;
	if(twopart)
	{
		lbuff[1]=highbyte;
		lbuff[2]=lowbyte;
		count = 3;
	}
	else
	{
		lbuff[1]=value;
		count = 2;
	}


	res = write (serial_params->fd,write_cmd,1);	/* Send write command */
	res = write (serial_params->fd,lbuff,count);	/* Send write command */
	if (ecu_caps & DUALTABLE)
		set_ms_page(0);
	g_free(write_cmd);

	/* We check to see if the last burn copy of the MS VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */
	res = memcmp(ms_data_last,ms_data,2*MS_PAGE_SIZE);
	if (res == 0)
	{
		set_store_buttons_state(BLACK);
		burn_needed = FALSE;
	}
	else
	{
		set_store_buttons_state(RED);
		burn_needed = TRUE;
	}
	if (restart_reader)
		start_serial_thread();

}

void burn_flash()
{
	extern unsigned char *ms_data;
	extern unsigned char *ms_data_last;
	gint res = 0;
	gboolean restart_reader = FALSE;
	extern unsigned int ecu_caps;
	gchar * tmpbuf;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		no_ms_connection();
		g_static_mutex_unlock(&mutex);
		return;		/* can't burn if disconnected */
	}
	if (raw_reader_running)
	{
		restart_reader = TRUE;
		stop_serial_thread();
	}
	tcflush(serial_params->fd, TCIOFLUSH);
	if (ecu_caps & DUALTABLE)
		set_ms_page(0);

	/* doing this may NOT be necessary,  but who knows... */
	res = write (serial_params->fd,"B",1);	/* Send Burn command */
	if (res != 1)
	{
		tmpbuf = g_strdup_printf(__FILE__": Burn Failure, write command failed %i\n",res);
		dbg_func(tmpbuf,CRITICAL);
	}

	dbg_func(__FILE__": Burn to Flash\n",SERIAL_WR);

	/* sync temp buffer with current burned settings */
	memcpy(ms_data_last,ms_data,2*MS_PAGE_SIZE);

	/* Take away the red on the "Store" button */
	set_store_buttons_state(BLACK);
	burn_needed = FALSE;

	tcflush(serial_params->fd, TCIOFLUSH);

	if (restart_reader)
		start_serial_thread();
	g_static_mutex_unlock(&mutex);
	return;
}
