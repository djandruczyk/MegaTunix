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

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"


extern int raw_reader_running;
extern GtkWidget *ser_statbar;
extern int ser_context_id;
char buff[60];
       
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
		serial_params.open = 0;
		g_snprintf(buff,60,"Error Opening COM%i",port_num);
		/* An Error occurred opening the port */
		gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
				ser_context_id);
		gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
				ser_context_id,
				buff);
	}
	else
	{
		/* SUCCESS */
		serial_params.open = 1;
		g_snprintf(buff,60,"COM%i opened successfully",port_num);
		/* An Error occurred opening the port */
		gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
				ser_context_id);
		gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
				ser_context_id,
				buff);

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

	return 0;
}

void close_serial()
{

	tcsetattr(serial_params.fd,TCSANOW,&serial_params.oldtio);
	close(serial_params.fd);
	serial_params.open = 0;

	g_snprintf(buff,60,"COM port closed ");
	/* An Error occurred opening the port */
	gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
			ser_context_id);
	gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
			ser_context_id,
			buff);
}

int check_ecu_comms(GtkWidget *widget, gpointer data)
{
        gint tmp;
        gint res;
        struct pollfd ufds;
	gchar buff[60];
        gint restart_thread = 0;

        if(serial_params.open)
        {
                if (raw_reader_running)
                {
                        raw_reader_running = 0;
                        restart_thread = 1;
			serial_raw_thread_stopper(); /* stops realtime read */
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
                        /* An Error occurred opening the port */
                        gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
                                        ser_context_id);
                        gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
                                        ser_context_id,
                                        buff);
                }
                else
                {
                        g_snprintf(buff,60,"ECU comms test successfull");
                        /* An Error occurred opening the port */
                        gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
                                        ser_context_id);
                        gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
                                        ser_context_id,
                                        buff);
                }

                serial_params.newtio.c_cc[VMIN]     = tmp; /*restore original*/
                tcflush(serial_params.fd, TCIFLUSH);
                tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

                if (restart_thread)
                        serial_raw_thread_starter();
        }
        else
        {
                g_snprintf(buff,60,"Serial port not opened, can't test ECU comms");
                /* An Error occurred opening the port */
                gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
                                ser_context_id);
                gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
                                ser_context_id,
                                buff);
        }
        return (0);

}

