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
#include <runtime_gui.h>


struct v1_2_Runtime_Gui runtime_data;
GtkWidget *run_statbar;
gint run_context_id;

int build_runtime(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *pbar;
	GtkWidget *button;

	vbox = gtk_vbox_new(FALSE,0);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Runtime Status Messages");
        gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

        vbox2 = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(frame),vbox2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);

        run_statbar = gtk_statusbar_new();
        gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(run_statbar),FALSE);
        gtk_box_pack_start(GTK_BOX(vbox2),run_statbar,TRUE,TRUE,0);
        run_context_id = gtk_statusbar_get_context_id(
                        GTK_STATUSBAR(run_statbar),
                        "Runtime Status");

	frame = gtk_frame_new("Real-Time Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(6,6,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width (GTK_CONTAINER (table), 10);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* Seconds Counter Label */
	label = gtk_label_new("Seconds");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	/* Seconds Counter from MS */
	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.secl_lab = label;

	/* O2 Voltage Label*/
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),"O<sub>2</sub> (V)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	/* O2 Voltage value*/
	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.ego_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.ego_pbar = pbar;
	
	/* MAP Sensor */	
	label = gtk_label_new("MAP (kPa)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.map_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.map_pbar = pbar;

	/* Coolant */
	label = gtk_label_new("Coolant (F)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.clt_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.clt_pbar = pbar;
	
	
	/* Manifold Air Temp */
	label = gtk_label_new("MAT (F)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.mat_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.mat_pbar = pbar;

	/* Battery Voltage */
	label = gtk_label_new("Batt (V)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.batt_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 3, 4,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.batt_pbar = pbar;
	
	
	/* RPM */
	label = gtk_label_new("RPM");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 3, 4,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.rpm_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 3, 4,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.rpm_pbar = pbar;

	/* TPS */
	label = gtk_label_new("TPS (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.tps_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 4, 5,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.tps_pbar = pbar;
	
	
	/* PW */
	label = gtk_label_new("PW (ms)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 4, 5,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 4, 5,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.pw_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 4, 5,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.pw_pbar = pbar;

	/* GammaE (sum of enrichments) */
	label = gtk_label_new("GammaE (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.gammae_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 5, 6,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.gammae_pbar = pbar;
	
	
	/* Duty Cycle */
	label = gtk_label_new("Duty Cyc(%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 5, 6,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 5, 6,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.dcycle_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 5, 6,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.dcycle_pbar = pbar;
	
	/* Corrections/Enrichments frame */

	frame = gtk_frame_new("Corrections/Enrichments (Percent)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);

	table = gtk_table_new(3,6,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width (GTK_CONTAINER (table), 10);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* EGOcorr  */
	label = gtk_label_new("EGO (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.egocorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.egocorr_pbar = pbar;
	
	
	/* Air Den Cycle */
	label = gtk_label_new("AirDen (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.aircorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.aircorr_pbar = pbar;
	
	/* Baro Correction  */
	label = gtk_label_new("Baro (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.barocorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.barocorr_pbar = pbar;
	
	
	/* Ve  */
	label = gtk_label_new("VE (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.vecurr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.vecurr_pbar = pbar;
	
	/* Warmup Correction  */
	label = gtk_label_new("Warmup (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.warmcorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.warmcorr_pbar = pbar;
	
	/* Accel Boost  */
	label = gtk_label_new("Accel (ms)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_table_attach (GTK_TABLE (table), label, 3, 4, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
        gtk_table_attach (GTK_TABLE (table), label, 4, 5, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.tpsaccel_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,100,-1);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
        gtk_table_attach (GTK_TABLE (table), pbar, 5, 6, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
	runtime_data.tpsaccel_pbar = pbar;
	
	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width (GTK_CONTAINER (table), 10);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Start Reading RT Vars");
        g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(START_REALTIME));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Stop Reading RT vars");
        g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(STOP_REALTIME));
        gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	return TRUE;
}

void update_runtime_vars()
{
	char buff[120];
	gint pos = 0;
	gfloat tmpf;
	extern struct ms_data_v1_and_v2 *runtime;
	extern struct ms_data_v1_and_v2 *runtime_last;
	/* test to see if data changed 
	 * Why bother wasting CPU to update the GUI when 
	 * you'd just print the same damn thing?
	 * Makes the code a little uglier, but the gui won't
	 * flicker the text widgets at high update rates
	 */

	gdk_threads_enter();
	if (runtime->secl != runtime_last->secl)
	{
		g_snprintf(buff,10,"%i",runtime->secl);
		gtk_label_set_text(GTK_LABEL(runtime_data.secl_lab),buff);
	}
	if (runtime->ego != runtime_last->ego)
	{
		g_snprintf(buff,10,"%.2f",runtime->ego);
		gtk_label_set_text(GTK_LABEL(runtime_data.ego_lab),buff);
		tmpf = runtime->ego/5.0 <= 1.0 ? runtime->ego/5.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.ego_pbar),
				tmpf);
	}
	if (runtime->map != runtime_last->map)
	{
		g_snprintf(buff,10,"%i",runtime->map);
		gtk_label_set_text(GTK_LABEL(runtime_data.map_lab),buff);
		tmpf = runtime->map/255.0 <= 1.0 ? runtime->map/255.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.map_pbar),
				tmpf);
	}
	if (runtime->clt != runtime_last->clt)
	{
		g_snprintf(buff,10,"%i",runtime->clt-40);
		gtk_label_set_text(GTK_LABEL(runtime_data.clt_lab),buff);
		tmpf = (runtime->clt-40)/255.0 <= 1.0 ? (runtime->clt-40)/255.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.clt_pbar),
				tmpf);
	}
	if (runtime->batt != runtime_last->batt)
	{
		g_snprintf(buff,10,"%.2f",runtime->batt);
		gtk_label_set_text(GTK_LABEL(runtime_data.batt_lab),buff);
		tmpf = runtime->batt/18.0 <= 1.0 ? runtime->batt/18.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.batt_pbar),
				tmpf);
	}
	if (runtime->gammae != runtime_last->gammae)
	{
		g_snprintf(buff,10,"%i",runtime->gammae);
		gtk_label_set_text(GTK_LABEL(runtime_data.gammae_lab),buff);
		tmpf = runtime->gammae/200.0 <= 1.0 ? runtime->gammae/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.gammae_pbar),
				tmpf);
	}
	if (runtime->mat != runtime_last->mat)
	{
		g_snprintf(buff,10,"%i",runtime->mat-40);
		gtk_label_set_text(GTK_LABEL(runtime_data.mat_lab),buff);
		tmpf = (runtime->mat-40)/255.0 <= 1.0 ? (runtime->mat-40)/255.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.mat_pbar),
				tmpf);
	}
	if (runtime->tps != runtime_last->tps)
	{
		g_snprintf(buff,10,"%i",runtime->tps);
		gtk_label_set_text(GTK_LABEL(runtime_data.tps_lab),buff);
		tmpf = runtime->tps/100.0 <= 1.0 ? runtime->tps/100.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.tps_pbar),
				tmpf);
	}
	if (runtime->rpm != runtime_last->rpm)
	{
		g_snprintf(buff,10,"%i",runtime->rpm);
		gtk_label_set_text(GTK_LABEL(runtime_data.rpm_lab),buff);
		tmpf = runtime->rpm/8000.0 <= 1.0 ? runtime->rpm/8000.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.rpm_pbar),
				tmpf);
	}
	if (runtime->pw != runtime_last->pw)
	{
		g_snprintf(buff,10,"%.1f",runtime->pw);
		gtk_label_set_text(GTK_LABEL(runtime_data.pw_lab),buff);
		tmpf = runtime->pw/25.5 <= 1.0 ? runtime->pw/25.5: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.pw_pbar),
				tmpf);
	}
	if (runtime->egocorr != runtime_last->egocorr)
	{
		g_snprintf(buff,10,"%i",runtime->egocorr);
		gtk_label_set_text(GTK_LABEL(runtime_data.egocorr_lab),buff);
		tmpf = runtime->egocorr/200.0 <= 1.0 ? runtime->egocorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.egocorr_pbar),
				tmpf);
	}
	if (runtime->barocorr != runtime_last->barocorr)
	{
		g_snprintf(buff,10,"%i",runtime->barocorr);
		gtk_label_set_text(GTK_LABEL(runtime_data.barocorr_lab),buff);
		tmpf = runtime->barocorr/200.0 <= 1.0 ? runtime->barocorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.barocorr_pbar),
				tmpf);
	}
	if (runtime->warmcorr != runtime_last->warmcorr)
	{
		g_snprintf(buff,10,"%i",runtime->warmcorr);
		gtk_label_set_text(GTK_LABEL(runtime_data.warmcorr_lab),buff);
		tmpf = runtime->warmcorr/200.0 <= 1.0 ? runtime->warmcorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.warmcorr_pbar),
				tmpf);
	}
	if (runtime->aircorr != runtime_last->aircorr)
	{
		g_snprintf(buff,10,"%i",runtime->aircorr);
		gtk_label_set_text(GTK_LABEL(runtime_data.aircorr_lab),buff);
		tmpf = runtime->aircorr/200.0 <= 1.0 ? runtime->aircorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.aircorr_pbar),
				tmpf);
	}
	if (runtime->vecurr != runtime_last->vecurr)
	{
		g_snprintf(buff,10,"%i",runtime->vecurr);
		gtk_label_set_text(GTK_LABEL(runtime_data.vecurr_lab),buff);
		tmpf = runtime->vecurr/200.0 <= 1.0 ? runtime->vecurr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.vecurr_pbar),
				tmpf);
	}
	if (runtime->tpsaccel != runtime_last->tpsaccel)
	{
		g_snprintf(buff,10,"%.1f",runtime->tpsaccel/10.0);
		gtk_label_set_text(GTK_LABEL(runtime_data.tpsaccel_lab),buff);
		tmpf = (runtime->tpsaccel/10.0)/25.5 <= 1.0 
				? (runtime->tpsaccel/10.0)/25.5: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.tpsaccel_pbar),
				tmpf);
	}
	if (runtime->dcycle != runtime_last->dcycle)
	{
		g_snprintf(buff,10,"%.1f",runtime->dcycle);
		tmpf = runtime->dcycle/100.0 <= 1.0 ? runtime->dcycle/100.0: 1.0;
		gtk_label_set_text(GTK_LABEL(runtime_data.dcycle_lab),buff);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(runtime_data.dcycle_pbar),
				tmpf);
	}
	if (runtime->engine.value != runtime_last->engine.value)
	{
		/* these first two are mutually exclusive */
		if (runtime->engine.bit.running)
			pos = g_snprintf(buff,120,"Engine is Running, ");
		if (runtime->engine.bit.crank)
			pos = g_snprintf(buff,120,"Engine is Cranking, ");
		if (runtime->engine.bit.startw)
			pos += g_snprintf(buff+pos,120,"AfterStart Enrich, ");
		if (runtime->engine.bit.warmup)
			pos += g_snprintf(buff+pos,120,"Normal Warmup Enrich ");
		if (runtime->engine.bit.tpsaen)
			pos += g_snprintf(buff+pos,120,"Accel Shot (TPSAEN) ");
		if (runtime->engine.bit.tpsden)
			pos += g_snprintf(buff+pos,120,"Engine is Decelerating (TPSDEN) ");
		if (runtime->engine.bit.mapaen)
			pos += g_snprintf(buff+pos,120,"MAP accel mode (MAPAEN) ");
		update_statusbar(run_statbar,run_context_id,buff);

		
	}
	gdk_threads_leave();
}
	
