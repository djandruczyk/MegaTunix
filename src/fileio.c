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
gboolean backup_opened;
gboolean restore_opened;


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

	/* Destroy filesavebox on "OK" */
	g_signal_connect_swapped (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			(gpointer) file_selector);

	g_signal_connect (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->cancel_button),
			"clicked",
			G_CALLBACK (check_filename),
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
	gchar *selected_filename;
	struct stat status;
	gint iotype = -1;
	static gboolean opened = FALSE;
	gchar * tmpbuf = NULL;
	gboolean preexisting = FALSE;
	gboolean new_file = FALSE;

	iotype = (FileIoType )g_object_get_data(G_OBJECT(file_selector),"iotype");
	if (iotype == DATALOG_EXPORT)
	{
		if (log_opened)
			close_file(DATALOG_EXPORT);	
	}

	selected_filename = (gchar *)gtk_file_selection_get_filename (
			GTK_FILE_SELECTION (file_selector));

	if (g_file_test(selected_filename,G_FILE_TEST_IS_DIR))
	{
		tmpbuf=g_strdup_printf("Invalid File or NO File Selected!!!\n");
		update_logbar(tools_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
		return;
	}
	else if (g_file_test(selected_filename,G_FILE_TEST_IS_REGULAR))
		preexisting = TRUE;
	else
		new_file = TRUE;

	if ((new_file) && ((iotype == FULL_RESTORE)||(iotype == VE_IMPORT)))
	{
		warn_input_file_not_exist(iotype, selected_filename);
		return;
	}
	
	if (preexisting)
		stat(selected_filename, &status);
	/* Open file in append mode, create if non-existant */
	io_file = fopen(selected_filename,"a"); 
	if(!io_file)
		opened = FALSE;
	else
	{
		opened = TRUE;
		io_file_name = g_strdup(selected_filename);
	}

	if (((iotype == DATALOG_EXPORT) 
		|| (iotype == VE_EXPORT) 
		|| (iotype == FULL_BACKUP)) 
			&& (preexisting == TRUE))
	{
		if (status.st_size > 0)
		{
			/* DO NOT overwrite, prompt user first... */
			warn_file_not_empty(iotype,selected_filename);
			gtk_widget_set_sensitive(buttons.tools_clear_but,TRUE);
			return;
		}
		else
			gtk_widget_set_sensitive(buttons.tools_clear_but,FALSE);
	}

	switch ((FileIoType)iotype)
	{
		case DATALOG_EXPORT:
			if (opened == TRUE)
			{
				gtk_widget_set_sensitive(buttons.stop_dlog_but,TRUE);
				gtk_widget_set_sensitive(buttons.start_dlog_but,TRUE);
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
				gtk_widget_set_sensitive(
						buttons.stop_dlog_but,FALSE);
				gtk_widget_set_sensitive(
						buttons.start_dlog_but,FALSE);
				tmpbuf = g_strdup_printf("Failure opening DataLog File, Error Code: %s\n",strerror(errno));
				update_logbar(dlog_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		case FULL_BACKUP:
			if (opened == TRUE)
			{
				backup_opened = TRUE;		
				backup_all_ms_settings(selected_filename);
				close_file(FULL_BACKUP);
			}
			else
			{
				backup_opened = FALSE;		
				tmpbuf = g_strdup_printf("File Couldn't be opened, NO Settings Backed Up to file!!!\n");
				update_logbar(tools_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		case FULL_RESTORE:
			if (opened == TRUE)
			{
				restore_opened = TRUE;		
				restore_all_ms_settings(selected_filename);
				close_file(FULL_RESTORE);
			}
			else
			{
				restore_opened = FALSE;
				tmpbuf = g_strdup_printf("File Couldn't be opened, NO Settings Restored from file!!!\n");
				update_logbar(tools_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
			}
			break;
		
		case VE_EXPORT:
			if (opened == TRUE)
			{
				vex_opened = TRUE;
				gtk_label_set_text(GTK_LABEL(
							labels.vex_file_lab),
						selected_filename);
				if (vex_comment == NULL)
					tmpbuf = g_strdup_printf("VEX File Opened. VEX Comment undefined, exporting without one.\n");
				else
					tmpbuf = g_strdup_printf("VEX File Opened, VEX Comment already stored\n");

				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				vetable_export();
				close_file (VE_EXPORT);
			}
			else
			{
				vex_opened = FALSE;
				tmpbuf = g_strdup_printf("File Couldn't be opened to export to!!!\n");
				update_logbar(tools_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
				close_file (VE_EXPORT);
			}
			break;
		case VE_IMPORT:
			/* needs fixed - doesn't get called */
			if (opened == TRUE)
			{
				vex_opened = TRUE;
				gtk_label_set_text(GTK_LABEL(
							labels.vex_file_lab),
						selected_filename);
				tmpbuf = g_strdup_printf("VEX File Opened.\n");
				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				vetable_import();
				close_file (VE_IMPORT);
			}
			else
			{
				vex_opened = FALSE;
				tmpbuf = g_strdup_printf("File Couldn't be opened to import from!!!\n");
				update_logbar(tools_view,"warning",tmpbuf,TRUE);
				g_free(tmpbuf);
				close_file (VE_IMPORT);
			}
			break;
		default:
			printf("ERROR in check_filename()\n");
			break;


	}
}

void close_file(FileIoType iotype)
{
	gchar *tmpbuf;
	switch (iotype)
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
		case FULL_BACKUP:
			if (backup_opened)
			{
				fclose(io_file);
				g_free(io_file_name);
				tmpbuf = g_strdup_printf("Full Backup File Closed\n");
				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				backup_opened = FALSE;
			}
			break;
			
		case FULL_RESTORE:
			if (restore_opened)
			{
				fclose(io_file);
				g_free(io_file_name);
				tmpbuf = g_strdup_printf("Full Restore File Closed\n");
				update_logbar(tools_view,NULL,tmpbuf,TRUE);
				g_free(tmpbuf);
				restore_opened = FALSE;
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
				vex_opened = FALSE;
			}
			break;
			

	}
}

void truncate_file(FileIoType filetype)
{
	gchar *tmpbuf;
	if (io_file_name)
		truncate(io_file_name,0);
	switch (filetype)
	{
		case DATALOG_EXPORT:
			tmpbuf = g_strdup_printf("DataLog Truncation successful\n");
			update_logbar(dlog_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			break;
		case VE_EXPORT:
			tmpbuf = g_strdup_printf("File Truncated successfully\n");
			update_logbar(tools_view,NULL,tmpbuf,TRUE);
			g_free(tmpbuf);
			gtk_widget_set_sensitive(buttons.tools_clear_but,FALSE);
			break;
		default:
			break;

	}
}

void backup_all_ms_settings(gchar * filename)
{
	fprintf(stderr,__FILE__": backup all MS settings to file isn't written yet...\n");
	fprintf(stderr,__FILE__": should backup to %s\n",filename);

}

void restore_all_ms_settings(gchar * filename)
{
	GIOChannel *iochannel;
	struct Ve_Const_Std *local_buffer;
	extern struct Ve_Const_Std *ve_const_p0;
	extern struct Ve_Const_Std *ve_const_p1;
	extern struct Ve_Const_Std *ve_const_p0;
	extern struct Ve_Const_Std *ve_const_p1;
	extern struct Ve_Const_Std *backup_ve_const_p0;
	extern struct Ve_Const_Std *backup_ve_const_p1;

	local_buffer = g_malloc(MS_PAGE_SIZE);
	/* Backup currently active parameters into backup structure */
        memcpy(backup_ve_const_p0, ve_const_p0, MS_PAGE_SIZE);
        memcpy(backup_ve_const_p1, ve_const_p1, MS_PAGE_SIZE);
	
	fprintf(stderr,__FILE__": restore all MS settings from file isn't written yet\n");
	fprintf(stderr,__FILE__": should restore from %s\n",filename);
	iochannel = g_io_channel_new_file(filename, "w", NULL);

	/* Do Something here... */
	g_io_channel_close(iochannel);
	
}
