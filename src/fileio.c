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
extern GtkWidget *tools_view;
extern GtkWidget *dlog_view;
extern struct DynamicLabels labels;
extern struct DynamicButtons buttons;
extern struct DynamicEntries entries;
static gboolean dlog_open = FALSE;
static gboolean vex_open = FALSE;
static gboolean backup_open = FALSE;
static gboolean restore_open = FALSE;
static gchar *vexfile;
static gchar *dlogfile;

void present_filesavebox(FileIoType iotype)
{
	/* The idea is to present a filesavebox and bind the datatype
	 * we want to handle to it.  The box handler (check_filename) 
	 * extracts this data and uses it to determine the proper course
	 * of action... This prevents repetition of lots of code...
	 */
	GtkWidget *file_selector;
	gchar *title = NULL;
	switch (iotype)
	{
		case FULL_BACKUP:
			title = g_strdup("Select File to backup ALL MS parameters...");
			break;
		case FULL_RESTORE:
			title = g_strdup("Select File to restore MS parameters from...");
			break;
		case VE_EXPORT:
			title = g_strdup("Select a file to export the VE-Table(s) to...");
			break;
		case VE_IMPORT:
			title = g_strdup("Select a file to import VE-Table(s) from...");
			break;
		case DATALOG_EXPORT:
			title = g_strdup("Select a file to save your MS Datalog to...");
			break;
		case DATALOG_IMPORT:
			title = g_strdup("Select a file to load your MS Datalog from...");
			break;
		default:
			title = g_strdup("Title not set BUG!! contact author\n");
			break;
		
			
	}

	file_selector = gtk_file_selection_new(title);
	if (title)
		g_free(title);
	g_object_set_data(G_OBJECT(file_selector),"iotype",
			GINT_TO_POINTER(iotype));

	/* Only allow one file to be selected.. */
	gtk_file_selection_set_select_multiple(
			GTK_FILE_SELECTION(file_selector),
			FALSE);

	/* Call handler on OK */
	g_signal_connect (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (check_filename),
			(gpointer) file_selector);

	/* Destroy filesavebox on "OK" AFTER previous handler runs */
	g_signal_connect_swapped (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			(gpointer) file_selector);

	/* Destroy widget on canel */
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
	gchar *selected_filename;
	struct stat status;
	struct Io_File *iofile;
	gint iotype = -1;
	gchar * tmpbuf = NULL;
	gboolean preexisting = FALSE;
	gboolean new_file = FALSE;

	iotype = (FileIoType )g_object_get_data(G_OBJECT(file_selector),"iotype");
	selected_filename = (gchar *)gtk_file_selection_get_filename (
			GTK_FILE_SELECTION (file_selector));

	if (g_file_test(selected_filename,G_FILE_TEST_IS_DIR))
	{
		tmpbuf=g_strdup_printf("You Selected a Directory, you need to select a file!!!\n");
		update_logbar(tools_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
		return;
	}
	else if (g_file_test(selected_filename,G_FILE_TEST_IS_REGULAR))
	{
		preexisting = TRUE;
		stat(selected_filename, &status);
	}
	else
		new_file = TRUE;

	/* If it's a new file, but we wantto read, throw a warning... */
	if ((new_file) && ((iotype == FULL_RESTORE) || (iotype == VE_IMPORT)))
	{
		warn_input_file_not_exist(iotype, selected_filename);
		return;
	}
	
	if (((iotype == DATALOG_EXPORT) 
		|| (iotype == VE_EXPORT) 
		|| (iotype == FULL_BACKUP)) 
		&& (preexisting == TRUE))
	{
		if (status.st_size > 0)
		{
			/* DO NOT overwrite, prompt user first... */
			if (!warn_file_not_empty(iotype,selected_filename))
			{
				if (iotype == DATALOG_EXPORT)
				{
					tmpbuf = g_strdup("File chosen for datalog already contained data, user chose not to over-write it...\n");
					update_logbar(dlog_view,"warning",tmpbuf,TRUE);
				}
				else
				{
					tmpbuf = g_strdup("File chosen already had data, user chose not to over-write it...\n");
					update_logbar(tools_view,"warning",tmpbuf,TRUE);
				}
				g_free(tmpbuf);
				return;
			}
		}
	}

	iofile = g_malloc0(sizeof(struct Io_File));
	iofile->filename = g_strdup(selected_filename);
	iofile->iotype = iotype;
				
	/* Open file in append mode, create if non-existant */
	iofile->iochannel = g_io_channel_new_file(selected_filename, "a+", NULL);

	if(iofile->iochannel == NULL)
	{
		tmpbuf = g_strdup_printf("File Open Failure\n");
		if (iotype == DATALOG_EXPORT)
			update_logbar(dlog_view,NULL,tmpbuf,TRUE);
		else
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
		g_free(tmpbuf);
		g_free(iofile->filename);
		g_free(iofile);
		return;
	}

	switch ((FileIoType)iotype)
	{
		case DATALOG_EXPORT:
			if (dlog_open)
			{
				fprintf(stderr,__FILE__": Datalog already open,  should not open it twice...BUG!!!\n");
				g_free(iofile);
				return;
			}
			dlogfile = g_strdup(selected_filename);
			dlog_open = TRUE;
			gtk_widget_set_sensitive(buttons.stop_dlog_but,TRUE);
			gtk_widget_set_sensitive(buttons.start_dlog_but,TRUE);
			g_object_set_data(G_OBJECT(buttons.close_dlog_but),
					"data",(gpointer)iofile);

			gtk_label_set_text(GTK_LABEL(
						labels.dlog_file_lab),
					selected_filename);
			tmpbuf = g_strdup_printf("DataLog File Opened\n");
			update_logbar(dlog_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			break;
		case FULL_BACKUP:
			if (backup_open)
			{
				fprintf(stderr,__FILE__": Settings Backup file already open,  should not open it twice...BUG!!!\n");
				g_free(iofile);
				return;
			}
			backup_open = TRUE;
			backup_all_ms_settings(iofile);
			close_file(iofile);
			break;
		case FULL_RESTORE:
			if (restore_open)
			{
				fprintf(stderr,__FILE__": Settings Restore file already open,  should not open it twice...BUG!!!\n");
				g_free(iofile);
				return;
			}
			restore_open = TRUE;
			restore_all_ms_settings(iofile);
			close_file(iofile);
			break;

		case VE_EXPORT:
			if (vex_open)
			{
				fprintf(stderr,__FILE__": VEX File already open,  should not open it twice...BUG!!!\n");
				g_free(iofile);
				return;
			}
			vexfile = g_strdup(selected_filename);
			vex_open = TRUE;
			if (vex_comment == NULL)
				tmpbuf = g_strdup_printf("VEX File Opened. VEX Comment undefined, exporting without one.\n");
			else
				tmpbuf = g_strdup_printf("VEX File Opened, VEX Comment already stored\n");

			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			vetable_export(iofile);
			close_file (iofile);
			break;
		case VE_IMPORT:
			if (vex_open)
			{
				fprintf(stderr,__FILE__": VEX File already open,  should not open it twice...BUG!!!\n");
				g_free(iofile);
				return;
			}
			vexfile = g_strdup(selected_filename);
			vex_open = TRUE;
			tmpbuf = g_strdup_printf("VEX File Opened.\n");
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			vetable_import(iofile);
			close_file (iofile);
			break;
		default:
			printf("ERROR in check_filename()\n");
			break;
	}
}

void close_file(void *ptr)
{
	struct Io_File *iofile = NULL;
	gchar *tmpbuf = NULL;
	GIOStatus status;
	
	if (ptr != NULL)
		iofile = (struct Io_File *)ptr;
	else
	{
		fprintf(stderr,__FILE__": close_file() pointer null\n");
		return;
	}

	if (iofile->iochannel != NULL)
	{
		status = g_io_channel_shutdown(iofile->iochannel,TRUE,NULL);
		if (status != G_IO_STATUS_NORMAL)
			fprintf(stderr,__FILE__": Error closing iochannel\n");
	}
	
	switch (iofile->iotype)
	{
		case DATALOG_EXPORT:
			gtk_label_set_text(GTK_LABEL(
						labels.dlog_file_lab),
					"No Log Selected Yet");
			tmpbuf = g_strdup_printf("Logfile Closed\n");
			update_logbar(dlog_view,NULL,tmpbuf,TRUE);
			gtk_widget_set_sensitive(									buttons.stop_dlog_but,FALSE);
			gtk_widget_set_sensitive(
					buttons.start_dlog_but,FALSE);
			g_object_set_data(G_OBJECT(buttons.close_dlog_but),"data",NULL);
			dlog_open = FALSE;
			g_free(dlogfile);
			break;
		case FULL_BACKUP:
			tmpbuf = g_strdup_printf("Full Backup File Closed\n");
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			backup_open = FALSE;
			break;

		case FULL_RESTORE:
			tmpbuf = g_strdup_printf("Full Restore File Closed\n");
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			restore_open = FALSE;
			break;

		case DATALOG_IMPORT: /* Not implemented yet */
			break;
		case VE_EXPORT: /* Fall Through */
		case VE_IMPORT: /* VE Export/import.. */
			tmpbuf = g_strdup_printf("VEX File Closed\n");
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			gtk_entry_set_text(GTK_ENTRY(
						entries.vex_comment_entry),"");
			vex_open = FALSE;
			g_free(vexfile);
			break;
	}

	g_free(iofile->filename);
	iofile->filename = NULL;
	g_free(iofile->iochannel);
	iofile->iochannel = NULL;
	g_free(iofile);
	iofile = NULL;
	if(tmpbuf)
		g_free(tmpbuf);
}

void truncate_file(FileIoType filetype, gchar *filename)
{
	gchar *tmpbuf;

	switch (filetype)
	{
		case DATALOG_EXPORT:
			if (truncate(filename,0) == 0)
				tmpbuf = g_strdup_printf("DataLog Truncation successful\n");
			else
				tmpbuf = g_strdup_printf("DataLog Truncation FAILED\n");
			update_logbar(dlog_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			break;
		case VE_EXPORT:
			if (truncate(filename,0) == 0)
				tmpbuf = g_strdup_printf("VE Export file Truncated successfully\n");
			else
				tmpbuf = g_strdup_printf("File Truncation FAILED\n");
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			break;
		default:
			printf("truncating nothing, iotype was %i\n",filetype);
			break;

	}
}

void backup_all_ms_settings(void *ptr)
{
	fprintf(stderr,__FILE__": backup all MS settings to file isn't written yet...\n");
	//fprintf(stderr,__FILE__": should backup to %s\n",filename);

}

void restore_all_ms_settings(void *ptr)
{
/*
	GIOChannel *iochannel;
	struct Ve_Const_Std *local_buffer;

	local_buffer = g_malloc(MS_PAGE_SIZE);
	// Backup currently active parameters into backup structure 
	
	fprintf(stderr,__FILE__": restore all MS settings from file isn't written yet\n");
	fprintf(stderr,__FILE__": should restore from %s\n",filename);
	iochannel = g_io_channel_new_file(filename, "w", NULL);

	g_io_channel_close(iochannel);
*/
	
}
