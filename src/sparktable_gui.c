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
#include <enums.h>
#include <gui_handlers.h>
#include <sparktable_gui.h>
#include <structures.h>

extern GtkWidget *ign_widgets[];

void build_sparktable(GtkWidget *parent_frame)
{
	gint x,y;
	gint index;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *basetable;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkWidget *frame;
	GtkWidget *button;
	GtkAdjustment *adj;
	extern GList *store_controls;
	extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),vbox2,TRUE,TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),0);

	label = gtk_label_new("Spark Advance Table");
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	basetable = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(basetable),5);
	gtk_table_set_row_spacings(GTK_TABLE(basetable),0);
	gtk_box_pack_start(GTK_BOX(hbox),basetable,FALSE,FALSE,0);

	frame = gtk_frame_new(NULL);
	label = gtk_label_new("MAP Bins");
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (basetable), frame, 0, 1, 0, 1,
			(GtkAttachOptions) (0),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(9,1,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Kpa");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	index = 0;
	for (y=0;y<8;y++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ign_widgets[IGN_KPA_BINS_OFFSET+index] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		/* Bind data to object for handlers */
		g_object_set_data(G_OBJECT(spinner),"ign_parm",
				GINT_TO_POINTER(TRUE));
		g_object_set_data(G_OBJECT(spinner),"offset",
				GINT_TO_POINTER(IGN_KPA_BINS_OFFSET+index));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(CONV_NOTHING));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_object_set_data(G_OBJECT(spinner),"handler",
				GINT_TO_POINTER(GENERIC));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spin_button_handler),
				NULL);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner,
				0, 1, (7 - y) + 1, (7 - y) + 2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		index++;

	}

	frame = gtk_frame_new("Spark Timing (\302\260 BTDC)");
	gtk_table_attach (GTK_TABLE (basetable), frame, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(9,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* SparkAdvance spinbuttons */
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
					0.0,0.0,89.648,0.3516,3.516,0);
			spinner = gtk_spin_button_new(adj,1,1);
			ign_widgets[IGN_TABLE_OFFSET+index] = spinner;
			/* Bind data to object for handlers */
			g_object_set_data(G_OBJECT(spinner),"ign_parm",
					GINT_TO_POINTER(TRUE));
			g_object_set_data(G_OBJECT(spinner),"offset",
					GINT_TO_POINTER(IGN_TABLE_OFFSET+index));
			g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
					GINT_TO_POINTER((gint)(2.84*100)));
			g_object_set_data(G_OBJECT(spinner),"conv_type",
					GINT_TO_POINTER(CONV_MULT));
			g_object_set_data(G_OBJECT(spinner),"dl_type",
					GINT_TO_POINTER(IMMEDIATE));
			g_object_set_data(G_OBJECT(spinner),"handler",
					GINT_TO_POINTER(GENERIC));
			g_signal_connect (G_OBJECT(spinner), "value_changed",
					G_CALLBACK (spin_button_handler),
					NULL);
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner),
					FALSE);
			gtk_table_attach (GTK_TABLE (table), spinner,
					x, x+1, (7 - y) + 1, (7 - y) + 2,
					(GtkAttachOptions) (GTK_EXPAND),
					(GtkAttachOptions) (0), 0, 0);
			index++;
		}
	}


	/* RPM Table */
	frame = gtk_frame_new("RPM Bins");
	gtk_table_attach (GTK_TABLE (basetable), frame, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(1,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	for(x=0;x<8;x++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(100.0,100.0,25500,100,100,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ign_widgets[IGN_RPM_BINS_OFFSET+x] = spinner;
		gtk_widget_set_size_request(spinner,54,-1);
		/* Bind data to object for handlers */
		g_object_set_data(G_OBJECT(spinner),"ign_parm",
				GINT_TO_POINTER(TRUE));
		g_object_set_data(G_OBJECT(spinner),"offset",
				GINT_TO_POINTER(IGN_RPM_BINS_OFFSET+x));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(100*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(CONV_DIV));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_object_set_data(G_OBJECT(spinner),"handler",
				GINT_TO_POINTER(GENERIC));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spin_button_handler),
				NULL);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, x, x+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

	}

	button = gtk_button_new_with_label("3D View");
	g_object_set_data(G_OBJECT(button),"table",GINT_TO_POINTER(3));
	g_signal_connect (G_OBJECT(button), "clicked",
			G_CALLBACK (create_3d_view),
			NULL);
	gtk_table_attach (GTK_TABLE (basetable), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE,5);
        gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
	frame = gtk_frame_new("Temporary Timing Offset Controls");
        gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);
        
        table = gtk_table_new(4,2,FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(table),15);
        gtk_table_set_row_spacings(GTK_TABLE(table),10);
        gtk_container_set_border_width(GTK_CONTAINER(table),5);
        gtk_container_add(GTK_CONTAINER(frame),table);

        label = gtk_label_new("Trim Angle:");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE(table),label,0,1,0,1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        /* Trim Angle */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,89.64,0.3516,3.516,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ign_widgets[82] = spinner;
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(82));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_object_set_data(G_OBJECT(spinner),"handler",
			GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new("Fixed Angle:");
        gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
        gtk_table_attach (GTK_TABLE(table),label,0,1,1,2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

        /* Fixed Angle */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,89.64,0.3516,3.516,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ign_widgets[81] = spinner;
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(81));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
	g_object_set_data(G_OBJECT(spinner),"handler",
			GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);


	frame = gtk_frame_new("Commands");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(READ_VE_CONST));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	button = gtk_button_new_with_label("Permanently Store Data in ECU");
	store_controls = g_list_append(store_controls,(gpointer)button);
	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed it, has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(BURN_MS_FLASH));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			NULL);

	/* Not written yet */
	return;
}
