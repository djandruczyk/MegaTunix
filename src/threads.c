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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/poll.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"


int serial_raw_thread_starter()
{
	/* Spawns the thread that reads realtime vars data from the MS box */
	int retcode = 0;
	if (raw_reader_running)
		printf("ERROR!, Seral raw data reader is already running\n");
	else
	{
		retcode = pthread_create(&raw_input_thread,\
				NULL, /*Thread attributes */
				raw_reader_thread,
				NULL /*thread args */);
	}
	if (retcode != 0)
		printf("ERROR attmepting to create serial raw reader thread\n");
	return retcode;
}

void * raw_reader_thread(void *params)
{
	struct pollfd ufds;
	int res = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	raw_reader_running = 1; /* make sure it starts */
	ufds.fd = serial_params.fd;
	ufds.events = POLLIN;
	
	while(raw_reader_running > 0) /* set it to zero jump out, thread will die*/
	{
                res = write(serial_params.fd,"A",1);
                res = poll (&ufds,1,serial_params.poll_timeout);
                if (res == 0)
                {
                        printf("I/O with MegaSquirt Timeout\n");
                        serial_params.errcount++;
                }
                else
                        handle_ms_data(REALTIME_VARS);

                usleep(read_wait_time *1000);
	}
	return 0;
}

