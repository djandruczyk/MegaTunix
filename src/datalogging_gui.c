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
#include <math.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>
#include <errno.h>
#include <datalogging.h>

#define TABLE_COLS 5

extern gint ready;
gint log_opened=FALSE;
gchar *delim;
static gint dlog_context_id;
static gint mode = CUSTOM_LOG;
static gint logging=FALSE;
static gint header_needed=FALSE;
static GtkWidget *logables_table;
static GtkWidget *file_selection;
static GtkWidget *delim_table;
static GtkWidget *format_table;
static GtkWidget *tab_delim_button;
static GtkWidget *dlog_statbar;
static GtkWidget *file_label;
static GtkWidget *stop_button;
static GtkWidget *start_button;
static int logfile;	/* DataLog File Handle*/
static gchar * log_file_name;
static gchar buff[100];
static gint total_logables = 0;
struct Logables logables;
const gchar *log_names[] = 
{
	"Hi-Res Clock", "MS Clock", "RPM", "TPS", "BATT",
	"MAP","BARO","O2","MAT","CLT",
	"VE","BaroCorr","EGOCorr","MATCorr","CLTCorr",
	"PW","INJ DutyCycle","EngineBits","GammaE"
};
/* index numbers of above array of things logged in a classic datalog.
 * I did NOT want to do it this way, as it's very inflexible...  Hopefully
 * I'll come up with a better idea later.... :|
 */
const gint classic[] =
{ 1,2,5,7,9,10,11,12,13,14,17,18 };

int build_datalogging(GtkWidget *parent_frame)
{
	gint i,j,k;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *button;
        GSList  *group;
	gint table_rows = 0;


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
	file_selection = hbox;
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

	frame = gtk_frame_new("Logable Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	total_logables = sizeof(log_names)/sizeof(gchar *);
	table_rows = ceil((float)total_logables/(float)TABLE_COLS);
	table = gtk_table_new(table_rows,TABLE_COLS,TRUE);
	logables_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),15);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,FALSE,0);

	j = 0;	
	k = 0;
	for (i=0;i<total_logables;i++)
	{
		button = gtk_check_button_new_with_label(log_names[i]);
		logables.widgets[i] = button;
		g_object_set_data(G_OBJECT(button),"bit_pos",
				GINT_TO_POINTER(i));
		g_object_set_data(G_OBJECT(button),"bitmask",
				GINT_TO_POINTER((gint)pow(2,i)));
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(log_value_set),
				NULL);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		j++;

		if (j == 5)
		{
			k++;
			j = 0;
		} 
	}

	frame = gtk_frame_new("Logging Format ");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(1,4,TRUE);
	format_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),15);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,FALSE,0);

	button = gtk_radio_button_new_with_label(NULL,"MegaTune \"Classic\" Format");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_mode),
                        GINT_TO_POINTER(CLASSIC_LOG));
	if (mode == CLASSIC_LOG)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	button = gtk_radio_button_new_with_label(group,"Custom Format");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_mode),
                        GINT_TO_POINTER(CUSTOM_LOG));
	if (mode == CUSTOM_LOG)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	frame = gtk_frame_new("Logging Delimiters ");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	table = gtk_table_new(1,4,TRUE);
	delim_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),15);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,FALSE,0);

	button = gtk_radio_button_new_with_label(NULL,"Comma Delimited");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_delimiter),
                        GINT_TO_POINTER(COMMA));

	button = gtk_radio_button_new_with_label(group,"Tab Delimited");
	tab_delim_button = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_delimiter),
                        GINT_TO_POINTER(TAB));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	button = gtk_radio_button_new_with_label(group,"Space Delimited");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_delimiter),
                        GINT_TO_POINTER(SPACE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	frame = gtk_frame_new("DataLogging Operations");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	button = gtk_button_new_with_label("Start Datalogging");
	start_button = button;
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,20);
	gtk_widget_set_sensitive(button,log_opened);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(START_DATALOGGING));

	button = gtk_button_new_with_label("Stop Datalogging");
	stop_button = button;
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,20);
	gtk_widget_set_sensitive(button,log_opened);
	g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(STOP_DATALOGGING));
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
			gtk_widget_set_sensitive(stop_button,FALSE);
			gtk_widget_set_sensitive(start_button,FALSE);
			g_snprintf(buff,100,"Failure creating datalogfile, Error Code: %s",strerror(errno));
			update_statusbar(dlog_statbar,dlog_context_id,buff);

		}
		else
		{
			log_opened=TRUE;
			gtk_widget_set_sensitive(stop_button,TRUE);
			gtk_widget_set_sensitive(start_button,TRUE);
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
			gtk_widget_set_sensitive(stop_button,FALSE);
			gtk_widget_set_sensitive(start_button,FALSE);
			g_snprintf(buff,100,"Failure opening datalogfile, Error Code: %s",strerror(errno));
			update_statusbar(dlog_statbar,
					dlog_context_id,buff);
		}
		else
		{	
			log_opened=TRUE;
			gtk_widget_set_sensitive(stop_button,TRUE);
			gtk_widget_set_sensitive(start_button,TRUE);
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
		gtk_widget_set_sensitive(stop_button,FALSE);
		gtk_widget_set_sensitive(start_button,FALSE);
				
	
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
	if (logging)
		return;   /* Logging already running ... */
	else
	{
		gtk_widget_set_sensitive(logables_table,FALSE);
		gtk_widget_set_sensitive(delim_table,FALSE);
		gtk_widget_set_sensitive(format_table,FALSE);
		gtk_widget_set_sensitive(file_selection,FALSE);
		header_needed = TRUE;
		logging = TRUE;
	}
	return;
}

