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
#include <datalogging_const.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <globals.h>
#include <glib/gprintf.h>
#include <gui_handlers.h>
#include <math.h>
#include <notifications.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


/* Local #defines */
#define TABLE_COLS 6
#define MAX_LOGABLES 64
/* Local #defines */

extern gint ready;
extern struct Runtime_Common *runtime;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern GdkColor white;
extern gboolean dualtable;
gchar *delim;
gfloat cumulative = 0.0;
gint logging_mode = CUSTOM_LOG;
static gint total_logables = 0;
static gint delimiter = SPACE;
static gboolean logging = FALSE;
static gboolean header_needed = FALSE;
static GtkWidget *file_selection;
static GtkWidget *format_table;
static GtkWidget *comma_delim_button;
static GtkWidget *space_delim_button;
gint max_logables = 0;
static gint offset_list[MAX_LOGABLES];
static gint size_list[MAX_LOGABLES];
struct timeval now;
struct timeval last;
GtkWidget *dlog_view;
GtkWidget *logables_table;
GtkWidget *delim_table;
GtkWidget *tab_delim_button;
struct Logables logables;


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
	GtkWidget *label;
	GtkWidget *sw;
	GtkWidget *view;
	GtkTextBuffer *textbuffer;
	extern GtkTooltips *tip;
	GSList  *group;
	gint table_rows = 0;


	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("DataLogging Status Messages");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox2),ebox,TRUE,TRUE,0);

	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(sw,0,55);
	gtk_container_add(GTK_CONTAINER(ebox),sw);

	view = gtk_text_view_new();
	dlog_view = view;
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view),FALSE);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);
	gtk_container_add(GTK_CONTAINER(sw),view);

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

	label = gtk_label_new("No Log Selected Yet");
	labels.dlog_file_lab = label;
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,30);

	button = gtk_button_new_with_label("Close Log File");
	buttons.close_dlog_but = button;
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,3);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(CLOSE_LOGFILE));

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
		gtk_tooltips_set_tip(tip,button,logable_names_tips[i],NULL);
		logables.widgets[i] = button;
//		if ((dualtable) && (i >= STD_LOGABLES))
//			gtk_widget_set_sensitive(button,FALSE);
		g_object_set_data(G_OBJECT(button),"index",
				GINT_TO_POINTER(i));
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
	buttons.start_dlog_but = button;
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,20);
	gtk_widget_set_sensitive(button,FALSE);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(START_DATALOGGING));

	button = gtk_button_new_with_label("Stop Datalogging");
	buttons.stop_dlog_but = button;
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,20);
	gtk_widget_set_sensitive(button,FALSE);
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(STOP_DATALOGGING));
	return TRUE;
}

void start_datalogging(void)
{
	gchar * tmpbuf;
	if (logging)
		return;   /* Logging already running ... */

	gtk_widget_set_sensitive(logables_table,FALSE);
	gtk_widget_set_sensitive(delim_table,FALSE);
	gtk_widget_set_sensitive(format_table,FALSE);
	gtk_widget_set_sensitive(file_selection,FALSE);
	header_needed = TRUE;
	logging = TRUE;
	tmpbuf = g_strdup_printf("DataLogging Started...\n");
	update_logbar(dlog_view,NULL,tmpbuf,TRUE);
	g_free(tmpbuf);
	std_button_handler(NULL,GINT_TO_POINTER(START_REALTIME));
	return;
}

void stop_datalogging()
{
	gchar *tmpbuf;
	logging = FALSE;
	if (logging_mode == CUSTOM_LOG)
	{
		gtk_widget_set_sensitive(logables_table,TRUE);
		gtk_widget_set_sensitive(delim_table,TRUE);
	}
	gtk_widget_set_sensitive(format_table,TRUE);
	gtk_widget_set_sensitive(file_selection,TRUE);
	tmpbuf = g_strdup_printf("DataLogging Stopped...\n");
	update_logbar(dlog_view,NULL,tmpbuf,TRUE);
	g_free(tmpbuf);
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
	gint index = 0;
	gint i = 0;

	index = (gint)g_object_get_data(G_OBJECT(widget),"index");

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		logables.index[index] = TRUE;
	else
		logables.index[index] = FALSE;

	total_logables = 0;
	for (i=0;i<max_logables;i++)
	{
		if (logables.index[i])
			total_logables++;
	}

	return TRUE;
}

