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
#include <defines.h>
#include <protos.h>
#include <globals.h>
#include <constants.h>

extern struct v1_2_Constants constants;

int build_constants(GtkWidget *parent_frame)
{
	GtkWidget *sep;
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkAdjustment *adj;
	GSList	*group;
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

//      Equation for determining Req_fuel_download from Req_fuel:
//
//      REQ_FUEL_DL = REQ*FUEL * (B * N)/NINJ
//
//      B = 1 if simultaneous, 2 = Alternate
//      N = divder_number = ncyl/numer_of_squirts
//      NINJ = Number of Inejctors   

	frame = gtk_frame_new("Required Fuel - One Cylinder (ms)");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Calculate\nRequired Fuel...");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
				GINT_TO_POINTER(REQD_FUEL_POPUP));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	/* Required Fuel */
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_1));
        constants.req_fuel_1_spin = spinner;
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(91));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_2));
	gtk_widget_set_sensitive(spinner,FALSE);
        constants.req_fuel_2_spin = spinner;
//	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(91));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Injector Opening Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);
	
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,60,-1);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(94));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(INJ_OPEN_TIME));
        constants.inj_open_time_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(94));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,60,-1);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(98));
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(BATT_CORR));
        constants.batt_corr_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(98));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Inj. Open Time\n(ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Batt Voltage\nCorrection (ms/V)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Injector Current Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);

	adj =  (GtkAdjustment *) gtk_adjustment_new(50.0,0.0,100.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,1,0);
        gtk_widget_set_size_request(spinner,60,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(PWM_CUR_LIM));
        constants.pwm_curr_lim_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(96));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,25.5,0.1,1.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,60,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(PWM_TIME_THRES));
        constants.pwm_time_max_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(97));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("PWM Current\n Limit (%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("PWM Time \nThreshold (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Fast Idle Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);


	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	adj =  (GtkAdjustment *) gtk_adjustment_new(140.0,0.0,250.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,60,-1);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(FAST_IDLE_THRES));
        constants.fast_idle_thresh_spin = spinner;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(122));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Fast Idle Threshold\n(Degrees F)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);



	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,TRUE,TRUE,0);

	frame = gtk_frame_new("Injection Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,TRUE,TRUE,0);

	/* Injection Control Section */
	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);

	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);
	
	/* Fuel Injection Control Strategy */
	label = gtk_label_new("Fuel Injection Control Strategy");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Speed Density");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	constants.speed_den_but = button;

	button = gtk_radio_button_new_with_label(group,"Alpha-N");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	constants.alpha_n_but = button;

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Injection Type selectors */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);

	label = gtk_label_new("Injection Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Multi-Port");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"Throttle-Body");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Engine stroke selectors */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);
	
	label = gtk_label_new("Engine Stroke");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Four-Stroke");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"Two-Stroke");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Engine Firing Type selectors */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);

	label = gtk_label_new("Engine Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Even Fire");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"Odd Fire");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* MAP Sensor Type selectors */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);

	label = gtk_label_new("MAP Sensor Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"115 kPa");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"250 kPa");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(11));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* O2 Sensor Type selectors */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);

	label = gtk_label_new("O2 Sensor Type");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Narrow-Band");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"Wide-Band");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Baro Correction Selectors */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);

	label = gtk_label_new("Baro Correction");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Enabled");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"Disabled");
	g_object_set_data(G_OBJECT(button),"config_num",GINT_TO_POINTER(13));
	g_object_set_data(G_OBJECT(button),"bit_pos",GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(button),"bit_val",GINT_TO_POINTER(0));
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox3),sep,FALSE,TRUE,0);

	/* Injector Staging */
	table = gtk_table_new(2,2,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox3),table,TRUE,TRUE,0);

	label = gtk_label_new("Injector Staging");
	gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_radio_button_new_with_label(NULL,"Simultaneous");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);
	button = gtk_radio_button_new_with_label(group,"Alternate");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 10, 0);
	g_signal_connect(G_OBJECT(button),"toggled",
			G_CALLBACK(toggle_button_handler),
			NULL);

	/* Injection Control cyls/injectors, etc.. */
	frame = gtk_frame_new("Cylinder/Injection Configuration");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);
	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_add(GTK_CONTAINER(frame),table);

	adj = (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			NULL);
//			GINT_TO_POINTER(INJ_PER_CYCLE));
        constants.inj_per_cycle_spin = spinner;
//	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(91));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("# of Injections per Cycle");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	adj = (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			NULL);
//			GINT_TO_POINTER(NUM_INJECTORS));
        constants.injectors_spin = spinner;
//	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(91));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("# of Fuel Injectors");
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	adj = (GtkAdjustment *) gtk_adjustment_new(0.0,1.0,12,1.0,1.0,0.0);
	spinner = gtk_spin_button_new(adj,1,0);
        gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
        g_signal_connect (G_OBJECT(spinner), "value_changed",
                        G_CALLBACK (spinner_changed),
			NULL);
//			GINT_TO_POINTER(NUM_CYLINDERS));
        constants.cylinders_spin = spinner;
//	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(91));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("# of Cylinders");
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Commands frame */
	frame = gtk_frame_new("Commands");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
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
	
	/* Not written yet */
	return TRUE;
}
