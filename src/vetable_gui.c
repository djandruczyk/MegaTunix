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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"
#include "protos.h"
#include "globals.h"

extern struct v1_2_Constants constants;
extern struct ms_ve_constants ve_constants;
static int kpa_offset = 109;
static int rpm_offset = 101;

int build_vetable(GtkWidget *parent_frame)
{
        GtkWidget *vbox;
        GtkWidget *vbox2;
        GtkWidget *hbox;
        GtkWidget *label;
        GtkWidget *table;
        GtkWidget *spinner;
        GtkWidget *frame;
        GtkWidget *button;
        GtkAdjustment *adj;
	gint x,y;
	gint index;

        vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
        gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	hbox = gtk_hbox_new(FALSE,5);
        gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

        vbox2 = gtk_vbox_new(FALSE,0);
        gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);

	frame = gtk_frame_new("MAP Bins");
        gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	table = gtk_table_new(9,1,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
        gtk_container_add(GTK_CONTAINER(frame),table);

	/* KPA spinbuttons */
	label = gtk_label_new("Kpa");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	for (y=0;y<8;y++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		gtk_widget_set_size_request(spinner,45,-1);
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (kpa_spinner_changed),
				GINT_TO_POINTER(kpa_offset+y));
		constants.kpa_bins_spin[y] = spinner;
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, y+1, y+2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

	}


	vbox2 = gtk_vbox_new(FALSE,0);
        gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);

	frame = gtk_frame_new("Volumetric Efficiency (%)");
        gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);
	
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
			adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
			spinner = gtk_spin_button_new(adj,1,0);
			gtk_widget_set_size_request(spinner,52,-1);
			g_signal_connect (G_OBJECT(spinner), "value_changed",
					G_CALLBACK (ve_spinner_changed),
					GINT_TO_POINTER(index));
			constants.ve_bins_spin[index] = spinner;
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
			gtk_table_attach (GTK_TABLE (table), spinner, x, x+1, y+1, y+2,
					(GtkAttachOptions) (GTK_EXPAND),
					(GtkAttachOptions) (0), 0, 0);

			index++;
		}
	}

	/* RPM Table */
	frame = gtk_frame_new("RPM Bins");
        gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
        gtk_container_add(GTK_CONTAINER(frame),table);

	for(x=0;x<8;x++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(100.0,100.0,25500,100,100,0);
		spinner = gtk_spin_button_new(adj,1,0);
		gtk_widget_set_size_request(spinner,52,-1);
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (rpm_spinner_changed),
				GINT_TO_POINTER(rpm_offset+x));
		constants.rpm_bins_spin[x] = spinner;
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
	
	button = gtk_button_new_with_label("Send Data to ECU");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(WRITE_TO_MS));
	


	return TRUE;
}
