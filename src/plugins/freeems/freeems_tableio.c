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
  \file src/freeems_tableio.c
  \ingroup CoreMtx
  \brief freeems_tableio
  \author David Andruczyk
  */

#include <firmware.h>
#include <freeems_plugin.h>
#include <freeems_tableio.h>
#include <gtk/gtk.h>

extern gconstpointer *global_data;

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
	The table ID and X/Y/Z dimensions have ALREADY been validated.
	*/
	/* X values first */
	bins = convert_bins(table_num, x_elements,_X_);
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
	g_free(bins);

	/* Y Bins */
	bins = convert_bins(table_num, y_elements,_Y_);
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
	/* WRITE new X axis values */
	g_free(bins);

	/* Z Bins */
	bins = convert_bins(table_num, z_elements,_Z_);
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
	/* WRITE new X axis values */
	g_free(bins);
}

/*!\brief converts the float "user" values to ECU units in prep for sending
 * to the ecu
 * \param table_num, the table number identifier
 * \param elements, array of user floats
 * \param a, axis enumeration
 * \returns integer array of converted values;
 * */
gint * convert_bins(gint table_num, gfloat *elements, Axis a)
{
	static Firmware_Details *firmware = NULL;
	gint mult = 0;
	gfloat *multiplier = NULL;
	gfloat *adder = NULL;
	gint *bins = NULL;
	gint count = 0;
	gint i = 0;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,NULL);

	switch (a)
	{
		case _X_:
			mult = get_multiplier_f(firmware->table_params[table_num]->x_size);
			count = firmware->table_params[table_num]->x_bincount;
			bins = (gint *)g_new0(gint, count);
			multiplier = firmware->table_params[table_num]->x_fromecu_mult;
			adder = firmware->table_params[table_num]->x_fromecu_add;
			break;
		case _Y_:
			mult = get_multiplier_f(firmware->table_params[table_num]->y_size);
			count = firmware->table_params[table_num]->y_bincount;
			bins = (gint *)g_new0(gint, count);
			multiplier = firmware->table_params[table_num]->y_fromecu_mult;
			adder = firmware->table_params[table_num]->y_fromecu_add;
			break;
		case _Z_:
			mult = get_multiplier_f(firmware->table_params[table_num]->z_size);
			count = firmware->table_params[table_num]->x_bincount * firmware->table_params[table_num]->y_bincount;
			bins = (gint *)g_new0(gint, count);
			multiplier = firmware->table_params[table_num]->z_fromecu_mult;
			adder = firmware->table_params[table_num]->z_fromecu_add;
			break;
	}

	for (i=0;i<count;i++)
	{
		if ((multiplier) && (adder))
			bins[i] = (GINT)((elements[i]/(*multiplier)) - (*adder));
		else if (multiplier)
			bins[i] = (GINT)(elements[i]/(*multiplier));
		else
			bins[i] = (GINT)elements[i];
	}
	return bins;
}

