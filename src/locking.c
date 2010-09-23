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

#include <args.h>
#include <config.h>
#include <defines.h>
#include <errno.h>
#include <locking.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __WIN32__
 #include <io.h>
 #include <windows.h>
#else
 #include <glib.h>
 #include <glib/gstdio.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <fcntl.h>
#endif


extern gconstpointer *global_data;

void create_mtx_lock()
{
	CmdLineArgs * args = DATA_GET(global_data,"args");
	if (args->network_mode)
		return;
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
	g_free(lockfile);

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
			printf (_("ERROR!! Multiple MegaTunix instances are not allowed UNLESS in network socket mode (see -n option)!\n"));
			dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,_("<b>MegaTunix</b> is already Running!\nMultiple instances are <b><u>NOT</u></b> allowed unless running\nin network socket mode, see the -n option!\n"));
		}
		g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"delete_event", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"destroy_event", G_CALLBACK(gtk_main_quit), dialog);
		gtk_widget_show_all(dialog);
		gtk_main();
		if (global_data)
		{
			g_dataset_destroy(global_data);
			g_free(global_data);
		}
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
			g_datalist_clear(global_data);
		exit(-1);
	}
	return;
#endif
}


void unlock_serial()
{
#ifndef __WIN32__
	gchar *fname = DATA_GET(global_data,"serial_lockfile");

	/*printf("told to unlock serial,  path \"%s\"\n",fname); */
	if (fname)
	{
		if (g_file_test(fname,G_FILE_TEST_IS_REGULAR))
		{
			g_remove(fname);
			DATA_SET(global_data,"serial_lockfile", NULL);
		}
	}
#endif
}


gboolean lock_serial(gchar * name)
{
#ifndef __WIN32__
	gchar *tmpbuf = NULL;
	gchar *lock = NULL;
	gchar **vector = NULL;
	gchar *contents = NULL;
	gboolean res = FALSE;
	GError *err = NULL;
	gint i = 0;
	gint pid = 0;

	/*printf("told to lock serial port %s\n",name); */
	/* If no /proc (i.e. os-X), just fake it and return */
	if (!g_file_test("/proc",G_FILE_TEST_IS_DIR))
		return TRUE;

	tmpbuf = g_strdup_printf("/var/lock/LCK..");
	vector = g_strsplit(name,PSEP,-1);
	for (i=0;i<g_strv_length(vector);i++)
	{
		if ((g_strcasecmp(vector[i],"") == 0) || (g_strcasecmp(vector[i],"dev") == 0))
			continue;
		lock = g_strconcat(tmpbuf,vector[i],NULL);
		g_free(tmpbuf);
	}
	g_strfreev(vector);
	if (g_file_test(lock,G_FILE_TEST_IS_REGULAR))
	{
//		printf("found existing lock!\n");
		if(g_file_get_contents(lock,&contents,NULL,&err))
		{
//			printf("read existing lock\n");
			vector = g_strsplit(g_strchug(contents)," ", -1);
//			printf("lock had %i fields\n",g_strv_length(vector));
			pid = (gint)g_ascii_strtoull(vector[0],NULL,10);
//			printf("pid in lock \"%i\"\n",pid);
			g_free(contents);
			g_strfreev(vector);
			tmpbuf = g_strdup_printf("/proc/%i",pid);
			res = g_file_test(tmpbuf,G_FILE_TEST_IS_DIR);
			g_free(tmpbuf);
			if (res)
			{
//				printf("process active\n");
				return FALSE;
			}
			else
				g_remove(lock);
		}
		
	}
	contents = g_strdup_printf("     %i",getpid());
	res = g_file_set_contents(lock,contents,-1,&err);
	g_free(contents);
	if (res)
	{
		DATA_SET_FULL(global_data,"serial_lockfile",(gpointer)lock,g_free);
		return TRUE;
	}
	else
		printf(_("Error setting serial lock %s\n"),(gchar *)strerror(errno));
#endif
	return TRUE;
}
