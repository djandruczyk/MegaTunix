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
#include "runtime_gui.h"


struct v1_2_Runtime_Gui runtime_data;

int build_runtime(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *sep;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Real-Time Variables");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	vbox2 = gtk_vbox_new(FALSE,10);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	hbox = gtk_hbox_new(FALSE,10);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,10);
	label = gtk_label_new("Seconds");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("MAP (Kpa)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Coolant Temp (deg F)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Batt Voltage");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Gamma");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.secl_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.map_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.clt_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.batt_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.gammae_val = entry;/*copy pointer to struct for update */

	sep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox),sep,TRUE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);
	label = gtk_label_new("Barometer (Kpa)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Manifold Air Temp (F)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Throttle Pos (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("RPM");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Pulse Width (ms)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.baro_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.mat_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.tps_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.rpm_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.pw_val = entry;/*copy pointer to struct for update */

	/* Corrections/Enrichments frame */

	frame = gtk_frame_new("Corrections/Enrichments (Percent)");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	vbox2 = gtk_vbox_new(FALSE,10);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	hbox = gtk_hbox_new(FALSE,10);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,10);
	label = gtk_label_new("EGO");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Barometer                  ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Warmup");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.egocorr_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.barocorr_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.warmcorr_val = entry;/*copy pointer to struct for update */

	sep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox),sep,TRUE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);
	label = gtk_label_new("Air Density ");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Volumetric Efficiency");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);
	label = gtk_label_new("Acceleration (millisec)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox3),label,FALSE,FALSE,0);

	vbox3 = gtk_vbox_new(TRUE,7);
	gtk_box_pack_start(GTK_BOX(hbox),vbox3,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.aircorr_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.vecurr_val = entry;/*copy pointer to struct for update */

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_widget_set_usize(entry,64,20);
//	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox3),entry,FALSE,FALSE,0);
	runtime_data.tpsaccel_val = entry;/*copy pointer to struct for update */




	return TRUE;
}

void update_runtime_vars()
{
	char buff[10];
	/* test to see if data changed 
	 * Why bother wasting CPU to update the GUI when 
	 * you'd just print the same damn thing?
	 * Makes the code a little uglier, but the gui won't
	 * flicker the text widgets at high update rates
	 */
	gdk_threads_enter();
	if (out.secl != out_last.secl)
	{
		g_snprintf(buff,10,"%i",out.secl);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.secl_val),buff);
	}
	if (out.map != out_last.map)
	{
		g_snprintf(buff,10,"%i",out.map);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.map_val),buff);
	}
	if (out.clt != out_last.clt)
	{
		g_snprintf(buff,10,"%i",out.clt);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.clt_val),buff);
	}
	if (out.batt != out_last.batt)
	{
		g_snprintf(buff,10,"%.2f",out.batt);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.batt_val),buff);
	}
	if (out.gammae != out_last.gammae)
	{
		g_snprintf(buff,10,"%i",out.gammae);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.gammae_val),buff);
	}
	if (out.baro != out_last.baro)
	{
		g_snprintf(buff,10,"%i",out.baro);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.baro_val),buff);
	}
	if (out.mat != out_last.mat)
	{
		g_snprintf(buff,10,"%i",out.mat);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.mat_val),buff);
	}
	if (out.tps != out_last.tps)
	{
		g_snprintf(buff,10,"%i",out.tps);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.tps_val),buff);
	}
	if (out.rpm != out_last.rpm)
	{
		g_snprintf(buff,10,"%i",out.rpm);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.rpm_val),buff);
	}
	if (out.pw != out_last.pw)
	{
		g_snprintf(buff,10,"%.1f",out.pw);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.pw_val),buff);
	}
	if (out.egocorr != out_last.egocorr)
	{
		g_snprintf(buff,10,"%i",out.egocorr);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.egocorr_val),buff);
	}
	if (out.barocorr != out_last.barocorr)
	{
		g_snprintf(buff,10,"%i",out.barocorr);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.barocorr_val),buff);
	}
	if (out.warmcorr != out_last.warmcorr)
	{
		g_snprintf(buff,10,"%i",out.warmcorr);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.warmcorr_val),buff);
	}
	if (out.aircorr != out_last.aircorr)
	{
		g_snprintf(buff,10,"%i",out.aircorr);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.aircorr_val),buff);
	}
	if (out.vecurr != out_last.vecurr)
	{
		g_snprintf(buff,10,"%i",out.vecurr);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.vecurr_val),buff);
	}
	if (out.tpsaccel != out_last.tpsaccel)
	{
		g_snprintf(buff,10,"%i",out.tpsaccel);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.tpsaccel_val),buff);
	}
	gdk_threads_leave();
}
	
