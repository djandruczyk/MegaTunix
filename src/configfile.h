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

/* Configfile structs. (derived from an older version of XMMS) */

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <glib.h>

typedef struct
{
        gchar *key;
        gchar *value;
}
ConfigLine;

typedef struct
{
        gchar *name;
        GList *lines;
}
ConfigSection;
typedef struct
{
        GList *sections;
}
ConfigFile;
#endif


