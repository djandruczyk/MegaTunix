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


void check_bcfreq(unsigned char tmp, gboolean update)
{
	extern struct DynamicButtons buttons;
	gint val;	
	val = (tmp &0x3);
	if (update)
	{
		switch (val)
		{
			case 1:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
						(buttons.boost_39hz),
						TRUE);
				break;
			case 2:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
						(buttons.boost_19hz),
						TRUE);
				break;
			case 3:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
						(buttons.boost_10hz),
						TRUE);
				break;
		}
	}

}

void check_config11(unsigned char tmp)
{
	extern gfloat map_pbar_divisor;
	extern unsigned char *kpa_conversion;
	extern unsigned char na_map[];
	extern unsigned char turbo_map[];

	/* checks some of the bits in the config11 variable and 
	 * adjusts some important things as necessary....
	 */
	if ((tmp &0x3) == 0)	
	{
		kpa_conversion = na_map;
		map_pbar_divisor = 115.0;
		//	g_printf("using 115KPA map sensor\n");
	}
	if ((tmp &0x3) == 1)	
	{
		kpa_conversion = turbo_map;
		map_pbar_divisor = 255.0;
		//	g_printf("using 250KPA map sensor\n");
	}
}

void check_config13(unsigned char tmp)
{
	GtkWidget *label;
	extern GList *enh_idle_controls;
	extern gfloat ego_pbar_divisor;
	extern GHashTable * dynamic_widgets;
	extern gint temp_units;
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
		label = g_hash_table_lookup(dynamic_widgets,"VE1_load_frame_title");
		if (label)
			gtk_label_set_text(GTK_LABEL(label),"TPS Bins");
		label = g_hash_table_lookup(dynamic_widgets,"VE1_load_table_units");
		if (label)
			gtk_label_set_text(GTK_LABEL(label),"TPS %");
				
	}
	else
	{
		label = g_hash_table_lookup(dynamic_widgets,"VE1_load_frame_title");
		if (label)
			gtk_label_set_text(GTK_LABEL(label),"MAP Bins");
		label = g_hash_table_lookup(dynamic_widgets,"VE1_load_table_units");
		if (label)
			gtk_label_set_text(GTK_LABEL(label),"Kpa");
	}

	/* Check Idle method */
	if (((tmp >> 4)&0x1) == 1) /* If Set turn idle controls, else turn off */
	{
		g_list_foreach(enh_idle_controls, 
				set_widget_state,(gpointer)TRUE);
		reset_temps(GINT_TO_POINTER(temp_units));
	}
	else
	{
		g_list_foreach(enh_idle_controls, 
				set_widget_state,(gpointer)FALSE);
		reset_temps(GINT_TO_POINTER(temp_units));
	}
      
		
}

void check_tblcnf(unsigned char tmp, gboolean update)
{
	extern struct DynamicButtons buttons;
	unsigned char val = 0;

	if ((tmp &0x1) == 0)
	{	/* B&G Simul style both channels driven from table 1*/
		if (update)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_table1),
					TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_table1),
					TRUE);
		}
		set_dt_table_mapping_state(FALSE);
		/* all other bits don't matter, quit now... */
		//return;
	}
	else
	{
		set_dt_table_mapping_state(TRUE);
		if (update)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.dt_mode),
					TRUE);
		}
	}
	if (update == FALSE)
		return;
	/* If update is true proceed to update all controls */
	val = (tmp >> 1)&0x3;  //(interested in bits 1-2) 
	switch (val)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_not_driven),
					TRUE);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_table1),
					TRUE);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_table2),
					TRUE);
			break;
		default:
			dbg_func(__FILE__": check_tblcnf(), bits 1-2 in tblcnf invalid\n",CRITICAL);
	}
	val = (tmp >> 3)&0x3;  //(interested in bits 3-4) 
	switch (val)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_not_driven),
					TRUE);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_table1),
					TRUE);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_table2),
					TRUE);
			break;
		default:
			dbg_func(__FILE__": check_tblcnf(), bits 1-2 in tblcnf invalid\n",CRITICAL);
	}
	/* Gammae for injection channel 1 */
	val = (tmp >> 5)&0x1;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj1_gammae),
				val);
		
	/* Gammae for injection channel 2 */
	val =  (tmp >> 6)&0x1;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj2_gammae),
				val);
	return;	
}
