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

#include <comms_gui.h>
#include <config.h>
#include <dataio.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <enums.h>
#include <globals.h>
#include <notifications.h>
#include <pthread.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>


pthread_t raw_input_thread;			/* thread handle */
extern gboolean dualtable;
gboolean raw_reader_running;			/* flag for thread */
gboolean raw_reader_stopped;			/* flag for thread */
extern gboolean connected;			/* valid connection with MS */
extern GtkWidget * comms_view;
extern struct DynamicMisc misc;
extern struct Serial_Params *serial_params;

void start_serial_thread()
{
	int retcode = 0;
	gchar *tmpbuf;
	if (!connected)
	{
		no_ms_connection();
		return;
	}

	if (!serial_params->open)
	{
		tmpbuf = g_strdup_printf("Serial Port Not Open, Can NOT Start Thread in This State\n");
		/* Serial not opened, can't start thread in this state */
		update_logbar(comms_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
		return;	
	}
	if (raw_reader_running)
	{
		tmpbuf = g_strdup_printf("Serial Reader Thread ALREADY Running\n");
		/* Thread already running, can't run more than 1 */
		update_logbar(comms_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
		return;
	}
	else
	{
		retcode = pthread_create(&raw_input_thread,\
				NULL, /*Thread attributes */
				raw_reader_thread,
				NULL /*thread args */);
	}
	if (retcode == 0)
	{
		/* SUCCESS */
		tmpbuf = g_strdup_printf("Successfull Start of Realtime Reader Thread\n");
		/* Thread started successfully */
		update_logbar(comms_view,NULL,tmpbuf,TRUE);
		g_free(tmpbuf);
	}
	else
	{
		/* FAILURE */
		tmpbuf = g_strdup_printf("FAILURE Attempting To Start Realtime Reader Thread\n");
		/* Thread failed to start */
		update_logbar(comms_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
	}
	return;
}

int stop_serial_thread()
{
	gchar *tmpbuf;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);
	if (raw_reader_stopped)
	{
		tmpbuf = g_strdup_printf("Realtime Reader Thread ALREADY Stopped\n");
		/* Thread not running, can't stop what hasn't started yet*/
		update_logbar(comms_view,"warning",tmpbuf,TRUE);
		g_free(tmpbuf);
		raw_reader_running = FALSE;
		raw_reader_stopped = TRUE;
		g_static_mutex_unlock(&mutex);
		return 0;	/* its already stopped */
	}
	else
	{
		pthread_cancel(raw_input_thread);
		/* this should be recoded as it's WRONG!!!! */
		while (raw_reader_stopped == FALSE)
			usleep(10000);

		tmpbuf = g_strdup_printf("Realtime Reader Thread Stopped Normally\n");
		/* Thread stopped normally */
		update_logbar(comms_view,NULL,tmpbuf,TRUE);
		g_free(tmpbuf);
	}
	usleep(100000);	/* a bug in here somewhere....  deadlocks without it */
	g_static_mutex_unlock(&mutex);

	return 0;
}
		
void reset_reader_locks(void * arg)
{
	/* Sets flags to "happy happy" state to prevent a deadlock.
	 * My implementation of this thread crap is probably wrong, so
	 * DO NOT model this code if you value your career as a 
	 * programmer...  (I'm not a programmer...)
	 */
	raw_reader_running = FALSE;
	raw_reader_stopped = TRUE;
	return ;
}

void *raw_reader_thread(void *params)
{
	struct pollfd ufds;
	int res = 0;

	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(reset_reader_locks, NULL);

	raw_reader_running = TRUE; /* make sure it starts */
	raw_reader_stopped = FALSE;	/* set opposite flag */

	while(raw_reader_running == TRUE) 
	{
		pthread_testcancel();
		if (dualtable)
			set_ms_page(0);
		write(serial_params->fd,"A",1);
		res = poll (&ufds,1,serial_params->poll_timeout);
		if (res == 0)
		{
			serial_params->errcount++;
			connected = FALSE;
		}
		else
		{
			res = handle_ms_data(REALTIME_VARS);
			if(res)
			{
				connected = TRUE;
				update_runtime_vars();
				run_datalog();
			}
			else
				printf("handle_ms_data reported a fault\n");
		}
		gdk_threads_enter();

		gtk_widget_set_sensitive(misc.status[0],
				connected);
		gtk_widget_set_sensitive(misc.ww_status[0],
				connected);

		update_errcounts(NULL,FALSE);
		gdk_threads_leave();

		pthread_testcancel();
		usleep(serial_params->read_wait * 1000); /* Sleep */

	}
	/* if we get here, the thread got killed, mark it as "stopped" */
	raw_reader_stopped = TRUE;
	raw_reader_running = FALSE;
	pthread_cleanup_pop(0);
	return 0;
}
