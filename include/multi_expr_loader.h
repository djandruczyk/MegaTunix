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
  \file include/multi_expr_loader.h
  \ingroup Headers
  \brief Header for the multi expression/source loader functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MULTI_EXPR_LOADER_H__
#define __MULTI_EXPR_LOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>


typedef struct _MultiExpr MultiExpr;
typedef struct _MultiSource MultiSource;


/*!
 * \brief _MultiExpr is a container struct used for Realtime var processing
 * for vars that depend on ECU state (MAP, need to know current sensor)
 */
struct _MultiExpr
{
	gint lower_limit;	/*!< Lower limit */	
	gint upper_limit;	/*!< Upper limit */
	gchar *lookuptable;	/*!< textual lookuptable name */
	gfloat *fromecu_mult;	/*!< upload (from ecu) conv multiplier */
	gfloat *fromecu_add;	/*!< upload (from ecu) conv adder */
};


/*!
 * \brief _MultiSource is a container struct used for Table data handling 
 * for the x/y/z axis's for properly scaling and displaying things on the
 * 3D displays as well as the 2D table scaling.  This allows things to be
 * significantly more adaptable 
 */
struct _MultiSource
{
	gint table_num;		/*!< Table number associated with this */
	gchar *source;		/*!< name of rtvars datasource */
	gfloat *multiplier;	/*!< conversion multipler ms units to real */
	gfloat *adder;		/*!< conversion adder ms units to real */
	void * ul_eval;		/*!< evaluator pointer */
	void * dl_eval;		/*!< evaluator pointer */
	gchar * suffix;		/*!< textual suffix for this evaluator*/
	gchar *lookuptable;	/*!< lookuptable */
	gint precision;		/*!< Precision for floating point */
};


/* Prototypes */
void free_multi_expr(gpointer);
void free_multi_source(gpointer, gpointer, gpointer);
MultiSource * get_multi(gint, GHashTable *, const gchar *);
void load_rtv_xml_multi_expressions(gconstpointer *, xmlNode *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
