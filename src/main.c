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
#include <serialio.h>
#include <structures.h>


extern gboolean fahrenheit;
extern struct Serial_Params *serial_params;
gboolean ready = FALSE;
gint statuscounts_id = -1;

int main(int argc, char ** argv)
{
	gint cfg_result;

	if(!g_thread_supported())
		g_thread_init(NULL);

	gdk_threads_init();

	gtk_init(&argc, &argv);

	gtk_set_locale();

	mem_alloc();		/* Allocate memory for DataStructures */
	init();			/* initialize global vars */
	make_megasquirt_dirs();	/*Create config file dirs if missing */

	cfg_result = read_config();
	setup_gui();		

	open_serial(serial_params->port_name);

	setup_serial_params();	/* Setup the serial port for I/O */

	read_conversions();

	/* Force a read of constants to populate the gui */

	/* Convert the gui based on temp preference.  This MUST BE DONE
	 * AFTER data has been read once to make sure it's displayed correctly
	 */
	if (fahrenheit)
		reset_temps(GINT_TO_POINTER(FAHRENHEIT));
	else
		reset_temps(GINT_TO_POINTER(CELSIUS));


	/* Populate the gui in 100milliseconds after entering gtk_main */
	gtk_timeout_add(300,(GtkFunction)populate_gui,NULL);
	/* Startup status counters timeout handler... */
	/* Run it about 20 times/second.. proc use seems negligable... */
	statuscounts_id = gtk_timeout_add(50,(GtkFunction)update_errcounts,NULL);
	ready = TRUE;
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}
