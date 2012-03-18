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
  \file src/mscommon_tableio.c
  \ingroup CoreMtx
  \brief mscommon_tableio
  \author David Andruczyk
  */

#include <conversions.h>
#include <firmware.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <mscommon_tableio.h>
#include <gtk/gtk.h>

extern gconstpointer *global_data;

/*!\brief Takes 3 arrays of (2 Axis's and table) converts them the ECU UNITS
 * and loads them to the ECU. The table ID and X/Y/Z dimensions have 
 * ALREADY been validated.
 * \parm table_num, the table number we are updating
 * \param x_elements, the X axis elements in floating point "USER" units
 * \param y_elements, the Y axis elements in floating point "USER" units
 * \param z_elements, the Table elements in floating point "USER" units
 */
G_MODULE_EXPORT void ecu_table_import(gint table_num,gfloat *x_elements, gfloat *y_elements, gfloat *z_elements)
{
	Firmware_Details *firmware = NULL;
	void *data = NULL;
	guchar *ptr = NULL;
    guint16 *ptr16 = NULL;
    guint32 *ptr32 = NULL;
	gint * bins = NULL;
	gint count = 0;
	gint mult = 0;
	gint i = 0;
	firmware = DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	/*
	What needs to happen is to take these values which are all "REAL WORLD"
	user units and convert them to ECU units and send to the ECU
	Ideally this would be in a nice burst instead of one write/packet
	per value.
	*/
	/* X values first */
	bins = convert_toecu_bins(table_num, x_elements,_X_);
	mult = get_multiplier_f(firmware->table_params[table_num]->x_size);
	count = firmware->table_params[table_num]->x_bincount;
	data = g_new0(guchar, mult * count);
	if (mult == 1)
	{
		ptr = (guchar *)data;
		for (i=0;i<count;i++)
			ptr[i] = bins[i];
	}
	if (mult == 2)
	{
		ptr16 = (guint16 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr16[i] = GINT16_TO_BE(bins[i]);
			else
				ptr16[i] = GINT16_TO_LE(bins[i]);
		}
	}
	if (mult == 4)
	{
		ptr32 = (guint32 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr32[i] = GINT_TO_BE(bins[i]);
			else
				ptr32[i] = GINT_TO_LE(bins[i]);
		}
	}
	/* WRITE new X axis values */
	ms_chunk_write(0,
			firmware->table_params[table_num]->x_page,
			firmware->table_params[table_num]->x_base,
			count*mult,
			data);
	g_free(bins);

	/* Y Bins */
	bins = convert_toecu_bins(table_num, y_elements,_Y_);
	mult = get_multiplier_f(firmware->table_params[table_num]->y_size);
	count = firmware->table_params[table_num]->y_bincount;
	data = g_new0(guchar, mult * count);
	if (mult == 1)
	{
		ptr = (guchar *)data;
		for (i=0;i<count;i++)
			ptr[i] = bins[i];
	}
	if (mult == 2)
	{
		ptr16 = (guint16 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr16[i] = GINT16_TO_BE(bins[i]);
			else
				ptr16[i] = GINT16_TO_LE(bins[i]);
		}
	}
	if (mult == 4)
	{
		ptr32 = (guint32 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr32[i] = GINT_TO_BE(bins[i]);
			else
				ptr32[i] = GINT_TO_LE(bins[i]);
		}
	}
	/* WRITE new Y axis values */
	g_free(bins);
	ms_chunk_write(0,
			firmware->table_params[table_num]->y_page,
			firmware->table_params[table_num]->y_base,
			count*mult,
			data);

	/* Z Bins */
	bins = convert_toecu_bins(table_num, z_elements,_Z_);
	mult = get_multiplier_f(firmware->table_params[table_num]->z_size);
	count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
	data = g_new0(guchar, mult * count);
	if (mult == 1)
	{
		ptr = (guchar *)data;
		for (i=0;i<count;i++)
			ptr[i] = bins[i];
	}
	if (mult == 2)
	{
		ptr16 = (guint16 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr16[i] = GINT16_TO_BE(bins[i]);
			else
				ptr16[i] = GINT16_TO_LE(bins[i]);
		}
	}
	if (mult == 4)
	{
		ptr32 = (guint32 *)data;
		for (i=0;i<count;i++)
		{
			if (firmware->bigendian)
				ptr32[i] = GINT_TO_BE(bins[i]);
			else
				ptr32[i] = GINT_TO_LE(bins[i]);
		}
	}
	/* WRITE new Z values */
	ms_chunk_write(0,
			firmware->table_params[table_num]->z_page,
			firmware->table_params[table_num]->z_base,
			count*mult,
			data);
	g_free(bins);
}


