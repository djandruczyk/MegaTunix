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

struct v1_2_Constants constants;

int build_enrichments(GtkWidget *parent_frame)
{
        GtkWidget *vbox;
        GtkWidget *vbox2;
        GtkWidget *vbox3;
        GtkWidget *hbox;
        GtkWidget *hbox2;
        GtkWidget *hbox3;
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
	gtk_table_set_col_spacings(GTK_TABLE(table),10);

	gtk_container_add(GTK_CONTAINER(frame),table);


	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_usize(spinner,32,20);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)CRANK_PULSE_NEG_40);
	constants.crank_pulse_40 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,0,1,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)CRANK_PULSE_170);
	constants.crank_pulse_170 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,1,2,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)CRANK_PRIMING_PULSE);
	constants.crank_priming_pulse = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,2,3,0,1);

	label = gtk_label_new("-40 Deg. F");
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);

	label = gtk_label_new("170 Deg. F");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);

	label = gtk_label_new("Priming Pulse");
	gtk_table_attach_defaults(GTK_TABLE(table),label,2,3,1,2);


	frame = gtk_frame_new("Afterstart Enrichment");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);

	gtk_container_add(GTK_CONTAINER(frame),table);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_usize(spinner,32,20);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)AFTERSTART_ENRICH);
	constants.afterstart_enrich = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,0,1,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,25.5,0.1,0.1,0);
        spinner = gtk_spin_button_new(adj,0,1);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)CRANK_PULSE_170);
	constants.afterstart_num_cycles = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,1,2,0,1);

	label = gtk_label_new("Enrichment (%)");
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	label = gtk_label_new("Num of Ign. Cycles");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);


	/* Warmup enrichments */
	frame = gtk_frame_new("Warmup Enrichment Bins (Percent)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(2,10,TRUE);

	gtk_container_add(GTK_CONTAINER(frame),table);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_NEG_40);
	constants.warmup_neg_40 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,0,1,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_NEG_20);
	constants.warmup_neg_20 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,1,2,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_0);
	constants.warmup_0 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,2,3,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_20);
	constants.warmup_20 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,3,4,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_40);
	constants.warmup_40 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,4,5,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_60);
	constants.warmup_60 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,5,6,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_80);
	constants.warmup_80 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,6,7,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_100);
	constants.warmup_100 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,7,8,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_130);
	constants.warmup_130 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,8,9,0,1);

	adj =  (GtkAdjustment *) gtk_adjustment_new(100,100,255,1,10,0);
        spinner = gtk_spin_button_new(adj,0,0);
        gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
                        GTK_SIGNAL_FUNC (spinner_changed),
			(gpointer)WARMUP_160);
	constants.warmup_160 = adj;

        gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table),spinner,9,10,0,1);

	label = gtk_label_new("-40");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	label = gtk_label_new("-20");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);
	label = gtk_label_new("0");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,2,3,1,2);
	label = gtk_label_new("20");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,3,4,1,2);
	label = gtk_label_new("40");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,4,5,1,2);
	label = gtk_label_new("60");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,5,6,1,2);
	label = gtk_label_new("80");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,6,7,1,2);
	label = gtk_label_new("100");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,7,8,1,2);
	label = gtk_label_new("130");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,8,9,1,2);
	label = gtk_label_new("160 deg");
	gtk_misc_set_alignment(GTK_MISC(label),0.2,0.0);
	gtk_table_attach_defaults(GTK_TABLE(table),label,9,10,1,2);
	

	/* Not written yet */
	return TRUE;
}
