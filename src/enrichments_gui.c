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
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkAdjustment *adj;
	GtkWidget *spinner;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);

	hbox = gtk_hbox_new(FALSE,0);
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
	constants.crank_pulse_40 = adj;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
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
	constants.crank_pulse_170 = adj;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
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
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,2,3,0,1);
	label = gtk_label_new("Priming Pulse");
	gtk_table_attach_defaults(GTK_TABLE(table),label,2,3,1,2);


	frame = gtk_frame_new("Afterstart Enrichment");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);

	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Afterstart Enrichment % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(AFTERSTART_ENRICH));
	constants.afterstart_enrich = adj;

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,0,1,0,1);
	label = gtk_label_new("Enrichment (%)");
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);

	/* Afterstart Enrichment Number of engine cycles */
	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,55,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_170));
	constants.afterstart_num_cycles = adj;

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,1,2,0,1);
	label = gtk_label_new("Num of Ign. Cycles");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);

	/* Warmup enrichments */
	frame = gtk_frame_new(NULL);
	label = gtk_label_new ("Warmup Enrichment Bins (Percent)");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(3,10,FALSE);

	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table), 10);

	/* -40 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.neg40_entry = entry;

	/* -20 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.neg20_entry = entry;

	/* 0 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.neg0_entry = entry;

	/* 20 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos20_entry = entry;

	/* 40 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 4, 5, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos40_entry = entry;

	/* 60 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 5, 6, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos60_entry = entry;

	/* 80 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 6, 7, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos80_entry = entry;

	/* 100 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 7, 8, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos100_entry = entry;

	/* 130 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 8, 9, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos130_entry = entry;

	/* 160 deg entry */
	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 9, 10, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 5);
	constants.pos160_entry = entry;

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

	label = gtk_label_new ("Engine Coolant Temperature");
	gtk_table_attach (GTK_TABLE (table), label, 1, 10, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

	/* Not written yet */
	return TRUE;
}
