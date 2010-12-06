/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <assert.h>
#include <config.h>
#include <configfile.h>
#include <debugging.h>
#include <defines.h>
#include <firmware.h>
#include <string.h>


extern gconstpointer *global_data;

/*!
 \brief get_ecu_data() is a func to return the data requested.
 \param canID, CAN Identifier (currently unused)
 \param page, (ecu firmware page)
 \param offset, (RAW BYTE offset)
 \param size, (size to be returned)
 */
G_MODULE_EXPORT gint get_ecu_data(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	/* Sanity checking */
	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	if ((offset < 0 ) || (offset > firmware->page_params[page]->length))
		return 0;

	return _get_sized_data(firmware->ecu_data[page],page,offset,size,firmware->bigendian);
}


/*!
 \brief get_ecu_data_last() is a func to return the data requested.
 \param canID, CAN Identifier (currently unused)
 \param page, (ecu firmware page)
 \param offset, (RAW BYTE offset)
 \param size, (size to be returned)
 */
G_MODULE_EXPORT gint get_ecu_data_last(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	if ((offset < 0 ) || (offset > firmware->page_params[page]->length))
		return 0;
	return _get_sized_data(firmware->ecu_data_last[page],page,offset,size,firmware->bigendian);
}


/*!
 \brief get_ecu_data_backup() is a func to return the data requested.
 \param canID, canIdentifier (currently unused)
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
G_MODULE_EXPORT gint get_ecu_data_backup(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	if ((offset < 0 ) || (offset > firmware->page_params[page]->length))
		return 0;
	return _get_sized_data(firmware->ecu_data_backup[page],page,offset,size,firmware->bigendian);
}

/*!
 \brief _get_sized_data() is a func to return the data requested.
 The data is casted to the passed type.
 \param data array of data to pull from.
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
G_MODULE_EXPORT gint _get_sized_data(guint8 *data, gint page, gint offset, DataSize size, gboolean bigendian) 
{
	/* canID unused currently */
	gint result = 0;
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;
	Firmware_Details *firmware = NULL;
	assert(offset >= 0);

	firmware = DATA_GET(global_data,"firmware");

	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	switch (size)
	{
		case MTX_CHAR:
		case MTX_U08:
/*			printf("8 bit, returning %i\n",(guint8)data[offset]);*/
			return (guint8)data[offset];
			break;
		case MTX_S08:
			return (gint8)data[offset];
			break;
		case MTX_U16:
			u16 = ((guint8)data[offset] +((guint8)data[offset+1] << 8));
			if (bigendian)
				result = GUINT16_FROM_BE(u16);
			else
				result = GUINT16_FROM_LE(u16);
/*			printf("U16 bit, returning %i\n",result);*/
			return (guint16)result;
			break;
		case MTX_S16:
			s16 = ((guint8)data[offset] +((guint8)data[offset+1] << 8));
			if (bigendian)
				result = GINT16_FROM_BE(s16);
			else
				result = GINT16_FROM_LE(s16);
			return (gint16)result;
/*			printf("S16 bit, returning %i\n",result);*/
			break;
		case MTX_U32:
			u32 = ((guint8)data[offset] +((guint8)data[offset+1] << 8)+((guint8)data[offset+2] << 16)+((guint8)data[offset+3] << 24));
			if (bigendian)
				result = GUINT_FROM_BE(u32);
			else
				result = GUINT_FROM_LE(u32);
			return (guint32)result;
			break;
		case MTX_S32:
			s32 = ((guint8)data[offset] +((guint8)data[offset+1] << 8)+((guint8)data[offset+2] << 16)+((guint8)data[offset+3] << 24));
			if (bigendian)
				result = GINT_FROM_BE(u32);
			else
				result = GINT_FROM_LE(u32);
			return (gint32)result;
			break;
		default:
			break;
	}
	return 0;
}
 

G_MODULE_EXPORT void set_ecu_data(gint canID, gint page, gint offset, DataSize size, gint new) 
{
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	_set_sized_data(firmware->ecu_data[page],offset,size,new,firmware->bigendian);
}


G_MODULE_EXPORT void _set_sized_data(guint8 *data, gint offset, DataSize size, gint new, gboolean bigendian) 
{
	/* canID unused currently */
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;

	if (!data)
		return;
	switch (size)
	{
		case MTX_CHAR:
		case MTX_U08:
			data[offset] = (guint8)new;
			break;
		case MTX_S08:
			data[offset] = (gint8)new;
			break;
		case MTX_U16:
			if (bigendian)
				u16 = GUINT16_TO_BE((guint16)new);
			else
				u16 = GUINT16_TO_LE((guint16)new);
			data[offset] = (guint8)u16;
			data[offset+1] = (guint8)((guint16)u16 >> 8);
			break;
		case MTX_S16:
			if (bigendian)
				s16 = GINT16_TO_BE((gint16)new);
			else
				s16 = GINT16_TO_LE((gint16)new);
			data[offset] = (guint8)s16;
			data[offset+1] = (guint8)((gint16)s16 >> 8);
			break;
		case MTX_U32:
			if (bigendian)
				u32 = GUINT32_TO_BE((guint32)new);
			else
				u32 = GUINT32_TO_LE((guint32)new);
			data[offset] = (guint8)u32;
			data[offset+1] = (guint8)((guint32)u32 >> 8);
			data[offset+2] = (guint8)((guint32)u32 >> 16);
			data[offset+3] = (guint8)((guint32)u32 >> 24);
			break;
		case MTX_S32:
			if (bigendian)
				s32 = GINT32_TO_BE((gint32)new);
			else
				s32 = GINT32_TO_LE((gint32)new);
			data[offset] = (guint8)s32;
			data[offset+1] = (guint8)((gint32)s32 >> 8);
			data[offset+2] = (guint8)((gint32)s32 >> 16);
			data[offset+3] = (guint8)((gint32)s32 >> 24);
			break;
		default:
			printf(_("ERROR! attempted set of data with NO SIZE defined\n"));
	}
}
 
G_MODULE_EXPORT void store_new_block(gint canID, gint page, gint offset, void * buf, gint count)
{
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if (!firmware)
		return;
	if (!firmware->ecu_data)
		return;
	if (!firmware->ecu_data[page])
		return;

	ecu_data = firmware->ecu_data;
	memcpy (ecu_data[page]+offset,buf,count);
}


G_MODULE_EXPORT void backup_current_data(gint canID, gint page)
{
	guint8 ** ecu_data = NULL;
	guint8 ** ecu_data_last = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if (!firmware)
		return;
	if (!firmware->ecu_data)
		return;
	if (!firmware->ecu_data_last)
		return;
	if (!firmware->ecu_data[page])
		return;
	if (!firmware->ecu_data_last[page])
		return;

	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;
	memcpy (ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length);
}


/*!
 \brief find_mtx_page() is a func to return the data requested.
 \param tableID, Table Identified (physical ecu page)
 \param mtx_page, The symbolic page mtx uses to get around the nonlinear
 nature of the page layout in certain MS firmwares
 \returns true on success, false on failure
 */
G_MODULE_EXPORT gboolean find_mtx_page(gint tableID,gint *mtx_page)
{
	Firmware_Details *firmware = NULL;
	gint i = 0;

	firmware = DATA_GET(global_data,"firmware");

	if (!firmware)
		return FALSE;
	if (!firmware->page_params)
		return FALSE;

	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i])
			return FALSE;
		if (firmware->page_params[i]->phys_ecu_page == tableID)
		{
			*mtx_page = i;
			return TRUE;
		}
	}
	return FALSE;
}
