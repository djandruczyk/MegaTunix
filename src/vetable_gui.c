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
#include <threads.h>
#include <vetable_gui.h>


/*!
 \brief rescale_table() is called to rescale a subset of a Table (doesn't
 matter what kind of table). 
 \param data (void *) is the widget name of the scaler widget that was used.
 From this widget we extract the table number and other needed data to 
 properly do the rescaling.
 */
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
	gboolean ign_parm = FALSE;
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
	rpm_bins = firmware->table_params[table_num]->x_bincount;
	load_bins = firmware->table_params[table_num]->y_bincount;
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
					ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
					page = (gint)g_object_get_data(G_OBJECT(widget),"page");
					offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
					value = ms_data[page][offset];
					value = (value*percentage)/100.0;
					ms_data[page][offset] = (gint)value;
					write_ve_const(widget,page,offset,(gint)value,ign_parm);
					g_list_foreach(ve_widgets[page][offset],update_widget,NULL);

					widget_grab(widget,NULL,GINT_TO_POINTER(TRUE));
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(scaler),100.0);

				}
			}
		}
	}



}
