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
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <gui_handlers.h>
#include <logviewer_gui.h>
#include <structures.h>
#include <threads.h>
#include <vetable_gui.h>


/*!
 \brief rescale_table() is called to rescale a subset of a Table (doesn't
 matter what kind of table). 
 \param widget_name (gchar *) is the widget name of the scaler widget 
 that was used. From this widget we extract the table number and other 
 needed data to properly do the rescaling.
 */
void rescale_table(gchar * widget_name)
{
	extern struct Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern GHashTable *dynamic_widgets;
	gint table_num = -1;
	gint z_base = -1;
	gint z_page = -1;
	gint x_bins = -1;
	gint y_bins = -1;
	gint old = 0;
	gint page = 0;
	gint offset = 0;
	gboolean ign_parm = FALSE;
	extern gint **ms_data;
	GtkWidget *scaler = NULL;
	GtkWidget *widget = NULL;
	gchar * tmpbuf = NULL;
	GList *list = NULL;
	gfloat percentage = 0.0;
	gint i = 0;
	gint j = 0;
	gint raw_lower = 0;
	gint raw_upper = 255;
	gfloat value = 0.0;
	gfloat real_value = 0.0;
	GdkColor color;
	extern GdkColor black;
	gboolean use_color = FALSE;

	scaler = g_hash_table_lookup(dynamic_widgets,widget_name);
	g_return_if_fail(GTK_IS_WIDGET(scaler));
	table_num = (gint)g_object_get_data(G_OBJECT(scaler),"table_num");
	z_base = firmware->table_params[table_num]->z_base;
	x_bins = firmware->table_params[table_num]->x_bincount;
	y_bins = firmware->table_params[table_num]->y_bincount;
	z_page = firmware->table_params[table_num]->z_page;

	percentage = gtk_spin_button_get_value(GTK_SPIN_BUTTON(scaler));
	for (i=z_base;i<(x_bins*y_bins);i++)
	{
		if (NULL != (list = ve_widgets[z_page][i]))
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
					use_color = (gint)g_object_get_data(G_OBJECT(widget),"use_color");
					if (g_object_get_data(G_OBJECT(widget),"raw_upper") != NULL)
						raw_upper = (gint)g_object_get_data(G_OBJECT(widget),"raw_upper");
					if (g_object_get_data(G_OBJECT(widget),"raw_lower") != NULL)
						raw_lower = (gint)g_object_get_data(G_OBJECT(widget),"raw_lower");
					value = ms_data[page][offset];
					value = (value*percentage)/100.0;
					if (value < raw_lower)
						value = raw_lower;
					if (value > raw_upper)
						value = raw_upper;

					/* What we are doing is doing the 
					 * forware/reverse conversion which
					 * will give us an exact value if the 
					 * user inputs something in
					 * between,  thus we can reset the 
					 * display to a sane value...
					 */
					old = ms_data[page][offset];
					ms_data[page][offset] = value;

					real_value = convert_after_upload(widget);
					ms_data[page][offset] = old;

					tmpbuf = g_strdup_printf("%i",(gint)real_value);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);

					write_ve_const(widget, page, offset, (gint)value, ign_parm, TRUE);
					gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
					widget_grab(widget,NULL,GINT_TO_POINTER(TRUE));
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(scaler),100.0);
					if (use_color)
					{
						color = get_colors_from_hue(((gfloat)value/256.0)*360.0,0.33, 1.0);
						gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
					}


				}
			}
		}
	}
}
