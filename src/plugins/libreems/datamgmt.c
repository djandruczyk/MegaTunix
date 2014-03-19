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
  \file src/plugins/libreems/datamgmt.c
  \ingroup LibreEMSPlugin,Plugins
  \brief All of the LibreEMS specific Data Management functions
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <firmware.h>
#include <libreems_plugin.h>
#include <string.h>


extern gconstpointer *global_data;

/*!
  \brief returns the ECU data corresponding to the fields attached to the 
  data object, data can be either a GtkWidget pointer or a gconstpointer
  \param data pointer to container, should contain the following fields
  offset, size, and location_id or page
  \returns the value at that ECU memory location
  */
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
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	widget = (GtkWidget *)data;
	container = (gconstpointer *)data;
	if (GTK_IS_WIDGET(widget))
	{
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
		if (OBJ_GET(widget,"location_id"))
		{
			locID = (GINT)OBJ_GET(widget,"location_id");
			g_return_val_if_fail(libreems_find_mtx_page(locID, &page),0);;
		}
		else if (OBJ_GET(widget,"page"))
			page = (GINT)OBJ_GET(widget,"page");
	}
	else
	{
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)(GINT)DATA_GET(container,"size");
		if (DATA_GET(container,"location_id"))
		{
			locID = (GINT)DATA_GET(container,"location_id");
			g_return_val_if_fail(libreems_find_mtx_page(locID, &page),0);;
		}
		else if (DATA_GET(container,"page"))
			page = (GINT)DATA_GET(container,"page");
	}

	/* Sanity checking */
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);

	EXIT();
	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief returns the ECU data at the location coordinates provided
 \param canID is the CANbus ID (unused currently)
 \param locID is the Location ID (internal to ECU)
 \param offset is the (RAW BYTE offset)
 \param size is the (size to be returned)
 \returns the value at those specific memory coordinates
 */
G_MODULE_EXPORT gint libreems_get_ecu_data(gint canID, gint locID, gint offset, DataSize size) 
{
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	gint page = 0;
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
 

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	/* Sanity checking */
	g_return_val_if_fail(libreems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);

	EXIT();
	return _get_sized_data(firmware->ecu_data[page],offset,size,firmware->bigendian);
}


/*!
 \brief returns the ECU data from the previous buffer (last) at the 
 location coordinates provided
 \param canID is the CANbus ID (unused currently)
 \param locID is the Location ID (internal to ECU)
 \param offset is the (RAW BYTE offset)
 \param size is the (size to be returned)
 \returns the value from ECU memory at the specified coordinates
 */
G_MODULE_EXPORT gint libreems_get_ecu_data_last(gint canID, gint locID, gint offset, DataSize size) 
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
 

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(libreems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);
	EXIT();
	return _get_sized_data(firmware->ecu_data_last[page],offset,size,firmware->bigendian);
}


/*!
 \brief returns the ECU data from the backup buffer at the coordinates 
 requested
 \param canID is the CANbus ID (unused currently)
 \param locID is the Location ID (internal to ECU)
 \param offset is the(RAW BYTE offset)
 \param size is the size to be returned...
 \returns the value from ECU memory at the specified coordinates
 */
G_MODULE_EXPORT gint libreems_get_ecu_data_backup(gint canID, gint locID, gint offset, DataSize size) 
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	static gint (*_get_sized_data)(guint8 *, gint, DataSize, gboolean) = NULL;
	ENTER();
	if (!_get_sized_data)
		get_symbol_f("_get_sized_data",(void **)&_get_sized_data);
 

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(libreems_find_mtx_page(locID, &page),0);
	g_return_val_if_fail(firmware,0);
	g_return_val_if_fail(firmware->page_params,0);
	g_return_val_if_fail(firmware->page_params[page],0);
	g_return_val_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length),0);
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
G_MODULE_EXPORT void set_ecu_data(gpointer data, gint *newval)
{
	gint canID = 0;
	gint locID = 0;
	gint page = 0;
	gint offset = 0;
	gint value = 0;
	DataSize size = MTX_U08;
	GtkWidget *widget = NULL;
	gconstpointer *container = NULL;
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	ENTER();
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void **)&_set_sized_data);

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	widget = (GtkWidget *)data;
	container = (gconstpointer *)data;
	if (GTK_IS_WIDGET(data))
	{
		if (OBJ_GET(widget,"location_id"))
		{
			locID = (GINT)OBJ_GET(widget,"location_id");
			g_return_if_fail(libreems_find_mtx_page(locID, &page));
		}
		else if (OBJ_GET(widget,"page"))
			page = (GINT)OBJ_GET(widget,"page");
		canID = (GINT)OBJ_GET(widget,"canID");
		offset = (GINT)OBJ_GET(widget,"offset");
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
		if (newval)
			value = *newval;
		else
			value = (GINT)OBJ_GET(widget,"value");
	}
	else
	{
		if (DATA_GET(container,"location_id"))
		{
			locID = (GINT)DATA_GET(container,"location_id");
			g_return_if_fail(libreems_find_mtx_page(locID, &page));
		}
		else if (DATA_GET(container,"page"))
			page = (GINT)DATA_GET(container,"page");
		canID = (GINT)DATA_GET(container,"canID");
		offset = (GINT)DATA_GET(container,"offset");
		size = (DataSize)(GINT)DATA_GET(container,"size");
		if (newval)
			value = *newval;
		else
			value = (GINT)DATA_GET(container,"value");
	}

	g_return_if_fail(firmware);
	g_return_if_fail(firmware->page_params);
	g_return_if_fail(firmware->page_params[page]);
	g_return_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length));
	_set_sized_data(firmware->ecu_data[page],offset,size,value,firmware->bigendian);
	EXIT();
	return;
}


