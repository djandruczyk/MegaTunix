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
  \file src/plugins/mscommon/datamgmt.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS Specific data management functions (common to all MS's)
  \author David Andruczyk
  */

#include <firmware.h>
#include <mscommon_plugin.h>
#include <string.h>


extern gconstpointer *global_data;

/*!
  \brief A generic function to returns the data from the representation of
  ECU memory. This will call an ECU specific function to do the heavy lifting
  \param data is either a GtkWidget pointer or a gconstpointer which contains
  name:value associations describng the location to return
  \returns the value requested or 0 if error
*/
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
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
	g_return_val_if_fail(_get_sized_data,0);

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	/* Sanity checking */
	g_return_val_if_fail(data,0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);

	widget = (GtkWidget *)data;
	container = (gconstpointer *)data;
	if (GTK_IS_WIDGET(widget))
	{
		canID = (GINT)OBJ_GET(widget,"canID");
		page = (GINT)OBJ_GET(widget,"page");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
	}
	else
	{
		canID = (GINT)DATA_GET(container,"canID");
		page = (GINT)DATA_GET(container,"page");
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)(GINT)DATA_GET(container,"size");
	}
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)),0);

	EXIT();
	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief MegaSquirt specific wrapper function to call the firmwre/ECU specific
 handler to return the requested data
 \param canID is the CAN Identifier (currently unused)
 \param page is the ecu firmware page
 \param offset is the RAW BYTE offset
 \param size is the size to be returned
 \returns thedata requested or 0 if error
 */
G_MODULE_EXPORT gint ms_get_ecu_data(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
	g_return_val_if_fail(_get_sized_data,0);
 

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	/* Sanity checking */
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(page < firmware->total_pages,0);
	g_return_val_if_fail(page >= 0,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	/*printf("offset %i, page[%i] length %i\n",offset,page,firmware->page_params[page]->length);*/
	g_return_val_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)),0);

	EXIT();
	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief Megasquirrt specific function to return the data from the "last" buffer
 \param canID is the CAN Identifier (currently unused)
 \param page is the ecu firmware page
 \param offset is the RAW BYTE offset
 \param size is the size to be returned
 \returns the value requested or 0 on error
 */
G_MODULE_EXPORT gint ms_get_ecu_data_last(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
	g_return_val_if_fail(_get_sized_data,0);
 

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(page < firmware->total_pages,0);
	g_return_val_if_fail(page >= 0,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)),0);
	EXIT();
	return _get_sized_data(firmware->ecu_data_last[page],offset,size,firmware->bigendian);
}


/*!
 \brief Megasquirrt specific function to return the data from the "backup" 
 buffer
 \param canID is the can Identifier (currently unused)
 \param page is the ecu firmware page
 \param offset is the RAW BYTE offset
 \param size is the size to be returned...
 \returns the data requested or 0 on error
 */
G_MODULE_EXPORT gint ms_get_ecu_data_backup(gint canID, gint page, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
	g_return_val_if_fail(_get_sized_data,0);
 

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(page < firmware->total_pages,0);
	g_return_val_if_fail(page >= 0,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)),0);
	EXIT();
	return _get_sized_data(firmware->ecu_data_backup[page],offset,size,firmware->bigendian);
}


/*!
  \brief Sets the ECU data at the coordinates specified in the data pointer
  \param data is the pointer to either a GtkWidget pointer or a 
  gconstpointer object container the coordinate information as to where 
  to store the new data.
  \param new is the pointer to the new data to be stored
  */
G_MODULE_EXPORT void set_ecu_data(gpointer data, gint *new_data)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint value = 0;
	gconstpointer *container = NULL;
	GtkWidget *widget = NULL;
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	ENTER();
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void **)&_set_sized_data);

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);
	g_return_if_fail(_set_sized_data);

	widget = (GtkWidget *)data;
	container = (gconstpointer *)data;
	if (GTK_IS_WIDGET(widget))
	{
		canID = (GINT)OBJ_GET(widget,"canID");
		page = (GINT)OBJ_GET(widget,"page");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
		if (new_data)
			value = *new_data;
		else
			value = (GINT)OBJ_GET(widget,"value");
	}
	else
	{
		canID = (GINT)DATA_GET(container,"canID");
		page = (GINT)DATA_GET(container,"page");
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)(GINT)DATA_GET(container,"size");
		if (new_data)
			value = *new_data;
		else
			value = (GINT)DATA_GET(container,"value");
	}
	g_return_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)));

	_set_sized_data(firmware->ecu_data[page],offset,size,value,firmware->bigendian);
	EXIT();
	return;
}


