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
#include <errno.h>
#include <fcntl.h>
#include <globals.h>
#include <notifications.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <threads.h>
#include <unistd.h>


extern gboolean dualtable;
extern gboolean raw_reader_running;
extern gint last_page;
extern GtkWidget *comms_view;
extern struct DynamicMisc misc;
extern struct Ve_Const_Std *ve_const_p0;
extern struct Ve_Const_Std *ve_const_p0_tmp;
extern struct Ve_Const_Std *ve_const_p1;
extern struct Ve_Const_Std *ve_const_p1_tmp;
extern gint last_page;
struct Serial_Params *serial_params;
gboolean connected;
static gboolean burn_needed = FALSE;
       
void open_serial(int port_num)
{
	/* We are using DOS/Win32 style com port numbers instead of unix
	 * style as its easier to think of COM1 instead of /dev/ttyS0
	 * thus com1=/dev/ttyS0, com2=/dev/ttyS1 and so on 
	 */
	gint result = -1;
	gchar *tmpbuf;
	gchar *devicename;	/* temporary unix name of the serial port */
	serial_params->comm_port = port_num; /* DOS/Win32 semantics here */

	/* Unix port names are always 1 lower than the DOS/Win32 semantics. 
	 * Thus com1 = /dev/ttyS0 on unix 
	 */
	devicename = g_strdup_printf("/dev/ttyS%i",port_num-1);
	/* Open Read/Write and NOT as the controlling TTY */
	result = open(devicename, O_RDWR | O_NOCTTY);
	if (result >= 0)
	{
		/* SUCCESS */
		/* NO Errors occurred opening the port */
		serial_params->open = TRUE;
		serial_params->fd = result;
		/* Save serial port status */
		tcgetattr(serial_params->fd,&serial_params->oldtio);
		tmpbuf = g_strdup_printf("COM%i Opened Successfully\n",port_num);
		update_logbar(comms_view,NULL,tmpbuf,TRUE);
		g_free(tmpbuf);
	}
	else
	{
		/* FAILURE */
		/* An Error occurred opening the port */
		serial_params->open = FALSE;
		tmpbuf = g_strdup_printf("Error Opening COM%i Error Code: %s\n",
				port_num,strerror(errno));
		update_logbar(comms_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
	}

	g_free(devicename);
	return;
}
	
int setup_serial_params()
{
	/* Sets up serial port for the modes we want to use. 
	 * NOTE: Original serial tio params are stored and restored 
	 * in the open_serial() and close_serial() functions.
	 */ 

	/*clear struct for new settings*/
	bzero(&serial_params->newtio, sizeof(serial_params->newtio)); 
	/* 
	 * BAUDRATE: Set bps rate. You could also use cfsetispeed and 
	 * cfsetospeed
	 * CRTSCTS : output hardware flow control (only used if the cable has
	 * all necessary lines. See sect. 7 of Serial-HOWTO)
	 * CS8     : 8n1 (8bit,no parity,1 stopbit)
	 * CLOCAL  : local connection, no modem contol
	 * CREAD   : enable receiving characters
	 */

	serial_params->newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	/*
	 * IGNPAR  : ignore bytes with parity errors
	 * ICRNL   : map CR to NL (otherwise a CR input on the other computer
	 * will not terminate input)
	 * otherwise make device raw (no other input processing)
	 */

	//      serial_params->newtio.c_iflag = IGNPAR |IGNBRK;
	/* RAW Input */
	serial_params->newtio.c_iflag = 0;
	/* RAW Ouput also */
	serial_params->newtio.c_oflag = 0;
	/* set input mode (non-canonical, no echo,...) */
	serial_params->newtio.c_lflag = 0;

	cfmakeraw(&serial_params->newtio);

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
	serial_params->newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	serial_params->newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arriv
							 es */
	serial_params->newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	serial_params->newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	serial_params->newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	serial_params->newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	serial_params->newtio.c_cc[VEOL]     = 0;     /* '\0' */
	serial_params->newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	serial_params->newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	serial_params->newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	serial_params->newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	serial_params->newtio.c_cc[VEOL2]    = 0;     /* '\0' */

	/* blocking read until proper number of chars arrive */

	tcflush(serial_params->fd, TCIFLUSH);
	tcsetattr(serial_params->fd,TCSANOW,&serial_params->newtio);

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

	tcsetattr(serial_params->fd,TCSANOW,&serial_params->oldtio);
	close(serial_params->fd);
	serial_params->open = FALSE;
	connected = FALSE;
	gtk_widget_set_sensitive(misc.status[CONNECTED],
			connected);
	gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
			connected);

	tmpbuf = g_strdup_printf("COM Port Closed\n");
	/* An Closing the comm port */
	update_logbar(comms_view,NULL,tmpbuf,TRUE);
	g_free(tmpbuf);
}

