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

#include <comms_gui.h>
#include <config.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <tabloader.h>
#include <unistd.h>
#include <widgetmgmt.h>

extern gint read_wait_time;
extern gint ms_reset_count;
extern gint ms_goodread_count;
extern gint ms_ve_goodread_count;
extern GdkColor black;
extern struct Serial_Params *serial_params;
gint interval_min;
gint interval_step;
gint interval_max;
GdkColor white = { 0, 65535, 65535, 65535 };


/*!
 \brief build_comms() builds the comms Tab for megatunix providing controls
 for selecting the serial port, ECU polloing speeds, and provides a textview
 (comms_view) and status boxes shows the I/O status of comms with the ECU.
 */
void build_comms(GtkWidget *parent_frame)
{
	extern GtkTooltips *tip;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *table;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *spinner;
	GtkWidget *ebox;
	GtkWidget *entry;
	GtkWidget *view;
	GtkWidget *sw;
	GtkTextBuffer * textbuffer;
	GtkAdjustment *adj;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	frame = gtk_frame_new("Serial Status Messages");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	gtk_box_pack_end(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);

	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(sw,0,55);
	gtk_container_add(GTK_CONTAINER(ebox),sw);

	view = gtk_text_view_new();
	register_widget("comms_view",view);

	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);

	gtk_container_add(GTK_CONTAINER(sw),view);

	hbox = gtk_hbox_new(TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,
			"Sets the comm port to use. Type in the device name of your serial connection (Typical values under linux would be /dev/ttyS0, under Mac OS-X with a USB/Serial adapter would be /dev/tty.usbserial0, and under FreeBSD /dev/cuaa0)",NULL);

	frame = gtk_frame_new("Select Communications Port");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox2);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2),3);

	table = gtk_table_new(1,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(hbox2),table,FALSE,TRUE,3);


	label = gtk_label_new("Comms Device");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new();
	if (serial_params->port_name)
		gtk_entry_set_text(GTK_ENTRY(entry),serial_params->port_name);
	gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
	g_signal_connect (G_OBJECT(entry), "changed",
			G_CALLBACK (entry_changed_handler),
			NULL);
	g_signal_connect (G_OBJECT(entry), "activate",
			G_CALLBACK (comm_port_change),
			GINT_TO_POINTER(SET_SER_PORT));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,
			"Attempts to communicate with the MegaSquirt Controller.  Check the status log at the bottom of the window for the results of this test.",NULL);

	frame = gtk_frame_new("Verify ECU Communication");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	hbox2 = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox2);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2),5);
	button = gtk_button_new_with_label("   Test ECU Communication...   ");
	gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(CHECK_ECU_COMMS));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,FALSE,0);
	gtk_tooltips_set_tip(tip,ebox,"   Allows you to specify the amount of time MegaTunix waits after issuing the command to read the runtime data from the ECU. (Polling Timeout)  A sane value here is no more than 100 milliseconds.  NOTE: This is the maximum window of time MegaTunix will wait for data.  Setting this too high can cause the display to get laggy in some cases.\n   The Serial interval delay is the amount of time to wait in between reads.  This determines the maximum update rate of the runtime page and the datalogging rate.  If you set this too low it's possible to have data problems due to the lack of checksumming on the input datastream.  A safe value is about 25 milliseconds.\n   The buttons on the right are pretty self explanitory, they start and stop reading of Runtime data respectivly.  NOTE: when datalogging is active, runtime data is being received, as this is the source for data for logging to a text file for future analysis.",NULL);
	frame = gtk_frame_new("Runtime Parameters Configuration");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox),5);

	table = gtk_table_new(2,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),1);
	gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,0);

	button = gtk_button_new_with_label("Start Reading RT Vars");
	gtk_tooltips_set_tip(tip,button,
			"Starts reading the RealTime variables from the MS.  This will cause the Runtime Disp. page to begin updating continuously.",NULL);

	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(START_REALTIME));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Delay Between Reads (ms)");
	gtk_misc_set_alignment (GTK_MISC (label), 0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	adj = (GtkAdjustment *) gtk_adjustment_new(
			100,interval_min,interval_max,
			interval_step,interval_step,0);
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,55,-1);
	gtk_tooltips_set_tip(tip,spinner,
			"Sets the time delay between read attempts for getting the RealTime variables from the MS, typically should be set around 50 for about 12-18 reads per second from the MS.  This will control the rate at which the Runtime Display page updates.",NULL);
	g_object_set_data(G_OBJECT(spinner),"handler",GINT_TO_POINTER(SER_INTERVAL_DELAY));
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spin_button_handler),
			NULL);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),serial_params->read_wait);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Stop Reading RT vars ");
	gtk_tooltips_set_tip(tip,button,
			"Stops reading the RT variables from the MS.  NOTE: you don't have to stop reading the RT vars to read the VEtable and Constants.  It is handled automatically for you.",NULL);
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(STOP_REALTIME));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,
			"This block shows you statistics on the number of good reads of the VE/Constants datablocks, RealTime datablocks and the MegaSquirt hard reset and Serial I/O error counts.  Hard resets are indicative of power problems or excessive electrical noise to the MS (causing cpu resets).  Serial I/O errors are indicative of a poor cable connection between this host computer and the MS.",NULL);

	frame = gtk_frame_new("MegaSquirt I/O Status");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(3,4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),1);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),2);
	gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,3);

	label = gtk_label_new("Good VE/Constants Reads");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new();
	register_widget("comms_vecount_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);


	label = gtk_label_new("Good RealTime Reads");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new();
	register_widget("comms_rtcount_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Hard Reset Count");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new();
	register_widget("comms_reset_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Serial I/O Error Count");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new();
	register_widget("comms_sioerr_entry",entry);
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Reset Status Counters...");
	g_signal_connect_swapped(G_OBJECT (button), "clicked",
			G_CALLBACK (reset_errcounts), \
			NULL);
	gtk_table_attach (GTK_TABLE (table), button, 0, 4, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 4);

	gtk_widget_show_all(vbox);

	return;
}


/*!
 \brief reset_errcounts() resets the error counters
 \param widget (GtkWidget *) unused
 \returns TRUE
 */
EXPORT gboolean reset_errcounts(GtkWidget *widget)
{
	ms_ve_goodread_count = 0;
	ms_goodread_count = 0;
	ms_reset_count = 0;
	serial_params->errcount = 0;
	return TRUE;
}


/*!
 \brief update_errcounts() updates the text entries on the gui with the 
 current statistical error and i/O counters
 \returns TRUE
 */
gboolean update_errcounts()
{
	gchar *tmpbuf = NULL;
	extern GHashTable *dynamic_widgets;
	GtkWidget * widget = NULL;

	tmpbuf = g_strdup_printf("%i",ms_ve_goodread_count);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"comms_vecount_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"runtime_good_ve_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",ms_goodread_count);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"comms_rtcount_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"runtime_good_rt_read_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",ms_reset_count);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"comms_reset_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	if(NULL != (widget = g_hash_table_lookup(dynamic_widgets,"runtime_hardreset_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	tmpbuf = g_strdup_printf("%i",serial_params->errcount);
	if (NULL != (widget = g_hash_table_lookup(dynamic_widgets,"comms_sioerr_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	if(NULL != (widget = g_hash_table_lookup(dynamic_widgets,"runtime_sioerr_entry")))
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	return TRUE;
}
