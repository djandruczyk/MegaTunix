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

#ifndef __TABLOADER_H__
#define __TABLOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>

typedef struct _Group Group;
typedef struct _BindGroup BindGroup;


/*!
 \brief _Group holds common settings from groups of control as defined in a 
 datamap file.  This should reduce redundancy and significantly shrink the 
 datamap files.
 */
struct _Group
{
	gchar **keys;		/*! String array for key names */
	gint *keytypes;		/*! Int array of key types... */
	GObject *object;	/*! To hold the data cleanly */
	gint num_keys;		/* How many keys we hold */
	gint num_keytypes;	/* How many keytypes we hold */
	gint page;		/* page of this group of data */
	gint canID;		/* can_id of this group of data */
};


/*!
 \brief _BindGroup is a small container used to pass multiple params into
 a function that is limited to a certain number of arguments...
 */
struct _BindGroup
{
	ConfigFile *cfgfile;	/*! where the configfile ptr goes... */
	GHashTable *groups;	/*! where the groups table goes */
};


/* Prototypes */
gboolean load_gui_tabs_pf(void);
void group_free(gpointer );
GHashTable * load_groups(ConfigFile *);
void bind_data(GtkWidget *, gpointer);
gint bind_group_data(ConfigFile *,GtkWidget *, GHashTable *, gchar *);
void bind_to_lists(GtkWidget * , gchar * );
void run_post_function_with_arg(gchar *, GtkWidget *);
void run_post_function(gchar * );
/* Prototypes */

#endif
