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
#include <ignition_gui.h>
#include <structures.h>


void build_ignition(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *table2;
	GtkWidget *label;
	GtkWidget *spinner;
	GtkAdjustment  *adj;
	extern GtkTooltips *tip;
	extern GList * store_controls;
	GSList *group;
	extern GtkWidget * ign_widgets[];
	extern struct DynamicButtons buttons;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

 /* Commands frame */
        frame = gtk_frame_new("Commands");
        gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

        table = gtk_table_new(1,2,FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table),7);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width(GTK_CONTAINER(table), 5);
        gtk_container_add(GTK_CONTAINER(frame),table);

        button = gtk_button_new_with_label("Get Data from ECU");
        gtk_tooltips_set_tip(tip,button,
                        "Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI",NULL);
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(READ_VE_CONST));

        button = gtk_button_new_with_label("Permanently Store Data in ECU");
        store_controls = g_list_append(store_controls,(gpointer)button);
        gtk_tooltips_set_tip(tip,button,
                        "Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(BURN_MS_FLASH));

/* Ignition Trigger and Cranking Settings */
	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	frame = gtk_frame_new("Ignition Trigger and Cranking Settings");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	
	table = gtk_table_new(5,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_row_spacings(GTK_TABLE(table),10);
        gtk_table_set_row_spacing(GTK_TABLE(table),1,5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Crank Timing:");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,0,1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	
	table2 = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table2),5);
	gtk_table_attach (GTK_TABLE(table),table2,1,3,0,2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Time based crank timing */
	button = gtk_radio_button_new_with_label(NULL, "Time Based");
	buttons.time_based_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        g_object_set_data(G_OBJECT(button),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(85));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(4));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        gtk_table_attach (GTK_TABLE (table2), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

	/* OR Trigger Return (hall effect pickups) based crank timing */
	button = gtk_radio_button_new_with_label(group, "Trigger Return");
	buttons.trig_return_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        g_object_set_data(G_OBJECT(button),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(85));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(4));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        gtk_table_attach (GTK_TABLE (table2), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

	label = gtk_label_new("Ignition Output:");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,1,2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Normal output */
	button = gtk_radio_button_new_with_label(NULL, "Normal");
	buttons.normal_out_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        g_object_set_data(G_OBJECT(button),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(85));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(8));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        gtk_table_attach (GTK_TABLE (table2), button, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

	/* OR Inverted */
	button = gtk_radio_button_new_with_label(group, "Inverted");
	buttons.invert_out_but = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        g_object_set_data(G_OBJECT(button),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(button),"offset",GINT_TO_POINTER(85));
        g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
        g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(button),"bitmask",GINT_TO_POINTER(8));
        g_object_set_data(G_OBJECT(button),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        gtk_table_attach (GTK_TABLE (table2), button, 1,2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(bitmask_button_handler),
                        NULL);

	label = gtk_label_new("Trigger Angle:");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,2,3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Trigger Angle */
        adj =  (GtkAdjustment *) gtk_adjustment_new(72.0,0.0,134.64,0.3516,3.516,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ign_widgets[80] = spinner;
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(80));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(TRIGGER_ANGLE));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_SHRINK|GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Crank Angle:");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,3,4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Crank Angle */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,89.64,0.3516,3.516,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ign_widgets[83] = spinner;
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(83));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Hold Ignition for :");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,4,5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Hold Cycles */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,10,1.0,1.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        ign_widgets[84] = spinner;
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(84));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((1*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(NOTHING));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("cycles at startup");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,2,3,4,5,
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Timing Offset Settings");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	
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
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(82));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
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
        g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(81));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",GINT_TO_POINTER(MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
                        GINT_TO_POINTER(GENERIC));
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	/* Not written yet */
	return;
}