/*!\brief converts the float "user" values to ECU units in prep for sending
 * to the ecu
 * \param table_num, the table number identifier
 * \param elements, array of user floats
 * \param a, axis enumeration
 * \returns integer array of converted values;
 * */
gint * convert_toecu_bins(gint table_num, gfloat *elements, Axis a)
{
	Firmware_Details *firmware = NULL;
	gint page = 0;
	gint base = 0;
	gint mult = 0;
	gint count = 0;
	gint i = 0;
	gint *bins = NULL;
	GList ***ecu_widgets = NULL;
	GList *list = NULL;
	GtkWidget * widget;
	
	firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,NULL);
	ecu_widgets = DATA_GET(global_data,"ecu_widgets");
	g_return_val_if_fail(firmware,NULL);

	switch (a)
	{
		case _X_:
			base = firmware->table_params[table_num]->x_base;
			page = firmware->table_params[table_num]->x_page;
			mult = get_multiplier_f(firmware->table_params[table_num]->x_size);
			count = firmware->table_params[table_num]->x_bincount;
			break;
		case _Y_:
			base = firmware->table_params[table_num]->y_base;
			page = firmware->table_params[table_num]->y_page;
			mult = get_multiplier_f(firmware->table_params[table_num]->y_size);
			count = firmware->table_params[table_num]->y_bincount;
			break;
		case _Z_:
			base = firmware->table_params[table_num]->z_base;
			page = firmware->table_params[table_num]->z_page;
			mult = get_multiplier_f(firmware->table_params[table_num]->z_size);
			count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
			break;
	}

	bins = (gint *)g_new0(gint, count);
	for (i=0;i<count;i++)
	{
		list = ecu_widgets[page][base+(i*mult)];
		/* First widget will be from the datamap */
		widget = g_list_nth_data(list,0);
		bins[i] = convert_before_download(widget,elements[i]);
	}
	return bins;
}


/*!\brief converts the integer ECU units to floating 'USER" units
 * \param table_num, the table number identifier
 * \param a, axis enumeration
 * \returns integer array of converted values;
 * */
gfloat * convert_fromecu_bins(gint table_num, Axis a)
{
	static Firmware_Details *firmware = NULL;
	gint page = 0;
	gint base = 0;
	gint mult = 0;
	gint count = 0;
	gint i = 0;
	gfloat *bins = NULL;
	GList ***ecu_widgets = NULL;
	GList *list = NULL;
	GtkWidget * widget;

	firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,NULL);
	ecu_widgets = DATA_GET(global_data,"ecu_widgets");
	g_return_val_if_fail(firmware,NULL);

	switch (a)
	{
		case _X_:
			page = firmware->table_params[table_num]->x_page;
			base = firmware->table_params[table_num]->x_base;
			mult = get_multiplier_f(firmware->table_params[table_num]->x_size);
			count = firmware->table_params[table_num]->x_bincount;
			break;
		case _Y_:
			page = firmware->table_params[table_num]->y_page;
			base = firmware->table_params[table_num]->y_base;
			mult = get_multiplier_f(firmware->table_params[table_num]->y_size);
			count = firmware->table_params[table_num]->y_bincount;
			break;
		case _Z_:
			page = firmware->table_params[table_num]->z_page;
			base = firmware->table_params[table_num]->z_base;
			mult = get_multiplier_f(firmware->table_params[table_num]->z_size);
			count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
			break;
	}

	bins = (gfloat *)g_new0(gfloat, count);
	for (i=0;i<count;i++)
	{
		list = ecu_widgets[page][base+(i*mult)];
		/* First widget will be from the datamap */
		widget = g_list_nth_data(list,0);
		bins[i] = convert_after_upload(widget);
	}
	return bins;
}


G_MODULE_EXPORT TableExport * ecu_table_export(gint table_num)
{
	TableExport *table = NULL;
	Firmware_Details *firmware = NULL;
	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,NULL);

	table = g_new0(TableExport, 1);

	table->x_len = firmware->table_params[table_num]->x_bincount;
	table->y_len = firmware->table_params[table_num]->y_bincount;
	table->x_bins = convert_fromecu_bins(table_num,_X_);
	table->y_bins = convert_fromecu_bins(table_num,_Y_);
	table->z_bins = convert_fromecu_bins(table_num,_Z_);
	/* This breaks to multi expressions! */
	table->x_label = firmware->table_params[table_num]->x_suffix;
	table->y_label = firmware->table_params[table_num]->y_suffix;
	table->z_label = firmware->table_params[table_num]->z_suffix;
	table->x_unit = firmware->table_params[table_num]->x_suffix;
	table->y_unit = firmware->table_params[table_num]->y_suffix;
	table->z_unit = firmware->table_params[table_num]->z_suffix;
	table->title = firmware->table_params[table_num]->table_name;
	table->desc = firmware->table_params[table_num]->table_name;
	return table;
}
