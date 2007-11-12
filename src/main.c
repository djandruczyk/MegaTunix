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


#include <args.h>
#include <binreloc.h>
#include <comms_gui.h>
#include <config.h>
#include <conversions.h>
#include <core_gui.h>
#include <defines.h>
#include <debugging.h>
#include <dispatcher.h>
#include <enums.h>
#include <gdk/gdkgl.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <init.h>
#include <main.h>
#include <serialio.h>
#include <stringmatch.h>
#include <structures.h>
#include <threads.h>
#include <timeout_handlers.h>


extern gint temp_units;
GThread * thread_dispatcher_id = NULL;
gboolean ready = FALSE;
gint dispatcher_id = -1;
gboolean gl_ability = FALSE;
Serial_Params *serial_params;
Io_Cmds *cmds;
GAsyncQueue *io_queue;
GAsyncQueue *dispatch_queue;

/*!
 \brief main() is the typical main function in a C program, it performs
 all core initialization, loading of all main parameters, initializing handlers
 and entering gtk_main to process events until program close
 \param argc (gint) count of command line arguments
 \param argv (char **) array of command line args
 \returns TRUE
 */
gint main(gint argc, gchar ** argv)
{
	if(!g_thread_supported())
		g_thread_init(NULL);

	gdk_threads_init();
	gdk_threads_enter();

	gtk_init(&argc, &argv);
	glade_init();
	gbr_init(NULL);

	gl_ability = gdk_gl_init_check(&argc, &argv);

	gtk_set_locale();


	/* Allocate memory  */
	serial_params = g_malloc0(sizeof(Serial_Params));
	cmds = g_malloc0(sizeof(Io_Cmds));

	open_debug();	/* Open debug log */
	init();			/* Initialize global vars */
	handle_args(argc,argv);
	make_megasquirt_dirs();	/* Create config file dirs if missing */
	/* Build table of strings to enum values */
	build_string_2_enum_table();

	/* Create Queue to listen for commands */
	io_queue = g_async_queue_new();
	dispatch_queue = g_async_queue_new();

	read_config();
	setup_gui();		

	/* Startup the serial General I/O handler.... */
	thread_dispatcher_id = g_thread_create(thread_dispatcher,
			NULL, // Thread args
			TRUE, // Joinable
			NULL); //GError Pointer

	dispatcher_id = g_timeout_add(10,(GtkFunction)dispatcher,NULL);

	/* Kickoff fast interrogation */
	g_timeout_add(250,(GtkFunction)early_interrogation,NULL);

	ready = TRUE;
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}
