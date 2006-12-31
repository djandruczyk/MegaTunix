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

#ifndef __DASHBOARD_H__
#define __DASHBOARD_H__

#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


/* Prototypes */
void load_dashboard(GtkFileChooser *, gpointer);
void load_elements(GtkWidget *, xmlNode * );
void load_geometry(GtkWidget *, xmlNode *);
void load_gauge(GtkWidget *, xmlNode *);
void load_integer_from_xml(xmlNode *, gint *);
void load_string_from_xml(xmlNode *, gchar **);

/* Prototypes */

#endif
