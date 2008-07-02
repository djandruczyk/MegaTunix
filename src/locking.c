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
#include <defines.h>
#include <locking.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __WIN32__
 #include <io.h>
 #include <windows.h>
#else
 #include <glib.h>
 #include <glib/gstdio.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
#endif


extern GObject *global_data;


void create_mtx_lock()
{

#ifdef __WIN32__
	win32_create_mtx_lock();
#else
	unix_create_mtx_lock();
#endif
}

void unix_create_mtx_lock()
{
#ifndef __WIN32__
	GtkWidget *dialog = NULL;
	gint lock = 0;
	gint tmpfd = 0;
	gchar * lockfile = NULL;
	struct flock lock_struct;
	lockfile = g_build_filename(g_get_tmp_dir(), ".MTXlock",NULL);
	tmpfd = g_open(lockfile,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);

	lock_struct.l_type=F_WRLCK;
	lock_struct.l_start=0;
	lock_struct.l_len=0;
	lock_struct.l_whence=SEEK_CUR;
	lock = fcntl(tmpfd,F_SETLK,&lock_struct);
	if (lock == -1)
	{
		/*
		if (tmpfd == -1)
		{
			printf ("ERROR!! PERMISSIONS issue while trying to create lockfile \"%s\"\n",lockfile);
			dialog = gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"There is a PERMISSIONS problem\nMegaTunix can't create the lockfile:\n\"%s\"",lockfile);
		}
		else
		*/
		{
			printf ("ERROR!! Multiple MegaTunix instances are not allowed!\n");
			dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"<b>MegaTunix</b> is already Running!\nMultiple instances are <b><u>NOT</u></b> allowed!\n");
		}
		g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"delete_event", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"destroy_event", G_CALLBACK(gtk_main_quit), dialog);
		gtk_widget_show_all(dialog);
		gtk_main();
		if (global_data)
			g_object_unref(global_data);
		exit(-1);
	}

#endif
}


void win32_create_mtx_lock(void)
{
#ifdef __WIN32__
	HANDLE fd;
	GtkWidget *dialog = NULL;
	gchar * file;

	file = g_build_filename(g_get_tmp_dir(), ".MTXlock",NULL);
	fd = CreateFile(file,(GENERIC_READ | GENERIC_WRITE),0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (fd == INVALID_HANDLE_VALUE)
	{
		dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"<b>MegaTunix</b> is already Running!\nMultiple instances are <b><u>NOT</u></b> allowed!\n");
		g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"delete_event", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"destroy_event", G_CALLBACK(gtk_main_quit), dialog);
		gtk_widget_show_all(dialog);
		gtk_main();
		if (global_data)
			g_object_unref(global_data);
		exit(-1);
	}

	return;
#endif
}

