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

#ifndef __INTERROGATE_H__
#define __INTERROGATE_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <configfile.h>
#include <enums.h>
#include <threads.h>


typedef enum
{
	RESULT_DATA=0x440,
	RESULT_TEXT
}Test_Result;

typedef enum
{
	COUNT=0x260,
	NUMMATCH,
	SUBMATCH,
	FULLMATCH,
	REGEX
}MatchClass;


/* Prototypes */
gboolean interrogate_ecu(void);
void request_firmware_version(gchar **);
void request_interface_version(gchar **, guint8 *, guint8 *, guint8 *);
GList *request_location_ids(void);

/* Prototypes */

#endif
