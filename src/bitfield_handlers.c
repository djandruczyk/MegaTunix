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


void check_config13(guchar tmp)
{
	GtkWidget *label;
	extern GHashTable * dynamic_widgets;
	/* checks bits of the confgi13 bitfield and forces
	 * gui to update/adapt as necessary...
	 */

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
