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


pthread_t raw_input_thread;	/* thread handle */
pthread_t starter;		/* thread handle */
volatile int raw_reader_running;		/* flag for thread */
int raw_reader_stopped;		/* flag for thread */
extern int ser_context_id;
extern GtkWidget *ser_statbar;
char buff[60];


void start_serial_thread()
{
	pthread_create(&starter,
			NULL,
			serial_raw_thread_starter,
			NULL);
	return;
}
void *serial_raw_thread_starter(void *params)
{
	/* Spawns the thread that reads realtime vars data from the MS box */
	int retcode = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	if (serial_params.open == 0)
	{
                g_snprintf(buff,60,"Serial Port Not Open, Can NOT Start Thread in This State");
                /* Serial not opened, can't start thread in this state */
		gdk_threads_enter();
		update_statusbar(ser_statbar,ser_context_id,buff);
		gdk_threads_leave();
		return 0;	
	}
	if (raw_reader_running)
	{
                g_snprintf(buff,60,"Serial Reader Thread ALREADY Running");
                /* Thread already running, can't run more than 1 */
		gdk_threads_enter();
		update_statusbar(ser_statbar,ser_context_id,buff);
		gdk_threads_leave();
		return 0;
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
                g_snprintf(buff,60,"Successful Start of Realtime Reader Thread");
                /* Thread started successfully */
		gdk_threads_enter();
		update_statusbar(ser_statbar,ser_context_id,buff);
		gdk_threads_leave();
	}
	else
	{
		/* FAILURE */
                g_snprintf(buff,60,"FAILURE Attempting To Start Realtime Reader Thread");
                /* Thread failed to start */
		gdk_threads_enter();
		update_statusbar(ser_statbar,ser_context_id,buff);
		gdk_threads_leave();
	}
	return 0;
}

int stop_serial_thread()
{
	gint tmp;
	if (raw_reader_stopped)
	{
                g_snprintf(buff,60,"Realtime Reader Thread ALREADY Stopped");
                /* Thread not running, can't stop what hasn't started yet*/
		update_statusbar(ser_statbar,ser_context_id,buff);
		raw_reader_running = 0;
		raw_reader_stopped = 1;
		return 0;	/* its already stopped */
	}
	else
	{
		raw_reader_running = 0; /* thread will die on next loop */
		tmp = pthread_cancel(raw_input_thread);
                g_snprintf(buff,60,"Realtime Reader Thread Stopped Normally");
                /* Thread stopped normally */
		update_statusbar(ser_statbar,ser_context_id,buff);
		raw_reader_running = 0;
		raw_reader_stopped = 1;
	}
	return 0;
}
		

void *raw_reader_thread(void *params)
{
	struct pollfd ufds;
	int res = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	raw_reader_running = 1; /* make sure it starts */
	raw_reader_stopped = 0;	/* set opposite flag */
	ufds.fd = serial_params.fd;
	ufds.events = POLLIN;
	
	while(raw_reader_running > 0) /* set it to zero jump out, thread will die*/
	{
                res = write(serial_params.fd,"A",1);
                res = poll (&ufds,1,serial_params.poll_timeout);
                if (res == 0)
		{
			serial_params.errcount++;
		}
                else
		{
                        res = handle_ms_data(REALTIME_VARS);
			if(res)
				update_runtime_vars();
			else
				printf("handle_ms_data reported a fault\n");
		}

		gdk_threads_enter();
		update_errcounts();
		gdk_threads_leave();

                usleep(serial_params.read_wait *1000);
	}
	/* if we get here, the thread got killed, mark it as "stopped" */
	raw_reader_stopped = 1;
	raw_reader_running = 0;
	return 0;
}

