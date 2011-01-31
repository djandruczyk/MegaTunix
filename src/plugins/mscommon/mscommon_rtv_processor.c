/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <config.h>
#include <configfile.h>
#include <datamgmt.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <mscommon_plugin.h>
#include <mscommon_rtv_processor.h>

extern gconstpointer *global_data;

G_MODULE_EXPORT gdouble common_rtv_processor(gconstpointer *object, gchar *symbol, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint size = MTX_U08;
	gint bitmask = 0;
	gint bitshift = 0;
	gchar *tmpbuf = NULL;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	switch (type)
	{
		case ECU_EMB_BIT:
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_page",symbol);
			page = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_bitmask",symbol);
			bitmask = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			bitshift = get_bitshift_f(bitmask);
			/*
			   printf("raw ecu at page %i, offset %i is %i\n",page,offset,ms_get_ecu_data(canID,page,offset,size));
			   printf("value masked by %i, shifted by %i is %i\n",bitmask,bitshift,(ms_get_ecu_data(canID,page,offset,size) & bitmask) >> bitshift);
			 */
			return ((ms_get_ecu_data(canID,page,offset,size) & bitmask) >> bitshift);
			break;
		case ECU_VAR:
			tmpbuf = g_strdup_printf("%s_page",symbol);
			page = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_size",symbol);
			size = (DataSize) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			return (gdouble)ms_get_ecu_data(canID,page,offset,size);
			break;
		default:
			return 0.0;
			break;
	}
	return 0.0;
}


G_MODULE_EXPORT gdouble common_rtv_processor_obj(GObject *object, gchar *symbol, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint size = MTX_U08;
	gint bitmask = 0;
	gint bitshift = 0;
	gchar *tmpbuf = NULL;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	switch (type)
	{
		case ECU_EMB_BIT:
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_page",symbol);
			page = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_bitmask",symbol);
			bitmask = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			bitshift = get_bitshift_f(bitmask);
			/*
			   printf("raw ecu at page %i, offset %i is %i\n",page,offset,ms_get_ecu_data(canID,page,offset,size));
			   printf("value masked by %i, shifted by %i is %i\n",bitmask,bitshift,(ms_get_ecu_data(canID,page,offset,size) & bitmask) >> bitshift);
			 */
			return ((ms_get_ecu_data(canID,page,offset,size) & bitmask) >> bitshift);
			break;
		case ECU_VAR:
			tmpbuf = g_strdup_printf("%s_page",symbol);
			page = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_size",symbol);
			size = (DataSize) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			return (gdouble)ms_get_ecu_data(canID,page,offset,size);
			break;
		default:
			return 0.0;
			break;
	}
	return 0.0;
}
