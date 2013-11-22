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
  \file include/tabloader.h
  \ingroup Headers
  \brief Header for the gui/tabloader core functionality
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TABLOADER_H__
#define __TABLOADER_H__

#include <configfile.h>
#include <glade/glade-parser.h>
#include <gtk/gtk.h>

typedef struct _Group Group;
typedef struct _BindGroup BindGroup;
typedef struct _TabInfo TabInfo;


/*!
 \brief _Group holds common settings from groups of control as defined in a 
 datamap file.  This should reduce redundancy and significantly shrink the 
 datamap files.
 */
struct _Group
{
	gchar **keys;		/*!< String array for key names */
	gint *keytypes;		/*!< Int array of key types... */
	GObject *object;	/*!< To hold the data cleanly */
	gint num_keys;		/*!< How many keys we hold */
	gint num_keytypes;	/*!< How many keytypes we hold */
	gint page;		/*!< page of this group of data */
	gint canID;		/*!< can_id of this group of data */
};


/*!
 \brief _BindGroup is a small container used to pass multiple params into
 a function that is limited to a certain number of arguments...
 */
struct _BindGroup
{
	ConfigFile *cfgfile;	/*!< where the configfile ptr goes... */
	GHashTable *groups;	/*!< where the groups table goes */
	gchar * map_file;	/*!< Tab datamap file */
	GList *widget_list;	/*!< List of widgets in this tab */
	GtkWidget *topframe;	/*!< Pointer to top frame for this tab */
};


/*!
 \brief _TabInfo is a small container used to pass the glade/datamap info into
 a thread that does the backgroud dependancy loading.
 */
struct _TabInfo
{
	gchar *glade_file;	/*!< Glade file */
	gchar *datamap_file;	/*!< Datamap file that goes with it */
	GtkNotebook *notebook;	/*!< Ptr to notebook */
	GtkWidget *tab_label;	/*!< Tab Label widget */
	gint page_num;		/*!< The number of this tab */
};


/* Prototypes */
void bind_data(GtkWidget *, gpointer);
gint bind_group_data(ConfigFile *,GObject *, GHashTable *, const gchar *);
void bind_to_lists(GtkWidget * , const gchar * );
void group_free(gpointer );
gboolean handle_dependant_tab_load(gchar *);
gboolean load_actual_tab(GtkNotebook *,gint);
GHashTable * load_groups(ConfigFile *);
gboolean load_gui_tabs_pf(void);
gboolean preload_deps(gpointer);
void remove_from_lists(const gchar *, gpointer);
void run_post_functions(const gchar * );
void run_post_functions_with_arg(const gchar *, GtkWidget *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
