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
#include <dep_loader.h>
#include <enums.h>
#include <keyparser.h>
#include <stdlib.h>
#include <stringmatch.h>



extern gint dbg_lvl;
extern GObject *global_data;
/*!
 \brief load_dependancies() is called when a "depend_on" key is found in
 a datamap or realtimemap, and triggers the loading of al lthe keys/values that
 will allow megatunix to process a dependancy (or multiple deps) on other
 variables
 \param object (GObject *) place to store the retrieved data
 \param cfgfile (ConfigFile *) pointer to cfgfile that contains the data
 \param section (gchar *) sectio nto read the data from
 \see check_dependancies
 */
void load_dependancies(GObject *object, ConfigFile *cfgfile,gchar * section)
{
	GObject *dep_obj = NULL;
	gchar *tmpbuf = NULL;
	gchar * key = NULL;
	gchar ** deps = NULL;
	gint num_deps = 0;
	gint tmpi = 0;
	gint type = 0;
	gint i = 0;
	
	if (!cfg_read_string(cfgfile,section,"depend_on",&tmpbuf))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t CAn't find \"depend_on\" in the \"[%s]\" section, exiting!\n",section));
		exit (-4);
	}
	else
	{
		deps = parse_keys(tmpbuf,&num_deps,",");
		g_free(tmpbuf);
	}
	/* Store list of deps.... */
	dep_obj = g_object_new(GTK_TYPE_INVISIBLE,NULL);
	g_object_ref(G_OBJECT(dep_obj));
	gtk_object_sink(GTK_OBJECT(dep_obj));

	OBJ_SET(dep_obj,"deps",deps);
	OBJ_SET(dep_obj,"num_deps",GINT_TO_POINTER(num_deps));

	for (i=0;i<num_deps;i++)
	{
		key = g_strdup_printf("%s_type",deps[i]);
		if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\", EXITING!!\n",key,section));
			exit (-4);
		}
		else
		{
			type = translate_string(tmpbuf);
			g_free(tmpbuf);
			OBJ_SET(dep_obj,key,GINT_TO_POINTER(type));
		}
		g_free(key);
		if (type == VE_EMB_BIT)
		{
			key = g_strdup_printf("%s_page",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
			key = g_strdup_printf("%s_offset",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
			key = g_strdup_printf("%s_size",deps[i]);
			if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			else
			{
				tmpi = translate_string(tmpbuf);
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
				g_free(tmpbuf);
			}
			g_free(key);
			key = g_strdup_printf("%s_bitshift",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
			key = g_strdup_printf("%s_bitmask",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
			key = g_strdup_printf("%s_bitval",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		if (type == VE_VAR)
		{
			key = g_strdup_printf("%s_page",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
			key = g_strdup_printf("%s_offset",deps[i]);
			if (!cfg_read_int(cfgfile,section,key,&tmpi))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\"!!\n",key,section));
			}
			else
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
			key = g_strdup_printf("%s_size",deps[i]);
			if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			else
			{
				tmpi = translate_string(tmpbuf);
				OBJ_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
				g_free(tmpbuf);
			}
			g_free(key);
		}
			
	}
	OBJ_SET(object,"dep_object",(gpointer)dep_obj);

}
