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

int build_comms(GtkWidget *parent_frame)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *vbox2;
	GtkWidget *button;
	GSList *group;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Serial Status Messages");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	ser_statbar = gtk_statusbar_new();
        gtk_box_pack_start(GTK_BOX(vbox2),ser_statbar,TRUE,TRUE,0);
	ser_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(ser_statbar),
			"Serial Status");

	frame = gtk_frame_new("Select Communications Port");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

        button = gtk_radio_button_new_with_label(NULL, "COM1");
        gtk_box_pack_start(GTK_BOX(vbox2),button,TRUE,TRUE,0);
        if (serial_params.comm_port == 1)
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        gtk_signal_connect(GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (set_serial_port), \
                        (gpointer)1);

        group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
        button = gtk_radio_button_new_with_label(group, "COM2");
        gtk_box_pack_start(GTK_BOX(vbox2),button,TRUE,TRUE,0);
        if (serial_params.comm_port == 2)
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        gtk_signal_connect(GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (set_serial_port), \
                        (gpointer)2);

        group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
        button = gtk_radio_button_new_with_label(group, "COM3");
        gtk_box_pack_start(GTK_BOX(vbox2),button,TRUE,TRUE,0);
        if (serial_params.comm_port == 3)
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        gtk_signal_connect(GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (set_serial_port), \
                        (gpointer)3);

        group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
        button = gtk_radio_button_new_with_label(group, "COM4");
        gtk_box_pack_start(GTK_BOX(vbox2),button,TRUE,TRUE,0);
        if (serial_params.comm_port == 4)
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        gtk_signal_connect(GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (set_serial_port), \
                        (gpointer)4);

	frame = gtk_frame_new("Verify ECU Communication");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	button = gtk_button_new_with_label("Test ECU Communication");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
        gtk_signal_connect(GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (check_ecu_comms), \
                        NULL);



	gtk_widget_show_all(vbox);
	return(0);
}

int set_serial_port(GtkWidget *widget, gpointer data)
{
	int result=0;
	gchar buff[60];
	if (GTK_TOGGLE_BUTTON(widget)->active)
	{
		switch ((int)(data))
		{
			case 1:
				serial_params.comm_port = 1; /*DOS/Win32 style*/
				close_serial();
				result = open_serial(serial_params.comm_port);
				setup_serial_params();
				break;
			case 2:
				serial_params.comm_port = 2; /*DOS/Win32 style*/
				close_serial();
				result = open_serial(serial_params.comm_port);
				setup_serial_params();
				break;
			case 3:
				serial_params.comm_port = 3; /*DOS/Win32 style*/
				close_serial();
				result = open_serial(serial_params.comm_port);
				setup_serial_params();
				break;
			case 4:
				serial_params.comm_port = 4; /*DOS/Win32 style*/
				close_serial();
				result = open_serial(serial_params.comm_port);
				setup_serial_params();
				break;
			default:
				serial_params.comm_port = 1; /*DOS/Win32 style*/
				close_serial();
				result = open_serial(serial_params.comm_port);
				setup_serial_params();
				break;
		}
		if (result < 0)
		{
			g_snprintf(buff,60,"Error Opening COM%i",(int)data);
			/* An Error occurred opening the port */
			gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
					ser_context_id);
			gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
					ser_context_id,
					buff);
		}
		else
		{
			g_snprintf(buff,60,"COM%i opened successfully",(int)data);
			/* An Error occurred opening the port */
			gtk_statusbar_pop(GTK_STATUSBAR(ser_statbar),
					ser_context_id);
			gtk_statusbar_push(GTK_STATUSBAR(ser_statbar),
					ser_context_id,
					buff);
		}
	}

	return TRUE;
}
