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
#include <bitfield_handlers.h>
#include <conversions.h>
#include <debugging.h>
#include <enums.h>
#include <mode_select.h>
#include <structures.h>
#include <timeout_handlers.h>


void check_config11(guchar tmp)
{
	extern gfloat map_pbar_divisor;
	extern guchar *kpa_conversion;
	extern guchar na_map[];
	extern guchar turbo_map[];

	/* checks some of the bits in the config11 variable and 
	 * adjusts some important things as necessary....
	 */
	if ((tmp &0x3) == 0)	
	{
		kpa_conversion = na_map;
		map_pbar_divisor = 115.0;
	}
	if ((tmp &0x3) == 1)	
	{
		kpa_conversion = turbo_map;
		map_pbar_divisor = 255.0;
	}
}

void check_config13(guchar tmp)
{
	GtkWidget *label;
	extern gfloat ego_pbar_divisor;
	extern GHashTable * dynamic_widgets;
	/* checks bits of the confgi13 bitfield and forces
	 * gui to update/adapt as necessary...
	 */


	/* check O2 sensor bit and adjust factor
	   so runtime display has a sane scale... */
	if (((tmp >> 1)&0x1) == 1)
	{
		ego_pbar_divisor = 5.0;
		force_an_update();
	}
	else
	{
		ego_pbar_divisor = 1.2;
		force_an_update();
	}

	/* Check SD/Alpha-N button and adjust VEtable labels
	 * to suit
	 */
	if (((tmp >> 2)&0x1) == 1)
	{
		if (dynamic_widgets)
		{
			label = g_hash_table_lookup(dynamic_widgets,"VE1_load_frame_title");
			if (label)
				gtk_label_set_text(GTK_LABEL(label),"TPS Bins");
			label = g_hash_table_lookup(dynamic_widgets,"VE1_load_table_units");
			if (label)
				gtk_label_set_text(GTK_LABEL(label),"TPS %");
		}

	}
	else
	{
		if (dynamic_widgets)
		{
			label = g_hash_table_lookup(dynamic_widgets,"VE1_load_frame_title");
			if (label)
				gtk_label_set_text(GTK_LABEL(label),"MAP Bins");
			label = g_hash_table_lookup(dynamic_widgets,"VE1_load_table_units");
			if (label)
				gtk_label_set_text(GTK_LABEL(label),"Kpa");
		}
	}


}
