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
GArray *raw_memory = NULL;

void build_memory(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *table2;
	GtkWidget *ebox;
	GtkWidget *frame;
	extern GdkColor white;
	GdkColor purple = { 0, 61000, 57000, 65535};
	gint rows = 32;
	gint cols = 8;
	gint x = 0;
	gint y = 0;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	table = gtk_table_new(rows,1,TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,10);

	for (y=0;y<rows;y++)
	{
		label = gtk_label_new(g_strdup_printf("%#.4x",y*cols));
		gtk_table_attach(GTK_TABLE(table),label,0,1,y,y+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);
	}

	table = gtk_table_new(rows,1,TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	raw_memory = g_array_new(FALSE,TRUE,sizeof(GtkWidget*));

	for (y=0;y<rows;y++)
	{
		frame = gtk_frame_new(NULL);
		ebox = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(frame),ebox);

		gtk_table_attach(GTK_TABLE(table),frame,0,1,y,y+1,
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
			raw_memory = g_array_insert_val(raw_memory,(y*8)+(x),label);
			gtk_container_add(GTK_CONTAINER(ebox),label);
			if (y%2)
				gtk_widget_modify_bg(ebox,GTK_STATE_NORMAL,&purple);
			else
				gtk_widget_modify_bg(ebox,GTK_STATE_NORMAL,&white);

		}
	}
	frame = gtk_frame_new("Raw Memory Settings");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	button = gtk_button_new_with_label("Read RAW Memory\n");
	/* Memory offset to retrieve... */
	g_object_set_data(G_OBJECT(button),"data",GINT_TO_POINTER(0));
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_RAW_MEMORY));
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,5);


	return;
}
