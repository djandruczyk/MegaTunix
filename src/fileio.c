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
#include <configfile.h>
#include <datalogging_gui.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <keyparser.h>
#include <logviewer_core.h>
#include <notifications.h>
#include <stdlib.h>
#include <structures.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>
#include <vex_support.h>
#ifdef __WIN32__
 #include <windows.h>
#else
 #include <sys/stat.h>
#endif


extern gchar * vex_comment;
extern GtkWidget *tools_view;
extern GtkWidget *dlog_view;
extern GHashTable *dynamic_widgets;
static gboolean dlog_open = FALSE;
static gboolean vex_open = FALSE;
static gboolean backup_open = FALSE;
static gboolean restore_open = FALSE;
static gboolean logview_open = FALSE;
static gchar *vexfile;
static gchar *dlogfile;
static gchar *logviewfile;


/*!
 \brief present_filesavebox() displays a file chooser to choose a file for
 import or export,  this is a generic routine used by several parts of 
 MegaTunix
 \param iotype (FileIoType enumeration) determines title used
 \param dest_widget (gpointer) pass through widget passed to the sig handler
 */
void present_filesavebox(FileIoType iotype,gpointer dest_widget)
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
	g_object_set_data(G_OBJECT(file_selector),"dest_widget",
			GINT_TO_POINTER(dest_widget));

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


/*!
 \brief check_filename() checks the filename passed to us from 
 present_filesavebox is valid and can be used for it's selected purpose.
 \param widget (GtkWidget *)
 \param file_selector (GtkFileSelection *) pointer to file selector.
 */
void check_filename (GtkWidget *widget, GtkFileSelection *file_selector) 
{
	gchar *selected_filename;
#ifndef __WIN32__
	struct stat status;
#endif
	gint size = 0;
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
		update_logbar("tools_view","warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		return;
	}
	else if (g_file_test(selected_filename,G_FILE_TEST_IS_REGULAR))
	{
		preexisting = TRUE;
#ifdef __WIN32__
		size = GetCompressedFileSize((LPCTSTR) selected_filename,NULL);
#else
		stat(selected_filename, &status);
		size = status.st_size;
#endif
	}
	else
		new_file = TRUE;

	/* If it's a new file, but we want to read, throw a warning... */
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
		if (size > 0)
		{
			/* DO NOT overwrite, prompt user first... */
			if (!warn_file_not_empty(iotype,selected_filename))
			{
				if (iotype == DATALOG_EXPORT)
				{
					tmpbuf = g_strdup("File chosen for datalogging already contained data, user has chosen not to over-write it...\n");
					update_logbar("dlog_view","warning",tmpbuf,TRUE,FALSE);
				}
				else
				{
					tmpbuf = g_strdup("File chosen already had data, user has chosen not to over-write it...\n");
					update_logbar("tools_view","warning",tmpbuf,TRUE,FALSE);
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
	if ((iotype == DATALOG_EXPORT) 
			|| (iotype == VE_EXPORT) 
			|| (iotype == FULL_BACKUP)) 
		iofile->iochannel = g_io_channel_new_file(selected_filename, "a+", NULL);
	else
		iofile->iochannel = g_io_channel_new_file(selected_filename, "r+", NULL);

	if(iofile->iochannel == NULL)
	{
		dbg_func(__FILE__": check_filename()\n\t iochannel could NOT be opened...\n",CRITICAL);
		tmpbuf = g_strdup_printf("File Open Failure\n");
		if (iotype == DATALOG_EXPORT)
			update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
		else
			update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
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
				dbg_func(__FILE__": check_filename()\n\tDatalog already open, should not open it twice...BUG!!!\n",CRITICAL);
				g_free(iofile);
				return;
			}
			dlogfile = g_strdup(selected_filename);
			dlog_open = TRUE;
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_stop_logging_button"),TRUE);
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_start_logging_button"),TRUE);
			g_object_set_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_close_log_button")),"data",(gpointer)iofile);
			gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,"dlog_file_label")),selected_filename);
					
			tmpbuf = g_strdup_printf("DataLog File Opened\n");
			update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
			g_free(tmpbuf);
			break;
		case DATALOG_IMPORT:
			if (logview_open)
			{
				dbg_func(__FILE__": check_filename()\n\tDatalog viewfile already open, should not open it twice...BUG!!!\n",CRITICAL);
				g_free(iofile);
				return;
			}
			logviewfile = g_strdup(selected_filename);
			logview_open = TRUE;
			tmpbuf = g_strdup_printf("DataLog ViewFile Opened\n");
			update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
			g_free(tmpbuf);
			load_logviewer_file(iofile);
			break;
		case FULL_BACKUP:
			if (backup_open)
			{
				dbg_func(__FILE__": check_filename()\n\tSettings Backup file already open, should not open it twice...BUG!!!\n",CRITICAL);
				g_free(iofile);
				return;
			}
			backup_open = TRUE;
			close_file(iofile);
			backup_all_ecu_settings(selected_filename);
			backup_open = FALSE;
			break;
		case FULL_RESTORE:
			if (restore_open)
			{
				dbg_func(__FILE__": check_filename()\n\tSettings Restore file already open, should not open it twice...BUG!!!\n",CRITICAL);
				g_free(iofile);
				return;
			}
			restore_open = TRUE;
			close_file(iofile);
			restore_all_ecu_settings(selected_filename);
			restore_open = FALSE;
			break;

		case VE_EXPORT:
			if (vex_open)
			{
				dbg_func(__FILE__": check_filename()\n\tVEX File already open, should not open it twice...BUG!!!\n",CRITICAL);
				g_free(iofile);
				return;
			}
			vexfile = g_strdup(selected_filename);
			vex_open = TRUE;
			if (vex_comment == NULL)
				tmpbuf = g_strdup_printf("VEX File Opened. VEX Comment undefined, exporting without one.\n");
			else
				tmpbuf = g_strdup_printf("VEX File Opened, VEX Comment already stored\n");

			update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
			g_free(tmpbuf);
			vetable_export(iofile);
			close_file (iofile);
			break;
		case VE_IMPORT:
			if (vex_open)
			{
				dbg_func(__FILE__": check_filename()\n\tVEX File already open, should not open it twice...BUG!!!\n",CRITICAL);
				g_free(iofile);
				return;
			}
			vexfile = g_strdup(selected_filename);
			vex_open = TRUE;
			tmpbuf = g_strdup_printf("VEX File Opened.\n");
			update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
			g_free(tmpbuf);
			vetable_import(iofile);
			close_file (iofile);
			break;
		default:
			dbg_func(__FILE__": ERROR in check_filename()\n",CRITICAL);
			break;
	}
}


