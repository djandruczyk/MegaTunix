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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"


extern int raw_reader_running;
extern int raw_reader_stopped;
extern int ser_context_id;
extern int read_wait_time;
extern GtkWidget *ser_statbar;

void leave(GtkWidget *widget, gpointer *data)
{
        save_config();
        raw_reader_running = 0;	/*causes realtime var reader thread to die */
	while (raw_reader_stopped == 0)
		usleep(10000);	/*wait for thread to die cleanly*/

        /* Free all buffers */
	close_serial();
        mem_dealloc();
        gtk_main_quit();
}

void text_entry_handler(GtkWidget * widget, gpointer *data)
{
	gchar *entry_text;
	gint tmp = 0;
	gfloat tmpf = 0;
	gchar buff[10];
	entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
	switch ((gint)data)
	{
		case SER_POLL_TIMEO:
			tmp = atoi(entry_text);
			if (tmp < poll_min)
				tmp = poll_min;
			else if (tmp > poll_max)
				tmp = poll_max;
			serial_params.poll_timeout = tmp;
			sprintf(buff,"%i",tmp);
			gtk_entry_set_text(GTK_ENTRY(widget),buff);
			break;
		case SER_INTERVAL_DELAY:
			tmp = atoi(entry_text);
			if (tmp < interval_min)
				tmp = interval_min;
			else if (tmp > interval_max)
				tmp = interval_max;
			read_wait_time = tmp;
			sprintf(buff,"%i",tmp);
			gtk_entry_set_text(GTK_ENTRY(widget),buff);
			break;
		case INJ_OPEN_TIME:
			tmpf = atof(entry_text);
			printf("inj open time set to %f\n",tmpf);
	}
	/* update the widget in case data was out of bounds */
}



int std_button_handler(GtkWidget *widget, gpointer *data)
{
	switch ((gint)data)
	{
		case START_REALTIME:
			serial_raw_thread_starter();
			break;
		case STOP_REALTIME:
			serial_raw_thread_stopper();
			break;
	}
	return TRUE;
}

void update_statusbar(GtkWidget *status_bar,int context_id, gchar * message)
{
	/* takes 3 args, 
	 * the GtkWidget pointer to the statusbar,
	 * the context_id of the statusbar in arg[0],
	 * and the string to be sent to that bar
	 *
	 * Fairly generic, should work for multiple statusbars
	 */

	gtk_statusbar_pop(GTK_STATUSBAR(status_bar),
			context_id);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar),
			context_id,
			message);
}

int calc_reqd_fuel_func(GtkWidget *widget, gpointer *data)
{
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *label;

	printf("Build required fuel window\n");
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_widget_set_usize(window,250,163);
	gtk_window_set_title(GTK_WINDOW(window),"Required Fuel Calc");
	gtk_container_set_border_width(GTK_CONTAINER(window),10);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);

	frame = gtk_frame_new("Constants for your vehicle");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	
	/* left column */
	vbox2 = gtk_vbox_new(TRUE,2);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);
	label = gtk_label_new("Engine Displacement (CID)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
	label = gtk_label_new("Number of Cylinders");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
	label = gtk_label_new("Injector Flow (lbs/hr)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);
	label = gtk_label_new("Air-Fuel Ratio");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,0);

	/* right column */
	vbox2 = gtk_vbox_new(TRUE,2);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox2),entry,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox2),entry,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox2),entry,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_widget_set_usize(entry,64,20);
	gtk_box_pack_start(GTK_BOX(vbox2),entry,FALSE,FALSE,0);
	
	
	frame = gtk_frame_new("Exit");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	button = gtk_button_new_with_label("OK");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);

	gtk_widget_show_all(window);
	
	return TRUE;
}

int spinner_changed(GtkWidget *widget, gpointer *data)
{
	printf("spinner_changed function called\n");
	return TRUE;

}
