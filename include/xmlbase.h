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
  \file include/xmlbase.h
  \ingroup Headers
  \brief Header for the XML base core functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __XMLBASE_H__
#define __XMLBASE_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


/* Prototypes */

/* import funcs */
gboolean generic_xml_gboolean_find(xmlNode *, const gchar *, gpointer);
gboolean generic_xml_gchar_find(xmlNode *, const gchar *, gpointer);
gboolean generic_xml_gfloat_find(xmlNode *, const gchar *, gpointer);
gboolean generic_xml_gint_find(xmlNode *, const gchar *, gpointer);
gboolean generic_xml_color_import(xmlNode *, gpointer);
gboolean generic_xml_gboolean_import(xmlNode *, gpointer);
gboolean generic_xml_gchar_import(xmlNode *, gpointer);
gboolean generic_xml_gint_import(xmlNode *, gpointer);
gboolean generic_xml_gfloat_import(xmlNode *, gpointer);

/* export funcs */
void generic_xml_color_export(xmlNode *, const gchar *, GdkColor *);
void generic_xml_gboolean_export(xmlNode *, const gchar *, gboolean *);
void generic_xml_gchar_export(xmlNode *, const gchar *, gchar **);
void generic_xml_gfloat_export(xmlNode *, const gchar *, gfloat *);
void generic_xml_gint_export(xmlNode *, const gchar *, gint *);

/* Oddball funcs */
gboolean xml_api_check(xmlNode *,gint , gint);

#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
