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
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"
#include "constants.h"

struct v1_2_Constants constants;

int build_enrichments(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkAdjustment *adj;
	GtkWidget *spinner;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	frame = gtk_frame_new("Cranking Pulsewidth (ms)");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(2,3,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Cranking pulsewidth at -40deg F */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
//	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_NEG_40));
	constants.crank_pulse_neg40 = adj;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,0,1,0,1);
	label = gtk_label_new("-40 Deg. F");
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);

	/* Cranking pulsewidth at 170deg F */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_170));
	constants.crank_pulse_pos170 = adj;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,1,2,0,1);
	label = gtk_label_new("170 Deg. F");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);

	/* Priming pulse  */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PRIMING_PULSE));
	constants.crank_priming_pulse = adj;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,2,3,0,1);
	label = gtk_label_new("Priming Pulse");
	gtk_table_attach_defaults(GTK_TABLE(table),label,2,3,1,2);


	frame = gtk_frame_new("Afterstart Enrich");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Afterstart Enrich % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(35.0,1.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(AFTERSTART_ENRICH));
	constants.afterstart_enrich = adj;

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,0,1,0,1);
	label = gtk_label_new("Enrich (%)");
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);

	/* Afterstart Enrich Number of engine cycles */
	adj =  (GtkAdjustment *) gtk_adjustment_new(250.0,1.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_170));
	constants.afterstart_num_cycles = adj;

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,1,2,0,1);
	label = gtk_label_new("# of Cycles");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);

	/* Warmup enrichments */
	frame = gtk_frame_new(NULL);
	label = gtk_label_new ("Warmup Enrich Bins (Percent)");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(3,10,FALSE);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,TRUE,0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table), 10);

	/* -40 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_neg_40 = entry;

	/* -20 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_neg_20 = entry;

	/* 0 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_0 = entry;

	/* 20 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_20 = entry;

	/* 40 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 4, 5, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_40 = entry;

	/* 60 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 5, 6, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_60 = entry;

	/* 80 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 6, 7, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_80 = entry;

	/* 100 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 7, 8, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_100 = entry;

	/* 130 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 8, 9, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_130 = entry;

	/* 160 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 9, 10, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
	constants.warmup_160 = entry;

	label = gtk_label_new ("-40");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("-20");
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("0");
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("20");
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("40");
	gtk_table_attach (GTK_TABLE (table), label, 4, 5, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("60");
	gtk_table_attach (GTK_TABLE (table), label, 5, 6, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("80");
	gtk_table_attach (GTK_TABLE (table), label, 6, 7, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("100");
	gtk_table_attach (GTK_TABLE (table), label, 7, 8, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("130");
	gtk_table_attach (GTK_TABLE (table), label, 8, 9, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("160");
	gtk_table_attach (GTK_TABLE (table), label, 9, 10, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	label = gtk_label_new ("Engine Temperature");
	gtk_table_attach (GTK_TABLE (table), label, 0, 10, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	frame = gtk_frame_new("Acceleration Enrichments");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);
	
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(4,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,TRUE,0);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,20);

	/* TPS trigger threashold */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	constants.tps_trig_thresh = entry;

	label = gtk_label_new("TPS Trigger Threshold\n(V/Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	constants.accel_duration = entry;

	label = gtk_label_new("Accel Enrich\n Duration (Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	constants.cold_accel_addon = entry;

	label = gtk_label_new("Cold Accel Enrich\nAdd-On (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	constants.cold_accel_mult = entry;

	label = gtk_label_new("Cold Accel Enrich\nMultiplier (%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox2,FALSE,TRUE,0);

	table = gtk_table_new(3,4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start(GTK_BOX(hbox2),table,FALSE,TRUE,20);

	label = gtk_label_new("Acceleration Enrichment Bins (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 4, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.accel_2v_sec = entry;
	
	label = gtk_label_new("2 V/Sec");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.accel_4v_sec = entry;

	label = gtk_label_new("4 V/Sec");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.accel_8v_sec = entry;

	label = gtk_label_new("8 V/Sec");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.accel_15v_sec = entry;

	label = gtk_label_new("15 V/Sec");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start(GTK_BOX(vbox2),table,TRUE,TRUE,0);
	
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.decel_cut = entry;

	label = gtk_label_new("Decel Fuel Cut\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);


	frame = gtk_frame_new("Exaust Gas Oxygen Feedback Settings");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(6,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,20);
	gtk_table_set_row_spacing(GTK_TABLE(table),3,20);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.ego_temp_active = entry;

	label = gtk_label_new("Coolant Temp Activation\n(Deg F.)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.ego_rpm_active = entry;

	label = gtk_label_new("EGO Active RPM");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.ego_sw_voltage = entry;

	label = gtk_label_new("EGO Switching Voltage");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.ego_step = entry;

	label = gtk_label_new("EGO Step (Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.ego_events = entry;

	label = gtk_label_new("# of Ignition Events\nBetween Steps");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	constants.ego_limit = entry;

	label = gtk_label_new("EGO +/- Limit (Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),50);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	button = gtk_button_new_with_label("Send Data to ECU");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	
	



	/* Not written yet */
	return TRUE;
}
