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

#include <3d_vetable.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>
#include <vetable_gui.h>



void rescale_table(void * data)
{
	extern struct Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern GHashTable *dynamic_widgets;
	gchar *widget_name = (gchar *)data;
	gint table_num = -1;
	gint tbl_base = -1;
	gint tbl_page = -1;
	gint rpm_bins = -1;
	gint load_bins = -1;
	gint page = 0;
	gint offset = 0;
	extern gint **ms_data;
	gboolean is_spark = FALSE;
	GtkWidget *scaler = NULL;
	GtkWidget *widget = NULL;
	GList *list = NULL;
	gfloat percentage = 0.0;
	gint i = 0;
	gint j = 0;
	gfloat value = 0.0;

	scaler = g_hash_table_lookup(dynamic_widgets,widget_name);
	g_return_if_fail(GTK_IS_WIDGET(scaler));
	table_num = (gint)g_object_get_data(G_OBJECT(scaler),"table_num");
	tbl_base = firmware->table_params[table_num]->tbl_base;
	rpm_bins = firmware->table_params[table_num]->rpm_bincount;
	load_bins = firmware->table_params[table_num]->load_bincount;
	is_spark = firmware->table_params[table_num]->is_spark;
	tbl_page = firmware->table_params[table_num]->tbl_page;

	percentage = gtk_spin_button_get_value(GTK_SPIN_BUTTON(scaler));
	for (i=tbl_base;i<(rpm_bins*load_bins);i++)
	{
		if (NULL != (list = ve_widgets[tbl_page][i]))
		{
			list = g_list_first(list);
			for (j=0;j<g_list_length(list);j++)
			{
				widget = (GtkWidget *)g_list_nth_data(list,j);
				if ((gboolean)g_object_get_data(G_OBJECT(widget),"marked"))
				{
					page = (gint)g_object_get_data(G_OBJECT(widget),"page");
					offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
					value = ms_data[page][offset];
					value = (value*percentage)/100.0;
					ms_data[page][offset] = value;
					g_list_foreach(ve_widgets[page][offset],update_widget,NULL);

					widget_grab(widget,NULL,GINT_TO_POINTER(TRUE));
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(scaler),100.0);

				}
			}
		}
	}



}

/* Currently unused due to being horribly slow... 
 * highlights the four boxes currounding the current VE point...
void hilite_ve_entries(gint rpm, gint map, gint table_num)
{
	// Highlights the VEtable entries around the current engine
	// Operating point to assist with tuning the table.
	//
	extern gint **ms_data;
	extern GdkColor red;
	extern GdkColor white;
	extern GList ***ve_widgets;
	static struct Indexes kpa_index[2],rpm_index[2];
	static struct Indexes l_kpa_index[2],l_rpm_index[2];
	static gint index[2][4] = {{-1,-1,-1,-1},{-1,-1,-1,-1}};
	gint offset = 0;
	gint i = 0;



	get_indexes(KPA,map, &kpa_index[table_num],table_num);
	get_indexes(RPM,rpm/100, &rpm_index[table_num],table_num);

	// If nothing has changed, exit now saving CPU time 
	if ((l_kpa_index[table_num].low == kpa_index[table_num].low) &&
			(l_kpa_index[table_num].high == kpa_index[table_num].high) &&
			(l_rpm_index[table_num].low == rpm_index[page].low) &&
			(l_rpm_index[page].high == rpm_index[page].high))
		return;

	for (i=0;i<4;i++)
	{
		if (index[page][i] >= 0)
		{
			gtk_widget_modify_base(ve_widgets[offset+index[page][i]],
					GTK_STATE_NORMAL,&white);
		}
	}
	dbg_func(g_strdup_printf(__FILE__": hilite_ve_entries() rpm %i,%i, kpa %i,%i\n",rpm_index[page].low,rpm_index[page].high,kpa_index[page].low,kpa_index[page].high),IO_PROCESS);

	index[page][0] = (kpa_index[page].low * 8) + rpm_index[page].low;
	index[page][1] = (kpa_index[page].low * 8) + rpm_index[page].high;
	index[page][2] = (kpa_index[page].high * 8) + rpm_index[page].low;
	index[page][3] = (kpa_index[page].high * 8) + rpm_index[page].high;

	for (i=0;i<4;i++)
	{
		gtk_widget_modify_base(ve_widgets[offset+index[page][i]],
				GTK_STATE_NORMAL,&red);
	}

	// save last run 
	l_kpa_index[page].low = kpa_index[page].low;
	l_kpa_index[page].high = kpa_index[page].high;
	l_rpm_index[page].low = rpm_index[page].low;
	l_rpm_index[page].high = rpm_index[page].high;

	return;

}

void get_indexes(TableType type, gint value, void *ptr,gint page)
{
	extern gint **ms_data;
	struct Indexes *index = (struct Indexes *) ptr;
	gint start = -1;
	const gint span = 8;
	gint i = -1;

	if (type == KPA)
		start = VE1_KPA_BINS_OFFSET;
	else if (type == RPM)
		start = VE1_RPM_BINS_OFFSET;
	else
	{
		dbg_func(__FILE__": get_indexes(), Invalid TableType passed to get_indexes()\n",CRITICAL);
		index->low = -1;
		index->high = -1;
		return;
	}
	
	for (i=0;i<span-1;i++)
	{
		// If value is out of page bounds on low side 
		if (value < ms_data[start])
		{
			index->low=0;
			index->high=0;
			break;
		}
		// Somewhere inside page.... 
		if ((value >= ms_data[start+i])&&(value <= ms_data[start+i+1]))
		{	
			index->low=i;
			index->high=i+1;
			break;
		}
		// Above page bounds 
		index->low = span-1;
		index->high = span-1;
	}

	return;
}
 */
