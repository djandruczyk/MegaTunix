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

#ifndef __RTV_MAP_LOADER_H__
#define __RTV_MAP_LOADER_H__

#include <gtk/gtk.h>
#include <configfile.h>


typedef struct _Rtv_Map Rtv_Map;
/*! 
 \brief _RtvMap is the RealTime Variables Map structure, containing fields to
 access the realtime derived data via a hashtable, and via raw index. Stores
 timestamps of each incoming data byte for advanced future use.
 */


struct _Rtv_Map
{
	gint derived_total;	/*! Number of derived variables */
	gint rtvars_size;	/*! total size of rtvars block */
	gchar **raw_list;	/*! Char List of raw variables by name */
	gchar *applicable_signatures;/*! Firmware signatures that use this map*/
	GHashTable *offset_hash;/*! Hashtable of rtv dervied values indexed by
				  it's raw offset in the RTV block */
	GArray *ts_array;	/*! Timestamp array */
	GArray *rtv_list;	/*! List of derived vars IN ORDER */
	GHashTable *rtv_hash;	/*! Hashtable of rtv derived values indexed by
				 * it's internal name */
};


/* Prototypes */
gboolean load_realtime_map(void );
void load_complex_params(GObject *, ConfigFile *, gchar * );
/* Prototypes */

#endif