void stop_datalogging()
{
	logging = FALSE;
	if (mode == CUSTOM_LOG)
	{
		gtk_widget_set_sensitive(logables_table,TRUE);
		gtk_widget_set_sensitive(delim_table,TRUE);
	}
	gtk_widget_set_sensitive(format_table,TRUE);
	gtk_widget_set_sensitive(file_selection,TRUE);
	return;
}

gint set_logging_mode(GtkWidget *widget, gpointer data)
{
	gint i = 0;
	gint max = sizeof(classic)/sizeof(gint);
	if (!ready)
		return FALSE;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
		switch ((gint)data)
		{
			case CLASSIC_LOG:
				mode = CLASSIC_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,FALSE);
				gtk_toggle_button_set_active(
						GTK_TOGGLE_BUTTON
						(tab_delim_button),
						TRUE);
				gtk_widget_set_sensitive(
						delim_table,FALSE);
				
				for (i=0;i<max;i++)
				{
					gtk_toggle_button_set_active(
							GTK_TOGGLE_BUTTON
							(logables.widgets[classic[i]]),
							TRUE);
				}
				break;
			case CUSTOM_LOG:
				mode = CUSTOM_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,TRUE);
				gtk_widget_set_sensitive(
						delim_table,TRUE);
				break;
		}
	}

	return TRUE;
}

void clear_logables(void)
{
	gint i = 0;
	/* Uncheck all logable choices */
	for (i=0;i<total_logables;i++)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(logables.widgets[i]),
				FALSE);
}

gint set_logging_delimiter(GtkWidget *widget, gpointer data)
{
	if (!ready)
		return FALSE;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{

		switch ((gint)data)
		{
			case COMMA:
				delim = g_strdup(",");
				break;
			case TAB:
				delim = g_strdup("\t");
				break;
			case SPACE:
				delim = g_strdup(" ");
				break;
			default:
				printf("delimiter not handled properly\n");
				break;
		}
	}
	return TRUE;
}
gint log_value_set(GtkWidget * widget, gpointer data)
{
	gint bit_pos = 0;
	gint bit_val = 0;
	gint bitmask = 0;
	gint tmp = 0;

	bit_pos = (gint)g_object_get_data(G_OBJECT(widget),"bit_pos");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		bit_val = 1;
	else
		bit_val = 0;

	tmp = logables.logbits.value;
	tmp = tmp & ~bitmask;
	tmp = tmp |(bit_val << bit_pos);
	logables.logbits.value = tmp;
		
/*
	printf("logables.logbits.bit.hr_clock = %i\n",logables.logbits.bit.hr_clock);
	printf("logables.logbits.bit.ms_clock = %i\n",logables.logbits.bit.ms_clock);
	printf("logables.logbits.bit.rpm = %i\n",logables.logbits.bit.rpm);
	printf("logables.logbits.bit.tps = %i\n",logables.logbits.bit.tps);
	printf("logables.logbits.bit.batt = %i\n",logables.logbits.bit.batt);
	printf("logables.logbits.bit.map = %i\n",logables.logbits.bit.map);
	printf("logables.logbits.bit.baro = %i\n",logables.logbits.bit.baro);
	printf("logables.logbits.bit.o2 = %i\n",logables.logbits.bit.o2);
	printf("logables.logbits.bit.mat = %i\n",logables.logbits.bit.mat);
	printf("logables.logbits.bit.clt = %i\n",logables.logbits.bit.clt);
	printf("logables.logbits.bit.ve = %i\n",logables.logbits.bit.ve);
	printf("logables.logbits.bit.barocorr = %i\n",logables.logbits.bit.barocorr);
	printf("logables.logbits.bit.egocorr = %i\n",logables.logbits.bit.egocorr);
	printf("logables.logbits.bit.matcorr = %i\n",logables.logbits.bit.matcorr);
	printf("logables.logbits.bit.cltcorr = %i\n",logables.logbits.bit.cltcorr);
	printf("logables.logbits.bit.pw = %i\n",logables.logbits.bit.pw);
	printf("logables.logbits.bit.dcycle = %i\n",logables.logbits.bit.dcycle);
	printf("logables.logbits.bit.engbits = %i\n",logables.logbits.bit.engbits);
	printf("logables.logbits.bit.gammae = %i\n",logables.logbits.bit.gammae);
*/
	return TRUE;
}

void run_datalog(void)
{
	if (logging == FALSE) /* Logging isn't enabled.... */
		return;
	else
	{
		if (header_needed)
		{
			write_log_header();
			header_needed = FALSE;
		}
		
	}

}

void write_log_header(void)
{
	
}
