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
#include <sys/poll.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"

int def_comm_port;

int main(int argc, char ** argv)
{

	g_thread_init(NULL);
        gtk_init(&argc, &argv);

	init();			/* initialize global vars */
	make_megasquirt_dirs();	/*Create config file dirs if missing */
	 
	if (read_config() < 0)		/* read config files */
		open_serial(def_comm_port);
	else	
		open_serial(serial_params.comm_port);

	setup_serial_params();	/* Setup the serial port for I/O */
	//serial_raw_thread_starter(); /*kicks off the loop */	

	setup_gui();		/* THIS DOES NOT return until you close
				 * the program!!! */
	gtk_main();
	return (0) ;
}
