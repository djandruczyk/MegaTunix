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

static GtkWidget * ser_statusbar;
static gint context_id;
static char buff[60];

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

	ser_statusbar = gtk_statusbar_new();
        gtk_box_pack_start(GTK_BOX(vbox2),ser_statusbar,TRUE,TRUE,0);
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(ser_statusbar),
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
			gtk_statusbar_pop(GTK_STATUSBAR(ser_statusbar),
					context_id);
			gtk_statusbar_push(GTK_STATUSBAR(ser_statusbar),
					context_id,
					buff);
		}
		else
		{
			g_snprintf(buff,60,"COM%i opened successfully",(int)data);
			/* An Error occurred opening the port */
			gtk_statusbar_pop(GTK_STATUSBAR(ser_statusbar),
					context_id);
			gtk_statusbar_push(GTK_STATUSBAR(ser_statusbar),
					context_id,
					buff);
		}
	}

	return TRUE;
}

int check_ecu_comms(GtkWidget *widget, gpointer data)
{
	gint tmp;
	gint res;
	struct pollfd ufds;
	gint restart_thread = 0;

	if(serial_params.open)
	{
		if (raw_reader_running)
		{
	//		printf("realtime reader thread running, stopping it\n");
			raw_reader_running = 0;
			restart_thread = 1;
		}
		while (raw_reader_stopped == 0)
		{
	//		printf("Waiting for thread to die\n");
			usleep(1000);
		}

		ufds.fd = serial_params.fd;
		ufds.events = POLLIN;
		/* save state */
		tmp = serial_params.newtio.c_cc[VMIN];
		serial_params.newtio.c_cc[VMIN]     = 1; /*wait for 1 char */
		tcflush(serial_params.fd, TCIFLUSH);
		tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

		res = write(serial_params.fd,"C",1);
		res = poll (&ufds,1,serial_params.poll_timeout);
		if (res == 0)
		{
			g_snprintf(buff,60,"I/O with MegaSquirt Timeout");
			/* An Error occurred opening the port */
			gtk_statusbar_pop(GTK_STATUSBAR(ser_statusbar),
					context_id);
			gtk_statusbar_push(GTK_STATUSBAR(ser_statusbar),
					context_id,
					buff);
		}
		else
		{
			g_snprintf(buff,60,"ECU comms test successfull");
			/* An Error occurred opening the port */
			gtk_statusbar_pop(GTK_STATUSBAR(ser_statusbar),
					context_id);
			gtk_statusbar_push(GTK_STATUSBAR(ser_statusbar),
					context_id,
					buff);
		}

		serial_params.newtio.c_cc[VMIN]     = tmp; /*restore original*/
		tcflush(serial_params.fd, TCIFLUSH);
		tcsetattr(serial_params.fd,TCSANOW,&serial_params.newtio);

		if (restart_thread)
		{
	//		printf("restarting thread\n");
			serial_raw_thread_starter();
		}


	}
	else
	{
		g_snprintf(buff,60,"Serial port not opened, can't test ECU comms");
		/* An Error occurred opening the port */
		gtk_statusbar_pop(GTK_STATUSBAR(ser_statusbar),
				context_id);
		gtk_statusbar_push(GTK_STATUSBAR(ser_statusbar),
				context_id,
				buff);
	}
	return (0);

}
                             
