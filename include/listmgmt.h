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

#ifndef __LISTMGMT_H__
#define __LISTMGMT_H__

#include <gtk/gtk.h>

typedef struct _ListElement ListElement;
struct _ListElement 
{
	gchar *filename;	/* Filename  of interrogation profile */
	gchar *name;		/* Shortname in choice box */
};

/* Prototypes */
GList * get_list(gchar * );
void store_list(gchar * , GList * );
gint list_sort(gconstpointer, gconstpointer);
void free_element(gpointer, gpointer);
void simple_free_element(gpointer, gpointer);
/* Prototypes */

#endif
