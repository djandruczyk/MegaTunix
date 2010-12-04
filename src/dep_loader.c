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
#include <firmware.h>
#include <keyparser.h>
#include <stdlib.h>
#include <stringmatch.h>
#include <widgetmgmt.h>


extern gconstpointer *global_data;

/*!
 \brief load_dependancies() is called when a "depend_on" key is found in
 a datamap or realtimemap, and triggers the loading of al lthe keys/values that
 will allow megatunix to process a dependancy (or multiple deps) on other
 variables
 \param object (gconstpointer *) place to store the retrieved data
 \param cfgfile (ConfigFile *) pointer to cfgfile that contains the data
 \param section (gchar *) section to read the data from
 \param source_key (gchar *) source key in section to read the data from
 \see check_dependancies
 */
G_MODULE_EXPORT void load_dependancies(gconstpointer *object, ConfigFile *cfgfile,gchar * section, gchar * source_key)
{
	gconstpointer *dep_obj = NULL;
	gchar *tmpbuf = NULL;
	gchar * key = NULL;
	gchar ** deps = NULL;
	gchar ** vector = NULL;
	gint num_deps = 0;
	gint tmpi = 0;
	gint type = 0;
	gint i = 0;
	gint len = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if (!cfg_read_string(cfgfile,section,source_key,&tmpbuf))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Can't find \"%s\" in the \"[%s]\" section, exiting!\n",source_key,section));
		exit (-4);
	}
	else
	{
		deps = parse_keys(tmpbuf,&num_deps,",");
		g_free(tmpbuf);
	}
	/* Store list of deps.... */
	dep_obj = g_new0(gconstpointer, 1);

	DATA_SET_FULL(dep_obj,"deps",deps,g_strfreev);
	DATA_SET(dep_obj,"num_deps",GINT_TO_POINTER(num_deps));

	for (i=0;i<num_deps;i++)
	{
		key = g_strdup_printf("%s",deps[i]);
		if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\", EXITING!!\n",key,section));
			exit (-4);
		}
		else
		{
			vector = g_strsplit(tmpbuf,",",-1);
			len = g_strv_length(vector);
			/* 7 args is VE_EMB_BIT, 4 args is VE_VAR */
			if (!((len == 7) || (len == 4)))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t invalid number of arguments \"%i\" in section \"[%s]\", EXITING!!\n",g_strv_length(vector),section));
				exit (-4);
			}
		}
		g_free(key);
		type = translate_string(vector[DEP_TYPE]);
		key = g_strdup_printf("%s_type",deps[i]);
		DATA_SET(dep_obj,key,GINT_TO_POINTER(type));
		g_free(key);
		if (type == VE_EMB_BIT)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			tmpi = translate_string(vector[DEP_SIZE]);
			if (!check_size(tmpi))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 1 (size) \"%s\" in section \"[%s]\" is missing, using U08 as a guess!!\n",vector[DEP_SIZE],section));
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 2 (page) \"%s\" in section \"[%s]\" is invalid, EXITING!!\n",vector[DEP_PAGE],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 3 (offset) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_OFFSET],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitmask",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITMASK],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 4 (bitmask) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_BITMASK],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitshift",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITSHIFT],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 8))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 5 (bitshift) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_BITSHIFT],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitval",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITVAL],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 6 (bitval) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_BITVAL],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		if (type == VE_VAR)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			tmpi = translate_string(vector[DEP_SIZE]);
			if (!check_size(tmpi))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 1 (size) \"%s\" in section \"[%s]\" is missing, using U08 as a guess!!\n",vector[DEP_SIZE],section));
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 2 (page) \"%s\" in section \"[%s]\" is invalid, EXITING!!\n",vector[DEP_PAGE],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 3 (offset) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_OFFSET],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		g_strfreev(vector);

	}
	DATA_SET(object,"dep_object",(gpointer)dep_obj);

}


/*!
 \brief load_dependancies_obj() is called when a "depend_on" key is found in
 a datamap or realtimemap, and triggers the loading of al lthe keys/values that
 will allow megatunix to process a dependancy (or multiple deps) on other
 variables
 \param object (gconstpointer *) place to store the retrieved data
 \param cfgfile (ConfigFile *) pointer to cfgfile that contains the data
 \param section (gchar *) section to read the data from
 \param source_key (gchar *) source key in section to read the data from
 \see check_dependancies
 */
G_MODULE_EXPORT void load_dependancies_obj(GObject *object, ConfigFile *cfgfile,gchar * section, gchar * source_key)
{
	gconstpointer *dep_obj = NULL;
	gchar *tmpbuf = NULL;
	gchar * key = NULL;
	gchar ** deps = NULL;
	gchar ** vector = NULL;
	gint num_deps = 0;
	gint tmpi = 0;
	gint type = 0;
	gint i = 0;
	gint len = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if (!cfg_read_string(cfgfile,section,source_key,&tmpbuf))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Can't find \"%s\" in the \"[%s]\" section, exiting!\n",source_key,section));
		exit (-4);
	}
	else
	{
		deps = parse_keys(tmpbuf,&num_deps,",");
		g_free(tmpbuf);
	}
	/* Store list of deps.... */

	dep_obj = g_new0(gconstpointer, 1);
	DATA_SET_FULL(dep_obj,"deps",deps,g_strfreev);
	DATA_SET(dep_obj,"num_deps",GINT_TO_POINTER(num_deps));

	for (i=0;i<num_deps;i++)
	{
		key = g_strdup_printf("%s",deps[i]);
		if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Key \"%s\" NOT FOUND in section \"[%s]\", EXITING!!\n",key,section));
			exit (-4);
		}
		else
		{
			vector = g_strsplit(tmpbuf,",",-1);
			len = g_strv_length(vector);
			/* 7 args is VE_EMB_BIT, 4 args is VE_VAR */
			if (!((len == 7) || (len == 4)))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t invalid number of arguments \"%i\" in section \"[%s]\", EXITING!!\n",g_strv_length(vector),section));
				exit (-4);
			}
		}
		g_free(key);
		type = translate_string(vector[DEP_TYPE]);
		key = g_strdup_printf("%s_type",deps[i]);
		DATA_SET(dep_obj,key,GINT_TO_POINTER(type));
		g_free(key);
		if (type == VE_EMB_BIT)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			tmpi = translate_string(vector[DEP_SIZE]);
			if (!check_size(tmpi))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 1 (size) \"%s\" in section \"[%s]\" is missing, using U08 as a guess!!\n",vector[DEP_SIZE],section));
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 2 (page) \"%s\" in section \"[%s]\" is invalid, EXITING!!\n",vector[DEP_PAGE],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 3 (offset) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_OFFSET],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitmask",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITMASK],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 4 (bitmask) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_BITMASK],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitshift",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITSHIFT],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 8))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 5 (bitshift) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_BITSHIFT],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitval",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITVAL],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 6 (bitval) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_BITVAL],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		if (type == VE_VAR)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			tmpi = translate_string(vector[DEP_SIZE]);
			if (!check_size(tmpi))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 1 (size) \"%s\" in section \"[%s]\" is missing, using U08 as a guess!!\n",vector[DEP_SIZE],section));
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 2 (page) \"%s\" in section \"[%s]\" is invalid, EXITING!!\n",vector[DEP_PAGE],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_dependancy()\n\t Argument 3 (offset) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n",vector[DEP_OFFSET],section));
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		g_strfreev(vector);

	}
	OBJ_SET_FULL(object,"dep_object",(gpointer)dep_obj,g_dataset_destroy);
}
