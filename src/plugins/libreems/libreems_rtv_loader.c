/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/plugins/libreems/libreems_rtv_loader.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS Personality specific runtime var loader handlers
  \author David Andruczyk
  */

#include <firmware.h>
#include <libreems_plugin.h>
#include <xmlbase.h>

extern gconstpointer *global_data;

/*! 
  \brief handles runtime variables with complex ECU/Family specific handlers
  \param object is a pointer to the object to store the data within
  \param cfgfile is a pointer to the ConfigFile structure
  \param section is the section within the file
  \param symbol is the symbol name within the section
  \param type is an enumeration representing the type of RT var
  */
G_MODULE_EXPORT void common_rtv_loader(gconstpointer *object, xmlNode *node, gchar *symbol, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gchar * name = NULL;
	gint tmpi = 0;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	switch (type)
	{
		case ECU_EMB_BIT:
			/* ECU Embedded bitfield 4 params */
			name=NULL;
			name=g_strdup_printf("%s_locID",symbol);

			if (!generic_xml_gint_find(node,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_EMB_BIT, failure looking for:%s\n"),name);
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_offset",symbol);
			if (!generic_xml_gint_find(node,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_EMB_BIT, failure looking for:%s\n"),name);
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_canID",symbol);
			if (!generic_xml_gint_find(node,name,&tmpi))
				tmpi = firmware->canID;
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_bitmask",symbol);
			if (!generic_xml_gint_find(node,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_EMB_BIT, failure looking for:%s\n"),name);
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			break;
		case ECU_VAR:
			/* ECU std variable, canID/locID/offset/size */
			name=NULL;
			name=g_strdup_printf("%s_canID",symbol);
			if (!generic_xml_gint_find(node,name,&tmpi))
				tmpi = firmware->canID;

			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_locID",symbol);
			if (!generic_xml_gint_find(node,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_VAR, failure looking for:%s\n"),name);
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_offset",symbol);
			if (!generic_xml_gint_find(node,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_VAR, failure looking for:%s\n"),name);
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_size",symbol);
			if (!generic_xml_gchar_find(node,name,&tmpbuf))
				tmpi = MTX_U08;
			else
			{
				tmpi = translate_string_f(tmpbuf);
				g_free(tmpbuf);
			}
			DATA_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			break;
		default:
			EXIT();
			return;
			break;
	}
	EXIT();
	return;
}


/*! 
  \brief handles runtime variables with complex ECU/Family specific handlers
  \param object is a pointer to the object to store the data within
  \param cfgfile is a pointer to the ConfigFile structure
  \param section is the section within the file
  \param symbol is the symbol name within the section
  \param type is an enumeration representing the type of RT var
  */
G_MODULE_EXPORT void common_rtv_loader_obj(GObject *object, ConfigFile *cfgfile, gchar * section, gchar *symbol, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gchar * name = NULL;
	gint tmpi = 0;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	switch (type)
	{
		case ECU_EMB_BIT:
			/* ECU Embedded bitfield 4 params */
			name=NULL;
			name=g_strdup_printf("%s_locID",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_EMB_BIT, failure looking for:%s\n"),name);
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_offset",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_EMB_BIT, failure looking for:%s\n"),name);
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_canID",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				tmpi = firmware->canID;
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_bitmask",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_EMB_BIT, failure looking for:%s\n"),name);
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			break;
		case ECU_VAR:
			/* ECU std variable, canID/locID/offset/size */
			name=NULL;
			name=g_strdup_printf("%s_canID",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				tmpi = firmware->canID;

			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_locID",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_VAR, failure looking for:%s\n"),name);
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_offset",symbol);
			if (!cfg_read_int(cfgfile,section,name,&tmpi))
				MTXDBG(RTMLOADER|COMPLEX_EXPR|CRITICAL,_("ECU_VAR, failure looking for:%s\n"),name);
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			name=g_strdup_printf("%s_size",symbol);
			if (!cfg_read_string(cfgfile,section,name,&tmpbuf))
				tmpi = MTX_U08;
			else
			{
				tmpi = translate_string_f(tmpbuf);
				g_free(tmpbuf);
			}
			OBJ_SET(object,name,GINT_TO_POINTER(tmpi));
			g_free(name);
			name=NULL;
			break;
		default:
			EXIT();
			return;
			break;
	}
	EXIT();
	return;
}
