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

#ifndef __DASH_RTV_PARSER_H__
#define __DASH_RTV_PARSER_H__

#include <defines.h>
#include <libxml/parser.h>
#include <gtk/gtk.h>


/* Datastructures */

typedef struct _Rtv_Data Rtv_Data;
typedef struct _Persona_Info Persona_Info;

/*!
  \brief _Rtv_Data holds an array of all RTV vlaues and a hash to mape them to
  specific ECU personalities
  */
struct _Rtv_Data
{
	GHashTable *persona_hash;	/*!< Hashtable of personas */
	GArray *persona_array;		/*!< Array of Personas */
	gint total_files;		/*!< File count */
};

/*!
  \brief _Persona_Info holds information about the persona and its associated
  internal and external name references
  specific ECU personalities
  */
struct _Persona_Info
{
	GHashTable *hash;		/*!< hash of all possible sources */
	GHashTable *int_ext_hash;	/*!< int/ext name hash */
	GList *rtv_list;		/*!< List of RT Variables */
	gchar *persona;			/*!< Personal in use */
};

/*!
  \brief Columns in the datasource listing
  */
enum
{
	VARNAME_COL,
	DATASOURCE_COL,
	NUM_COLS
};

/* Prototypes */

void retrieve_rt_vars(void);
void load_rtvars(gchar **, Rtv_Data *);
gint sort(gconstpointer , gconstpointer );
void load_rtv_defaults(xmlNode *, Rtv_Data *, Persona_Info **);
gboolean parse_rtv_xml_for_dash(xmlNode *, Rtv_Data *);
void parse_derived_var(xmlNode *, Rtv_Data *, Persona_Info *);

		 
/* Prototypes */

#endif
