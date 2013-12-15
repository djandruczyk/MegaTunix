/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file mtxcommon/getfiles.c
  \ingroup MtxCommon
  \brief Helper routines for getting files/lists of files in personal/system
  wide directories
  \author David Andruczyk
  */

#include <debugging.h>
#include <defines.h>
#include <fcntl.h>
#include <getfiles.h>
#include <glib/gstdio.h>
#include <unistd.h>

#ifdef DEBUG
 #undef ENTER
 #undef EXIT
 #define ENTER() ""
 #define EXIT() ""
#endif

static void remove_filter(gpointer, gpointer);

/*!
 \brief get_files() returns a list of files located at the pathstub passed
 this function will first search starting from ~/mtx/<PROJECT>/+pathstub and
 then in the system path of $PREFIX/share/MegaTunix/+pathstub, it'll return
 the list as a vector char array. (free with g_strfreev)
 \param input is the partial path to search for files
 \param extension is the extension to search for 
 \param classes is a pointer to an Array of classes to be filled based on the
 search so we know which paths are system wide and which are personal
 \returns vector char array of filenames or NULL if none found
 */
gchar ** get_files(const gchar *prj, const gchar *pathstub, const gchar * extension, GArray **classes)
{
	gchar *path = NULL;
	gchar *parent = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	FileClass tmp = PERSONAL;
	GDir *dir = NULL;
	const gchar *project = NULL;

	ENTER();
	if (prj)
		project = prj;
	else
		project = DEFAULT_PROJECT;
	
	path = g_build_filename(HOME(),"mtx",project,pathstub,NULL);
	/*printf("get_files, personal path is %s\n",path);*/
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
			list = g_build_filename(path,filename,NULL);
		else
		{
			tmpbuf = g_strconcat(list,",",path,PSEP,filename,NULL);
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
	parent = g_strdup(MTXSYSDATA);
	path = g_build_filename(parent,pathstub,NULL);
	g_free(parent);
	/*printf("get_files, syspath is %s\n",path);*/
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
			list = g_build_filename(path,filename,NULL);
		else
		{
			tmpbuf = g_strconcat(list,",",path,PSEP,filename,NULL);
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
	if (!list)
	{
		/*dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);*/
		EXIT();
		return NULL;
	}
	vector = g_strsplit(list,",",0);
	g_free(list);
	EXIT();
	return (vector);
}


/*!
 \brief get_dirs() returns a list of dirs located at the pathstub passed
 this function will first search starting from ~/mtx/<PROJECT>+pathstub and
 then in the system path of $PREFIX/share/MegaTunix/+pathstub, it'll return
 the list as a vector char array. (free with g_strfreev)
 \param input is the partial path to search for files
 \param classes is a pointer to an Array of classes to be filled based on the
 search so we know which paths are system wide and which are personal
 \returns vector char array of dir names or NULL if none found
 */
gchar ** get_dirs(const gchar *prj, const gchar *pathstub, GArray **classes)
{
	gchar *path = NULL;
	gchar *dirpath = NULL;
	gchar *parent = NULL;
	gchar *list = NULL;
	gchar * filename = NULL;
	gchar **vector = NULL;
	gchar * tmpbuf = NULL;
	FileClass tmp = PERSONAL;
	GDir *dir = NULL;
	const gchar *project = NULL;

	ENTER();
	if (prj)
		project = prj;
	else
		project = DEFAULT_PROJECT;
	/* Personal files first */
	path = g_build_filename(HOME(),"mtx",project,pathstub,NULL);
	/*printf("get_dirs, personal path is %s\n",path);*/
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

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup(dirpath);
		else
		{
			tmpbuf = g_strconcat(list,",",dirpath,NULL);
			g_free(list);
			list = tmpbuf;
		}
		g_free(dirpath);
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
	parent = g_strdup(MTXSYSDATA);
	path = g_build_filename(parent,pathstub,NULL);
	/*printf("get_dirs, system path is %s\n",path);*/
	g_free(parent);
	dir = g_dir_open(path,0,NULL);
	if (!dir)
	{
		/*printf("unable to open path %s\n",path);*/
		g_free(path);
		goto finish;
	}
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		/*printf("Checking dir %s\n",filename);*/
		dirpath = g_build_filename(path,filename,NULL);
		if (!g_file_test(dirpath,G_FILE_TEST_IS_DIR))
		{
			/*printf("Not a dir...\n");*/
			filename = (gchar *)g_dir_read_name(dir);
			g_free(dirpath);
			continue;
		}

		/* Create name of file and store temporarily */
		if (!list)
			list = g_strdup(dirpath);
		else
		{
			tmpbuf = g_strconcat(list,",",dirpath,NULL);
			g_free(list);
			list = tmpbuf;
		}
		g_free(dirpath);
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
	if (!list)
	{
		/*dbg_func(g_strdup(__FILE__": get_files()\n\t File list was NULL\n"),CRITICAL);*/
		/*printf("List was EMPTY!\n");*/
		EXIT();
		return NULL;
	}
	vector = g_strsplit(list,",",0);
	/*printf("Returning list of %i dirs\n",g_strv_length(vector));*/
	g_free(list);
	EXIT();
	return (vector);
}


/*!
 \brief get_file() gets a single file defnied by pathstub, first searching in
 ~/mtx/<PROJECT>+pathstub, and then in $PREFIX/share/MegaTunix/+pathstub,
 \param pathstub is the partial path to filename
 \param extension is the extension wanted..
 \returns filename if found or NULL if not found
 */
gchar * get_file(const gchar *prj, const gchar *pathstub, const gchar *extension)
{
	gchar *filename = NULL;
	gchar *file = NULL;
	const gchar *project = NULL;
	ENTER();
	if (extension)
		file = g_strconcat(pathstub,".",extension,NULL);
	else
		file = g_strdup(pathstub);

	if (prj)
		project = prj;
	else
		project = DEFAULT_PROJECT;

	filename = g_build_filename(HOME(),"mtx",project,file,NULL);
	/*printf("get_file, personal filename is %s\n",filename);*/
	if (g_file_test(filename,(GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
	{
		g_free(file);
		EXIT();
		return filename;
	}
	else 
	{
		g_free(filename);
		filename = g_build_filename(MTXSYSDATA,file,NULL);
		/*printf("get_file, system filename is %s\n",filename);*/

		g_free(file);

		if (g_file_test(filename,(GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)))
		{
			EXIT();
			return filename;
		}
		else
			g_free(filename);
	}
	EXIT();
	return NULL;
}


/*!
  \brief pops up a Filechooser dialog to select a file
  \param data is a pointer to a MtxFileIO structure which contains important
  bits like the path to start, the filter, default filename and so on
  \see MtxFileIO
  \returns the path to the file or NULL if cancelled
  */
gchar * choose_file(MtxFileIO *data)
{
	GtkWidget *dialog = NULL;
	GtkFileFilter *filter = NULL;
	GSList *filters = NULL;
	gchar *path = NULL;
	gchar *defdir = NULL;
	gchar *filename = NULL;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	const gchar *project = NULL;
	gint response = 0;
	gboolean res = FALSE;
	guint i = 0;

	ENTER();
	if (!GTK_IS_WINDOW(data->parent))
		data->parent = NULL;
	printf("choose_file\n");
	printf("parent %p\n",(void *)data->parent);
	printf("on_top %s\n",data->on_top? "TRUE":"FALSE");
	printf("filter %s\n",data->filter);
	printf("filename %s\n",data->filename);
	printf("default_filename %s\n",data->default_filename);
	printf("default_extension %s\n",data->default_extension);
	printf("absolute_path %s\n",data->absolute_path);
	printf("default_path %s\n",data->default_path);
	printf("external_path %s\n",data->external_path);
	printf("shortcut_folders %s\n",data->shortcut_folders);
	printf("project %s\n",data->project);
	printf("title %s\n",data->title);
	if (data->project)
		project = data->project;
	else
		project = DEFAULT_PROJECT;

	if (!data->title)
		data->title = g_strdup("Open File");

	if ((data->action == GTK_FILE_CHOOSER_ACTION_OPEN) || 
			(data->action == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER))
	{
		printf("ACTION_OPEN before gtk_file_chooser_dialog_new\n");
		dialog = gtk_file_chooser_dialog_new(data->title,
				/*GTK_WINDOW(data->parent), */
				0,
				data->action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
		printf("after gtk_file_chooser_dialog_new\n");
		if ((data->on_top) && (GTK_IS_WIDGET(data->parent)))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(data->parent));

		if ((data->absolute_path) && (!data->default_path))
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),data->absolute_path);
		else if (data->default_path)
		{
			printf("should be using system path + %s\n",data->default_path);
			path = g_build_filename(MTXSYSDATA,data->default_path,NULL);
			if (!g_file_test(path,G_FILE_TEST_IS_DIR))
			{
				g_free(path);
				path = g_build_filename(HOME(),"mtx",data->project,data->default_path, NULL);
				printf("System path is not found, falling back to user path %s\n",path);
			}
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),path);
			g_free(path);
		}
	}
	else if (data->action == GTK_FILE_CHOOSER_ACTION_SAVE)
	{
		printf("ACTION_SAVE calling gtk_file_chooser_dialog_new\n");
		dialog = gtk_file_chooser_dialog_new(data->title,
				/*GTK_WINDOW(data->parent), */
				0,
				data->action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);	
		printf("after gtk_file_chooser_dialog_new\n");

		/*		if ((data->on_top) && (GTK_IS_WIDGET(data->parent)))
					gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(data->parent));
		*/

		if (data->default_path)
		{
			defdir = g_build_filename(HOME(),"mtx",data->project,data->default_path, NULL);
			if (!g_file_test(defdir,G_FILE_TEST_IS_DIR))
				g_mkdir(defdir,0755);
		}
		else
			defdir = g_build_filename(HOME(),"mtx",data->project, NULL);
		/* If filename passed check/adj path  */
		if (data->filename)
		{
			if (g_strrstr(data->filename,DATA_DIR) != NULL)
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
	{
		EXIT();
		return NULL;
	}
	/* Add shortcut folders...  */
	if (data->shortcut_folders)
	{
		vector = g_strsplit(data->shortcut_folders,",",-1);
		for (i=0;i<g_strv_length(vector);i++)
		{
			path = g_build_filename(MTXSYSDATA,vector[i],NULL);
			gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),path,NULL);
			g_free(path);
			path = g_build_filename(HOME(),"mtx",data->project,vector[i],NULL);
			gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),path,NULL);
			g_free(path);
		}
		g_strfreev(vector);
	}
	
	/* If default path switch to that place  */
	if ((data->external_path) && (!(data->default_path)))
	{
		printf("external path with no default path\n");
		path = g_build_filename(HOME(),"mtx",data->project,data->external_path,NULL);
		if (!g_file_test(path,G_FILE_TEST_IS_DIR))
			g_mkdir(path,0755);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),path);
		g_free(path);

	}

	/* If filters, assign them 
	  CAUSES SEGFAULTS???
	  */
	/*
	if (data->filter)
	{
		printf("data->filter is set to \"%s\"\n",data->filter);
		vector = g_strsplit(data->filter,",",-1);
		if (g_strv_length(vector)%2 > 0)
			goto afterfilter;
		for (i=0;i<g_strv_length(vector);i+=2)
		{
			filter = gtk_file_filter_new();
			gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),vector[i]);
			printf("Filter glob is set to \"%s\"\n",vector[i]);
			gtk_file_filter_set_name(GTK_FILE_FILTER(filter),vector[i+1]);
			printf("Filter name is set to \"%s\"\n",vector[i+1]);
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
		}
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog),filter);
		g_strfreev(vector);
	}
	*/
