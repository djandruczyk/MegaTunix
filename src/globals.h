/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/* Global Variables */

#include <config.h>
#include <sys/types.h>
#include <termios.h>
#include <gtk/gtk.h>

/* I/O related */
int def_comm_port;		/* default com port (DOS/WIN32 style) */
int read_wait_time;		/* Time delay between rawreads (milliseconds) */
int reset_count;			/* number of MS Power resets */

struct Serial_Params
{
	int fd;			/* File descriptor */
	int comm_port;		/* DOS/Windows COM port number, 1-8 typically */
	int open;		/* flag, 1 for open 0 for closed */
	int poll_timeout;	/* Pollng interval in MILLISECONDS */
	int raw_bytes;		/* number of bytes to read for realtime vars */
	struct termios oldtio;	/* serial port settings before we touch it */
	struct termios newtio;	/* serial port settings we use when running */
	int errcount;		/* Serial I/O errors read error count */
} serial_params;

/* Thread related */
pthread_t raw_input_thread;	/* thread handle */
int raw_reader_running;		/* flag for thread */
int raw_reader_stopped;		/* flag for thread */
int just_starting;		/* to handle errors */

/* Data storage */
struct ms_data_v1_and_v2 out;		/* processed data structure */
struct ms_raw_data_v1_and_v2 *raw;	/* RAW data pointer */


/* GUI */
int width;				/* main window width */
int height;				/* main window height */
int def_width;				/* main window width */
int def_height;				/* main window height */
GtkWidget	*main_window;		
int main_x_origin;			/* main window position */
int main_y_origin;			/* main window position */
int ready;
