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

#include <assert.h>
#include <config.h>
#include <configfile.h>
#include <datamgmt.h>
#include <debugging.h>
#include <defines.h>
#include <firmware.h>
#include <freeems_plugin.h>
#include <string.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT gint get_ecu_data(gpointer data)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint value = 0 ;
	GtkWidget *widget = NULL;
	gconstpointer *container = NULL;
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);

	firmware = DATA_GET(global_data,"firmware");
	widget = (GtkWidget *)data;
	container = (gconstpointer *)data;
	/* Sanity checking */
	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	if ((offset < 0 ) || (offset > firmware->page_params[page]->length))
		return 0;
	if (GTK_IS_WIDGET(widget))
	{
		canID = (GINT)OBJ_GET(widget,"canID");
		page = (GINT)OBJ_GET(widget,"page");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)OBJ_GET(widget,"size");
	}
	else
	{
		canID = (GINT)DATA_GET(container,"canID");
		page = (GINT)DATA_GET(container,"page");
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)DATA_GET(container,"size");
	}

	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief freeems_get_ecu_data() is a func to return the data requested.
 \param canID, CAN Identifier (currently unused)
 \param page, (ecu firmware page)
 \param offset, (RAW BYTE offset)
 \param size, (size to be returned)
 */
G_MODULE_EXPORT gint freeems_get_ecu_data(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);
 

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

	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief freeems_get_ecu_data_last() is a func to return the data requested.
 \param canID, CAN Identifier (currently unused)
 \param page, (ecu firmware page)
 \param offset, (RAW BYTE offset)
 \param size, (size to be returned)
 */
G_MODULE_EXPORT gint freeems_get_ecu_data_last(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);
 

	firmware = DATA_GET(global_data,"firmware");
	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	if ((offset < 0 ) || (offset > firmware->page_params[page]->length))
		return 0;
	return _get_sized_data(firmware->ecu_data_last[page],offset,size,firmware->bigendian);
}


/*!
 \brief freeems_get_ecu_data_backup() is a func to return the data requested.
 \param canID, canIdentifier (currently unused)
 \param page ( ecu firmware page)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
G_MODULE_EXPORT gint freeems_get_ecu_data_backup(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);
 

	firmware = DATA_GET(global_data,"firmware");
	if (!firmware)
		return 0;
	if (!firmware->page_params)
		return 0;
	if (!firmware->page_params[page])
		return 0;
	if ((offset < 0 ) || (offset > firmware->page_params[page]->length))
		return 0;
	return _get_sized_data(firmware->ecu_data_backup[page],offset,size,firmware->bigendian);
}


G_MODULE_EXPORT void set_ecu_data(gconstpointer *data)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint value = 0 ;
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void*)&_set_sized_data);

	firmware = DATA_GET(global_data,"firmware");
	if (GTK_IS_WIDGET(data))
	{
		canID = (GINT)OBJ_GET(data,"canID");
		page = (GINT)OBJ_GET(data,"page");
		offset = (GINT)OBJ_GET(data,"offset");
		value = (GINT)OBJ_GET(data,"value");                
		size = (DataSize)OBJ_GET(data,"size");
	}
	else
	{
		canID = (GINT)DATA_GET(data,"canID");
		page = (GINT)DATA_GET(data,"page");
		offset = (GINT)DATA_GET(data,"offset");
		value = (GINT)DATA_GET(data,"value");                
		size = (DataSize)DATA_GET(data,"size");
	}

	_set_sized_data(firmware->ecu_data[page],offset,size,value,firmware->bigendian);
}


G_MODULE_EXPORT void freeems_set_ecu_data(gint canID, gint page, gint offset, DataSize size, gint new) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void*)&_set_sized_data);


	firmware = DATA_GET(global_data,"firmware");
	_set_sized_data(firmware->ecu_data[page],offset,size,new,firmware->bigendian);
}


G_MODULE_EXPORT void store_new_block(gconstpointer *block)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint count = 0;
	guint8 *data = NULL;
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

	canID = (GINT)DATA_GET(block,"canID");
	page = (GINT)DATA_GET(block,"page");
	offset = (GINT)DATA_GET(block,"offset");
	data = (guint8 *)DATA_GET(block,"data");

	memcpy (ecu_data[page]+offset,data,count);
}


G_MODULE_EXPORT void freeems_store_new_block(gint canID, gint page, gint offset, void * buf, gint count)
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


G_MODULE_EXPORT void freeems_backup_current_data(gint canID, gint page)
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
 nature of the page layout in certain firmwares
 \returns true on success, false on failure
 */
G_MODULE_EXPORT gboolean freeems_find_mtx_page(gint tableID, gint *mtx_page)
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