void run_datalog(void)
{
	gint i = 0;
	gint offset = 0;
	gint size = 0;
	gint begin = FALSE;
	gint pos = 0;
	gint count = 0;
	void *data;
	gchar * tmpbuf;
	struct Io_File *iofile = NULL;
	unsigned char * uchar_ptr = (unsigned char *)runtime;
	short * short_ptr = (short *)runtime;
	float * float_ptr = (float *)runtime;

	if (!logging) /* Logging isn't enabled.... */
		return;
	else
	{
		tmpbuf = malloc(4096);
		data =  g_object_get_data(G_OBJECT(buttons.close_dlog_but),"data");
		if (data != NULL)
			iofile = (struct Io_File *)data;
		else
			fprintf(stderr,__FILE__": run_datalog, iofile undefined\n");

		if (header_needed)
		{
			write_log_header((void *)iofile);
			begin = TRUE;
			header_needed = FALSE;
		}
		for(i=0;i<total_logables;i++)
		{
			offset = offset_list[i];
			size = size_list[i];
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
						pos += g_sprintf(tmpbuf+pos,"%.3f",0.0);
					}
					else
					{
						gettimeofday(&now,NULL);
						cumulative += (now.tv_sec-last.tv_sec)+
							((double)(now.tv_usec-last.tv_usec)/
							 1000000.0);
						last.tv_sec = now.tv_sec;
						last.tv_usec = now.tv_usec;
						pos += g_sprintf(tmpbuf+pos,"%.3f",cumulative);
					}
					break;
				default:
					switch (size)
					{
						case FLOAT:
							pos += g_sprintf(tmpbuf+pos,"%.3f",(float)float_ptr[offset/FLOAT]);
							break;
						case UCHAR:
							pos += g_sprintf(tmpbuf+pos,"%i",(unsigned char)uchar_ptr[offset]);
							break;
						case SHORT:
							pos += g_sprintf(tmpbuf+pos,"%i",short_ptr[offset/SHORT]);
							break;
						default:
							printf("SIZE not defiend\n");
							break;
					}

					break;
			}
			/* Print delimiter to log here so there isnt an extra
			 * char at the end fo the line 
			 */
			if (i < (total_logables-1))
				pos += g_sprintf(tmpbuf+pos,"%s",delim);
		}
		pos += g_sprintf(tmpbuf+pos,"\n");
		g_io_channel_write_chars(iofile->iochannel,tmpbuf,pos,&count,NULL);
		g_free(tmpbuf);

	}
}

void write_log_header(void *ptr)
{
	gint i = 0;
	gint j = 0;
	gint total_logables = 0;
	gint count = 0;
	gint pos = 0;
	gchar *tmpbuf;
	struct Io_File *iofile = NULL;
	if (ptr != NULL)
		iofile = (struct Io_File *)ptr;
	else
		fprintf(stderr,__FILE__": iofile pointer was undefined...\n");
		

	tmpbuf = malloc(1024);
	for (i=0;i<max_logables;i++)
	{
		if (logables.index[i])/* If bit is set, increment counter */
			total_logables++;
	}

	for (i=0;i<max_logables;i++)
	{
		if (logables.index[i]) /* If bit is set, we log this variable */
		{
			offset_list[j] = logging_offset_map[i];
			size_list[j] = logging_datasizes_map[i];
			j++;
			pos = g_sprintf(tmpbuf, "\"%s\"",logable_names[i]);
			g_io_channel_write_chars(iofile->iochannel,tmpbuf,pos,&count,NULL);
			if (j < (total_logables))
			{
				pos = g_sprintf(tmpbuf,"%s",delim);
				g_io_channel_write_chars(iofile->iochannel,tmpbuf,pos,&count,NULL);
			}
		}
	}
	pos = g_sprintf(tmpbuf,"\n");
	g_io_channel_write_chars(iofile->iochannel,tmpbuf,pos,&count,NULL);
	g_free(tmpbuf);

}
