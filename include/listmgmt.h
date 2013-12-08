/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/listmgmt.h
  \ingroup Headers
  \brief Header for list management convenience functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LISTMGMT_H__
#define __LISTMGMT_H__

#include <gtk/gtk.h>
#include <firmware.h>

typedef struct _ListElement ListElement;
/*!
  \brief _ListElement holds the infomation specific to each choice
  */
struct _ListElement 
{
	gchar *filename;	/*!< Filename */
	gchar *dirname;		/*!< Dirname */
	gchar *name;		/*!< Shortname in choice box */
	gint baud;		/*!< Random data */
	gboolean def;		/*!< Default choice */
};

/* Prototypes */
void free_element(gpointer, gpointer);
GList *get_list(const gchar * );
gint list_sort(gconstpointer, gconstpointer);
gint list_object_sort(gconstpointer, gconstpointer, gpointer);
void remove_list(const gchar *);
void store_list(const gchar * , GList * );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