afterfilter:
	/* Turn on overwriteconfirmation  */
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	g_signal_connect(G_OBJECT(dialog),"confirm-overwrite",
			G_CALLBACK (confirm_overwrite_callback), NULL);
	if (data->action == GTK_FILE_CHOOSER_ACTION_OPEN)
	{
		if (data->default_filename)
		{
			printf("data->default_filename is set to \"%s\"\n",data->default_filename);
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(dialog),data->default_filename);
		}
	}

	printf("initiating dialog to run\n");
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	printf("it returned \n");
	if (response == GTK_RESPONSE_ACCEPT)
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
		g_free(tmpbuf);
	}
	filters = gtk_file_chooser_list_filters(GTK_FILE_CHOOSER(dialog));
	if (filters)
	{
		g_slist_foreach(filters, remove_filter ,dialog);
		g_slist_free(filters);
	}

	gtk_widget_destroy (dialog);
	EXIT();
	return (filename);
}

#if GTK_MINOR_VERSION >= 8
/*!
  \brief The pops up the overwrite confirmation dialog
  \param chooser is a pointer to the active filechooser dialog
  \param data is unused
  \returns a GtkFileChooserConfirmation enumeration
  */
GtkFileChooserConfirmation
confirm_overwrite_callback (GtkFileChooser *chooser, gpointer UNUSED(data))
{
	GtkWidget *dialog = NULL;
	char *filename;
	gint result = -1;
	gchar *msg = NULL;
	gint handle = -1;

	ENTER();
	filename = gtk_file_chooser_get_filename (chooser);

	/* If read only select again */
	if ((handle = g_open(filename,O_RDWR,O_CREAT|O_APPEND)) == -1)
	{
		dialog = gtk_message_dialog_new(NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				"File %s\nis READ ONLY, Please choose another file...",filename);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		EXIT();
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
				"File %s already exists!\n Do you wish to overwrite it?",filename);
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
			EXIT();
			return GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
		}
		else
		{
			gtk_widget_destroy(dialog);
			EXIT();
			return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
		}
	}
	EXIT();
	return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
}
#endif


