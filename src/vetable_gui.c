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
#include <globals.h>
#include <gui_handlers.h>
#include <structures.h>
#include <vetable_gui.h>

extern struct DynamicButtons buttons;
GtkWidget *map_tps_frame;
GtkWidget *map_tps_label;
extern struct Ve_Widgets *page0_widgets;
extern struct Ve_Widgets *page1_widgets;

int build_vetable(GtkWidget *parent_frame)
{
	GtkWidget *sep;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkWidget *frame;
	GtkWidget *button;
	GtkWidget *swin;
	GtkAdjustment *adj;
	gint x,y;
	gint index;
	extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	swin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox),swin,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin),
			vbox2);

	label = gtk_label_new("Vetable 1 (All MS Variants)");
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,TRUE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);


	frame = gtk_frame_new(NULL);
	label = gtk_label_new("MAP Bins");
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(vbox3),frame,FALSE,FALSE,0);
	map_tps_frame = frame;

	table = gtk_table_new(9,1,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* KPA spinbuttons */
	map_tps_label = gtk_label_new("Kpa");
	gtk_table_attach (GTK_TABLE (table), map_tps_label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	index = 0;
	for (y=0;y<8;y++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		page0_widgets->widget[VE1_KPA_BINS_OFFSET+index] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE1_KPA_BINS_OFFSET+index));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(1*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(NOTHING));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spinner_changed),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, y+1, y+2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		index++;

	}


	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);

	frame = gtk_frame_new("Volumetric Efficiency (%)");
	gtk_box_pack_start(GTK_BOX(vbox3),frame,FALSE,FALSE,0);

	table = gtk_table_new(9,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* VeTable spinbuttons */
	label = gtk_label_new(" ");
	gtk_table_attach (GTK_TABLE (table), label, 0, 8, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	index = 0;
	for (y=0;y<8;y++)
	{
		for (x=0;x<8;x++)
		{
			adj =  (GtkAdjustment *) gtk_adjustment_new(
					1.0,1.0,255,1,10,0);
			spinner = gtk_spin_button_new(adj,1,0);
			page0_widgets->widget[VE1_TABLE_OFFSET+index] = spinner;
			g_object_set_data(G_OBJECT(spinner),"page",
					GINT_TO_POINTER(0));
			g_object_set_data(G_OBJECT(spinner),"offset", 
					GINT_TO_POINTER(VE1_TABLE_OFFSET+index));
			g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
					GINT_TO_POINTER(1*100));
			g_object_set_data(G_OBJECT(spinner),"conv_type",
					GINT_TO_POINTER(NOTHING));
			g_object_set_data(G_OBJECT(spinner),"dl_type",
					GINT_TO_POINTER(IMMEDIATE));
			g_signal_connect (G_OBJECT(spinner), "value_changed",
					G_CALLBACK (spinner_changed),
					GINT_TO_POINTER(GENERIC));
			/* Bind data to object for handlers */
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), 
					FALSE);
			gtk_table_attach (GTK_TABLE (table), spinner, 
					x, x+1, y+1, y+2,
					(GtkAttachOptions) (GTK_EXPAND),
					(GtkAttachOptions) (0), 0, 0);
			index++;
		}
	}

	/* RPM Table */
	frame = gtk_frame_new("RPM Bins");
	gtk_box_pack_start(GTK_BOX(vbox3),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	for(x=0;x<8;x++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(100.0,100.0,25500,100,100,0);
		spinner = gtk_spin_button_new(adj,1,0);
		page0_widgets->widget[VE1_RPM_BINS_OFFSET+x] = spinner;
		gtk_widget_set_size_request(spinner,54,-1);
		g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(0));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE1_RPM_BINS_OFFSET+x));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(100*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(DIV));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spinner_changed),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, x, x+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

	}
	/* VEtable 2 *DUAL TABLE ONLY* */
	label = gtk_label_new("\n");
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,TRUE,0);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox2),sep,FALSE,TRUE,0);
	label = gtk_label_new("Vetable 2 (Dualtable ONLY)");
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,TRUE,0);
	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox2),sep,FALSE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_widget_set_sensitive(hbox,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,TRUE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);

	frame = gtk_frame_new(NULL);
	label = gtk_label_new("MAP Bins");
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(vbox3),frame,FALSE,FALSE,0);
	map_tps_frame = frame;

	table = gtk_table_new(9,1,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* KPA spinbuttons */
	map_tps_label = gtk_label_new("Kpa");
	gtk_table_attach (GTK_TABLE (table), map_tps_label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	index = 0;
	for (y=0;y<8;y++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		page1_widgets->widget[VE2_KPA_BINS_OFFSET+index] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(1));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE2_KPA_BINS_OFFSET+index));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(1*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(NOTHING));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spinner_changed),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, y+1, y+2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		index++;

	}


	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);

	frame = gtk_frame_new("Volumetric Efficiency (%)");
	gtk_box_pack_start(GTK_BOX(vbox3),frame,FALSE,FALSE,0);

	table = gtk_table_new(9,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* VeTable spinbuttons */
	label = gtk_label_new(" ");
	gtk_table_attach (GTK_TABLE (table), label, 0, 8, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	index = 0;
	for (y=0;y<8;y++)
	{
		for (x=0;x<8;x++)
		{
			adj =  (GtkAdjustment *) gtk_adjustment_new(
					1.0,1.0,255,1,10,0);
			spinner = gtk_spin_button_new(adj,1,0);
			page1_widgets->widget[VE2_TABLE_OFFSET+index] = spinner;
			g_object_set_data(G_OBJECT(spinner),"page",
					GINT_TO_POINTER(1));
			g_object_set_data(G_OBJECT(spinner),"offset", 
					GINT_TO_POINTER(VE2_TABLE_OFFSET+index));
			g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
					GINT_TO_POINTER(1*100));
			g_object_set_data(G_OBJECT(spinner),"conv_type",
					GINT_TO_POINTER(NOTHING));
			g_object_set_data(G_OBJECT(spinner),"dl_type",
					GINT_TO_POINTER(IMMEDIATE));
			g_signal_connect (G_OBJECT(spinner), "value_changed",
					G_CALLBACK (spinner_changed),
					GINT_TO_POINTER(GENERIC));
			/* Bind data to object for handlers */
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), 
					FALSE);
			gtk_table_attach (GTK_TABLE (table), spinner, 
					x, x+1, y+1, y+2,
					(GtkAttachOptions) (GTK_EXPAND),
					(GtkAttachOptions) (0), 0, 0);
			index++;
		}
	}

	/* RPM Table */
	frame = gtk_frame_new("RPM Bins");
	gtk_box_pack_start(GTK_BOX(vbox3),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	for(x=0;x<8;x++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(100.0,100.0,25500,100,100,0);
		spinner = gtk_spin_button_new(adj,1,0);
		page1_widgets->widget[VE2_RPM_BINS_OFFSET+x] = spinner;
		gtk_widget_set_size_request(spinner,54,-1);
		g_object_set_data(G_OBJECT(spinner),"page",GINT_TO_POINTER(1));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE2_RPM_BINS_OFFSET+x));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(100*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(DIV));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spinner_changed),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, x, x+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
	}


	frame = gtk_frame_new("Commands");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),50);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_FROM_MS));

	button = gtk_button_new_with_label("Permanently Store Data in ECU");
	buttons.vetable_store_but = button;
	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed it, has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(WRITE_TO_MS));



	return TRUE;
}
