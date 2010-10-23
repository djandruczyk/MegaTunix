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
#include <firmware.h>

typedef struct _ListElement ListElement;
struct _ListElement 
{
	gchar *filename;	/* Filename */
	gchar *dirname;		/* Dirname */
	gchar *name;		/* Shortname in choice box */
	gint baud;		/* Random data */
	gboolean def;		/* Default choice */
};

/* Prototypes */
GList * get_list(gchar * );
void store_list(gchar * , GList * );
void remove_list(gchar *);
gint list_sort(gconstpointer, gconstpointer);
gint list_object_sort(gconstpointer, gconstpointer, gpointer);
void free_element(gpointer, gpointer);
void simple_free_element(gpointer, gpointer);
/* Prototypes */

#endif
