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
#include <datalogging.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <enums.h>
#include <errno.h>
#include <fcntl.h>
#include <globals.h>
#include <gui_handlers.h>
#include <math.h>
#include <notifications.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Local #defines */
#define TABLE_COLS 6
#define MAX_LOGABLES 32
#define STD_LOGABLES 18
#define DUALTABLE_LOGABLES 21
#define DUALTABLE 64
/* Local #defines */

gint ms_type = 0;
extern gint ready;
extern struct Raw_Runtime_Std *raw_runtime;
gboolean log_opened = FALSE;
gchar *delim;
gfloat cumulative = 0.0;
extern GdkColor white;
struct timeval now;
struct timeval last;
static gint dlog_context_id;
static gint total_logables = 0;
gint logging_mode = CUSTOM_LOG;
static gint delimiter = SPACE;
static gboolean logging = FALSE;
static gboolean header_needed = FALSE;
GtkWidget *logables_table;
GtkWidget *delim_table;
GtkWidget *tab_delim_button;
static GtkWidget *file_selection;
static GtkWidget *format_table;
static GtkWidget *comma_delim_button;
static GtkWidget *space_delim_button;
static GtkWidget *dlog_statbar;
static GtkWidget *file_label;
static GtkWidget *stop_button;
static GtkWidget *start_button;
static FILE * logfile;				/* DataLog File Handle*/
static gchar * log_file_name;			/* log pathname */
static gchar buff[100];				/* General purpose buffer */
static gint max_logables = 0;
struct Logables logables;
static gint offset_list[MAX_LOGABLES];
const gchar *logable_names[] = 
{
	"Hi-Res Clock", "MS Clock", "RPM", "TPS", "BATT","MAP","BARO","O2","MAT","CLT",	"VE","BaroCorr","EGOCorr","MATCorr","CLTCorr","PW","EngineBits","GammaE","PW2","VE2","IdleDC"
	
};
/* logging_offset_map is a mapping between the logable_names[] list above and 
 * the byte offset into the raw_runtime_std datastructure. The index 
 * is the index number of the logable variable from above, The value at that
 * index point is the offset into the struct for the data. Offset 0
 * is the first value in the struct (secl), offset 99 is a special case
 * for the Hi-Res clock which isn't stored in the structure...
 */
const gint logging_offset_map[] = 
{ 99,0,13,7,8,4,3,9,5,6,18,16,10,11,12,14,2,17,19,20,21 }; 

int build_datalogging(GtkWidget *parent_frame)
{
	gint i,j,k;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *button;
	GtkWidget *ebox;
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

	ebox = gtk_event_box_new();
        gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);
        dlog_statbar = gtk_statusbar_new();
        gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(dlog_statbar),FALSE);
	gtk_container_add(GTK_CONTAINER(ebox),dlog_statbar);
        dlog_context_id = gtk_statusbar_get_context_id(
                        GTK_STATUSBAR(dlog_statbar),
                        "DataLogging Status");
        gtk_widget_modify_bg(GTK_WIDGET(ebox),
                        GTK_STATE_NORMAL,&white);


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

	max_logables = sizeof(logable_names)/sizeof(gchar *);
	table_rows = ceil((float)max_logables/(float)TABLE_COLS);
	table = gtk_table_new(table_rows,TABLE_COLS,TRUE);
	logables_table = table;
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
        gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox2),table,FALSE,FALSE,0);

	j = 0;	
	k = 0;
	for (i=0;i<max_logables;i++)
	{
		button = gtk_check_button_new_with_label(logable_names[i]);
		logables.widgets[i] = button;
		if ((ms_type != DUALTABLE) && (i >= STD_LOGABLES))
			gtk_widget_set_sensitive(button,FALSE);
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
                        GINT_TO_POINTER(MT_CLASSIC_LOG));
	if (logging_mode == MT_CLASSIC_LOG)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	button = gtk_radio_button_new_with_label(group,"MegaTune \"Full\" Format");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_mode),
                        GINT_TO_POINTER(MT_FULL_LOG));
	if (logging_mode == MT_FULL_LOG)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	button = gtk_radio_button_new_with_label(group,"Custom Format");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(set_logging_mode),
                        GINT_TO_POINTER(CUSTOM_LOG));
	if (logging_mode == CUSTOM_LOG)
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
	comma_delim_button = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(COMMA));
	if (delimiter == COMMA)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON
				(button),
				TRUE);
		g_signal_emit_by_name(button,"toggled",GINT_TO_POINTER(COMMA));
	}

	button = gtk_radio_button_new_with_label(group,"Tab Delimited");
	tab_delim_button = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(TAB));
	if (delimiter == TAB)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON
				(button),
				TRUE);
		g_signal_emit_by_name(button,"toggled",GINT_TO_POINTER(TAB));
	}

	button = gtk_radio_button_new_with_label(group,"Space Delimited");
	space_delim_button = button;
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
        gtk_table_attach (GTK_TABLE (table), button, 2, 3, 0, 1,
                        (GtkAttachOptions) (GTK_FILL),
                        (GtkAttachOptions) (0), 10, 0);
        g_signal_connect(G_OBJECT(button),"toggled",
                        G_CALLBACK(toggle_button_handler),
                        GINT_TO_POINTER(SPACE));
	if (delimiter == SPACE)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON
				(button),
				TRUE);
		g_signal_emit_by_name(button,"toggled",GINT_TO_POINTER(SPACE));
	}

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
		//logfile = open(selected_filename,
		//		O_CREAT|O_APPEND|O_RDWR, /* Create, append mode */
		//		S_IRUSR|S_IWUSR); /* User RW access */

		/* Append, create if not exists */
		logfile = fopen(selected_filename,"a"); 
				
		if(!logfile)
		{
			log_opened = FALSE;
			gtk_widget_set_sensitive(stop_button,FALSE);
			gtk_widget_set_sensitive(start_button,FALSE);
			g_snprintf(buff,100,"Failure creating datalogfile, Error Code: %s",strerror(errno));
			update_statusbar(dlog_statbar,dlog_context_id,buff);

		}
		else
		{
			log_opened = TRUE;
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

		//logfile = open(selected_filename,
		//		O_CREAT|O_APPEND|O_RDWR, 
		//		S_IRUSR|S_IWUSR); /* User RW access */
		/* Append, create if not exists */
		logfile = fopen(selected_filename,"a"); 
				
		if(!logfile)
		{
			log_opened = FALSE;
			gtk_widget_set_sensitive(stop_button,FALSE);
			gtk_widget_set_sensitive(start_button,FALSE);
			g_snprintf(buff,100,"Failure opening datalogfile, Error Code: %s",strerror(errno));
			update_statusbar(dlog_statbar,
					dlog_context_id,buff);
		}
		else
		{	
			log_opened = TRUE;
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
		fclose(logfile); 
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
	if (log_opened == TRUE)
	{
		truncate(log_file_name,0);
		if (errno)
			g_snprintf(buff,100,"DataLog Truncation Error: %s",strerror(errno));
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
		std_button_handler(NULL,GINT_TO_POINTER(START_REALTIME));
		gtk_widget_set_sensitive(logables_table,FALSE);
		gtk_widget_set_sensitive(delim_table,FALSE);
		gtk_widget_set_sensitive(format_table,FALSE);
		gtk_widget_set_sensitive(file_selection,FALSE);
		header_needed = TRUE;
		logging = TRUE;
		g_snprintf(buff,100,"DataLogging Started...");
		update_statusbar(dlog_statbar,dlog_context_id,buff);
	}
	return;
}

void stop_datalogging()
{
	logging = FALSE;
	if (logging_mode == CUSTOM_LOG)
	{
		gtk_widget_set_sensitive(logables_table,TRUE);
		gtk_widget_set_sensitive(delim_table,TRUE);
	}
	gtk_widget_set_sensitive(format_table,TRUE);
	gtk_widget_set_sensitive(file_selection,TRUE);
	g_snprintf(buff,100,"DataLogging Stopped...");
	update_statusbar(dlog_statbar,dlog_context_id,buff);
	return;
}


void clear_logables(void)
{
	gint i = 0;
	/* Uncheck all logable choices */
	for (i=0;i<max_logables;i++)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(logables.widgets[i]),
				FALSE);
}

