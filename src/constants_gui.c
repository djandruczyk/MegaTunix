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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"
#include "constants.h"

extern struct v1_2_Constants constants;

int build_constants(GtkWidget *parent_frame)
{
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *entry;
	GtkWidget *hbox;
//	GtkWidget *hbox2;
	GtkWidget *hbox3;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkAdjustment *adj;
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,4);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);

	frame = gtk_frame_new("Required Fuel - One Cylinder (ms)");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,TRUE,0);

	hbox3 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox3);
	gtk_container_set_border_width(GTK_CONTAINER(hbox3),5);
	button = gtk_button_new_with_label("Calculate\nRequired Fuel...");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(reqd_fuel_popup),
				NULL);
	
	gtk_box_pack_start(GTK_BOX(hbox3),button,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox3),vbox3,FALSE,FALSE,0);
	
	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox3),spinner,FALSE,FALSE,0);

	adj = (GtkAdjustment *) gtk_adjustment_new(15.5,0.1,25.5,0.1,0.1,1.0);
	spinner = gtk_spin_button_new(adj,1.0,1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox3),spinner,FALSE,FALSE,0);
	gtk_widget_set_sensitive(spinner,FALSE);


	frame = gtk_frame_new("Injector Opening Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(1,2,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);
	
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(adj), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(INJ_OPEN_TIME));
        constants.inj_open_time = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);

	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,10.0,0.1,1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(adj), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(BATT_CORR));
        constants.batt_corr = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);

	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(1,2,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,0);

	label = gtk_label_new("Inj. Open Time\n(ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Batt Voltage\nCorrection (ms/Volt)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Injector Current Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3),0);

	table = gtk_table_new(1,2,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,5);

	adj =  (GtkAdjustment *) gtk_adjustment_new(50.0,0.0,100.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(adj), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(PWM_CUR_LIM));
        constants.pwm_curr_lim = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,0.0,10.0,0.1,1.0,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(adj), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(PWM_TIME_THRES));
        constants.pwm_time_thresh = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(1,2,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),table,FALSE,FALSE,0);

	label = gtk_label_new("PWM Current Limit\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Time Threshold for\nPWM mode (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);


	frame = gtk_frame_new("Fast Idle Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);

	hbox3 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox3),hbox3,FALSE,FALSE,4);

	adj =  (GtkAdjustment *) gtk_adjustment_new(140.0,0.0,250.0,1.0,10.0,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_widget_set_size_request(spinner,55,-1);
        g_signal_connect (G_OBJECT(adj), "value_changed",
                        G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(FAST_IDLE_THRES));
        constants.fast_idle_thresh = adj;
        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox3),spinner,FALSE,FALSE,65);

	label = gtk_label_new("Fast Idle Threshold\n(Degrees F)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);


	frame = gtk_frame_new("Fuel Injection Control Strategy");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);
	frame = gtk_frame_new("Injection Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);
	
	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	/* Not written yet */
	return TRUE;
}
