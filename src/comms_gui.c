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
#include <sys/poll.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"

static GtkWidget *ms_reset_entry;	/* MS reset count */
static GtkWidget *ms_sioerr_entry;	/* MS Serial I/O error count */
static GtkWidget *ms_readcount_entry;	/* MS Good read counter */
static GtkWidget *ms_ve_readcount_entry;	/* MS Good read counter */
int ser_context_id;			/* for ser_statbar */
GtkWidget *ser_statbar;			/* serial statusbar */ 
extern int read_wait_time;
extern int raw_reader_running;
extern int ms_reset_count;
extern int ms_goodread_count;
extern int ms_ve_goodread_count;

int build_comms(GtkWidget *parent_frame)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *table;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *spinner;
	GtkAdjustment *adj;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new("Serial Status Messages");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);

	ser_statbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ser_statbar,TRUE,TRUE,0);
	ser_context_id = gtk_statusbar_get_context_id(
			GTK_STATUSBAR(ser_statbar),
			"Serial Status");

	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	frame = gtk_frame_new("Select Communications Port");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);

	hbox2 = gtk_hbox_new(FALSE,10);
	gtk_container_add(GTK_CONTAINER(frame),hbox2);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 5);

	label = gtk_label_new("COMM Port");
	gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE,TRUE,0);

	adj = (GtkAdjustment *) gtk_adjustment_new(1,1,8,1,1,0);
	spinner = gtk_spin_button_new(adj,0,0);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(SET_SER_PORT));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),serial_params.comm_port);

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_box_pack_start (GTK_BOX (hbox2), spinner, FALSE, TRUE, 0);

	frame = gtk_frame_new("Verify ECU Communication");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);

	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox2);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2),5);
	button = gtk_button_new_with_label("Test ECU Communication");
	gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,TRUE,0);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (check_ecu_comms), \
			NULL);

	frame = gtk_frame_new("Runtime Parameters Configuration");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	label = gtk_label_new("Polling Timeout (ms)");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	adj = (GtkAdjustment *) gtk_adjustment_new(
			100,poll_min,poll_max,5,5,0);
	spinner = gtk_spin_button_new(adj,0,0);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(SER_POLL_TIMEO));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),serial_params.poll_timeout);
	gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Start Reading RT Vars");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(START_REALTIME));

	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

	label = gtk_label_new("Serial interval delay\n between samples");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	adj = (GtkAdjustment *) gtk_adjustment_new(
			100,interval_min,interval_max,25,25,0);
	spinner = gtk_spin_button_new(adj,0,0);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(SER_INTERVAL_DELAY));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),serial_params.read_wait);
	gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Stop Reading RT vars");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(STOP_REALTIME));

	frame = gtk_frame_new("MegaSquirt I/O Status");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);

	table = gtk_table_new(3,4,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,FALSE,5);

	label = gtk_label_new("Good VE/Constants Reads");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	ms_ve_readcount_entry = gtk_entry_new();
	gtk_entry_set_width_chars (GTK_ENTRY (ms_ve_readcount_entry), 7);
	gtk_table_attach (GTK_TABLE (table), ms_ve_readcount_entry, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

     
	label = gtk_label_new("Good RealTime Reads");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.0);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
     
	ms_readcount_entry = gtk_entry_new();
	gtk_entry_set_width_chars (GTK_ENTRY (ms_readcount_entry), 7);
	gtk_table_attach (GTK_TABLE (table), ms_readcount_entry, 3, 4, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Hard Reset Count\n");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.0);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	ms_reset_entry = gtk_entry_new();
	gtk_entry_set_width_chars (GTK_ENTRY (ms_reset_entry), 7);
	gtk_table_attach (GTK_TABLE (table), ms_reset_entry, 3, 4, 1, 2,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Serial I/O Error Count\n");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.0);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	ms_sioerr_entry = gtk_entry_new();
	gtk_entry_set_width_chars (GTK_ENTRY (ms_sioerr_entry), 7);
	gtk_table_attach (GTK_TABLE (table), ms_sioerr_entry, 3, 4, 2, 3,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);

	gtk_widget_show_all(vbox);
	return(0);
}

void update_errcounts()
{
	char buff[10];

	g_snprintf(buff,10,"%i",ms_ve_goodread_count);
	gtk_entry_set_text(GTK_ENTRY(ms_ve_readcount_entry),buff);

	g_snprintf(buff,10,"%i",ms_goodread_count);
	gtk_entry_set_text(GTK_ENTRY(ms_readcount_entry),buff);

	g_snprintf(buff,10,"%i",ms_reset_count);
	gtk_entry_set_text(GTK_ENTRY(ms_reset_entry),buff);

	g_snprintf(buff,10,"%i",serial_params.errcount);
	gtk_entry_set_text(GTK_ENTRY(ms_sioerr_entry),buff);

	return;
}
