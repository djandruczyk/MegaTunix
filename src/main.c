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
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <defines.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>


gint def_comm_port;
gboolean ready = FALSE;

int main(int argc, char ** argv)
{
	gint cfg_result;

	g_thread_init(NULL);
	gdk_threads_init();

	gtk_init(&argc, &argv);

	gtk_set_locale();

//	if (file_exists("MegaTunixrc"))
//		gtk_rc_add_default_file ("MegaTunixrc");

	init();			/* initialize global vars */
	mem_alloc();
	make_megasquirt_dirs();	/*Create config file dirs if missing */

	cfg_result = read_config();
	setup_gui();		

	if (cfg_result < 0)
		open_serial(def_comm_port);
	else	
		open_serial(serial_params.comm_port);

	setup_serial_params();	/* Setup the serial port for I/O */
	read_conversions("std_conversions");
	

	ready = TRUE;
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}

gboolean file_exists (const char *filename)
{
  struct stat statbuf;

  return stat (filename, &statbuf) == 0;
}

