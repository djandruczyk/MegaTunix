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
  \file src/plugins/mscommon/dep_loader.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS Specific dependancy management functions (common to all MS's)
  \author David Andruczyk
  */

#include <dep_loader.h>
#include <firmware.h>
#include <mscommon_plugin.h>
#include <stdlib.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*!
 \brief load_dependencies() is called when a "depend_on" key is found in
 a datamap or realtime map, and triggers the loading of all the 
 keys/values that will allow megatunix to process a dependancy 
 (or multiple deps) on other variables
 \param object is a pointer to a place to store the retrieved data
 \param node is the pointer to the XML node that contains the data
 \param source_key is the source key in the above section to read the data from
 \see check_dependencies
 */
G_MODULE_EXPORT void load_dependencies(gconstpointer *object, xmlNode *node, const gchar * source_key)
{
	gconstpointer *dep_obj = NULL;
	gchar *tmpbuf = NULL;
	gchar * key = NULL;
	gchar ** deps = NULL;
	gchar ** vector = NULL;
	gint num_deps = 0;
	gint tmpi = 0;
	DataSize size = MTX_U08;
	gint type = 0;
	gint i = 0;
	gint len = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!generic_xml_gchar_find(node,source_key,&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Can't find \"%s\" in the rtv XML, exiting!\n"),source_key);
		exit (-4);
	}
	else
	{
		deps = parse_keys_f(tmpbuf,&num_deps,",");
		g_free(tmpbuf);
	}
	/* Store list of deps.... */
	dep_obj = (gconstpointer *)g_new0(gconstpointer, 1);

	DATA_SET_FULL(dep_obj,"deps",deps,g_strfreev);
	DATA_SET(dep_obj,"num_deps",GINT_TO_POINTER(num_deps));

	for (i=0;i<num_deps;i++)
	{
		key = g_strdup_printf("%s",deps[i]);
		if (!generic_xml_gchar_find(node,key,&tmpbuf))
		{
			MTXDBG(CRITICAL,_("Key \"%s\" NOT FOUND in RTV XML, EXITING!!\n"),key);
			exit (-4);
		}
		else
		{
			vector = g_strsplit(tmpbuf,",",-1);
			g_free(tmpbuf);
			len = g_strv_length(vector);
			/* 7 args is ECU_EMB_BIT, 4 args is ECU_VAR */
			if (!((len == 7) || (len == 4)))
			{
				MTXDBG(CRITICAL,_("Invalid number of arguments \"%i\" in RTV XML, EXITING!!\n"),g_strv_length(vector));
				exit (-4);
			}
		}
		g_free(key);
		type = translate_string_f(vector[DEP_TYPE]);
		key = g_strdup_printf("%s_type",deps[i]);
		DATA_SET(dep_obj,key,GINT_TO_POINTER(type));
		g_free(key);
		if (type == ECU_EMB_BIT)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			size = (DataSize)translate_string_f(vector[DEP_SIZE]);
			if (!check_size(size))
			{
				MTXDBG(CRITICAL,_("Argument 1 (size) \"%s\" in RTV XML is missing, using U08 as a guess!!\n"),vector[DEP_SIZE]);
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER((GINT)size));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				MTXDBG(CRITICAL,_("Argument 2 (page) \"%s\" in RTV XML is invalid, EXITING!!\n"),vector[DEP_PAGE]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				MTXDBG(CRITICAL,_("Argument 3 (offset) \"%s\" in RTV XML is out of bounds, EXITING!!\n"),vector[DEP_OFFSET]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitmask",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITMASK],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				MTXDBG(CRITICAL,("Argument 4 (bitmask) \"%s\" in RTV XML is out of bounds, EXITING!!\n"),vector[DEP_BITMASK]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitshift",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITSHIFT],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 8))
			{
				MTXDBG(CRITICAL,_("Argument 5 (bitshift) \"%s\" in RTV XML is out of bounds, EXITING!!\n"),vector[DEP_BITSHIFT]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitval",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITVAL],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				MTXDBG(CRITICAL,_("Argument 6 (bitval) \"%s\" in RTV XML is out of bounds, EXITING!!\n"),vector[DEP_BITVAL]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		if (type == ECU_VAR)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			size = (DataSize)translate_string_f(vector[DEP_SIZE]);
			if (!check_size(size))
			{
				MTXDBG(CRITICAL,_("Argument 1 (size) \"%s\" in RTV XML is missing, using U08 as a guess!!\n"),vector[DEP_SIZE]);
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER((GINT)size));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				MTXDBG(CRITICAL,_("Argument 2 (page) \"%s\" in RTV XML is invalid, EXITING!!\n"),vector[DEP_PAGE]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				MTXDBG(CRITICAL,_("Argument 3 (offset) \"%s\" in RTV XML is out of bounds, EXITING!!\n"),vector[DEP_OFFSET]);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		g_strfreev(vector);

	}
	DATA_SET_FULL(object,"dep_object",(gpointer)dep_obj,g_dataset_destroy);
	EXIT();
	return;
}


/*!
 \brief load_dependencies_obj() is called when a "depend_on" key is found in
 a datamap or realtimemap, and triggers the loading of all of the keys/values that
 will allow megatunix to process a dependancy (or multiple deps) on other
 variables
 \param object is a pointer to a place to store the retrieved data
 \param cfgfile is the pointer to cfgfile that contains the data
 \param section is the section to read the data from
 \param source_key is the source key in section to read the data from
 \see check_dependencies
 */
G_MODULE_EXPORT void load_dependencies_obj(GObject *object, ConfigFile *cfgfile,const gchar * section, const gchar * source_key)
{
	gconstpointer *dep_obj = NULL;
	gchar *tmpbuf = NULL;
	gchar * key = NULL;
	gchar ** deps = NULL;
	gchar ** vector = NULL;
	DataSize size = MTX_U08;
	gint num_deps = 0;
	gint tmpi = 0;
	gint type = 0;
	gint i = 0;
	gint len = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!cfg_read_string(cfgfile,section,source_key,&tmpbuf))
	{
		MTXDBG(CRITICAL,_("Can't find \"%s\" in the \"[%s]\" section, exiting!\n"),source_key,section);
		exit (-4);
	}
	else
	{
		deps = parse_keys_f(tmpbuf,&num_deps,",");
		g_free(tmpbuf);
	}
	dep_obj = g_new0(gconstpointer, 1);
	/* Store list of deps.... */
	DATA_SET_FULL(dep_obj,"deps",deps,g_strfreev);
	DATA_SET(dep_obj,"num_deps",GINT_TO_POINTER(num_deps));

	for (i=0;i<num_deps;i++)
	{
		key = g_strdup_printf("%s",deps[i]);
		if (!cfg_read_string(cfgfile,section,key,&tmpbuf))
		{
			MTXDBG(CRITICAL,_("Key \"%s\" NOT FOUND in section \"[%s]\", EXITING!!\n"),key,section);
			exit (-4);
		}
		else
		{
			vector = g_strsplit(tmpbuf,",",-1);
			g_free(tmpbuf);
			len = g_strv_length(vector);
			/* 7 args is ECU_EMB_BIT, 4 args is ECU_VAR */
			if (!((len == 7) || (len == 4)))
			{
				MTXDBG(CRITICAL,_("Invalid number of arguments \"%i\" in section \"[%s]\", EXITING!!\n"),g_strv_length(vector),section);
				exit (-4);
			}
		}
		g_free(key);
		type = translate_string_f(vector[DEP_TYPE]);
		key = g_strdup_printf("%s_type",deps[i]);
		DATA_SET(dep_obj,key,GINT_TO_POINTER(type));
		g_free(key);
		if (type == ECU_EMB_BIT)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			size = (DataSize)translate_string_f(vector[DEP_SIZE]);
			if (!check_size(size))
			{
				MTXDBG(CRITICAL,_("Argument 1 (size) \"%s\" in section \"[%s]\" is missing, using U08 as a guess!!\n"),vector[DEP_SIZE],section);
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(size));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				MTXDBG(CRITICAL,_("Argument 2 (page) \"%s\" in section \"[%s]\" is invalid, EXITING!!\n"),vector[DEP_PAGE],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				MTXDBG(CRITICAL,_("Argument 3 (offset) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n"),vector[DEP_OFFSET],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitmask",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITMASK],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				MTXDBG(CRITICAL,_("Argument 4 (bitmask) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n"),vector[DEP_BITMASK],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitshift",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITSHIFT],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 8))
			{
				MTXDBG(CRITICAL,_("Argument 5 (bitshift) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n"),vector[DEP_BITSHIFT],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_bitval",deps[i]);
			tmpi = (gint)strtol(vector[DEP_BITVAL],NULL,10);
			if ((tmpi < 0 ) || (tmpi > 255))
			{
				MTXDBG(CRITICAL,_("Argument 6 (bitval) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n"),vector[DEP_BITVAL],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

		}
		if (type == ECU_VAR)
		{
			key = g_strdup_printf("%s_size",deps[i]);
			size = (DataSize)translate_string_f(vector[DEP_SIZE]);
			if (!check_size(size))
			{
				MTXDBG(CRITICAL,_("Argument 1 (size) \"%s\" in section \"[%s]\" is missing, using U08 as a guess!!\n"),vector[DEP_SIZE],section);
				DATA_SET(dep_obj,key,GINT_TO_POINTER(MTX_U08));
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(size));
			g_free(key);

			key = g_strdup_printf("%s_page",deps[i]);
			tmpi = (gint)strtol(vector[DEP_PAGE],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->total_pages))
			{
				MTXDBG(CRITICAL,_("Argument 2 (page) \"%s\" in section \"[%s]\" is invalid, EXITING!!\n"),vector[DEP_PAGE],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);

			key = g_strdup_printf("%s_offset",deps[i]);
			tmpi = (gint)strtol(vector[DEP_OFFSET],NULL,10);
			if ((tmpi < 0 ) || (tmpi > firmware->page_params[(gint)strtol(vector[DEP_PAGE],NULL,10)]->length))
			{
				MTXDBG(CRITICAL,_("Argument 3 (offset) \"%s\" in section \"[%s]\" is out of bounds, EXITING!!\n"),vector[DEP_OFFSET],section);
				exit (-4);
			}
			else
				DATA_SET(dep_obj,key,GINT_TO_POINTER(tmpi));
			g_free(key);
		}
		g_strfreev(vector);
	}
	OBJ_SET_FULL(object,"dep_object",(gpointer)dep_obj,g_dataset_destroy);
	EXIT();
	return;
}


G_MODULE_EXPORT gboolean check_size(DataSize size)
{
	ENTER();
	switch (size)
	{
		case MTX_CHAR:
		case MTX_U08:
		case MTX_S08:
		case MTX_U16:
		case MTX_S16:
		case MTX_U32:
		case MTX_S32:
			EXIT();
			return TRUE;
			break;
		default:
			EXIT();
			return FALSE;
			break;
	}
	EXIT();
	return FALSE;
}
