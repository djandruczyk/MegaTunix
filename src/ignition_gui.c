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

GList *launch_controls;
GList *enhanced_controls;

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
	extern GList *store_controls;
	GSList *group;
	extern GList *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];
	extern struct DynamicButtons buttons;
	extern struct DynamicLabels labels;

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
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(READ_VE_CONST));
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
			NULL);

        button = gtk_button_new_with_label("Permanently Store Data in ECU");
        store_controls = g_list_append(store_controls,(gpointer)button);
        gtk_tooltips_set_tip(tip,button,
                        "Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(BURN_MS_FLASH));
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
			NULL);

/* Ignition Trigger and Cranking Settings */
	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	frame = gtk_frame_new("Ignition Trigger and Cranking Settings");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	
	table = gtk_table_new(5,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_row_spacings(GTK_TABLE(table),10);
//        gtk_table_set_row_spacing(GTK_TABLE(table),1,5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Crank Timing:");
	labels.timing_multi_lab = label;
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
	buttons.multi_spark_but = button;
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
	buttons.norm_spark_but = button;
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
	labels.output_boost_lab = label;
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,1,2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Normal output */
	button = gtk_radio_button_new_with_label(NULL, "Normal");
	buttons.normal_out_but = button;
	buttons.boost_retard_but = button;
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
	buttons.noboost_retard_but = button;
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
        ve_widgets[1][80] = g_list_append(ve_widgets[1][80],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,50,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(80));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(TRIGGER_ANGLE));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
			NULL);
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
        ve_widgets[1][83] = g_list_append(ve_widgets[1][83],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,50,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(83));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
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
        ve_widgets[1][84] = g_list_append(ve_widgets[1][84],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,50,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(84));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_NOTHING));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("cycles at startup");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,2,3,4,5,
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Rev Limiter Settings");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	
	table = gtk_table_new(5,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacing(GTK_TABLE(table),0,15);
	gtk_table_set_row_spacings(GTK_TABLE(table),3);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Soft RevLimit RPM: ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,0,1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft RevLimit RPM */
        adj =  (GtkAdjustment *) gtk_adjustment_new(7000.0,000.0,25500.0,100.0,1000.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        ve_widgets[1][86] = g_list_append(ve_widgets[1][86],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(86));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((100*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_DIV));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Soft RevLimit SparkAngle: ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,1,2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft RevLimit Sparkangle */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,89.64,0.3516,3.516,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ve_widgets[1][87] = g_list_append(ve_widgets[1][87],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(87));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Max Time on Soft Limiter: ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,2,3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft RevLimit MaxTime */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ve_widgets[1][88] = g_list_append(ve_widgets[1][88],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(88));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((10*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Soft Limiter Cool Down Time: ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,3,4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft RevLimit Cooldown time */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
        spinner = gtk_spin_button_new(adj,1,1);
        ve_widgets[1][89] = g_list_append(ve_widgets[1][89],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(89));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((10*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Hard Rev Limiter RPM: ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,4,5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Hard RevLimit RPM */
        adj =  (GtkAdjustment *) gtk_adjustment_new(7000.0,000.0,25500.0,100.0,1000.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        ve_widgets[1][90] = g_list_append(ve_widgets[1][90],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(90));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((100*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_DIV));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	frame = gtk_frame_new("Launch Control Settings");
        launch_controls = g_list_append(launch_controls,(gpointer)frame);
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	
	table = gtk_table_new(5,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacing(GTK_TABLE(table),0,15);
	gtk_table_set_row_spacings(GTK_TABLE(table),3);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	label = gtk_label_new("Soft Launch Limit RPM: ");
        launch_controls = g_list_append(launch_controls,(gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,0,1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft Launch Limit RPM */
        adj =  (GtkAdjustment *) gtk_adjustment_new(5000.0,000.0,25500.0,100.0,1000.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        launch_controls = g_list_append(launch_controls,(gpointer)spinner);
        ve_widgets[1][95] = g_list_append(ve_widgets[1][95],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(95));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((100*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_DIV));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Soft Launch Limit SparkAngle: ");
        launch_controls = g_list_append(launch_controls,(gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,1,2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft RevLimit Sparkangle */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,89.64,0.3516,3.516,0);
        spinner = gtk_spin_button_new(adj,1,1);
        launch_controls = g_list_append(launch_controls,(gpointer)spinner);
        ve_widgets[1][96] = g_list_append(ve_widgets[1][96],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(96));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((gint)(2.84*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_MULT));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Hard Launch Limit RPM: ");
        launch_controls = g_list_append(launch_controls,(gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,2,3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Soft Launch Limit RPM */
        adj =  (GtkAdjustment *) gtk_adjustment_new(5500.0,000.0,25500.0,100.0,1000.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        launch_controls = g_list_append(launch_controls,(gpointer)spinner);
        ve_widgets[1][97] = g_list_append(ve_widgets[1][97],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(97));
        g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
                        GINT_TO_POINTER((100*100)));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_DIV));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Throttle Pos. to enable Limiter: ");
        launch_controls = g_list_append(launch_controls,(gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,3,4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Throttle Position to enable Limiter */
        adj =  (GtkAdjustment *) gtk_adjustment_new(128.0,0.0,255.0,1,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        launch_controls = g_list_append(launch_controls,(gpointer)spinner);
        ve_widgets[1][98] = g_list_append(ve_widgets[1][98],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(98));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_NOTHING));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Max boost before spark drop: ");
        launch_controls = g_list_append(launch_controls,(gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE(table),label,0,1,4,5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Max boost before spark drop */
        adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        launch_controls = g_list_append(launch_controls,(gpointer)spinner);
        ve_widgets[1][99] = g_list_append(ve_widgets[1][99],(gpointer)spinner);
        gtk_widget_set_size_request(spinner,55,-1);
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_object_set_data(G_OBJECT(spinner),"offset",
			GINT_TO_POINTER(99));
        g_object_set_data(G_OBJECT(spinner),"conv_type",
			GINT_TO_POINTER(CONV_NOTHING));
        g_object_set_data(G_OBJECT(spinner),"ign_parm",
			GINT_TO_POINTER(TRUE));
        g_object_set_data(G_OBJECT(spinner),"dl_type",
                        GINT_TO_POINTER(IMMEDIATE));
        g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(GENERIC));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spin_button_handler),
                        NULL);
        gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	/* Not written yet */
	return;
}
