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
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"



void leave(GtkWidget *widget, gpointer *data)
{
        save_config();
        raw_reader_running = 0;	/*causes realtime var reader thread to die */
	while (raw_reader_stopped == 0)
		usleep(10000);	/*wait for thread to die cleanly*/

        /* Free all buffers */
	close_serial();
        mem_dealloc();
        gtk_main_quit();
}

