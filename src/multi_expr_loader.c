/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/multi_expr_loader.c
  \ingroup CoreMtx 
  \brief Deals with RT map vars that are rare special cases that have 
  different math functions based on other variables' state
  \author David Andruczyk
  */

#include <debugging.h>
#include <init.h>
#include <multi_expr_loader.h>
#include <mtxmatheval.h>
#include <keyparser.h>
#include <stdlib.h>
#include <string.h>
#include <xmlbase.h>

/*!
  \brief load_rtv_xml_multi_expressions() is called when a "multi_expr_keys" key is found in
  a realtime map, and triggers the loading of al lthe keys/values that
  will allow megatunix to process a special variable that requires handling of
  multiple circumstances
  \param object is the place to store the retrieved data
  \param node is the xml node that contains the data
  \see check_dependancies
  */
G_MODULE_EXPORT void load_rtv_xml_multi_expressions(gconstpointer *object, xmlNode *node)
{
	gchar *tmpbuf = NULL;
	gchar ** keys = NULL;
	gchar ** l_limits = NULL;
	gchar ** u_limits = NULL;
	gchar ** ltables = NULL;
	gchar ** ul_mults = NULL;
	gchar ** ul_adds = NULL;
	gint num_keys = 0;
	gint i = 0;
	gint lowest = 0;
	gint highest = 0;
	GHashTable *hash = NULL;
	MultiExpr *multi = NULL;

	if (!generic_xml_gchar_find(node,"multi_expr_keys",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Can't find \"multi_expr_keys\" in the xml, exiting!\n"));
		exit (-4);
	}
	else
	{
		keys = parse_keys(tmpbuf,&num_keys,",");
		g_free(tmpbuf);
	}

	if (!generic_xml_gchar_find(node,"lower_limits",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Key \"lower_limits\" NOT FOUND in xml, EXITING!!\n"));
		exit (-4);
	}
	else
	{
		l_limits = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!generic_xml_gchar_find(node,"upper_limits",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Key \"upper_limits\" NOT FOUND in xml, EXITING!!\n"));
		exit (-4);
	}
	else
	{
		u_limits = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!generic_xml_gchar_find(node,"multi_lookuptables",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Key \"multi_lookuptables\" NOT FOUND in xml, EXITING!!\n"));
		exit (-4);
	}
	else
	{
		ltables = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!generic_xml_gchar_find(node,"fromecu_mults",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Key \"fromecu_mults\" NOT FOUND in xml, EXITING!!\n"));
		exit (-4);
	}
	else
	{
		ul_mults = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!generic_xml_gchar_find(node,"fromecu_adds",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Key \"fromecu_adds\" NOT FOUND in xml, EXITING!!\n"));
		exit (-4);
	}
	else
	{
		ul_adds = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!generic_xml_gchar_find(node,"source_key",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Key \"source_key\" NOT FOUND in xml, EXITING!!\n"));
		exit (-4);
	}
	else
	{
		DATA_SET_FULL(object,"source_key",g_strdup(tmpbuf),g_free);
		g_free(tmpbuf);
	}
	/* Create hash table to store structures for each one */
	hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_expr);
	lowest = G_MAXINT32;
	highest = G_MININT32;
	for (i=0;i<num_keys;i++)
	{
		multi = g_new0(MultiExpr, 1);
		multi->lower_limit = (GINT)strtol(l_limits[i],NULL,10);
		multi->upper_limit = (GINT)strtol(u_limits[i],NULL,10);
		if (multi->lower_limit < lowest)
			lowest = multi->lower_limit;
		if (multi->upper_limit > highest)
			highest = multi->upper_limit;

		if (strlen(ltables[i]) == 0)
			multi->lookuptable = NULL;
		else
			multi->lookuptable = g_strdup(ltables[i]);
		multi->fromecu_mult = g_new0(gfloat, 1);
		multi->fromecu_add = g_new0(gfloat, 1);
		*multi->fromecu_mult = (gfloat)g_strtod(ul_mults[i],NULL);
		*multi->fromecu_add = (gfloat)g_strtod(ul_adds[i],NULL);

		g_hash_table_insert(hash,g_strdup(keys[i]),multi);
	}
	DATA_SET_FULL(object,"real_lower",g_strdup_printf("%i",lowest),g_free);
	DATA_SET_FULL(object,"real_upper",g_strdup_printf("%i",highest),g_free);
	g_strfreev(l_limits);
	g_strfreev(u_limits);
	g_strfreev(ltables);
	g_strfreev(ul_mults);
	g_strfreev(ul_adds);
	g_strfreev(keys);
	DATA_SET_FULL(object,"multi_expr_hash",hash,g_hash_table_destroy);
}


/*!
  \brief frees up the resources for a MultiExpr structure
  \param data is the pointer to MultiExpr structure to deallocate
  */
G_MODULE_EXPORT void free_multi_expr(gpointer data)
{
	MultiExpr *multi = (MultiExpr *)data;
	if (!multi)
		return;
	cleanup(multi->fromecu_mult);	
	cleanup(multi->fromecu_add);	
	cleanup(multi->lookuptable);	
	cleanup(multi);
}


/*!
  \brief frees up the resource for a MultiSource structure
  \param data is the pointer to MultiSource structure to deallocate
  */
G_MODULE_EXPORT void free_multi_source(gpointer key, gpointer value, gpointer user_data)
{
	MultiSource *multi = (MultiSource *)value;
	gchar * initial_key = (gchar *)key;
	if (!multi)
		return;
	cleanup(initial_key);
	cleanup(multi->source);	
	cleanup(multi->multiplier);	
	cleanup(multi->adder);	
	cleanup(multi->suffix);
	cleanup(multi->lookuptable);
	if (multi->dl_eval)
		evaluator_destroy(multi->dl_eval);
	if (multi->ul_eval)
		evaluator_destroy(multi->ul_eval);
	cleanup(multi);
}
