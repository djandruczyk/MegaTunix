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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <defines.h>
#include <protos.h>
#include <constants.h>
#include <globals.h>
#include <runtime_gui.h>
#include <errno.h>


extern gboolean raw_reader_running;
extern gint ser_context_id;
extern GtkWidget *ser_statbar;
char buff[60];
static gboolean burn_needed = FALSE;
extern struct v1_2_Runtime_Gui runtime_data;
extern struct ve_const_std *ve_constants;
extern struct ve_const_std *ve_const_tmp;
gboolean connected;
       
int open_serial(int port_num)
{
	/* We are using DOS/Win32 style com port numbers instead of unix
	 * style as its easier to think of COM1 instead of /dev/ttyS0
	 * thus com1=/dev/ttyS0, com2=/dev/ttyS1 and so on 
	 */
	char devicename[11]; /* temporary unix name of the serial port */
	serial_params.comm_port = port_num;
	g_snprintf(devicename,11,"/dev/ttyS%i",port_num-1);
	serial_params.fd = open(devicename, O_RDWR | O_NOCTTY);
	if (serial_params.fd < 0)
	{
		/* FAILURE */
		/* An Error occurred opening the port */
		serial_params.open = FALSE;
		g_snprintf(buff,60,"Error Opening COM%i Error Code: %s",port_num,strerror(errno));
		update_statusbar(ser_statbar,ser_context_id,buff);
	}
	else
	{
		/* SUCCESS */
		/* NO Errors occurred opening the port */
		serial_params.open = TRUE;
		g_snprintf(buff,60,"COM%i Opened Successfully, Suggest Testing ECU Comms",port_num);
		update_statusbar(ser_statbar,ser_context_id,buff);

	}
	return serial_params.fd;
}
	
int setup_serial_params()
{
	tcgetattr(serial_params.fd,&serial_params.oldtio); /* save current port settings */

	bzero(&serial_params.newtio, sizeof(serial_params.newtio)); /*clear struct for new settings*/
	/* 
	 * BAUDRATE: Set bps rate. You could also use cfsetispeed and 
	 * cfsetospeed
	 * CRTSCTS : output hardware flow control (only used if the cable has
	 * all necessary lines. See sect. 7 of Serial-HOWTO)
	 * CS8     : 8n1 (8bit,no parity,1 stopbit)
	 * CLOCAL  : local connection, no modem contol
	 * CREAD   : enable receiving characters
	 */

	serial_params.newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	/*
	 * IGNPAR  : ignore bytes with parity errors
	 * ICRNL   : map CR to NL (otherwise a CR input on the other computer
	 * will not terminate input)
	 * otherwise make device raw (no other input processing)
	 */

	//      serial_params.newtio.c_iflag = IGNPAR |IGNBRK;
	/* RAW Input */
	serial_params.newtio.c_iflag = 0;
	/* RAW Ouput also */
	serial_params.newtio.c_oflag = 0;
	/* set input mode (non-canonical, no echo,...) */
	serial_params.newtio.c_lflag = 0;

	cfmakeraw(&serial_params.newtio);

	/* 
	   initialize all control characters 
	   default values can be found in /usr/include/termios.h, and are given
	   in the comments, but we don't need them here
	 */
	serial_params.newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	serial_params.newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	serial_params.newtio.c_cc[VERASE]   = 0;     /* del */
	serial_params.newtio.c_cc[VKILL]    = 0;     /* @ */
	serial_params.newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
	serial_params.newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	serial_params.newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arriv
					  es */
	serial_params.newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	serial_params.newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	serial_params.newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	serial_params.newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	serial_params.newtio.c_cc[VEOL]     = 0;     /* '\0' */
	serial_params.newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	serial_params.newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	serial_params.newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	serial_params.newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	serial_params.newtio.c_cc[VEOL2]    = 0;     /* '\0' */

	serial_params.newtio.c_cc[VMIN]     = serial_params.raw_bytes; 

	/* blocking read until proper number of chars arrive */

	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

	/* No hurt in checking to see if the MS is present, if it is
	 * It'll update the serial statusbar, and set the "Connected" flag
	 * which is visible in the Runtime screen 
	 */
	check_ecu_comms(NULL,NULL);

	return 0;
}

void close_serial()
{

	tcsetattr(serial_params.fd,TCSANOW,&serial_params.oldtio);
	close(serial_params.fd);
	serial_params.open = FALSE;
	connected = FALSE;
        gtk_widget_set_sensitive(runtime_data.status[0],
                        connected);

	g_snprintf(buff,60,"COM Port Closed ");
	/* An Closing the comm port */
	update_statusbar(ser_statbar,ser_context_id,buff);
}

