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
  \file include/rtv_map_loader.h
  \ingroup Headers
  \brief Header for the runtime var map loader functionality
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __RTV_MAP_LOADER_H__
#define __RTV_MAP_LOADER_H__

#include <gtk/gtk.h>
#include <configfile.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


typedef struct _Rtv_Map Rtv_Map;

/*!
 \brief _RtvMap is the RealTime Variables Map structure, containing fields to
 access the realtime derived data via a hashtable, and via raw index. Stores
 timestamps of each incoming data byte for advanced future use.
 */


struct _Rtv_Map
{
	guint derived_total;	/*!< Number of derived variables */
	guint rtvars_size;	/*!< total size of rtvars block */
	gchar **raw_list;	/*!< Char List of raw variables by name */
	gchar *applicable_signatures;/*!< Firmware revisions that use this map*/
	GHashTable *offset_hash;/*!< Hashtable of rtv dervied values indexed by
				  it's raw offset in the RTV block */
	GArray *ts_array;	/*!< Timestamp array */
	GPtrArray *rtv_list;	/*!< List of derived vars IN ORDER */
	GHashTable *rtv_hash;	/*!< Hashtable of rtv derived values indexed by
				 * it's internal name */
};


/* Prototypes */
void load_complex_params(gconstpointer *, ConfigFile *, gchar * );
void load_complex_params_obj(GObject *, ConfigFile *, gchar * );
void load_derived_var(xmlNode *, Rtv_Map *);
gboolean load_realtime_map_pf(void );
void load_rtv_defaults(xmlNode *, Rtv_Map *);
void load_rtv_xml_complex_expression(gconstpointer *, xmlNode *);
void load_rtv_xml_dependencies(gconstpointer *, xmlNode *);
gboolean load_rtv_xml_elements(xmlNode *, Rtv_Map *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
