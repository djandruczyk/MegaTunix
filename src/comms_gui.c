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
#include <sys/poll.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"

GtkWidget *ser_statbar;                 /* serial statusbar */ 
int ser_context_id;                     /* for ser_statbar */
extern int read_wait_time;
extern int raw_reader_running;

int build_comms(GtkWidget *parent_frame)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *label;
	GtkWidget *spinner;
	GtkAdjustment *adj;
	GSList *group;
	char buff[10];

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Serial Status Messages");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	ser_statbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ser_statbar,TRUE,TRUE,0);
	ser_context_id = gtk_statusbar_get_context_id(
			GTK_STATUSBAR(ser_statbar),
			"Serial Status");

	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	frame = gtk_frame_new("Select Communications Port");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	hbox2 = gtk_hbox_new(FALSE,10);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox2,FALSE,TRUE,0);

	label = gtk_label_new("COMM PORT");

	gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE,TRUE,0);

	adj = (GtkAdjustment *) gtk_adjustment_new(1,1,8,1,1,0);
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
			GTK_SIGNAL_FUNC (set_serial_port),
			(gpointer)spinner);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),serial_params.comm_port);

	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                       GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (hbox2), spinner, FALSE, TRUE, 0);

	frame = gtk_frame_new("Verify ECU Communication");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,TRUE,0);
	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox2);
	button = gtk_button_new_with_label("Test ECU Communication");
	gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,TRUE,0);
	gtk_signal_connect(GTK_OBJECT (button), "clicked",
			GTK_SIGNAL_FUNC (check_ecu_comms), \
			NULL);

	frame = gtk_frame_new("Runtime Parameters Configuration");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	label = gtk_label_new("Serial polling timeout in milliseconds");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	entry = gtk_entry_new();
	sprintf(buff,"%i",serial_params.poll_timeout);
	gtk_entry_set_text(GTK_ENTRY(entry),buff);
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
	gtk_signal_connect(GTK_OBJECT(entry), "activate",
                       GTK_SIGNAL_FUNC(text_entry_handler),
                       (gpointer)SER_POLL_TIMEO);
	gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	label = gtk_label_new("Serial interval delay between samples");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	entry = gtk_entry_new();
	sprintf(buff,"%i",read_wait_time);
	gtk_entry_set_text(GTK_ENTRY(entry),buff);
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
	gtk_signal_connect(GTK_OBJECT(entry), "activate",
                       GTK_SIGNAL_FUNC(text_entry_handler),
                       (gpointer)SER_INTERVAL_DELAY);
	gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);


	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Start Reading RealTime vars");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT (button), "clicked",
			GTK_SIGNAL_FUNC (std_button_handler), \
			(gpointer)START_REALTIME);

	button = gtk_button_new_with_label("Stop Reading RealTime vars");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT (button), "clicked",
			GTK_SIGNAL_FUNC (std_button_handler), \
			(gpointer)STOP_REALTIME);



	gtk_widget_show_all(vbox);
	return(0);
}

int set_serial_port(GtkWidget *widget, gpointer data)
{
	int port = gtk_spin_button_get_value_as_int(data);
		if(serial_params.open)
		{
			if (raw_reader_running)
				serial_raw_thread_stopper();
			close_serial();
		}
		open_serial((int)port);
		setup_serial_params();
	return TRUE;
}