/*!
  \brief ECU specific function to set a value in the representation of ECU 
  memory
  \param canID is the CAN Identifier
  \param locID is the Location ID
  \param offset is the offset in bytes from thebeginning on this location ID
  \param size is the  size representation enumeration
  \param newval is the new value to store
  */
G_MODULE_EXPORT void libreems_set_ecu_data(gint canID, gint locID, gint offset, DataSize size, gint newval) 
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	static gint (*_set_sized_data)(guint8 *, gint, DataSize, gint, gboolean) = NULL;
	ENTER();
	if (!_set_sized_data)
		get_symbol_f("_set_sized_data",(void **)&_set_sized_data);


	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(libreems_find_mtx_page(locID, &page));
	g_return_if_fail(firmware);
	g_return_if_fail(firmware->page_params);
	g_return_if_fail(firmware->page_params[page]);
	g_return_if_fail((offset >= 0) && (offset < firmware->page_params[page]->length));

	_set_sized_data(firmware->ecu_data[page],offset,size,newval,firmware->bigendian);
	EXIT();
	return;
}


/*!
  \brief Generic function to store a blob of data to a specific location in
  ECU representative memory. The block variable must contain the necessary
  fields in order to store properly
  \param block is a pointer to a gconstpointer containing the location ID, 
  offset, length and data to store
  */
G_MODULE_EXPORT void store_new_block(gpointer block)
{
	gint locID = 0;
	gint page = 0;
	gint offset = 0;
	gint num_bytes = 0;
	GtkWidget *widget = NULL;
	gconstpointer *container = NULL;
	guint8 *data = NULL;
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;

	g_return_if_fail(firmware);
	g_return_if_fail(ecu_data);

	locID = (GINT)DATA_GET(block,"location_id");
	offset = (GINT)DATA_GET(block,"offset");
	num_bytes = (GINT)DATA_GET(block,"num_bytes");
	data = (guint8 *)DATA_GET(block,"data");

	g_return_if_fail(libreems_find_mtx_page(locID, &page));
	g_return_if_fail(ecu_data[page]);

	memcpy (ecu_data[page]+offset,data,num_bytes);
	EXIT();
	return;
}


/*!
  \brief ECU specific function to store a new blob of data at a specific 
  location in ECU representative memory
  \param canID is the CAN identifier
  \param locID is the Location ID
  \param offset is the byte offset from the beginning of the Location ID
  \param buf is the pointer to the buffer to copy from
  \param count is the number of bytes to copy to the destination
  */
G_MODULE_EXPORT void libreems_store_new_block(gint canID, gint locID, gint offset, void * buf, gint count)
{
	gint page = 0;
	Firmware_Details *firmware = NULL;
	guint8 ** ecu_data = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;

	g_return_if_fail(firmware);
	g_return_if_fail(libreems_find_mtx_page(locID, &page));
	g_return_if_fail(ecu_data);
	g_return_if_fail(ecu_data[page]);

	if ((offset + count ) <= (firmware->page_params[page]->length))
		memcpy (ecu_data[page]+offset,buf,count);
	else
		MTXDBG(CRITICAL,_("Attempted to write beyond end of Location ID (%i), page (%i)\n Loc ID size %i, write offset %i, length %i\n"),locID,page,firmware->page_params[page]->length,offset,count);
		
	EXIT();
	return;
}


/*!
  \brief copies current ECU memory representation to backup buffer
  \param canID is unused
  \param locID is the location ID (page representation) to backup
  */
G_MODULE_EXPORT void libreems_backup_current_data(gint canID,gint locID)
{
	gint page = 0;
	guint8 ** ecu_data = NULL;
	guint8 ** ecu_data_last = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;

	g_return_if_fail(firmware);
	g_return_if_fail(libreems_find_mtx_page(locID, &page));
	g_return_if_fail(firmware->ecu_data);
	g_return_if_fail(firmware->ecu_data_last);
	g_return_if_fail(firmware->ecu_data[page]);
	g_return_if_fail(firmware->ecu_data_last[page]);

	memcpy (ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length);
	EXIT();
	return;
}


/*!
 \brief Finds the MTX page associated with this Location ID
 \param locID is the ecu firmware location Identifier
 \param mtx_page is the symbolic page mtx uses to get around the nonlinear
 nature of the page layout in certain firmwares
 \returns true on success, false on failure
 */
G_MODULE_EXPORT gboolean libreems_find_mtx_page(gint locID, gint *mtx_page)
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
		if (firmware->page_params[i]->phys_ecu_page == locID)
		{
			*mtx_page = i;
			EXIT();
			return TRUE;
		}
	}
	EXIT();
	return FALSE;
}
