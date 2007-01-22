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

#include <binreloc.h>
#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <getfiles.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <structures.h>

/* Static private functions */
static GtkFileChooserConfirmation confirm_overwrite_callback (GtkFileChooser *, gpointer );



/*!
 \brief get_files() returns a list of files located at the pathstub passed
 this function will first search starting from ~/.MegaTunix+pathstub and
 then in the system path of $PREFIX/share/MegaTunix/+pathstub, it'll return
 the list as a vector char array. (free with g_strfreev)
 \param pathstub (gchar *) partial path to search for files
 \param extension (gchar *) extension to search for 
 \returns vector char array of filenames or NULL if none found
 */
gchar ** get_files(gchar *pathstub, gchar * extension)
{
	gchar *path = NULL;
	gchar *parent = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	GDir *dir = NULL;


	path = g_build_filename(HOME(), ".MegaTunix",pathstub,NULL);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto syspath;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		if (!g_str_has_suffix(filename,extension))
		{
			filename = (gchar *)g_dir_read_name(dir);
			continue;
		}

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
		{
			tmpbuf = g_strconcat(list,",",path,filename,NULL);
			g_free(list);
			list = tmpbuf;
		}

		filename = (gchar *)g_dir_read_name(dir);
		
	}
	g_free(path);
	g_dir_close(dir);

	syspath:
	parent = gbr_find_data_dir(DATA_DIR);
	path = g_build_filename(parent,pathstub,NULL);
	g_free(parent);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto finish;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		if (!g_str_has_suffix(filename,extension))
		{
			filename = (gchar *)g_dir_read_name(dir);
			continue;
		}

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
		{
			tmpbuf = g_strconcat(list,",",path,filename,NULL);
			g_free(list);
			list = tmpbuf;
		}

		filename = (gchar *)g_dir_read_name(dir);
	}
	g_free(path);
	g_dir_close(dir);

	finish:
	g_free(pathstub);
	g_free(extension);
	if (!list)
	{
		//dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);
		return NULL;
	}
	vector = g_strsplit(list,",",0);
	g_free(list);
	return (vector);
}


/*!
 \brief get_file() gets a single file defnied by pathstub, first searching in
 ~/.MegaTunix+pathstub, and then in $PREFIX/share/MegaTunix/+pathstub,
 \param pathstub (gchar *) partial path to filename
 \param extension (gchar *) extension wanted..
 \returns filename if found or NULL if not found
 */
gchar * get_file(gchar *pathstub,gchar *extension)
{
	gchar *filename = NULL;
	gchar *dir = NULL;
	gchar *ext = NULL;
	gchar *file = NULL;
	if (extension)
		ext = g_strconcat(".",extension,NULL);
	else
		ext = g_strdup("");

	file = g_strconcat(pathstub,ext,NULL);

	g_free(ext);

	filename = g_build_filename(HOME(), ".MegaTunix",file,NULL);
	if (g_file_test(filename,(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
	{
		g_free(pathstub);
		g_free(extension);
		g_free(file);
		return filename;
	}
	else 
	{
		g_free(filename);
		dir = gbr_find_data_dir(DATA_DIR);
		filename = g_build_filename(dir,file,NULL);

		g_free(dir);
		g_free(file);

		if (g_file_test(filename,(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
		{
			g_free(pathstub);
			g_free(extension);
			return filename;
		}
		else
			g_free(filename);
	}
	g_free(pathstub);
	g_free(extension);
	return NULL;
}


gchar * choose_file(MtxFileIO *data)
{
	GtkWidget *dialog = NULL;
	GtkFileFilter *filter = NULL;
	gchar * path = NULL;
	gchar *filename = NULL;
	gint i = 0;

	/*
	printf("choose_file\n");
	printf("filter %s\n",data->filter);
	printf("filename %s\n",data->filename);
	printf("stub_path %s\n",data->stub_path);
	printf("title %s\n",data->title);
	*/
	if (!data->title)
		data->title = g_strdup("Open File");

	if (data->action == GTK_FILE_CHOOSER_ACTION_OPEN)
	{
		dialog = gtk_file_chooser_dialog_new(data->title,
				NULL,
				data->action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
	}
	else if (data->action == GTK_FILE_CHOOSER_ACTION_SAVE)
	{
		dialog = gtk_file_chooser_dialog_new(data->title,
				NULL,
				data->action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
	}
	else
		return NULL;
	/* Add shortcut folders... */
	if (data->shortcut_folders)
	{
		
		for (i=0;i<g_strv_length(data->shortcut_folders);i++)
		{
#ifndef __WIN32__
			path = g_build_path(PSEP,DATA_DIR,data->shortcut_folders[i],NULL);
			gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),path,NULL);
			g_free(path);
#endif
			path = g_build_path(PSEP,HOME(),".MegaTunix",data->shortcut_folders[i],NULL);
			gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),path,NULL);
			g_free(path);
		}
	}
	/* If stub path switch to that place */
	if (data->stub_path)
	{
		path = g_build_path(PSEP,HOME(),data->stub_path,NULL);
		if (!g_file_test(path,G_FILE_TEST_IS_DIR))
			g_mkdir(path,0755);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),path);
		g_free(path);
	}
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),HOME());

	/* If filters, assign them */
	if (data->filter)
	{
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),"*.*");
		gtk_file_filter_set_name(GTK_FILE_FILTER(filter),"All Files");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),data->filter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog),filter);
	}
	/* Turn on overwriteconfirmation */
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
		g_signal_connect(G_OBJECT(dialog),"confirm-overwrite",
				G_CALLBACK (confirm_overwrite_callback), NULL);
	}
#endif

	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),TRUE);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);

	return (filename);
}

static GtkFileChooserConfirmation
confirm_overwrite_callback (GtkFileChooser *chooser, gpointer data)
{
	GtkWidget *dialog = NULL;
	char *filename;
	gint result = -1;

	filename = gtk_file_chooser_get_filename (chooser);

	/* If read only select again */
	if (g_open(filename,O_RDWR,O_CREAT|O_APPEND) == -1)
	{
		dialog = gtk_message_dialog_new(NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				"File %s\nis READ ONLY, Pleae Choose Another",filename);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
	}
	/* File exists but is NOT R/O,  prompt to truncate or not */
	else
	{
		dialog = gtk_message_dialog_new(NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"File %s exists!\n Do you wish to overwrite it?",filename);
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
		{
			gtk_widget_destroy(dialog);
			result = g_remove(filename);
			if (result == 0)
				return GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
		}
		else
		{
			gtk_widget_destroy(dialog);
			return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
		}
	}
	return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
}


void free_mtxfileio(MtxFileIO *data)
{
	if (!data)
		return;
	if (data->title)
		g_free(data->title);
	if (data->filename)
		g_free(data->filename);
	if (data->stub_path)
		g_free(data->stub_path);
	if (data->filter)
		g_free(data->filter);
	if (data->shortcut_folders)
		g_strfreev(data->shortcut_folders);
	return;
}
