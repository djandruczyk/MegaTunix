/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Electronic Fuel Injection tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __DASH_XML_H__
#define __DASH_XML_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>



/* Prototypes */

void import_dash_xml(gchar *);
void export_dash_xml(gchar *);
void clear_dashboard(GtkWidget *);
void load_elements(GtkWidget *, xmlNode * );
void load_geometry(GtkWidget *, xmlNode *);
void load_gauge(GtkWidget *, xmlNode *);
void load_integer_from_xml(xmlNode *, gint *);
void load_string_from_xml(xmlNode *, gchar **);

#endif