/*!
  \brief frees the resources of a MtxFileIO structure
  \param data is a pointer to a MtxFileIO structure to be deallocated
  \see MtxFileIO
  */
void free_mtxfileio(MtxFileIO *data)
{
	ENTER();
	if (!data)
	{
		EXIT();
		return;
	}
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
	g_free(data);
	EXIT();
	return;
}


/*!
  \brief pops up an error dialog with the passed text
  \param text is the text to display to the user
  */
void getfiles_errmsg(const gchar * text)
{
	GtkWidget *dialog = NULL;

	ENTER();
	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			__FILE__":\n -- Error: %s\n",
			text);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	EXIT();
	return;
}


/*!
  \brief checks for the presence of files in a specific path
  \param path is the path to look for files
  \param ext is the extentions of the files to look for
  \returns TRUE if files found, FALSE otherwise
  */
gboolean check_for_files(const gchar * path, const gchar *ext)
{
	GDir * dir = NULL;
	const gchar * file = NULL;

	ENTER();
	dir=g_dir_open(path,0,NULL);
	if (!dir)
	{
		EXIT();
		return FALSE;
	}
	while ((file = g_dir_read_name(dir)))
	{
		if (g_str_has_suffix(file,ext))
		{
			g_dir_close(dir);
			EXIT();
			return TRUE;
		}
	}
	g_dir_close(dir);
	EXIT();
	return FALSE;
}


static void remove_filter(gpointer filter, gpointer dialog)
{
	printf("Removing file filter!\n");
	gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(dialog),GTK_FILE_FILTER(filter));
}
