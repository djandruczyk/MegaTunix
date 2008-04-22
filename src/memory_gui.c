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
extern gint dbg_lvl;
extern GObject *global_data;


/*!
 \brief finish_memviewer() puts the final finishing touches on the memory
 viewer including populataing the notebook andtabels with GtkLabels so we
 can see the memviewer data
 */
EXPORT void finish_memviewer(void)
{
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *button = NULL;
	GtkWidget *table = NULL;
	gchar *tblname = NULL;
	gchar *name = NULL;
	gint rows = 32;
	gint cols = 8;
	gint row = 0;
	gint col = 0;
	gint j = 0;
	gint y = 0;
	gint z = 0;
	gint base = 0;
	gint range = 0;
	gchar * tmpbuf = NULL;
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
			tmpbuf = g_strdup_printf("0x%.4X",(z*256)+(y*cols));
			label = gtk_label_new(tmpbuf);
			g_free(tmpbuf);
			gtk_table_attach(GTK_TABLE(table),label,0,1,y,y+1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
		}
		gtk_widget_show_all(table);

		tblname = g_strdup_printf("memviewer_tab%i_data_table",z);
		table = g_hash_table_lookup(dynamic_widgets,tblname);
		g_free(tblname);

		row = 0;
		col = 0;
		for (j=0;j<rows*cols;j++)
		{
			entry = gtk_entry_new();
			gtk_entry_set_width_chars(GTK_ENTRY(entry),4);
                        gtk_entry_set_has_frame(GTK_ENTRY(entry),FALSE);
                        gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);

			raw_memory_widgets = g_array_insert_val(raw_memory_widgets,(z*range)+j,entry);
			gtk_table_attach(GTK_TABLE(table),entry,col,col+1,row,row+1,(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL), 0, 0);

			if (row%2)
				gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&purple);
			else
				gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&white);
			col++;
			if (col >= cols)
			{
				col = 0;
				row++;
			}

		}

		if (mem_view_style[z] == HEX_VIEW)
		{
			name = g_strdup_printf("memviewer_tab%i_hex_radio_button",z);
			button = g_hash_table_lookup(dynamic_widgets,name);
			if (!button)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": finish_memviewer()\n\tButton %s NOT found\n",name));
			}
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			g_free(name);
		}
		if (mem_view_style[z] == BINARY_VIEW)
		{
			name = g_strdup_printf("memviewer_tab%i_binary_radio_button",z);
			button = g_hash_table_lookup(dynamic_widgets,name);
			if (!button)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": finish_memviewer()\n\tButton %s NOT found\n",name));
			}
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			g_free(name);
		}
		if (mem_view_style[z] == DECIMAL_VIEW)
		{
			name = g_strdup_printf("memviewer_tab%i_decimal_radio_button",z);
			button = g_hash_table_lookup(dynamic_widgets,name);
			if (!button)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": finish_memviewer()\n\tButton %s NOT found\n",name));
			}
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
			g_free(name);
		}

		base += range;
		gtk_widget_show_all(table);
	}


	return;
}
