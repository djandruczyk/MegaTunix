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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <config.h>
#include <sys/types.h>
#include <termios.h>
#include <gtk/gtk.h>

struct Serial_Params
{
	int fd;			/* File descriptor */
	int comm_port;		/* DOS/Windows COM port number, 1-8 typically */
	int open;		/* flag, 1 for open 0 for closed */
	int poll_timeout;	/* Pollng interval in MILLISECONDS */
	int read_wait;		/* time delay between each read */
	int raw_bytes;		/* number of bytes to read for realtime vars */
	int veconst_size;	/* Size of VEtable/constants datablock */
	struct termios oldtio;	/* serial port settings before we touch it */
	struct termios newtio;	/* serial port settings we use when running */
	int errcount;		/* Serial I/O errors read error count */
} serial_params;

#endif
