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

/* Currently unused due to being horribly slow... 
 * highlights the four boxes currounding the current VE point...
void hilite_ve_entries(gint rpm, gint map, gint page)
{
	// Highlights the VEtable entries around the current engine
	// Operating point to assist with tuning the table.
	//
	extern unsigned char *ms_data;
	extern GdkColor red;
	extern GdkColor white;
	struct Ve_Const_Std *ve_const = NULL;
	extern GtkWidget *ve_widgets[];
	static struct Indexes kpa_index[2],rpm_index[2];
	static struct Indexes l_kpa_index[2],l_rpm_index[2];
	static gint index[2][4] = {{-1,-1,-1,-1},{-1,-1,-1,-1}};
	gint offset = 0;
	gint i = 0;


	ve_const = (struct Ve_Const_Std *) ms_data;
	if (page == 2)
		offset = MS_PAGE_SIZE;

	get_indexes(KPA,map, &kpa_index[page],page);
	get_indexes(RPM,rpm/100, &rpm_index[page],page);

	// If nothing has changed, exit now saving CPU time 
	if ((l_kpa_index[page].low == kpa_index[page].low) &&
			(l_kpa_index[page].high == kpa_index[page].high) &&
			(l_rpm_index[page].low == rpm_index[page].low) &&
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
	extern unsigned char *ms_data;
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
	if (page == 2)
		start += MS_PAGE_SIZE;
	
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
