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
#include <enums.h>
#include <gui_handlers.h>
#include <memory_gui.h>

GList *raw_mem_controls = NULL;
GArray *raw_memory_widgets = NULL;
static gint num_pages = 4;
gint mem_view_style[] = {HEX_VIEW,HEX_VIEW,HEX_VIEW,HEX_VIEW};


void build_memory(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *notebook;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *table2;
	GtkWidget *ebox;
	GtkWidget *frame;
	GtkWidget *sw;
	GSList * group = NULL;
	extern GdkColor white;
	GdkColor purple = { 0, 61000, 57000, 65535};
	gint rows = 32;
	gint cols = 8;
	gint x = 0;
	gint y = 0;
	gint z = 0;
	gint base = 0;
	gint range = 0;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_box_pack_start(GTK_BOX(vbox),notebook,TRUE,TRUE,0);
	
	raw_memory_widgets = g_array_new(FALSE,TRUE,sizeof(GtkWidget*));
	base = 0;
	range = 256;
	for (z=0;z<num_pages;z++)
	{
		frame = gtk_frame_new(g_strdup_printf("256 bytes from address 0x%.4X",base));
		label = gtk_label_new(g_strdup_printf("0x%.4X-0x%.4X",base,base+range-1));
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);

		vbox2 = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(frame),vbox2);

		sw = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				GTK_POLICY_AUTOMATIC,
				GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start(GTK_BOX(vbox2),sw,TRUE,TRUE,0);

		hbox = gtk_hbox_new(FALSE,5);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),hbox);	

		table = gtk_table_new(rows,1,TRUE);
		gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,10);

		for (y=0;y<rows;y++)
		{
			label = gtk_label_new(g_strdup_printf("0x%.4X",(z*256)+(y*cols)));
			gtk_table_attach(GTK_TABLE(table),label,0,1,y,y+1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
		}

		table = gtk_table_new(rows,1,TRUE);
		gtk_table_set_row_spacings(GTK_TABLE(table),1);
		gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);


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
		frame = gtk_frame_new("Raw Memory Settings");
		gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,TRUE,5);

		hbox = gtk_hbox_new(FALSE,5);
		gtk_container_add(GTK_CONTAINER(frame),hbox);
		gtk_container_set_border_width(GTK_CONTAINER(hbox),5);

		button = gtk_button_new_with_label(g_strdup_printf("Read Memory from 0x%.4X to 0x%.4X",base,base+range-1));
		/* Memory offset to retrieve... */
		g_object_set_data(G_OBJECT(button),"data",GINT_TO_POINTER(z));
		g_signal_connect(G_OBJECT(button),"clicked",
				G_CALLBACK(std_button_handler),
				GINT_TO_POINTER(READ_RAW_MEMORY));
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,5);
	
		button = gtk_radio_button_new_with_label(NULL,"Hex");
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
		g_object_set_data(G_OBJECT(button),"data",GINT_TO_POINTER(z));
		if (mem_view_style[z] == HEX_VIEW)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		g_signal_connect(G_OBJECT(button),"clicked",
				G_CALLBACK(toggle_button_handler),
				GINT_TO_POINTER(HEX_VIEW));
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,5);

		button = gtk_radio_button_new_with_label(group,"Binary");
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
		g_object_set_data(G_OBJECT(button),"data",GINT_TO_POINTER(z));
		if (mem_view_style[z] == BINARY_VIEW)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		g_signal_connect(G_OBJECT(button),"clicked",
				G_CALLBACK(toggle_button_handler),
				GINT_TO_POINTER(BINARY_VIEW));
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,5);

		button = gtk_radio_button_new_with_label(group,"Decimal");
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
		g_object_set_data(G_OBJECT(button),"data",GINT_TO_POINTER(z));
		if (mem_view_style[z] == DECIMAL_VIEW)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		g_signal_connect(G_OBJECT(button),"clicked",
				G_CALLBACK(toggle_button_handler),
				GINT_TO_POINTER(DECIMAL_VIEW));

		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,5);

		base += range;
	}


	return;
}
