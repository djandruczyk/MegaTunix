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

struct v1_2_Constants constants;
const gchar *warmup_labels[] = {"-40","-20",  "0", "20", "40",
			         "60", "80","100","130","160"};
const gchar *accel_labels[] = {"2V/Sec","4V/sec","8V/Sec","15V/Sec"};

int build_enrichments(GtkWidget *parent_frame)
{
	gint i;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkAdjustment *adj;
	GtkWidget *spinner;
	extern GtkTooltips *tip;

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
	constants.cr_pulse_neg40_spin = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(64));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_NEG_40));
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
	constants.cr_pulse_pos170_spin = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(65));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PULSE_170));
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
	constants.cr_priming_pulse_spin = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(119));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(CRANK_PRIMING_PULSE));
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
	spinner = gtk_spin_button_new(adj,0,0);
	constants.as_enrich_spin = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(66));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(AFTERSTART_ENRICH));
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
	constants.as_num_cycles_spin = spinner;
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(67));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(AFTERSTART_NUM_CYCLES));

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
	gtk_table_set_col_spacings (GTK_TABLE (table), 3);

	/* Warmup enrichment bins */
	for (i=0;i<10;i++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		constants.warmup_bins_spin[i] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		g_object_set_data(G_OBJECT(spinner),"class", 
				GINT_TO_POINTER(WARMUP));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(WARMUP_BINS_OFFSET+i));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (classed_spinner_changed),
				NULL);
		gtk_table_attach (GTK_TABLE (table), spinner, i, i+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

		label = gtk_label_new (warmup_labels[i]);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
		gtk_table_attach (GTK_TABLE (table), label, i, i+1, 1, 2,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
	}

	label = gtk_label_new ("Engine Temperature");
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 10, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

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
	gtk_table_set_row_spacing(GTK_TABLE(table),1,10);

	/* TPS trigger threashold */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,1);
	constants.tps_trig_thresh_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner), "offset", 
			GINT_TO_POINTER(83));
	g_object_set_data(G_OBJECT(spinner), "dl_type", 
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(TPS_TRIG_THRESH));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("TPS Trigger Threshold\n(V/Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Accel Enrich Duration (seconds) */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,1);
	constants.accel_duration_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(84));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(ACCEL_ENRICH_DUR));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Accel Enrich\n Duration (Sec)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Cold Accel Addon at -40 deg */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,1);
	constants.cold_accel_addon_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(82));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(COLD_ACCEL_ENRICH));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Cold Accel Enrich\nAdd-On (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Cold Accel Multiplier */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	constants.cold_accel_mult_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(123));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(COLD_ACCEL_MULT));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

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
	gtk_box_pack_start(GTK_BOX(hbox2),table,FALSE,TRUE,10);

	label = gtk_label_new("Acceleration Enrichment Bins (ms)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 4, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Acceleration Enrichment Bins (4) */
	for(i=0;i<4;i++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
		spinner = gtk_spin_button_new(adj,1,1);
		constants.accel_bins_spin[i] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		g_object_set_data(G_OBJECT(spinner),"class", 
				GINT_TO_POINTER(ACCEL));
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(ACCEL_BINS_OFFSET+i));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (classed_spinner_changed),
				NULL);
		/* Bind data to he object for the handlers */
		gtk_table_attach (GTK_TABLE (table), spinner, i, i+1, 1, 2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

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
	
	/* Decel Cut % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	constants.decel_cut_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(85));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(DECEL_CUT));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Decel Fuel Cut(%)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);


	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),
			"Exhaust O<sub>2</sub> Feedback Settings");
			
	frame = gtk_frame_new(NULL);
	gtk_frame_set_label_widget(GTK_FRAME(frame),GTK_WIDGET(label));
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	table = gtk_table_new(6,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,10);
	gtk_table_set_row_spacing(GTK_TABLE(table),3,10);

	/* EGO Temp Activation */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	constants.ego_temp_active_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(86));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(EGO_TEMP_ACTIVE));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Coolant Temp\nActivation(Deg F.)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Active RPM */
	adj =  (GtkAdjustment *) gtk_adjustment_new(
			0.0,100.0,25500.0,100,1000.0,0);
	spinner = gtk_spin_button_new(adj,0,0);
	constants.ego_rpm_active_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(120));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(EGO_RPM_ACTIVE));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO Active RPM");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Switching Voltage */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,25.5,0.1,1.0,0);
	spinner = gtk_spin_button_new(adj,1,2);
	constants.ego_sw_voltage_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(122));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(EGO_SW_VOLTAGE));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO Switching\nVoltage");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Step in % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,100,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	constants.ego_step_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(88));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(EGO_STEP));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("EGO Step\n(Percent)");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO # of ignition events between steps */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	constants.ego_events_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(87));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(EGO_EVENTS));
	gtk_table_attach (GTK_TABLE (table), spinner, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("# of Ignition Events\nBetween Steps");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* EGO Limit % */
	adj =  (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,100.0,1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,1,0);
	constants.ego_limit_spin = spinner;
	gtk_widget_set_size_request(spinner,60,-1);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
	g_object_set_data(G_OBJECT(spinner),"offset",GINT_TO_POINTER(89));
	g_object_set_data(G_OBJECT(spinner),"dl_type",
			GINT_TO_POINTER(IMMEDIATE));
	g_signal_connect(G_OBJECT(spinner),"value_changed",
			G_CALLBACK(spinner_changed),
			GINT_TO_POINTER(EGO_LIMIT));
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

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
	
	button = gtk_button_new_with_label("Permanently Store Data in ECU");
	gtk_tooltips_set_tip(tip,button,
        "Even though MegaTunix writes data to the MS as soon as its changed it has only written it to the MegaSquirt units RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(WRITE_TO_MS));
	
	/* Not written yet */
	return TRUE;
}
