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
	button = gtk_button_new_with_label("Calculate\nRequired Fuel...");
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(calc_reqd_fuel_func),
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

	hbox3 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox3);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox3),vbox3,FALSE,FALSE,0);

	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,4);
	gtk_signal_connect(GTK_OBJECT(entry),"activate",
			GTK_SIGNAL_FUNC (text_entry_handler),
			(gpointer)INJ_OPEN_TIME);

	label = gtk_label_new("Injector Open Time\n(milliseconds)");
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox3),vbox3,FALSE,FALSE,0);

	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,4);
	gtk_signal_connect(GTK_OBJECT(entry),"activate",
			GTK_SIGNAL_FUNC (text_entry_handler),
			(gpointer)BATT_CORR);

	label = gtk_label_new("Battery Voltage\nCorrection (ms/Volt)");
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	frame = gtk_frame_new("Injector Current Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	hbox3 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox3);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox3),vbox3,FALSE,FALSE,0);

	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,4);
	gtk_signal_connect(GTK_OBJECT(entry),"activate",
			GTK_SIGNAL_FUNC (text_entry_handler),
			(gpointer)PWM_CUR_LIM);

	label = gtk_label_new("PWM Current Limit\n(Percent)");
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox3),vbox3,FALSE,FALSE,0);

	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,4);
	gtk_signal_connect(GTK_OBJECT(entry),"activate",
			GTK_SIGNAL_FUNC (text_entry_handler),
			(gpointer)PWM_TIME_THRES);

	label = gtk_label_new("Time Threshold for\nPWM mode (ms)");
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);


	frame = gtk_frame_new("Fast Idle Control");
	gtk_box_pack_start(GTK_BOX(vbox2),frame,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox3);

	hbox3 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox3),hbox3,FALSE,FALSE,4);
	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(hbox3),entry,FALSE,FALSE,65);
	gtk_signal_connect(GTK_OBJECT(entry),"activate",
			GTK_SIGNAL_FUNC (text_entry_handler),
			(gpointer)FAST_IDLE_THRES);

	label = gtk_label_new("Fast Idle Threshold\n(Degrees F)");
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
