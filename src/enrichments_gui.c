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
const gchar *warmup_labels[] = {"-40","-20",  "0", "20", "40",
			         "60", "80","100","130","160"};
const gint warmup_ptrs[] = {WARMUP_NEG_40,WARMUP_NEG_20,WARMUP_0,WARMUP_20,
				WARMUP_40,WARMUP_60,WARMUP_80,WARMUP_100,
				WARMUP_130,WARMUP_160};
const gchar *accel_labels[] = {"2V/Sec","4V/sec","8V/Sec","15V/Sec"};
static gint warmup_bins_offset = 69;
static gint accel_bins_offset = 79;

int build_enrichments(GtkWidget *parent_frame)
{
	gint i;
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
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_NEG_40));
	constants.cr_pulse_neg40_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(65));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("-40 Deg. F");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);

	/* Cranking pulsewidth at 170deg F */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_170));
	constants.cr_pulse_pos170_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(66));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("170 Deg. F");
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);

	/* Priming pulse  */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PRIMING_PULSE));
	constants.cr_priming_pulse_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(120));
	gtk_table_attach (GTK_TABLE (table), spinner, 2, 3, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("Priming Pulse");
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);

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
	constants.as_enrich_spin = spinner;

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(67));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("Enrich (%)");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);

	/* Afterstart Enrich Number of engine cycles */
	adj =  (GtkAdjustment *) gtk_adjustment_new(250.0,1.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_170));
	constants.as_num_cycles_spin = spinner;

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	gtk_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(68));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new("# of Cycles");
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);

	/* Warmup enrichments */
	frame = gtk_frame_new(NULL);
	label = gtk_label_new ("Warmup Enrichment Bins (Percent)");
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

	/* Warmup enrichment bins */
	for (i=0;i<10;i++)
	{
		entry = gtk_entry_new ();
		gtk_table_attach (GTK_TABLE (table), entry, i, i+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		gtk_entry_set_width_chars(GTK_ENTRY (entry), 4);
		gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
		gtk_object_set_data(G_OBJECT(entry),"offset", GINT_TO_POINTER(warmup_bins_offset+i));
		constants.warmup_bins_ent[i] = entry;
		g_signal_connect(G_OBJECT(entry),"changed",
				G_CALLBACK(text_entry_handler),
				GINT_TO_POINTER(warmup_ptrs[i]));

		label = gtk_label_new (warmup_labels[i]);
		gtk_table_attach (GTK_TABLE (table), label, i, i+1, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	}

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
	gtk_entry_set_width_chars(GTK_ENTRY (entry), 8);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(84));
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
//	g_signal_connect(G_OBJECT(entry),"changed",
//			G_CALLBACK(text_entry_handler)
//			GINT_TO_POINTER(
	constants.tps_trig_thresh_ent = entry;

	label = gtk_label_new("TPS Trigger Threshold\n(V/Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars(GTK_ENTRY (entry), 8);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(85));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.accel_duration_ent = entry;

	label = gtk_label_new("Accel Enrich\n Duration (Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars(GTK_ENTRY (entry), 8);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(84));
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.cold_accel_addon_ent = entry;

	label = gtk_label_new("Cold Accel Enrich\nAdd-On (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars(GTK_ENTRY (entry), 8);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(124));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.cold_accel_mult_ent = entry;

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

	/* Acceleration Enrichment Bins (4) */
	for(i=0;i<4;i++)
	{
		entry = gtk_entry_new ();
		gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
		gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
		gtk_object_set_data(G_OBJECT(entry),"offset",
				GINT_TO_POINTER(accel_bins_offset+i));
		gtk_table_attach (GTK_TABLE (table), entry, i, i+1, 1, 2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		constants.accel_bins_ent[i] = entry;

		label = gtk_label_new(accel_labels[i]);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
		gtk_table_attach (GTK_TABLE (table), label, i, i+1, 2, 3,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
	}

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start(GTK_BOX(vbox2),table,TRUE,TRUE,0);
	
	entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(86));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.decel_cut_ent = entry;

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
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(87));
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.ego_temp_active_ent = entry;

	label = gtk_label_new("Coolant Temp\nActivation(Deg F.)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(121));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.ego_rpm_active_ent = entry;

	label = gtk_label_new("EGO Active RPM");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(123));
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.ego_sw_voltage_ent = entry;

	label = gtk_label_new("EGO Switching\nVoltage");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(89));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.ego_step_ent = entry;

	label = gtk_label_new("EGO Step\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(88));
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.ego_events_ent = entry;

	label = gtk_label_new("# of Ignition Events\nBetween Steps");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 7);
	gtk_entry_set_max_length(GTK_ENTRY (entry), 3);
	gtk_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(90));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	constants.ego_limit_ent = entry;

	label = gtk_label_new("EGO +/- Limit\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);

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
