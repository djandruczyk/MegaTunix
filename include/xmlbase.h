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

#ifndef __XMLBASE_H__
#define __XMLBASE_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


/* Prototypes */

/* import funcs */
void generic_xml_gint_import(xmlNode *, gpointer);
void generic_xml_gchar_import(xmlNode *, gpointer);
void generic_xml_gfloat_import(xmlNode *, gpointer);
void generic_xml_gboolean_import(xmlNode *, gpointer);
void generic_xml_color_import(xmlNode *, gpointer);

/* export funcs */
void generic_xml_gint_export(xmlNode *, gchar *, gint *);
void generic_xml_gchar_export(xmlNode *, gchar *, gchar **);
void generic_xml_gfloat_export(xmlNode *, gchar *, gfloat *);
void generic_xml_gboolean_export(xmlNode *, gchar *, gboolean *);
void generic_xml_color_export(xmlNode *, gchar *, GdkColor *);

#endif
