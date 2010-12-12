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
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/*!
 \brief get_files() returns a list of files located at the pathstub passed
 this function will first search starting from ~/.MegaTunix+pathstub and
 then in the system path of $PREFIX/share/MegaTunix/+pathstub, it'll return
 the list as a vector char array. (free with g_strfreev)
 \param pathstub (gchar *) partial path to search for files
 \param extension (gchar *) extension to search for 
 \param class (FileClass) enumeration pointer to array of classes for each
 file found
 \returns vector char array of filenames or NULL if none found
 */
gchar ** get_files(gchar *input, gchar * extension, GArray **classes)
{
	gchar *pathstub = NULL;
	gchar *path = NULL;
	gchar *parent = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	FileClass tmp = PERSONAL;
	GDir *dir = NULL;


	if (!g_str_has_suffix(input, PSEP))
		pathstub = g_strconcat(input,PSEP,NULL);
	else
		pathstub = g_strdup(input);
	g_free(input);

	/* Personal files first */
	path = g_build_filename(get_home(), ".MegaTunix",pathstub,NULL);
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
		tmp = PERSONAL;
		if (!*classes)
		{
			*classes = g_array_new(FALSE,TRUE,sizeof(FileClass));
			g_array_append_val(*classes,tmp);
		}
		else
			g_array_append_val(*classes,tmp);


		filename = (gchar *)g_dir_read_name(dir);

	}
	g_free(path);
	g_dir_close(dir);

syspath:
#ifdef __WIN32__
	parent = g_build_path(PSEP,get_home(),"dist",NULL);
#else
	parent = g_strdup(DATA_DIR);
#endif
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
		tmp = SYSTEM;
		if (!*classes)
		{
			*classes = g_array_new(FALSE,TRUE,sizeof(FileClass));
			g_array_append_val(*classes,tmp);
		}
		else
			g_array_append_val(*classes,tmp);


		filename = (gchar *)g_dir_read_name(dir);
	}
	g_free(path);
	g_dir_close(dir);

finish:
	g_free(pathstub);
	g_free(extension);
	if (!list)
	{
		/*dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);*/
		return NULL;
	}
	vector = g_strsplit(list,",",0);
	g_free(list);
	return (vector);
}


/*!
 \brief get_dirs() returns a list of dirs located at the pathstub passed
 this function will first search starting from ~/.MegaTunix+pathstub and
 then in the system path of $PREFIX/share/MegaTunix/+pathstub, it'll return
 the list as a vector char array. (free with g_strfreev)
 \param pathstub (gchar *) partial path to search for files
 \param extension (gchar *) extension to search for 
 \param class (FileClass) enumeration pointer to array of classes for each
 file found
 \returns vector char array of filenames or NULL if none found
 */
gchar ** get_dirs(gchar *input, GArray **classes)
{
	gchar *pathstub = NULL;
	gchar *path = NULL;
	gchar *dirpath = NULL;
	gchar *parent = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	FileClass tmp = PERSONAL;
	GDir *dir = NULL;


	if (!g_str_has_suffix(input, PSEP))
		pathstub = g_strconcat(input,PSEP,NULL);
	else
		pathstub = g_strdup(input);
	g_free(input);

	/* Personal files first */
	path = g_build_filename(get_home(), ".MegaTunix",pathstub,NULL);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		g_free(path);
		goto syspath;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		dirpath = g_build_filename(path,filename,NULL);
		if (!g_file_test(dirpath,G_FILE_TEST_IS_DIR))
		{
			filename = (gchar *)g_dir_read_name(dir);
			g_free(dirpath);
			continue;
		}
		g_free(dirpath);

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
		{
			tmpbuf = g_strconcat(list,",",path,filename,NULL);
			g_free(list);
			list = tmpbuf;
		}
		tmp = PERSONAL;
		if (!*classes)
		{
			*classes = g_array_new(FALSE,TRUE,sizeof(FileClass));
			g_array_append_val(*classes,tmp);
		}
		else
			g_array_append_val(*classes,tmp);


		filename = (gchar *)g_dir_read_name(dir);

	}
	g_free(path);
	g_dir_close(dir);

syspath:
#ifdef __WIN32__
	parent = g_build_path(PSEP,get_home(),"dist",NULL);
#else
	parent = g_strdup(DATA_DIR);
