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
#include <datalogging_gui.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <gtk/gtk.h>
#include <notifications.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vex_support.h>


FILE *io_file;
extern gchar * vex_comment;
extern gboolean log_opened;
extern gchar * io_file_name;
extern GtkWidget *tools_view;
extern GtkWidget *dlog_view;
extern struct DynamicLabels labels;
extern struct DynamicButtons buttons;
extern struct DynamicEntries entries;
gboolean vex_opened;



void present_filesavebox(FileIoType data)
{
	GtkWidget *file_selector;

	file_selector = gtk_file_selection_new(NULL);
	g_object_set_data(G_OBJECT(file_selector),"iotype",
			GINT_TO_POINTER(data));

	gtk_file_selection_set_select_multiple(
			GTK_FILE_SELECTION(file_selector),
			FALSE);

	g_signal_connect (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (check_filename),
			(gpointer) file_selector);

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
	gint iotype = -1;
	static gboolean opened = FALSE;
	gchar * tmpbuf;

	iotype = (FileIoType )g_object_get_data(G_OBJECT(file_selector),"iotype");

	if (iotype == DATALOG_EXPORT)
	{
		if (log_opened)
			close_file(DATALOG_EXPORT);	
	}

	selected_filename = gtk_file_selection_get_filename (
			GTK_FILE_SELECTION (file_selector));

	/* Test to see if it DOES NOT exist */
	if (lstat(selected_filename, &status) == -1)
	{
		/* Open in Append mode, create if it doesn't exist */
		io_file = fopen(selected_filename,"a"); 

		if(!io_file)
			opened = FALSE;
		else
			opened = TRUE;
	}
	else /* Now see if it DOES exist.. */
	{
		if ((iotype == DATALOG_EXPORT))
		{
			if (status.st_size > 0)
				warn_file_not_empty();
		}
		/* Open in Append mode, create if it doesn't exist */
		io_file = fopen(selected_filename,"a"); 

		if(!io_file)
			opened = FALSE;
		else
			opened = TRUE;
	}

	switch ((FileIoType)iotype)
	{
		case DATALOG_EXPORT:
			if (opened == TRUE)
			{
				gtk_widget_set_sensitive(buttons.stop_dlog_but,TRUE);
				gtk_widget_set_sensitive(buttons.start_dlog_but,TRUE);
				io_file_name = g_strdup(selected_filename);
				gtk_label_set_text(GTK_LABEL(
							labels.dlog_file_lab),
						selected_filename);
				tmpbuf = g_strdup_printf("DataLog File Opened\n");
				update_logbar(dlog_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				log_opened = TRUE;
			}
			else  /* Failed to open */
			{
				log_opened = FALSE;
				opened = FALSE;
				gtk_widget_set_sensitive(
						buttons.stop_dlog_but,FALSE);
				gtk_widget_set_sensitive(
						buttons.start_dlog_but,FALSE);
				tmpbuf = g_strdup_printf("Failure opening DataLog File, Error Code: %s\n",strerror(errno));
				update_logbar(dlog_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		case VE_EXPORT:
			if (opened == TRUE)
			{
				vex_opened = TRUE;
				io_file_name = g_strdup(selected_filename);
				gtk_label_set_text(GTK_LABEL(
							labels.vex_file_lab),
						selected_filename);
				gtk_widget_set_sensitive(
						buttons.ve_export_but,TRUE);
				/* this needs to be put in VE_IMPORT or something */
				gtk_widget_set_sensitive(
						buttons.ve_import_but,TRUE);				
				gtk_widget_set_sensitive(
						buttons.ve_clear_vex_but,TRUE);
				if (vex_comment == NULL)
					tmpbuf = g_strdup_printf("VEX File Opened. Suggest setting a comment to describe the file\n");
				else
					tmpbuf = g_strdup_printf("VEX File Opened, VEX Comment already stored\n");

				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			else
			{
				vex_opened = FALSE;
				tmpbuf = g_strdup_printf("File Couldn't be opened to export to!!!\n");
				update_logbar(tools_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		case VE_IMPORT:
			/* needs fixed - doesn't get called */
			if (opened == TRUE)
			{
				vex_opened = TRUE;
				io_file_name = g_strdup(selected_filename);
				gtk_label_set_text(GTK_LABEL(
							labels.vex_file_lab),
						selected_filename);
				gtk_widget_set_sensitive(
						buttons.ve_import_but,TRUE);
				gtk_widget_set_sensitive(
						buttons.ve_clear_vex_but,TRUE);
				tmpbuf = g_strdup_printf("VEX File Opened.\n");
				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			else
			{
				vex_opened = FALSE;
				tmpbuf = g_strdup_printf("File Couldn't be opened to import from!!!\n");
				update_logbar(tools_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		default:
			printf("ERROR in check_filename()\n");
			break;


	}
}

void close_file(FileIoType filetype)
{
	gchar *tmpbuf;
	switch (filetype)
	{
		case DATALOG_EXPORT:
			if (log_opened == TRUE)
			{
				fclose(io_file);
				g_free(io_file_name);
				gtk_label_set_text(GTK_LABEL(
							labels.dlog_file_lab),
						"No Log Selected Yet");
				tmpbuf = g_strdup_printf("Logfile Closed\n");
				update_logbar(dlog_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				gtk_widget_set_sensitive(									buttons.stop_dlog_but,FALSE);
				gtk_widget_set_sensitive(
						buttons.start_dlog_but,FALSE);
				log_opened = FALSE;
			}
			break;
		default: /* Everything else.. */
			if (vex_opened == TRUE)
			{
				fclose(io_file);
				g_free(io_file_name);
				gtk_label_set_text(GTK_LABEL(
							labels.vex_file_lab),
						"No VEX File Selected Yet");
				tmpbuf = g_strdup_printf("VEX File Closed\n");
				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				gtk_entry_set_text(GTK_ENTRY(
							entries.vex_comment_entry),
						"");
				gtk_widget_set_sensitive(
						buttons.ve_export_but,FALSE);
				gtk_widget_set_sensitive(
						buttons.ve_import_but,FALSE);				
				gtk_widget_set_sensitive(
						buttons.ve_clear_vex_but,FALSE);
				vex_opened = FALSE;
			}
			break;

	}
}

void truncate_file(FileIoType filetype)
{
	gchar *tmpbuf;
	switch (filetype)
	{
		case DATALOG_EXPORT:
			if (log_opened == TRUE)
			{
				truncate(io_file_name,0);
				if (errno)
					tmpbuf = g_strdup_printf("DataLog Truncation Error: %s\n",strerror(errno));
				else
					tmpbuf = g_strdup_printf("DataLog Truncation successful\n");
				update_logbar(dlog_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		default:
			if (vex_opened == TRUE)
			{
				truncate(io_file_name,0);
				tmpbuf = g_strdup_printf("VEX File Truncated\n");
				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;

	}
}
