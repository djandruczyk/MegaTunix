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
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <memory_gui.h>

GArray *raw_memory_widgets = NULL;
gint num_mem_pages = 4;
gint mem_view_style[] = {HEX_VIEW,HEX_VIEW,HEX_VIEW,HEX_VIEW};


EXPORT void finish_memviewer(void)
{
	GtkWidget *label = NULL;
	GtkWidget *button = NULL;
	GtkWidget *table = NULL;
	GtkWidget *table2 = NULL;
	GtkWidget *ebox = NULL;
	gchar *tblname = NULL;
	gchar *name = NULL;
	gint rows = 32;
	gint cols = 8;
	gint x = 0;
	gint y = 0;
	gint z = 0;
	gint base = 0;
	gint range = 0;
	GdkColor purple = { 0, 61000, 57000, 65535};
	extern GdkColor white;
	extern GHashTable *dynamic_widgets;

	raw_memory_widgets = g_array_new(FALSE,TRUE,sizeof(GtkWidget*));
	base = 0;
	range = 256;
	for (z=0;z<num_mem_pages;z++)
	{
		tblname = g_strdup_printf("memviewer_tab%i_row_lbl_table",z);
		table = g_hash_table_lookup(dynamic_widgets,tblname);
		g_free(tblname);

		for (y=0;y<rows;y++)
		{
			label = gtk_label_new(g_strdup_printf("0x%.4X",(z*256)+(y*cols)));
			gtk_table_attach(GTK_TABLE(table),label,0,1,y,y+1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
		}
		gtk_widget_show_all(table);

		tblname = g_strdup_printf("memviewer_tab%i_data_table",z);
		table = g_hash_table_lookup(dynamic_widgets,tblname);
		g_free(tblname);

		y = 0;
		x = 0;
		for (y=0;y<rows;y++)
		{
			//	frame = gtk_frame_new(NULL);
			ebox = gtk_event_box_new();
			//	gtk_container_add(GTK_CONTAINER(frame),ebox);

			gtk_table_attach(GTK_TABLE(table),ebox,0,1,y,y+1,
					(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
					(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL), 0, 0);

			table2 = gtk_table_new(1,cols,TRUE);
			gtk_container_add(GTK_CONTAINER(ebox),table2);
			gtk_table_set_col_spacings(GTK_TABLE(table2),1);

			for (x=0;x<cols;x++)
			{

				ebox = gtk_event_box_new();
				gtk_table_attach(GTK_TABLE(table2),ebox,x,x+1,0,1,
						(GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
						(GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND), 0, 0);
				label = gtk_label_new(NULL);
				/* (z+range)+(y*8)+x
				 * z = block of 256 labels,
				 * y = column in table,
				 * x = row in table.
				 * If it works right, the array is populated in
				 * order with 1024 widgets assuming z=4, and 
				 * range = 256 (number of elements per table)
				 */
				raw_memory_widgets = g_array_insert_val(raw_memory_widgets,(z*range)+(y*8)+(x),label);
				gtk_container_add(GTK_CONTAINER(ebox),label);
				if (y%2)
					gtk_widget_modify_bg(ebox,GTK_STATE_NORMAL,&purple);
				else
					gtk_widget_modify_bg(ebox,GTK_STATE_NORMAL,&white);

			}
		}

		if (mem_view_style[z] == HEX_VIEW)
		{
			name = g_strdup_printf("memviewer_tab%i_hex_radio_button",z);
			button = g_hash_table_lookup(dynamic_widgets,name);
			if (!button)
				dbg_func(g_strdup_printf(__FILE__": finish_memviewer()\n\tButton %s NOT found\n",name),CRITICAL);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			g_free(name);
		}
		if (mem_view_style[z] == BINARY_VIEW)
		{
			name = g_strdup_printf("memviewer_tab%i_binary_radio_button",z);
			button = g_hash_table_lookup(dynamic_widgets,name);
			if (!button)
				dbg_func(g_strdup_printf(__FILE__": finish_memviewer()\n\tButton %s NOT found\n",name),CRITICAL);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			g_free(name);
		}
		if (mem_view_style[z] == DECIMAL_VIEW)
		{
			name = g_strdup_printf("memviewer_tab%i_decimal_radio_button",z);
			button = g_hash_table_lookup(dynamic_widgets,name);
			if (!button)
				dbg_func(g_strdup_printf(__FILE__": finish_memviewer()\n\tButton %s NOT found\n",name),CRITICAL);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			g_free(name);
		}

		base += range;
		gtk_widget_show_all(table);
	}


	return;
}
