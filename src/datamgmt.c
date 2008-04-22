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

#include <config.h>
#include <configfile.h>
#include <datamgmt.h>
#include <debugging.h>
#include <firmware.h>
#include <string.h>


extern GObject *global_data;
/*!
 \brief get_ecu_data() is a func to return the data requested.
 \param canID, canIdentifier (currently unused)
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
gint get_ecu_data(gint canID, gint page, gint offset, DataSize size) 
{
	extern Firmware_Details *firmware;
	return _get_sized_data(firmware->ecu_data[page],page,offset,size);
}


/*!
 \brief get_ecu_data_last() is a func to return the data requested.
 \param canID, canIdentifier (currently unused)
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
gint get_ecu_data_last(gint canID, gint page, gint offset, DataSize size) 
{
	extern Firmware_Details *firmware;
	return _get_sized_data(firmware->ecu_data_last[page],page,offset,size);
}


/*!
 \brief get_ecu_data_backup() is a func to return the data requested.
 \param canID, canIdentifier (currently unused)
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
gint get_ecu_data_backup(gint canID, gint page, gint offset, DataSize size) 
{
	extern Firmware_Details *firmware;
	return _get_sized_data(firmware->ecu_data_backup[page],page,offset,size);
}

/*!
 \brief _get_sized_data() is a func to return the data requested.
 The data is casted to the passed type.
 \param data array of data to pull from.
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
gint _get_sized_data(guint8 *data, gint page, gint offset, DataSize size) 
{
	/* canID unused currently */
	extern Firmware_Details *firmware;
	gint result = 0;
	guint8 byte[4];

	switch (size)
	{
		case MTX_CHAR:
		case MTX_U08:
//			printf("8 bit, returning %i\n",(guint8)data[offset]);
			return (guint8)data[offset];
			break;
		case MTX_S08:
			return (gint8)data[offset];
			break;
		case MTX_U16:
			byte[1] = (guint8)data[offset];
			byte[0] = (guint8)data[offset+1];
			result = byte[0] + (byte[1] << 8);
//			printf("U16 bit, returning %i\n",result);
			return (guint16)result;
			break;
		case MTX_S16:
			byte[1] = (guint8)data[offset];
			byte[0] = (guint8)data[offset+1];
			result = byte[0] + (byte[1] << 8);
			return (gint16)result;
//			printf("S16 bit, returning %i\n",result);
			break;
		case MTX_U32:
			byte[3] = (guint8)data[offset];
			byte[2] = (guint8)data[offset+1];
			byte[1] = (guint8)data[offset+2];
			byte[0] = (guint8)data[offset+3];
			result = byte[0] + (byte[1] << 8) + (byte[2] << 16) + (byte[3] << 24);
			return (guint32)result;
			break;
		case MTX_S32:
			byte[3] = (guint8)data[offset];
			byte[2] = (guint8)data[offset+1];
			byte[1] = (guint8)data[offset+2];
			byte[0] = (guint8)data[offset+3];
			result = byte[0] + (byte[1] << 8) + (byte[2] << 16) + (byte[3] << 24);
			return (gint32)result;
			break;
		default:
			break;
	}
	return 0;
}
 

void set_ecu_data(gint canID, gint page, gint offset, DataSize size, gint new) 
{
	/* canID unused currently */
	extern Firmware_Details *firmware;
	guint8 *data = firmware->ecu_data[page];
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
		case MTX_S16:
			data[offset+1] = (guint8)new;
			data[offset] = (guint8)((guint16)new >> 8);
			break;
		case MTX_U32:
		case MTX_S32:
			data[offset+3] = (guint8)new;
			data[offset+2] = (guint8)((guint32)new >> 8);
			data[offset+1] = (guint8)((guint32)new >> 16);
			data[offset] = (guint8)((guint32)new >> 24);
		default:
			printf("ERROR< attempted set of data with NO SIZE defined\n");
	}
}
 
void store_new_block(gint canID, gint page, gint offset, void * buf, gint count)
{
	extern Firmware_Details *firmware;
	guint8 ** ecu_data = firmware->ecu_data;
	memcpy (ecu_data[page]+offset,buf,count);
}


void backup_current_data(gint canID, gint page)
{
	extern Firmware_Details *firmware;
	guint8 ** ecu_data = firmware->ecu_data;
	guint8 ** ecu_data_last = firmware->ecu_data_last;
	memcpy (ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length);
}
