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
#include <errno.h>

gint dlog_context_id;
gint log_opened=FALSE;
GtkWidget *dlog_statbar;
GtkWidget *custom_logables;
static GtkWidget *file_label;
static int logfile;	/* DataLog File Handle*/
static gchar * log_file_name;
static gchar buff[100];

int build_datalogging(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *button;
        GSList  *group;


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

	frame = gtk_frame_new("Data Log File Selection");
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

	button = gtk_button_new_with_label("Close Log File");
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,3);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(CLOSE_LOGFILE));

	button = gtk_button_new_with_label("Clear Log File");
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,3);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(TRUNCATE_LOGFILE));

	frame = gtk_frame_new("DataLogging Configuration");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	table = gtk_table_new(7,5,TRUE);
	custom_logables = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),15);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

	button = gtk_check_button_new_with_label("Hi-Res Clock");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("MS Clock");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("Engine RPM");
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("TPS");
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("BATT");
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("MAP");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("BARO");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("O2");
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("MAT");
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("CLT");
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 1, 2,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("VE");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("Baro Corr");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("EGO Corr");
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("MAT Corr");
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("CLT Gamma");
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 2, 3,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("Pulse-Width");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("Inj DutyCycle");
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("EngineBits");
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	button = gtk_check_button_new_with_label("Total Gamma");
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 3, 4,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(1,4,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),15);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

	button = gtk_radio_button_new_with_label(NULL,"MegaTune \"Classic\" Format");
	g_object_set_data(G_OBJECT(button),"config_num", 
			GINT_TO_POINTER(99));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(CLASSIC_LOG));

	button = gtk_radio_button_new_with_label(group,"Custom Format");
	g_object_set_data(G_OBJECT(button),"config_num", 
			GINT_TO_POINTER(99));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(CUSTOM_LOG));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);





	return TRUE;
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

	selected_filename = gtk_file_selection_get_filename (
			GTK_FILE_SELECTION (file_selector));
//	g_print ("Selected filename: %s\n", selected_filename);

	if (log_opened)
		close_logfile();	

	/* Test to see if it exists or not */
	if (lstat(selected_filename, &status) == -1)
	{
		logfile = open(selected_filename,
				O_CREAT|O_APPEND|O_RDWR, /* Create, append mode */
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
			log_file_name = g_strdup(selected_filename);
			gtk_label_set_text(GTK_LABEL(file_label),selected_filename);
			g_snprintf(buff,100,"DataLog File Opened");
			update_statusbar(dlog_statbar,dlog_context_id,buff);
		}
	}
	else
	{
		if (status.st_size > 0)
		{	/* warn user for non empty file*/
			warn_datalog_not_empty();
		}

		logfile = open(selected_filename,
				O_CREAT|O_APPEND|O_RDWR, 
				S_IRUSR|S_IWUSR); /* User RW access */
		if(!logfile)
		{
			log_opened=FALSE;
			g_snprintf(buff,100,"Failure opening datalogfile, Error Code: %s",strerror(errno));
			update_statusbar(dlog_statbar,
					dlog_context_id,buff);
		}
		else
		{	
			log_opened=TRUE;
			log_file_name = g_strdup(selected_filename);
			gtk_label_set_text(GTK_LABEL(file_label),selected_filename);
			g_snprintf(buff,100,"DataLog File Opened");
			update_statusbar(dlog_statbar,
					dlog_context_id,buff);
		}

	}


}

void close_logfile(void)
{
	if (log_opened == TRUE)
	{
		close(logfile); 
		g_free(log_file_name);
		gtk_label_set_text(GTK_LABEL(file_label),"No Log Selected Yet");
		log_opened = FALSE;
		logfile = 0;
		g_snprintf(buff,100,"Logfile Closed");
		update_statusbar(dlog_statbar,dlog_context_id,buff);
				
	
	}
}

void truncate_log()
{
	gint result;
	/* Not written yet */
	if (log_opened == TRUE)
	{
		result = ftruncate(logfile,0);
		if (result < 0)
			g_snprintf(buff,100,"Truncation error: %s", strerror(errno));
		else
			g_snprintf(buff,100,"DataLog Truncation successful");

		update_statusbar(dlog_statbar,
				dlog_context_id,buff);
		
	}
}

void start_datalogging()
{
	/* Not written yet */
}

void stop_datalogging()
{
	/* Not written yet */
}
