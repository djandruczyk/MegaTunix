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
#include <raweditor.h>
#include <structures.h>

GArray *raw_editor_widgets = NULL;

EXPORT void finish_raweditor(void)
{
	GtkWidget *sw = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *lbl_table = NULL;
	GtkWidget *table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *top = NULL;
	GtkWidget *entry = NULL;
	gint row = 0;
	gint col = 0;
	gint i = 0;
	gint j = 0;
	gint cols = 8;
	GdkColor purple = { 0, 61000, 57000, 65535};
	extern GdkColor white;
	extern struct Firmware_Details *firmware;
	extern GHashTable *dynamic_widgets;
	extern GList *** ve_widgets;

	top = g_hash_table_lookup(dynamic_widgets,"raweditor_top_vbox1");
	if (!GTK_IS_WIDGET(top))
		return;

	notebook = gtk_notebook_new ();
	        gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
		        gtk_box_pack_start(GTK_BOX(top),notebook,TRUE,TRUE,0);

	for (i=0;i<firmware->total_pages;i++)
	{
		label = gtk_label_new(g_strdup_printf("Page %i",i));
		frame = gtk_frame_new(g_strdup_printf("Page %i RAW ECU data (in HEX)",i));
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);

		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
		gtk_container_add(GTK_CONTAINER(frame),vbox);

		sw = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				GTK_POLICY_AUTOMATIC,
				GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start(GTK_BOX(vbox),sw,TRUE,TRUE,5);

		hbox = gtk_hbox_new(FALSE,0);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),hbox);
		lbl_table = gtk_table_new((firmware->page_params[i]->length)/cols,1,TRUE);
		gtk_box_pack_start(GTK_BOX(hbox),lbl_table,FALSE,TRUE,5);

		table = gtk_table_new((firmware->page_params[i]->length)/cols,cols,TRUE);
		gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,5);
		row = 0;
		col = 0;
		for (j=0;j<firmware->page_params[i]->length;j++)
		{


			entry = gtk_entry_new();
			gtk_entry_set_alignment(GTK_ENTRY(entry),0.5);
			g_object_set_data(G_OBJECT(entry),"page",GINT_TO_POINTER(i));
			g_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(j));
			if (firmware->page_params[i]->is_spark)
				g_object_set_data(G_OBJECT(entry),"ign_parm",GINT_TO_POINTER(TRUE));

			ve_widgets[i][j] = g_list_append(ve_widgets[i][j],(gpointer)entry);
			g_object_set_data(G_OBJECT(entry),"handler",
					GINT_TO_POINTER(GENERIC));
			g_signal_connect (G_OBJECT(entry), "changed",
					G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect (G_OBJECT(entry), "activate",
					G_CALLBACK(std_entry_handler),NULL);

			gtk_entry_set_width_chars(GTK_ENTRY(entry),4);
			gtk_entry_set_has_frame(GTK_ENTRY(entry),TRUE);
			gtk_table_attach(GTK_TABLE(table),entry,col,col+1,row,row+1,
					(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
					(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL), 0, 0);

			if (row%2)
				gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&purple);
			else
				gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&white);
			if (col == 0)
			{
				label = gtk_label_new(g_strdup_printf("0x%.4X",(row*cols)));
				gtk_table_attach(GTK_TABLE(lbl_table),label,0,1,row,row+1,
						(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
						(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL), 0, 0);
			}
			col++;
			if (col >= cols)
			{
				col = 0;
				row++;
			}
		}
		


	}
	gtk_widget_show_all(top);
	
}
	/*
				label = gtk_label_new(NULL);
				*/
				/* (z+range)+(y*8)+x
				 * z = block of 256 labels,
				 * y = column in table,
				 * x = row in table.
				 * If it works right, the array is populated in
				 * order with 1024 widgets assuming z=4, and 
				 * range = 256 (number of elements per table)
				 */
	/*
				raw_memory_widgets = g_array_insert_val(raw_memory_widgets,(z*range)+(y*8)+(x),label);
				gtk_container_add(GTK_CONTAINER(ebox),label);

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
*/
