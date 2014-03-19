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
  \file src/plugins/libreems/libreems_rtv_processor.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS Personality specific RTV processor functions
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <firmware.h>
#include <libreems_plugin.h>
#include <stdio.h>

extern gconstpointer *global_data;


/*!
  \brief processes commen runtime variables
  \param object is the pointer to the object containing the info abot the 
  complex expression
  \param symbol is the name of the symbol
  \param type is the type of rtv expression
  \returns the result
  */
G_MODULE_EXPORT gdouble common_rtv_processor(gconstpointer *object, gchar *symbol, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gint canID = 0;
	gint locID = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint bitmask = 0;
	gint bitshift = 0;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	switch (type)
	{
		case ECU_EMB_BIT:
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_locID",symbol);
			locID = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_bitmask",symbol);
			bitmask = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			bitshift = get_bitshift_f(bitmask);
			/*
			   printf("raw ecu at locID %i, offset %i is %i\n",locID,offset,libreems_get_ecu_data(canID,locID,offset,size));
			   printf("value masked by %i, shifted by %i is %i\n",bitmask,bitshift,(libreems_get_ecu_data(canID,locID,offset,size) & bitmask) >> bitshift);
			 */
			EXIT();
			return ((libreems_get_ecu_data(canID,locID,offset,size) & bitmask) >> bitshift);
			break;
		case ECU_VAR:
			tmpbuf = g_strdup_printf("%s_locID",symbol);
			locID = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_size",symbol);
			size = (DataSize)(GINT) DATA_GET(object,tmpbuf);
			g_free(tmpbuf);
			EXIT();
			return (gdouble)libreems_get_ecu_data(canID,locID,offset,size);
			break;
		default:
			EXIT();
			return 0.0;
			break;
	}
	EXIT();
	return 0.0;
}


/*!
  \brief processes commen runtime variables
  \param object is the pointer to the GObject containing the info abot the 
  complex expression
  \param symbol is the name of the symbol
  \param type is the type of rtv expression
  \returns the result
  */
G_MODULE_EXPORT gdouble common_rtv_processor_obj(GObject *object, gchar *symbol, ComplexExprType type)
{
	static Firmware_Details *firmware = NULL;
	gint canID = 0;
	gint locID = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint bitmask = 0;
	gint bitshift = 0;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_val_if_fail(firmware,0.0);
	g_return_val_if_fail(object,0.0);
	g_return_val_if_fail(symbol,0.0);

	switch (type)
	{
		case ECU_EMB_BIT:
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_locID",symbol);
			locID = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_bitmask",symbol);
			bitmask = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			bitshift = get_bitshift_f(bitmask);
			
			/*
			   printf("raw ecu at locID %i, offset %i is %i\n",locID,offset,libreems_get_ecu_data(canID,locID,offset,size));
			   printf("value masked by %i, shifted by %i is %i\n",bitmask,bitshift,(libreems_get_ecu_data(canID,locID,offset,size) & bitmask) >> bitshift);
			*/ 
			EXIT();
			return ((libreems_get_ecu_data(canID,locID,offset,size) & bitmask) >> bitshift);
			break;
		case ECU_VAR:
			tmpbuf = g_strdup_printf("%s_locID",symbol);
			locID = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_offset",symbol);
			offset = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_canID",symbol);
			canID = (GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("%s_size",symbol);
			size = (DataSize)(GINT) OBJ_GET(object,tmpbuf);
			g_free(tmpbuf);
			EXIT();
			return (gdouble)libreems_get_ecu_data(canID,locID,offset,size);
			break;
		default:
			EXIT();
			return 0.0;
			break;
	}
	EXIT();
	return 0.0;
}
