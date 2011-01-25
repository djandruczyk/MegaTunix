/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __DASH_RTV_PARSER_H__
#define __DASH_RTV_PARSER_H__

#include <defines.h>
#include <gtk/gtk.h>


/* Datastructures */

typedef struct _Rtv_Data Rtv_Data;
typedef struct _Persona_Info Persona_Info;

struct _Rtv_Data
{
	GHashTable *persona_hash;
	GArray *persona_array;
	gint total_files;
};

struct _Persona_Info
{
	GHashTable *hash;
	GHashTable *int_ext_hash;
	GList *rtv_list;
	gchar *persona;
};

enum
{
	VARNAME_COL,
	PERSONA_COL,
	DATASOURCE_COL,
	NUM_COLS
};

/* Prototypes */

void retrieve_rt_vars(void);
void load_rtvars(gchar **, Rtv_Data *);
gint sort(gconstpointer , gconstpointer );
			 
/* Prototypes */

#endif