/*!
  \brief ECU specific function to set a value in the representation of ECU 
  memory
  \param canID is the CAN Identifier
  \param page is the MTX page(may not be the same as the ECU page)
  \param offset is the offset in bytes from the beginning of this page
  \param size is the size representation enumeration
  \param new_data is the new value to store
  */
G_MODULE_EXPORT void ms_set_ecu_data(gint canID, gint page, gint offset, DataSize size, gint new_data) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void **)&_set_sized_data);
	g_return_if_fail(_set_sized_data);
	g_return_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)));

	_set_sized_data(firmware->ecu_data[page],offset,size,new_data,firmware->bigendian);
	EXIT();
	return;
}


/*!
  \brief Generic function to store a blob of data to a specific location in
  ECU representative memory. The block variable must contain the necessary
  fields in order to store properly
  \param block is a pointer to a gconstpointer containing the page, 
  offset, length and data to store
  */
G_MODULE_EXPORT void store_new_block(gconstpointer *block)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint num_bytes = 0;
	guint8 *data = NULL;
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(firmware->ecu_data);

	ecu_data = firmware->ecu_data;

	canID = (GINT)DATA_GET(block,"canID");
	page = (GINT)DATA_GET(block,"page");
	offset = (GINT)DATA_GET(block,"offset");
	num_bytes = (GINT)DATA_GET(block,"num_bytes");
	data = (guint8 *)DATA_GET(block,"data");

	g_return_if_fail(page <= firmware->total_pages);
	g_return_if_fail(firmware->ecu_data[page]);
	g_return_if_fail(((offset >= 0 ) && (offset < firmware->page_params[page]->length)));
	memcpy (ecu_data[page]+offset,data,num_bytes);
	EXIT();
	return;
}

/*!
  \brief ECU specific function to store a new blob of data at a specific 
  location in ECU representative memory
  \param canID is the CAN identifier
  \param page is the MTX ecu page (may not necessarily match the phys ECU page)
  \param offset is the byte offset from the beginning of the Location ID
  \param buf is the pointer to the buffer to copy from
  \param count is the number of bytes to copy to the destination
  */
G_MODULE_EXPORT void ms_store_new_block(gint canID, gint page, gint offset, void * buf, gint count)
{
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(firmware->ecu_data);
	g_return_if_fail(page <= firmware->total_pages);
	g_return_if_fail(firmware->ecu_data[page]);

	ecu_data = firmware->ecu_data;
	memcpy (ecu_data[page]+offset,buf,count);
	EXIT();
	return;
}


/*!
  \brief copies current ECU memory representation to backup buffer
  \param canID is unused
  \param page is the Mtx page (may not necessarily match the phys ECU page)
  */
G_MODULE_EXPORT void ms_backup_current_data(gint canID, gint page)
{
	guint8 ** ecu_data = NULL;
	guint8 ** ecu_data_last = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(firmware->ecu_data);
	g_return_if_fail(firmware->ecu_data_last);
	g_return_if_fail(page <= firmware->total_pages);
	g_return_if_fail(firmware->ecu_data[page]);
	g_return_if_fail(firmware->ecu_data_last[page]);

	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;
	memcpy (ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length);
	EXIT();
	return;
}


/*!
 \brief Returns the MTX physical page given the Mtx page number
 \param tableID is the Table Identified (physical ecu page)
 \param mtx_page is the pointer to a place to store the symbolic page mtx 
 uses to get around the nonlinear nature of the page layout in certain 
 MS firmwares
 \returns TRUE on success, FALSE on failure
 */
G_MODULE_EXPORT gboolean ms_find_mtx_page(gint tableID,gint *mtx_page)
{
	Firmware_Details *firmware = NULL;
	gint i = 0;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(firmware->page_params,FALSE);

	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i])
		{
			EXIT();
			return FALSE;
		}
		if (firmware->page_params[i]->phys_ecu_page == tableID)
		{
			*mtx_page = i;
			EXIT();
			return TRUE;
		}
	}
	EXIT();
	return FALSE;
}