gint log_value_set(GtkWidget * widget, gpointer data)
{
	gint bit_pos = 0;
	gint bit_val = 0;
	gint bitmask = 0;
	gint tmp = 0;
	gint i = 0;

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
	
	total_logables = 0;
	for (i=0;i<max_logables;i++)
	{
//		if ((tmp >> i) &0x1)
//			printf("Logging %s\n",logable_names[i]);
		total_logables += (tmp >> i) &0x1;
	}

//	printf("Total number of variables logged %i\n",total_logables);
		
	return TRUE;
}

void run_datalog(void)
{
	gint i = 0;
	gint offset = 0;
	gint begin = FALSE;
	unsigned char * log_ptr;
	if (!logging) /* Logging isn't enabled.... */
		return;
	else
	{
		log_ptr = (unsigned char *)raw_runtime;
		if (header_needed)
		{
			write_log_header();
			begin = TRUE;
			header_needed = FALSE;
		}
		for(i=0;i<total_logables;i++)
		{
			offset = offset_list[i];
			switch (offset)
			{
				case 99:
					/* Special Hi-Res clock to be logged */
					if (begin == TRUE)
					{	
						gettimeofday(&now,NULL);
						last.tv_sec = now.tv_sec;
						last.tv_usec = now.tv_usec;
						begin = FALSE;
						fprintf(logfile,"%f%s",0.0,delim);
					}
					else
					{
						gettimeofday(&now,NULL);
						cumulative += (now.tv_sec-last.tv_sec)+
							((double)(now.tv_usec-last.tv_usec)/
							 1000000.0);
						last.tv_sec = now.tv_sec;
						last.tv_usec = now.tv_usec;
						fprintf(logfile,"%f%s",cumulative,delim);
					}
					break;
				case 13: /* RPM */
					fprintf(logfile,"%i%s",100*log_ptr[offset],delim);
					break;
				default:
					fprintf(logfile,"%i%s",log_ptr[offset],delim);
					break;
			}
		}
		fprintf(logfile,"\n");
	}
}

void write_log_header(void)
{
	gint i = 0;
	gint j = 0;
	gint tmp = logables.logbits.value;
	
	for (i=0;i<max_logables;i++)
	{
		if ((tmp >> i) &0x1) /* If bit is set, we log this variable */
		{
			offset_list[j] = logging_offset_map[i];
			j++;
			fprintf(logfile, "\"%s\"%s",logable_names[i],delim);
		}
	}
	fprintf(logfile,"\n");
	
}
