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
#include <conversions.h>
#include <core_gui.h>
#include <defines.h>
#include <enums.h>
#include <gui_handlers.h>
#include <init.h>
#include <main.h>
#include <runtime_controls.h>
#include <serialio.h>
#include <structures.h>
#include <threads.h>
#include <timeout_handlers.h>


extern gint temp_units;
extern struct Serial_Params *serial_params;
GThread * serio_thread = NULL;
gboolean ready = FALSE;
gint statuscounts_id = -1;

int main(int argc, char ** argv)
{
	if(!g_thread_supported())
		g_thread_init(NULL);

	gdk_threads_init();

	gtk_init(&argc, &argv);

	gtk_set_locale();

	mem_alloc();		/* Allocate memory for DataStructures */
	init();			/* Initialize global vars */
	make_megasquirt_dirs();	/* Create config file dirs if missing */

	read_config();
	create_default_controls();
	setup_gui();		

	open_serial(serial_params->port_name);
	setup_serial_params();	/* Setup the serial port for I/O */
	/* Startup the serial General I/O handler.... */
	serio_thread = g_thread_create(serial_io_handler,
			NULL, // Thread args
			TRUE, // Joinable
			NULL); //GError Pointer

	// Load runtime controls layout definitions from the main configfile 
	load_controls();

	/* Convert the gui based on temp preference.  This MUST BE DONE
	 * AFTER data has been read once to make sure it's displayed correctly
	 */

	/* Kickoff fast interrogation */
//	gtk_timeout_add(250,(GtkFunction)early_interrogation,NULL);

	/* Startup status counters timeout handler... */
	/* Run it about 15 times/second.. proc use seems negligable... */
	statuscounts_id = gtk_timeout_add(66,(GtkFunction)update_errcounts,NULL);
	ready = TRUE;
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}