int check_ecu_comms(GtkWidget *widget, gpointer data)
{
	gint tmp;
	gint res;
	struct pollfd ufds;
	char buf[1024];
	gint restart_reader = FALSE;
	static gboolean locked;
	gchar *tmpbuf;

	if (locked)
		return 0;
	else
		locked = TRUE;
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
		tmp = serial_params->newtio.c_cc[VMIN];
		serial_params->newtio.c_cc[VMIN]     = 1; /*wait for 1 char */
		tcflush(serial_params->fd, TCIFLUSH);
		tcsetattr(serial_params->fd,TCSANOW,&serial_params->newtio);

		/* request one batch of realtime vars */
		if (last_page != 0)
			set_ms_page(0);
		res = write(serial_params->fd,"A",1);
		res = poll (&ufds,1,serial_params->poll_timeout);
		if (res)
		{
			/* Command succeeded,  but we still need to drain the
			 * buffer...
			 */
			while (poll(&ufds,1,serial_params->poll_timeout))
				res = read(serial_params->fd,&buf,64);

			tmpbuf = g_strdup_printf("ECU Comms Test Successfull\n");
			/* COMMS test succeeded */
			update_logbar(comms_view,NULL,tmpbuf,TRUE);
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
			update_logbar(comms_view,"warning",tmpbuf,TRUE);
			g_free(tmpbuf);
			connected = FALSE;
			gtk_widget_set_sensitive(misc.status[CONNECTED],
					connected);
			gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
					connected);

		}

		serial_params->newtio.c_cc[VMIN]     = tmp; /*restore original*/
		tcflush(serial_params->fd, TCIFLUSH);
		tcsetattr(serial_params->fd,TCSANOW,&serial_params->newtio);

		if (restart_reader)
			start_serial_thread();
	}
	else
	{
		tmpbuf = g_strdup_printf("Serial Port NOT Opened, Can NOT Test ECU Communications\n");
		/* Serial port not opened, can't test */
		update_logbar(comms_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
	}
	locked = FALSE;
	return (connected);

}

void read_ve_const()
{
	gboolean restart_reader = FALSE;
	struct pollfd ufds;
	int res = 0;

	if (!connected)
	{
		no_ms_connection();
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
	tcflush(serial_params->fd, TCIFLUSH);

	if (last_page != 0)
		set_ms_page(0);
	res = write(serial_params->fd,"V",1);
	res = poll (&ufds,1,serial_params->poll_timeout);
	if (res == 0)	/* Error */
	{
		serial_params->errcount++;
		connected = FALSE;
	}
	else		/* Data arrived */
	{
		connected = TRUE;
		res = handle_ms_data(VE_AND_CONSTANTS_1);

	}
	if (dualtable)
	{
		if (last_page != 1)
			set_ms_page(1);
		res = write(serial_params->fd,"V",1);
		res = poll (&ufds,1,serial_params->poll_timeout);
		if (res == 0)	// Error 
		{
			serial_params->errcount++;
			connected = FALSE;
		}
		else		// Data arrived 
		{
			connected = TRUE;
			res = handle_ms_data(VE_AND_CONSTANTS_2);

		}
	}
	gtk_widget_set_sensitive(misc.status[CONNECTED],
			connected);
	gtk_widget_set_sensitive(misc.ww_status[CONNECTED],
			connected);

	update_errcounts(NULL,FALSE);

	tcflush(serial_params->fd, TCIFLUSH);

	if (restart_reader)
		start_serial_thread();

	return;
}

void set_ms_page(gint ms_page)
{
	gint res = 0;
	gchar buf = ms_page & 0x01;
#ifdef DEBUG
	fprintf(stderr,__FILE__": Changing page on MS to %i\n",ms_page);
#endif
	res = write(serial_params->fd,"P",1);
	res = write(serial_params->fd,&buf,1);
	if (res != 1)
		fprintf(stderr,__FILE__": FAILURE changing page on MS to %i\n",ms_page);
	last_page = ms_page;	
}

void write_ve_const(gint value, gint offset, gint page)
{
	gint highbyte = 0;
	gint lowbyte = 0;
	gboolean twopart = 0;
	gint res = 0;
	gint count = 0;
	char lbuff[3] = {0, 0, 0};

	if (!connected)
	{
		no_ms_connection();
		return;		/* can't write anything if disconnected */
	}
#ifdef DEBUG
	printf("MS Serial Write, Value %i, Mem Offset %i\n",value,offset);
#endif
	if (value > 255)
	{
		//	printf("large value, %i, offset %i\n",value,offset);
		highbyte = (value & 0xff00) >> 8;
		lowbyte = value & 0x00ff;
		twopart = TRUE;
	}
	if (value < 0)
	{
		printf("WARNING!!, value sent is below 0\n");
		return;
	}

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
	if (page != last_page)
		set_ms_page(page);

	res = write (serial_params->fd,"W",1);	/* Send write command */
	res = write (serial_params->fd,lbuff,count);	/* Send write command */

	if (page == 1)
		printf("DualTable write operation...\n");

	/* We check to see if the last burn copy of the MS VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */
	res = memcmp(ve_const_p0_tmp,ve_const_p0,sizeof(struct Ve_Const_Std)) +
		memcmp(ve_const_p1_tmp,ve_const_p1,sizeof(struct Ve_Const_Std));
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

}

void burn_flash()
{
	if (!connected)
	{
		no_ms_connection();
		return;		/* can't burn if disconnected */
	}
	/* doing this may NOT be necessary,  but who knows... */
	set_ms_page(1);
	write (serial_params->fd,"B",1);	/* Send Burn command */
	set_ms_page(0);
	write (serial_params->fd,"B",1);	/* Send Burn command */

	/* sync temp buffer with current VE_constants */
	memcpy(ve_const_p0_tmp,ve_const_p0,sizeof(struct Ve_Const_Std));
	memcpy(ve_const_p1_tmp,ve_const_p1,sizeof(struct Ve_Const_Std));
	/* Take away the red on the "Store" button */
	set_store_buttons_state(BLACK);
	burn_needed = FALSE;
}
