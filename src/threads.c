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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/poll.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>
#include <runtime_gui.h>


pthread_t raw_input_thread;			/* thread handle */
gboolean raw_reader_running;			/* flag for thread */
gboolean raw_reader_stopped;			/* flag for thread */
extern gboolean connected;			/* valid connection with MS */
extern gint ser_context_id;			/* Statusbar related */
extern struct v1_2_Runtime_Gui runtime_data;
extern GtkWidget *ser_statbar;			/* Statusbar */
char buff[60];

void start_serial_thread()
{
	int retcode = 0;
	if (!connected)
	{
		no_ms_connection();
		return;
	}

	if (!serial_params.open)
	{
		g_snprintf(buff,60,"Serial Port Not Open, Can NOT Start Thread in This State");
		/* Serial not opened, can't start thread in this state */
		update_statusbar(ser_statbar,ser_context_id,buff);
		return;	
	}
	if (raw_reader_running)
	{
		g_snprintf(buff,60,"Serial Reader Thread ALREADY Running");
		/* Thread already running, can't run more than 1 */
		update_statusbar(ser_statbar,ser_context_id,buff);
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
		g_snprintf(buff,60,"Successfull Start of Realtime Reader Thread");
		/* Thread started successfully */
		update_statusbar(ser_statbar,ser_context_id,buff);
	}
	else
	{
		/* FAILURE */
		g_snprintf(buff,60,"FAILURE Attempting To Start Realtime Reader Thread");
		/* Thread failed to start */
		update_statusbar(ser_statbar,ser_context_id,buff);
	}
	return;
}

int stop_serial_thread()
{
	static gboolean locked;
	if (locked == TRUE)
		return 0;
	else
		locked = TRUE;
	if (raw_reader_stopped)
	{
		g_snprintf(buff,60,"Realtime Reader Thread ALREADY Stopped");
		/* Thread not running, can't stop what hasn't started yet*/
		update_statusbar(ser_statbar,ser_context_id,buff);
		raw_reader_running = FALSE;
		raw_reader_stopped = TRUE;
		locked = FALSE;
		return 0;	/* its already stopped */
	}
	else
	{
		pthread_cancel(raw_input_thread);
		while (raw_reader_stopped == FALSE)
			usleep(100);
		g_snprintf(buff,60,"Realtime Reader Thread Stopped Normally");
		/* Thread stopped normally */
		update_statusbar(ser_statbar,ser_context_id,buff);
	}
	locked = FALSE;
	return 0;
}
		
void *reset_reader_locks(void * arg)
{
	raw_reader_running = FALSE;
	raw_reader_stopped = TRUE;
	return 0;
}

void *raw_reader_thread(void *params)
{
	struct pollfd ufds;
	int res = 0;
	void * arg = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(reset_reader_locks, (void *)arg);

	raw_reader_running = TRUE; /* make sure it starts */
	raw_reader_stopped = FALSE;	/* set opposite flag */

	while(raw_reader_running == TRUE) 
	{
		pthread_testcancel();
		ufds.fd = serial_params.fd;
		ufds.events = POLLIN;
		res = write(serial_params.fd,"A",1);
		res = poll (&ufds,1,serial_params.poll_timeout);
		if (res == 0)
		{
			serial_params.errcount++;
			connected = FALSE;
			gtk_widget_set_sensitive(runtime_data.status[0],
					connected);
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
		update_errcounts();
		gdk_threads_leave();

		pthread_testcancel();
		usleep(serial_params.read_wait * 1000); /* Sleep */

	}
	/* if we get here, the thread got killed, mark it as "stopped" */
	raw_reader_stopped = TRUE;
	raw_reader_running = FALSE;
	pthread_cleanup_pop(0);
	return 0;
}

