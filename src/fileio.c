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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vex_support.h>


extern gboolean log_opened;
extern FILE *io_file;
extern gchar * io_file_name;
extern GtkWidget *stop_button;
extern GtkWidget *start_button;
extern GtkWidget *file_label;
extern GtkWidget *dlog_statbar;
extern gint dlog_context_id;
extern GtkWidget *tools_statbar;
extern gint tools_context_id;


static gchar buff[100];  

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
	static gboolean opened;

	iotype = (FileIoType )g_object_get_data(G_OBJECT(file_selector),"iotype");

	if (log_opened)
		close_logfile();	

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
		if ((iotype == DATALOG_EXPORT) || (iotype == VE_EXPORT))
		{
			printf("filetype is dlog or ve export\n");
			if (status.st_size > 0)
				warn_datalog_not_empty();
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
				gtk_widget_set_sensitive(stop_button,TRUE);
				gtk_widget_set_sensitive(start_button,TRUE);
				io_file_name = g_strdup(selected_filename);
				gtk_label_set_text(GTK_LABEL(file_label),
						selected_filename);
				g_snprintf(buff,100,"DataLog File Opened");
				update_statusbar(dlog_statbar,
						dlog_context_id,buff);
				log_opened = TRUE;
			}
			else  /* Failed to open */
			{
				log_opened = FALSE;
				opened = FALSE;
				gtk_widget_set_sensitive(stop_button,FALSE);
				gtk_widget_set_sensitive(start_button,FALSE);
				g_snprintf(buff,100,"Failure opening DataLog File, Error Code: %s",strerror(errno));
				update_statusbar(dlog_statbar,
						dlog_context_id,buff);
			}
			break;
		case VE_EXPORT:
			if (opened == TRUE)
			{
				if(vetable_export())
				{
					g_snprintf(buff,100,"VE_Export Success!");
					update_statusbar(tools_statbar,
							tools_context_id,buff);
				}
				else
				{
					g_snprintf(buff,100,"VE_Export FAILED!!");
					update_statusbar(tools_statbar,
							tools_context_id,buff);
				}
			}
			else
			{
				g_snprintf(buff,100,"File Couldn't be opened to export to!!!");
				update_statusbar(tools_statbar,
						tools_context_id,buff);
			}
			break;
		case VE_IMPORT:
			if (opened == TRUE)
			{
				if (vetable_import())
				{
					g_snprintf(buff,100,"VE_Import Success!");
					update_statusbar(tools_statbar,
							tools_context_id,buff);
				}
				else
				{
					g_snprintf(buff,100,"VE_Import Failure,  File not compliant!!!");
					update_statusbar(tools_statbar,
							tools_context_id,buff);
				}
			}
			else
			{
				g_snprintf(buff,100,"File Couldn't be opened to import from!!!");
				update_statusbar(tools_statbar,
						tools_context_id,buff);

			}
			break;
		default:
			printf("ERROR in check_filename()\n");
			break;


	}
}
