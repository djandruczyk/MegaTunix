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
/*#define BAUDRATE B115200 */
#define _POSIX_SOURCE 1 /* POSIX compliant source */

/* Windows specific for exporting symbols for glade... */
#ifdef __WIN32__
#define EXPORT __declspec (dllexport)
#define DEFAULT_PORT "COM1"
#define PSEP "\\"
#define HOME g_get_current_dir
#else
#define EXPORT 
#define DEFAULT_PORT "/dev/ttyS0"
#define PSEP "/"
#define HOME g_get_home_dir
#endif

/* g_object_get/set macros */

#define OBJ_GET(object, name) g_object_get_data(G_OBJECT(object),name)
#define OBJ_SET(object, name, data) g_object_set_data(G_OBJECT(object),name,data)

/* Download modes */
#define IMMEDIATE		0x10
#define DEFERRED		0x11
#define IGNORED			0x12


/* For datalogging and Logviewer */
#define TABLE_COLS 6

/* Multitherm */
#define IAT 0
#define CLT 1

#define ASCII_LOWER_CAPS_DIFF 32

#define DEFAULT_BIAS 2490

#define TEMP_C_100_DENSITY 21.1111
#define TEMP_K_100_DENSITY 294.2611

#define C_TO_K 273



#endif