int check_ecu_comms(GtkWidget *widget, gpointer data)
{
        gint tmp;
        gint res;
        struct pollfd ufds;
	gchar buff[60];
        gint restart_reader = FALSE;

        if(serial_params.open)
	{
		if (raw_reader_running)
		{
			restart_reader = TRUE;
			stop_serial_thread(); /* stops realtime read */
			usleep(100000);	/* sleep 100 ms to be sure thread ends */
		}

		ufds.fd = serial_params.fd;
		ufds.events = POLLIN;
		/* save state */
		tmp = serial_params.newtio.c_cc[VMIN];
		serial_params.newtio.c_cc[VMIN]     = 1; /*wait for 1 char */
		tcflush(serial_params.fd, TCIFLUSH);
		tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

		res = write(serial_params.fd,"C",1);
		res = poll (&ufds,1,serial_params.poll_timeout);
		if (res == 0)
		{
			g_snprintf(buff,60,"I/O with MegaSquirt Timeout");
			/* An I/O Error occurred with the MegaSquirt ECU */
			update_statusbar(ser_statbar,ser_context_id,buff);
			connected = FALSE;
			gtk_widget_set_sensitive(runtime_data.status[0],
					connected);
		}
		else
		{
			g_snprintf(buff,60,"ECU Comms Test Successfull");
			/* COMMS test succeeded */
			update_statusbar(ser_statbar,ser_context_id,buff);
			connected = TRUE;
			gtk_widget_set_sensitive(runtime_data.status[0],
					connected);
		}

		serial_params.newtio.c_cc[VMIN]     = tmp; /*restore original*/
		tcflush(serial_params.fd, TCIFLUSH);
		tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

		if (restart_reader)
			start_serial_thread();
	}
        else
        {
                g_snprintf(buff,60,"Serial Port NOT Opened, Can NOT Test ECU Communications");
                /* Serial port not opened, can't test */
		update_statusbar(ser_statbar,ser_context_id,buff);
        }
        return (0);

}

void read_ve_const()
{
	int restart_reader = FALSE;
	struct pollfd ufds;
	int res = 0;
	int tmp = 0;

	if (!connected)
	{
		no_ms_connection();
		return;		/* can't do anything if not connected */
	}
	if (raw_reader_running)
	{
		restart_reader = TRUE;
		stop_serial_thread(); /* stops realtime read */
		usleep(100000);	/* sleep 100 ms to be sure thread ends */
	}

	ufds.fd = serial_params.fd;
	ufds.events = POLLIN;

	/* save state */
	tmp = serial_params.newtio.c_cc[VMIN]; /* wait for VE table */
	serial_params.newtio.c_cc[VMIN]     = 125;
	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

	res = write(serial_params.fd,"V",1);
	res = poll (&ufds,1,serial_params.poll_timeout*20);
	if (res == 0)	/* Error */
	{
		serial_params.errcount++;
		connected = FALSE;
	}
	else		/* Data arrived */
	{
		connected = TRUE;
		res = handle_ms_data(VE_AND_CONSTANTS);
		
	}
	gtk_widget_set_sensitive(runtime_data.status[0],
			connected);

	update_errcounts();

	/* restore previous serial port settings */
	serial_params.newtio.c_cc[VMIN]     = tmp; /*restore original*/
	tcflush(serial_params.fd, TCIFLUSH);
	tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

	if (restart_reader)
	{
		start_serial_thread();
	}

	return;
}


void write_ve_const(gint value, gint offset, gint page)
{
	gint highbyte = 0;
	gint lowbyte = 0;
	gint twopart = 0;
	gint res = 0;
	gint count = 0;
	char buff[3] = {0, 0, 0};

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

	buff[0]=offset;
	if(twopart)
	{
		buff[1]=highbyte;
		buff[2]=lowbyte;
		count = 3;
	}
	else
	{
		buff[1]=value;
		count = 2;
	}
	if (page == 0)
	{
		res = write (serial_params.fd,"W",1);	/* Send write command */
		res = write (serial_params.fd,buff,count);	/* Send write command */
	}
	else if (page == 1)
	{	/* DUAL Table code only thus far.... */
		printf("DualTable write operation...\n");
		res = write (serial_params.fd,"P1",2);	/* Send write command */
		res = write (serial_params.fd,"W",1);	/* Send write command */
		res = write (serial_params.fd,buff,count);	/* Send write command */
	
	}

	/* We check to see if the last burn copy of the MS VE/constants matches 
	 * the currently set, if so take away the "burn now" notification.
	 * avoid unnecessary burns to the FLASH 
	 */
	res = memcmp(ve_const_tmp,ve_constants,sizeof(struct ve_const_std));
	if (res == 0)
	{
		set_store_black();
		burn_needed = FALSE;
	}
	else
	{
		set_store_red();
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
	write (serial_params.fd,"B",1);	/* Send Burn command */

	/* sync temp buffer with current VE_constants */
	memcpy(ve_const_tmp,ve_constants,sizeof(struct ve_const_std));
	/* Take away the red on the "Store" button */
	set_store_black();
	burn_needed = FALSE;
}

