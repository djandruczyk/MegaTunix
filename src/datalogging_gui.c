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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>

gint dlog_context_id;
gint log_opened=FALSE;
GtkWidget *dlog_statbar;
static GtkWidget *file_label;
static int logfile;	/* DataLog File Handle*/

int build_datalogging(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *button;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("DataLogging Status Messages");
        gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

        vbox2 = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(frame),vbox2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);

        dlog_statbar = gtk_statusbar_new();
        gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(dlog_statbar),FALSE);
        gtk_box_pack_start(GTK_BOX(vbox2),dlog_statbar,TRUE,TRUE,0);
        dlog_context_id = gtk_statusbar_get_context_id(
                        GTK_STATUSBAR(dlog_statbar),
                        "DataLogging Status");

	frame = gtk_frame_new("DataLogging Configuration");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	button = gtk_button_new_with_label("Select Log File");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(SELECT_LOGFILE));

	file_label = gtk_label_new("No Log Selected Yet");
	gtk_box_pack_start(GTK_BOX(hbox),file_label,FALSE,FALSE,30);

	button = gtk_button_new_with_label("Clear Log File");
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(TRUNCATE_LOGFILE));

	return TRUE;
}

void close_datalog(void)
{
	if (log_opened == TRUE)
	{
		close(logfile); 
		printf("Closing Datalog file\n");
	}
}

void create_dlog_filesel(void)
{
	GtkWidget *file_selector;

	/* Create the selector */

	file_selector = gtk_file_selection_new ("Please select a file for The DataLog.");
	gtk_file_selection_set_select_multiple(
			GTK_FILE_SELECTION(file_selector),
			FALSE);

	g_signal_connect (GTK_OBJECT 
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (check_filename),
			(gpointer) file_selector);

	/* Ensure that the dialog box is destroyed when the user clicks a button. */

	g_signal_connect_swapped (GTK_OBJECT 
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (gtk_widget_destroy), 
			(gpointer) file_selector); 

	g_signal_connect_swapped (GTK_OBJECT 
			(GTK_FILE_SELECTION (file_selector)->cancel_button),
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			(gpointer) file_selector); 

	/* Display that dialog */

	gtk_widget_show (file_selector);
}

void check_filename (GtkWidget *widget, GtkFileSelection *file_selector) 
{
	const gchar *selected_filename;
	struct stat status;
	gchar buff[100];
	extern int errno;

	selected_filename = gtk_file_selection_get_filename (
			GTK_FILE_SELECTION (file_selector));
	g_print ("Selected filename: %s\n", selected_filename);

	log_opened=FALSE;
	if (lstat(selected_filename, &status) == -1)
	{
		logfile = open(selected_filename,
				O_CREAT|O_APPEND, /* Create, append mode */
				S_IRUSR|S_IWUSR); /* User RW access */
		if(!logfile)
		{
			log_opened=FALSE;
			g_snprintf(buff,100,"Failure creating datalogfile, Error Code: %s",strerror(errno));
			update_statusbar(dlog_statbar,dlog_context_id,buff);

		}
		else
		{
			log_opened=TRUE;
			gtk_label_set_text(GTK_LABEL(file_label),selected_filename);
			g_snprintf(buff,100,"DataLog File Opened");
			update_statusbar(dlog_statbar,dlog_context_id,buff);
		}
	}
	else
	{
		if (status.st_size == 0)
		{
			/* File is empty, we can use it safely without 
			 * over-writing anything important... 
			 */
			logfile = open(selected_filename,
					O_CREAT|O_APPEND, 
					S_IRUSR|S_IWUSR); /* User RW access */
			if(!logfile)
			{
				log_opened=FALSE;
				g_snprintf(buff,100,"Failure creating datalogfile, Error Code: %s",strerror(errno));
				update_statusbar(dlog_statbar,
				dlog_context_id,buff);
			}
			else
			{	
				log_opened=TRUE;
				gtk_label_set_text(GTK_LABEL(file_label),selected_filename);
				g_snprintf(buff,100,"DataLog File Opened");
				update_statusbar(dlog_statbar,
						dlog_context_id,buff);
			}
		}
		else
		{
			g_snprintf(buff,100,"File is Not Empty, NOT OPENING!!");
			update_statusbar(dlog_statbar,
					dlog_context_id,buff);
			printf("File not empty, warning user\n");
		}
	
	}
	

}

void truncate_log()
{
	/* Not written yet */
}

void start_datalogging()
{
	/* Not written yet */
}

void stop_datalogging()
{
	/* Not written yet */
}
