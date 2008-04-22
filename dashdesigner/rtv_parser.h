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

#ifndef __DASH_RTV_PARSER_H__
#define __DASH_RTV_PARSER_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


/* Datastructures */
struct Rtv_Data
{
	GList *rtv_list;
	GHashTable *rtv_hash;
	GHashTable *int_ext_hash;
	gint total_files;
};

enum
{
	VARNAME_COL,
	TYPE_COL,
	DATASOURCE_COL,
	NUM_COLS
};

/* Prototypes */

void retrieve_rt_vars(void);
void load_rtvars(gchar **, struct Rtv_Data *);
gint sort(gconstpointer , gconstpointer );
			 
/* Prototypes */

#endif
