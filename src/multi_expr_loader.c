/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <config.h>
#include <configfile.h>
#include <debugging.h>
#include <defines.h>
#include <multi_expr_loader.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <enums.h>
#include <keyparser.h>
#include <stdlib.h>
#include <stringmatch.h>



extern gint dbg_lvl;
/*!
 \brief multi_exprt_loader() is called when a "multi_expr_keys" key is found in
 a realtimemap, and triggers the loading of al lthe keys/values that
 will allow megatunix to process a special variable that requires handling of
 multiple circumstances
 \param object (GObject *) place to store the retrieved data
 \param cfgfile (ConfigFile *) pointer to cfgfile that contains the data
 \param section (gchar *) sectio nto read the data from
 \see check_dependancies
 */
void load_multi_expressions(GObject *object, ConfigFile *cfgfile,gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar ** keys = NULL;
	gchar ** l_limits = NULL;
	gchar ** u_limits = NULL;
	gchar ** lookuptables = NULL;
	gchar ** dl_exprs = NULL;
	gchar ** ul_exprs = NULL;
	gint num_keys = 0;
	gint i = 0;
	GHashTable *hash = NULL;
	MultiExpr *multi = NULL;

	if (!cfg_read_string(cfgfile,section,"multi_expr_keys",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_multi_expressions()\n\t Can't find \"multi_expr_keys\" in the \"[%s]\" section, exiting!\n",section));
		exit (-4);
	}
	else
	{
		keys = parse_keys(tmpbuf,&num_keys,",");
		g_free(tmpbuf);
	}



	if (!cfg_read_string(cfgfile,section,"lower_limits",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_multi_expression()\n\t Key \"lower_limits\" NOT FOUND in section \"[%s]\", EXITING!!\n",section));
		exit (-4);
	}
	else
	{
		l_limits = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!cfg_read_string(cfgfile,section,"upper_limits",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_multi_expression()\n\t Key \"upper_limits\" NOT FOUND in section \"[%s]\", EXITING!!\n",section));
		exit (-4);
	}
	else
	{
		u_limits = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!cfg_read_string(cfgfile,section,"multi_lookuptables",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_multi_expression()\n\t Key \"multi_lookuptables\" NOT FOUND in section \"[%s]\", EXITING!!\n",section));
		exit (-4);
	}
	else
	{
		lookuptables = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!cfg_read_string(cfgfile,section,"dl_conv_exprs",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_multi_expression()\n\t Key \"multi_lookuptables\" NOT FOUND in section \"[%s]\", EXITING!!\n",section));
		exit (-4);
	}
	else
	{
		dl_exprs = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	if (!cfg_read_string(cfgfile,section,"ul_conv_exprs",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_multi_expression()\n\t Key \"multi_lookuptables\" NOT FOUND in section \"[%s]\", EXITING!!\n",section));
		exit (-4);
	}
	else
	{
		ul_exprs = g_strsplit(tmpbuf,",",-1);
		g_free(tmpbuf);
	}
	/* Create hash table to store structures for each one */
	hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_multi_expr);
	for (i=0;i<num_keys;i++)
	{
		multi = g_new0(MultiExpr, 1);
		multi->lower_limit = (gint)strtol(l_limits[i],NULL,10);
		multi->upper_limit = (gint)strtol(u_limits[i],NULL,10);
		multi->lookuptable = g_strdup(lookuptables[i]);
		multi->dl_conv_expr = g_strdup(dl_exprs[i]);
		multi->ul_conv_expr = g_strdup(ul_exprs[i]);
		multi->dl_eval = evaluator_create(multi->dl_conv_expr);
		multi->ul_eval = evaluator_create(multi->ul_conv_expr);
		g_hash_table_insert(hash,g_strdup(keys[i]),multi);
	}
	g_strfreev(l_limits);
	g_strfreev(u_limits);
	g_strfreev(lookuptables);
	g_strfreev(dl_exprs);
	g_strfreev(ul_exprs);
	g_strfreev(keys);
	g_object_set_data(G_OBJECT(object),"multi_expr_hash",hash);

}

void free_multi_expr(gpointer data)
{
	MultiExpr *multi = (MultiExpr *)data;
	if (multi->dl_conv_expr)
		g_free(multi->dl_conv_expr);	
	if (multi->ul_conv_expr)
		g_free(multi->ul_conv_expr);	
	if (multi->lookuptable)
		g_free(multi->lookuptable);	
	if (multi->dl_eval)
		evaluator_destroy(multi->dl_eval);
	if (multi->ul_eval)
		evaluator_destroy(multi->ul_eval);
}


void free_multi_source(gpointer data)
{
	MultiSource *multi = (MultiSource *)data;
	if (multi->source)
		g_free(multi->source);	
	if (multi->conv_expr)
		g_free(multi->conv_expr);	
	if (multi->evaluator)
		evaluator_destroy(multi->evaluator);
	if (multi->suffix)
		g_free(multi->suffix);
	g_free(multi);
}