#endif
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
		dirpath = g_build_filename(path,filename,NULL);
		if (!g_file_test(dirpath,G_FILE_TEST_IS_DIR))
		{
			filename = (gchar *)g_dir_read_name(dir);
			g_free(dirpath);
			continue;
		}
		g_free(dirpath);

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup_printf("%s%s",path,filename);
		else
		{
			tmpbuf = g_strconcat(list,",",path,filename,NULL);
			g_free(list);
			list = tmpbuf;
		}
		tmp = SYSTEM;
		if (!*classes)
		{
			*classes = g_array_new(FALSE,TRUE,sizeof(FileClass));
			g_array_append_val(*classes,tmp);
		}
		else
			g_array_append_val(*classes,tmp);


		filename = (gchar *)g_dir_read_name(dir);
	}
	g_free(path);
	g_dir_close(dir);

finish:
	g_free(pathstub);
	if (!list)
	{
		/*dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);*/
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

	filename = g_build_filename(get_home(), ".MegaTunix",file,NULL);
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
#ifdef __WIN32__
		dir = g_build_path(PSEP,get_home(),"dist",NULL);
#else
		dir = g_strdup(DATA_DIR);
#endif
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
	gchar * defdir = NULL;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	gboolean res = FALSE;
	guint i = 0;

	if (!GTK_IS_WINDOW(data->parent))
		data->parent = NULL;
	/*
	   printf("choose_file\n");
	   printf("filter %s\n",data->filter);
	   printf("filename %s\n",data->filename);
	   printf("default_path %s\n",data->default_path);
	   printf("title %s\n",data->title);
	   */
	if (!data->title)
		data->title = g_strdup("Open File");

	if (data->action == GTK_FILE_CHOOSER_ACTION_OPEN)
	{
		dialog = gtk_file_chooser_dialog_new(data->title,
				GTK_WINDOW(data->parent),
				data->action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
		if ((data->on_top) && (GTK_IS_WIDGET(data->parent)))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(data->parent));

		if ((data->absolute_path) && (!data->default_path))
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),data->absolute_path);
		else if (data->default_path)
		{
#ifdef __WIN32__	/* Disallows modifying distributed files */
			path = g_build_path(PSEP,get_home(),"dist",data->default_path,NULL);
#else
			path = g_build_path(PSEP,DATA_DIR,data->default_path,NULL);
#endif
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),path);
			g_free(path);
		}
	}
	else if (data->action == GTK_FILE_CHOOSER_ACTION_SAVE)
	{

		dialog = gtk_file_chooser_dialog_new(data->title,
				GTK_WINDOW(data->parent),
				data->action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);	

		if ((data->on_top) && (GTK_IS_WIDGET(data->parent)))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(data->parent));

		defdir = g_build_path(PSEP,get_home(), ".MegaTunix",data->default_path, NULL);
		if (!g_file_test(defdir,G_FILE_TEST_IS_DIR))
			g_mkdir(defdir,0755);
		/* If filename passed check/adj path */
		if (data->filename)
		{
#ifdef __WIN32__	/* Disallows modifying distributed files */
			if (g_strrstr(data->filename,"dist") != NULL)
#else
				if (g_strrstr(data->filename,DATA_DIR) != NULL)
#endif
				{
					vector = g_strsplit(data->filename,PSEP,-1);
					tmpbuf = g_strconcat(defdir,PSEP,vector[g_strv_length(vector)-1],NULL);
					gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),defdir);
					gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),vector[g_strv_length(vector)-1]);
					g_strfreev(vector);
					g_free(tmpbuf);
				}
				else
					gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),data->filename);
		}
		else
		{
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),defdir);
			gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), data->default_filename );
		}
		g_free(defdir);

	}
	else
		return NULL;
	/* Add shortcut folders... */
	if (data->shortcut_folders)
	{

		vector = g_strsplit(data->shortcut_folders,",",-1);
		for (i=0;i<g_strv_length(vector);i++)
		{
			/* For differences in system path between win32/unix */
#ifdef __WIN32__
			path = g_build_path(PSEP,get_home(),"dist",vector[i],NULL);
#else
			path = g_build_path(PSEP,DATA_DIR,vector[i],NULL);
#endif
			gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),path,NULL);
			g_free(path);
			path = g_build_path(PSEP,get_home(),".MegaTunix",vector[i],NULL);
			gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),path,NULL);
			g_free(path);
		}
		g_strfreev(vector);
	}
	/* If default path switch to that place */
	if ((data->external_path) && (!(data->default_path)))
	{
		path = g_build_path(PSEP,get_home(),data->external_path,NULL);
		if (!g_file_test(path,G_FILE_TEST_IS_DIR))
			g_mkdir(path,0755);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),path);
		g_free(path);

	}

	/* If filters, assign them */
	if (data->filter)
	{
		vector = g_strsplit(data->filter,",",-1);
		if (g_strv_length(vector)%2 > 0)
			goto afterfilter;
		for (i=0;i<g_strv_length(vector);i+=2)
		{
			filter = gtk_file_filter_new();
			gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),vector[i]);
			gtk_file_filter_set_name(GTK_FILE_FILTER(filter),vector[i+1]);
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
		}
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog),filter);
		g_strfreev(vector);
	}