/*!
 \brief close_file() closes the open file
 \param iofile (struct Io_File *) pointer to the struct Io_File
 */
void close_file(struct Io_File *iofile)
{
	gchar *tmpbuf = NULL;
	GIOStatus status;
	GError *error = NULL;

	if (!iofile)
	{
//		dbg_func(__FILE__": close_file()\n\tIo_File pointer is NULL, REturning NOW!!\n",CRITICAL);
		return;
	}

	if (iofile->iochannel != NULL)
	{
		status = g_io_channel_shutdown(iofile->iochannel,TRUE,&error);
		if (status != G_IO_STATUS_NORMAL)
			dbg_func(g_strdup_printf(__FILE__": close_file()\n\tError closing iochannel. message %s\n",error->message),CRITICAL);
	}

	switch (iofile->iotype)
	{
		case DATALOG_EXPORT:
			gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,"dlog_file_label")),"No Log Selected Yet");
					
			tmpbuf = g_strdup_printf("Logfile Closed\n");
			update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_stop_logging_button"),FALSE);
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"dlog_start_logging_button"),FALSE);
			g_object_set_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_close_log_button")),"data",NULL);
			dlog_open = FALSE;
			g_free(dlogfile);
			break;
		case DATALOG_IMPORT:
			tmpbuf = g_strdup_printf("LogView File Closed\n");
			update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
			logview_open = FALSE;
			g_free(logviewfile);
			break;
		case FULL_BACKUP:
			tmpbuf = g_strdup_printf("Full Backup File Closed\n");
			update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
			backup_open = FALSE;
			break;

		case FULL_RESTORE:
			tmpbuf = g_strdup_printf("Full Restore File Closed\n");
			update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
			restore_open = FALSE;
			break;

		case VE_EXPORT: /* Fall Through */
		case VE_IMPORT: /* VE Export/import.. */
			tmpbuf = g_strdup_printf("VEX File Closed\n");
			update_logbar("tools_view",NULL,tmpbuf,TRUE,FALSE);
			gtk_entry_set_text(GTK_ENTRY(g_hash_table_lookup(dynamic_widgets,"tools_vex_comment_entry")),"");

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


