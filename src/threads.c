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
#include <notifications.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <string.h>
#include <structures.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>


GThread *raw_input_thread;			/* thread handle */
gboolean raw_reader_running;			/* flag for thread */
extern gboolean connected;			/* valid connection with MS */
extern GtkWidget * comms_view;
extern struct DynamicMisc misc;
extern struct Serial_Params *serial_params;

void start_serial_thread()
{
	int retcode = 0;
	gchar *tmpbuf;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		no_ms_connection();
		g_static_mutex_unlock(&mutex);
		return;
	}

	if (!serial_params->open)
	{
		tmpbuf = g_strdup_printf("Serial Port Not Open, Can NOT Start Thread in This State\n");
		/* Serial not opened, can't start thread in this state */
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		g_static_mutex_unlock(&mutex);
		return;	
	}
	if (raw_reader_running)
	{
		tmpbuf = g_strdup_printf("Serial Reader Thread ALREADY Running\n");
		/* Thread already running, can't run more than 1 */
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		g_static_mutex_unlock(&mutex);
		return;
	}
	else
	{
		raw_input_thread = g_thread_create(raw_reader_thread,\
				NULL, /*Thread args */
				TRUE, // Joinable
				NULL /*GERROR ptr */);
	}
	if (retcode == 0)
	{
		/* SUCCESS */
		tmpbuf = g_strdup_printf("Successfull Start of Realtime Reader Thread\n");
		/* Thread started successfully */
		update_logbar(comms_view,NULL,tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
	}
	else
	{
		/* FAILURE */
		tmpbuf = g_strdup_printf("FAILURE Attempting To Start Realtime Reader Thread\n");
		/* Thread failed to start */
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
	}
	g_static_mutex_unlock(&mutex);
	return;
}

int stop_serial_thread()
{
	gchar *tmpbuf;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock(&mutex);
	if (!raw_reader_running)
	{
		tmpbuf = g_strdup_printf("Realtime Reader Thread ALREADY Stopped\n");
		/* Thread not running, can't stop what hasn't started yet*/
		update_logbar(comms_view,"warning",tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
		g_static_mutex_unlock(&mutex);
		return 0;	/* its already stopped */
	}
	else
	{
		/* What this does is simple:  Set the state to "FALSE"
		 * This causes the thread to jumpout of it's endless loop
		 * Call g_thread_join to wait until it COMPLETELY EXITS
		 * g_thread_join will release when the thread evaporates
		 */
		//printf("attempting to stop thread\n");
		raw_reader_running = FALSE; /* should cause thread to die */
		g_thread_join(raw_input_thread); /*wait for it to evaporate */
		//printf("thread stopped\n");

		tmpbuf = g_strdup_printf("Realtime Reader Thread Stopped Normally\n");
		/* Thread stopped normally */
		update_logbar(comms_view,NULL,tmpbuf,TRUE,FALSE);
		g_free(tmpbuf);
	}
	tcflush(serial_params->fd, TCIOFLUSH);

	g_static_mutex_unlock(&mutex);

	return 0;
}
		
void *raw_reader_thread(void *params)
{
	struct pollfd ufds;
	int res = 0;

	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;

	raw_reader_running = TRUE; /* make sure it starts */

	//printf("thread staring\n");
	while(raw_reader_running == TRUE) 
	{
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
				run_datalog();
			}
			else
				printf("handle_ms_data reported a fault\n");
		}

		gtk_widget_set_sensitive(misc.status[0],
				connected);
		gtk_widget_set_sensitive(misc.ww_status[0],
				connected);

		usleep(serial_params->read_wait * 900); /* Sleep */

	}
	/* if we get here, the thread got killed, mark it as "stopped" */
	raw_reader_running = FALSE;
	return 0;
}