afterfilter:
	/* Turn on overwriteconfirmation */
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	g_signal_connect(G_OBJECT(dialog),"confirm-overwrite",
			G_CALLBACK (confirm_overwrite_callback), NULL);
	if (data->action == GTK_FILE_CHOOSER_ACTION_OPEN)
		if (data->default_filename)
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(dialog),data->default_filename);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		tmpbuf = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if ((data->action == GTK_FILE_CHOOSER_ACTION_SAVE) && (data->default_extension))
		{
			if(!g_str_has_suffix (tmpbuf,data->default_extension))
				filename = g_strjoin(".",tmpbuf,data->default_extension,NULL);
			else
				filename = g_strdup(tmpbuf);
		}
		else
			filename = g_strdup(tmpbuf);
	}
	gtk_widget_destroy (dialog);

	return (filename);
}

#if GTK_MINOR_VERSION >= 8
GtkFileChooserConfirmation
confirm_overwrite_callback (GtkFileChooser *chooser, gpointer data)
{
	GtkWidget *dialog = NULL;
	char *filename;
	gint result = -1;
	gchar *msg = NULL;
	gint handle = -1;

	filename = gtk_file_chooser_get_filename (chooser);

	/* If read only select again */
	if ((handle = g_open(filename,O_RDWR,O_CREAT|O_APPEND)) == -1)
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
		close(handle);
		dialog = gtk_message_dialog_new(NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"File %s exists!\n Do you wish to overwrite it?",filename);
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
		{
			gtk_widget_destroy(dialog);
			result = g_remove(filename);
			if (result == -1)
			{
				msg = g_strdup_printf("Removal of \"%s\" FAILED!!!",filename);
				getfiles_errmsg(msg);
				g_free(msg);
			}
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
#endif


void free_mtxfileio(MtxFileIO *data)
{
	if (!data)
		return;
	if (data->title)
		g_free(data->title);
	if (data->filename)
		g_free(data->filename);
	if (data->external_path)
		g_free(data->external_path);
	if (data->absolute_path)
		g_free(data->absolute_path);
	if (data->default_path)
		g_free(data->default_path);
	if (data->filter)
		g_free(data->filter);
	if (data->shortcut_folders)
		g_free(data->shortcut_folders);
	if (data->default_filename)
		g_free(data->default_filename);
	if (data->default_extension)
		g_free(data->default_extension);
	return;
}


void getfiles_errmsg(const gchar * text)
{
	GtkWidget *dialog = NULL;

	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			__FILE__":\n -- Error: %s\n",
			text);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

gchar * get_home()
{
#ifdef __WIN32__
	gchar *fullpath = NULL;
	gchar *dir = NULL;
	fullpath = g_find_program_in_path("megatunix.exe");
	if (fullpath)
	{
		dir = g_path_get_dirname(fullpath);
		g_free(fullpath);
		return(dir);
	}
	else
		return (g_get_current_dir());
#else
	return((gchar *)g_get_home_dir());
#endif
}

gboolean check_for_files(const gchar * path, const gchar *ext)
{
	GDir * dir = NULL;
	const gchar * file = NULL;

	dir=g_dir_open(path,0,NULL);
	if (!dir)
		return FALSE;
	while ((file = g_dir_read_name(dir)))
	{
		if (g_str_has_suffix(file,ext))
		{
			g_dir_close(dir);
			return TRUE;
		}
	}
	g_dir_close(dir);
	return FALSE;
}