/*!
 \brief truncate_file() truncates the file to 0 bytes
 \param filetype (FileIoType enumeration) used to select a course of action
 \param filename (gchar *) filename to truncate
 */
void truncate_file(FileIoType filetype, gchar *filename)
{
	gchar *tmpbuf = NULL;
	gchar *base=NULL;

	switch (filetype)
	{
		case DATALOG_EXPORT:
			base = g_strdup("DataLog ");
			break;
		case VE_EXPORT:
			base = g_strdup("VE Export ");
			break;
		case FULL_BACKUP:
			base = g_strdup("MS Data Backup ");
			break;
		default:
			base = g_strdup("Unknown Type ");
			break;
	}
	if (truncate(filename,0) == 0)
		tmpbuf = g_strdup_printf("%sFile Truncation successful\n",base);
	else
		tmpbuf = g_strdup_printf("%sFile Truncation FAILED\n",base);
	update_logbar("dlog_view",NULL,tmpbuf,TRUE,FALSE);
	g_free(tmpbuf);
	g_free(base);
}


/*!
 \brief backup_all_ecu_settings() backs up the ECU to a filename passed
 \param filename (gchar *) filename to backup the ECU to
 */
void backup_all_ecu_settings(gchar *filename)
{
	extern struct Firmware_Details *firmware;
	ConfigFile *cfgfile;
	gchar * section = NULL;
	gint i = 0;
	gint x = 0;
	extern gint **ms_data;
	GString *string = NULL;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		cfgfile = cfg_new();

	cfg_write_string(cfgfile,"Firmware","name",firmware->name);
	for(i=0;i<firmware->total_pages;i++)
	{
		string = g_string_sized_new(64);
		section = g_strdup_printf("page_%i",i);
		cfg_write_int(cfgfile,section,"num_variables",firmware->page_params[i]->length);
		for(x=0;x<firmware->page_params[i]->length;x++)
		{
			string = g_string_append(string,g_strdup_printf("%i",ms_data[i][x]));
			if (x < (firmware->page_params[i]->length-1))
				string = g_string_append(string,",");
		}
		cfg_write_string(cfgfile,section,"data",string->str);
		g_string_free(string,TRUE);
	}
	cfg_write_file(cfgfile,filename);
	cfg_free(cfgfile);

	if (section)
		g_free(section);
}


/*!
 \brief restore_all_ecu_settings() reads the filename passed and if all checks
 pass the file will be loaded and any values that differ from the values
 currently in the ECU will be replaced.
 \param filename (filename to read for ecu restoration
 */
void restore_all_ecu_settings(gchar *filename)
{
	extern struct Firmware_Details *firmware;
	ConfigFile *cfgfile;
	gchar * section = NULL;
	gint i = 0;
	gint x = 0;
	gint tmpi = 0;
	gchar *tmpbuf = NULL;
	gchar **keys = NULL;
	gint num_keys = 0;
	extern gint **ms_data;
	extern gint **ms_data_last;

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		cfg_read_string(cfgfile,"Firmware","name",&tmpbuf);
		if (g_strcasecmp(tmpbuf,firmware->name) != 0)
		{
			dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tFirmware name mismatch: \"%s\" != \"%s\",\ncannot load this file for restoration\n",tmpbuf,firmware->name),CRITICAL);
			if (tmpbuf)
				g_free(tmpbuf);
			cfg_free(cfgfile);
			return;
		}
		for (i=0;i<firmware->total_pages;i++)
		{
			section = g_strdup_printf("page_%i",i);
			if(cfg_read_int(cfgfile,section,"num_variables",&tmpi))
				if (tmpi != firmware->page_params[i]->length)
					dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n",tmpi,firmware->page_params[i]->length),CRITICAL);
			if (cfg_read_string(cfgfile,section,"data",&tmpbuf))
			{
				keys = parse_keys(tmpbuf,&num_keys,",");
				if (num_keys != firmware->page_params[i]->length)
					dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n",num_keys,firmware->page_params[i]->length),CRITICAL);
				for (x=0;x<num_keys;x++)
				{
					ms_data[i][x]=atoi(keys[x]);
					if (ms_data[i][x] != ms_data_last[i][x])
					{
						write_ve_const(NULL,i,x,ms_data[i][x],firmware->page_params[i]->is_spark);
					}
				}

				g_strfreev(keys);
				g_free(tmpbuf);
			}

		}
	}
	update_ve_const();
}
