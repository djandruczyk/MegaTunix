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
	gint locID = 0;
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
	if (GTK_IS_WIDGET(widget))
	{
		locID = (GINT)OBJ_GET(widget,"location_id");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)OBJ_GET(widget,"size");
	}
	else
	{
		locID = (GINT)DATA_GET(container,"location_id");
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)DATA_GET(container,"size");
	}
	/* Sanity checking */

	g_return_val_if_fail(freeems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);

	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief freeems_get_ecu_data() is a func to return the data requested.
 \param locID, Location ID (internal to ECU)
 \param offset, (RAW BYTE offset)
 \param size, (size to be returned)
 */
G_MODULE_EXPORT gint freeems_get_ecu_data(gint locID, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	gint page = 0;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);
 

	firmware = DATA_GET(global_data,"firmware");
	/* Sanity checking */
	g_return_val_if_fail(freeems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);

	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief freeems_get_ecu_data_last() is a func to return the data requested.
 \param locID, Location ID (internal to ECU)
 \param offset, (RAW BYTE offset)
 \param size, (size to be returned)
 */
G_MODULE_EXPORT gint freeems_get_ecu_data_last(gint locID, gint offset, DataSize size) 
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);
 

	firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(freeems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);
	return _get_sized_data(firmware->ecu_data_last[page],offset,size,firmware->bigendian);
}


/*!
 \brief freeems_get_ecu_data_backup() is a func to return the data requested.
 \param locID, Location ID (internal to ECU)
 \param offset (RAW BYTE offset)
 \param size (size to be returned...
 */
G_MODULE_EXPORT gint freeems_get_ecu_data_backup(gint locID, gint offset, DataSize size) 
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void*)&_get_sized_data);
 

	firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(freeems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);
	return _get_sized_data(firmware->ecu_data_backup[page],offset,size,firmware->bigendian);
}


G_MODULE_EXPORT void set_ecu_data(gpointer data, gint *new)
{
	gint locID = 0;
	gint page = 0;
	gint offset = 0;
	gint value = 0;
	DataSize size = MTX_U08;
	GtkWidget *widget = NULL;
	gconstpointer *container = NULL;
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void*)&_set_sized_data);

	firmware = DATA_GET(global_data,"firmware");
	widget = (GtkWidget *)data;
	container = (gconstpointer *)data;
	if (GTK_IS_WIDGET(data))
	{
		if (!OBJ_GET(widget,"location_id"))
		{
			page = (GINT)OBJ_GET(widget,"page");
			locID = firmware->page_params[page]->phys_ecu_page;
		}
		else
			locID = (GINT)OBJ_GET(widget,"location_id");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)OBJ_GET(widget,"size");
		if (new)
			value = *new;
		else
			value = (GINT)OBJ_GET(widget,"value");
	}
	else
	{
		if (!DATA_GET(container,"location_id"))
		{
			page = (GINT)DATA_GET(container,"page");
			locID = firmware->page_params[page]->phys_ecu_page;
		}
		else
			locID = (GINT)DATA_GET(container,"location_id");
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)DATA_GET(container,"size");
		if (new)
			value = *new;
		else
			value = (GINT)DATA_GET(container,"value");
	}

	g_return_if_fail(freeems_find_mtx_page(locID, &page));
	g_return_if_fail(firmware);
	g_return_if_fail(firmware->page_params);
	g_return_if_fail(firmware->page_params[page]);
	g_return_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length));
	_set_sized_data(firmware->ecu_data[page],offset,size,value,firmware->bigendian);
}


G_MODULE_EXPORT void freeems_set_ecu_data(gint locID, gint offset, DataSize size, gint new) 
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void*)&_set_sized_data);


	firmware = DATA_GET(global_data,"firmware");
	g_return_if_fail(freeems_find_mtx_page(locID, &page));
	g_return_if_fail(firmware);
	g_return_if_fail(firmware->page_params);
	g_return_if_fail(firmware->page_params[page]);
	g_return_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length));

	_set_sized_data(firmware->ecu_data[page],offset,size,new,firmware->bigendian);
}


G_MODULE_EXPORT void store_new_block(gpointer block)
{
	gint locID = 0;
	gint page = 0;
	gint offset = 0;
	gint count = 0;
	GtkWidget *widget = NULL;
	gconstpointer *container = NULL;
	guint8 *data = NULL;
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;

	g_return_if_fail(firmware);
	g_return_if_fail(ecu_data);

	locID = (GINT)DATA_GET(block,"location_id");
	offset = (GINT)DATA_GET(block,"offset");
	data = (guint8 *)DATA_GET(block,"data");

	g_return_if_fail(freeems_find_mtx_page(locID, &page));
	g_return_if_fail(ecu_data[page]);

	memcpy (ecu_data[page]+offset,data,count);
}


G_MODULE_EXPORT void freeems_store_new_block(gint locID, gint offset, void * buf, gint count)
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;

	g_return_if_fail(firmware);
	g_return_if_fail(freeems_find_mtx_page(locID, &page));
	g_return_if_fail(ecu_data);
	g_return_if_fail(ecu_data[page]);

	memcpy (ecu_data[page]+offset,buf,count);
}


G_MODULE_EXPORT void freeems_backup_current_data(gint locID)
{
	gint page = 0;
	guint8 ** ecu_data = NULL;
	guint8 ** ecu_data_last = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;

	g_return_if_fail(firmware);
	g_return_if_fail(freeems_find_mtx_page(locID, &page));
	g_return_if_fail(firmware->ecu_data);
	g_return_if_fail(firmware->ecu_data_last);
	g_return_if_fail(firmware->ecu_data[page]);
	g_return_if_fail(firmware->ecu_data_last[page]);

	memcpy (ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length);
}


/*!
 \brief freeems_find_mtx_page() is a func to return the data requested.
 \param tableID, Table Identified (physical ecu page)
 \param mtx_page, The symbolic page mtx uses to get around the nonlinear
 nature of the page layout in certain firmwares
 \returns true on success, false on failure
 */
G_MODULE_EXPORT gboolean freeems_find_mtx_page(gint locID, gint *mtx_page)
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
		if (firmware->page_params[i]->phys_ecu_page == locID)
		{
			*mtx_page = i;
			return TRUE;
		}
	}
	return FALSE;
}
