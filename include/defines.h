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

#ifndef __DEFINES_H__
#define __DEFINES_H__

/* Definitions */
#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MS_PAGE_SIZE 256
#define MAX_SUPPORTED_PAGES 8

/* Windows speciifc for exporting symbols forglade... */
#ifdef __WIN32__
#define EXPORT __declspec (dllexport)
#else
#define EXPORT 
#endif


/* Download modes */
#define IMMEDIATE		0x10
#define DEFERRED		0x11


/* For datalogging and Logviewer */
#define TABLE_COLS 5


#endif
