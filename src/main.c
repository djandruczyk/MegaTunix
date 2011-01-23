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
#include <binlogger.h>
#include <comms_gui.h>
#include <config.h>
#include <conversions.h>
#include <core_gui.h>
#include <defines.h>
#include <debugging.h>
#include <dispatcher.h>
#include <enums.h>
#include <errno.h>
#include <glib.h>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>
#include <getfiles.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <init.h>
#include <locale.h>
#include <locking.h>
#include <main.h>
#include <serialio.h>
#include <sleep_calib.h>
#include <stringmatch.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <xmlcomm.h>

gboolean gl_ability = FALSE;
gconstpointer *global_data = NULL;

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
	Serial_Params *serial_params = NULL;
	GAsyncQueue *queue = NULL;
	GCond *cond = NULL;
	GMutex *mutex = NULL;
	gint id = 0;
	setlocale(LC_ALL,"");
#ifdef __WIN32__
	bindtextdomain(PACKAGE, "C:\\Program Files\\MegaTunix\\dist\\locale");
#else
	bindtextdomain(PACKAGE, LOCALEDIR);
#endif
	textdomain (PACKAGE);

	if(!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();

	global_data = g_new0(gconstpointer, 1);

	/* Condition variables */
	cond = g_cond_new();
	DATA_SET(global_data,"statuscounts_cond",cond);
	cond = g_cond_new();
	DATA_SET(global_data,"io_dispatch_cond",cond);
	cond = g_cond_new();
	DATA_SET(global_data,"gui_dispatch_cond",cond);
	cond = g_cond_new();
	DATA_SET(global_data,"pf_dispatch_cond",cond);
	cond = g_cond_new();
	DATA_SET(global_data,"rtv_thread_cond",cond);

	/* Mutexes */
	mutex = g_mutex_new();
	DATA_SET(global_data,"serio_mutex",mutex);
	mutex = g_mutex_new();
	DATA_SET(global_data,"rtt_mutex",mutex);
	mutex = g_mutex_new();
	DATA_SET(global_data,"rtv_mutex",mutex);
	mutex = g_mutex_new();
	DATA_SET(global_data,"dash_mutex",mutex);

	gdk_threads_enter();
	gtk_init(&argc, &argv);
	glade_init();

	gdk_gl_init_check(&argc, &argv);
	gl_ability = gtk_gl_init_check(&argc, &argv);

	/* For testing if gettext works
	   printf(_("Hello World!\n"));
	 */

	/* This will exit mtx if the locking fails! */
	//create_mtx_lock();

	/* Allocate memory  */
	build_string_2_enum_table();
	serial_params = g_malloc0(sizeof(Serial_Params));
	DATA_SET(global_data,"serial_params",serial_params);

	handle_args(argc,argv);	/* handle CLI arguments */
	open_debug();		/* Open debug log */
	init();			/* Initialize global vars */
	make_megasquirt_dirs();	/* Create config file dirs if missing */
	/* Build table of strings to enum values */

	/* Create Message passing queues */
	queue = g_async_queue_new();
	DATA_SET_FULL(global_data,"io_data_queue",queue,g_async_queue_unref);
	queue = g_async_queue_new();
	DATA_SET_FULL(global_data,"slave_msg_queue",queue,g_async_queue_unref);
	queue = g_async_queue_new();
	DATA_SET_FULL(global_data,"pf_dispatch_queue",queue,g_async_queue_unref);
	queue = g_async_queue_new();
	DATA_SET_FULL(global_data,"gui_dispatch_queue",queue,g_async_queue_unref);
	queue = g_async_queue_new();
	DATA_SET_FULL(global_data,"io_repair_queue",queue,g_async_queue_unref);

	read_config();
	setup_gui();		

	gtk_rc_parse_string("style \"override\"\n{\n\tGtkTreeView::horizontal-separator = 0\n\tGtkTreeView::vertical-separator = 0\n}\nwidget_class \"*\" style \"override\"");

	id = g_timeout_add(12,(GSourceFunc)pf_dispatcher,NULL);
	DATA_SET(global_data,"pf_dispatcher_id",GINT_TO_POINTER(id));
	id = g_timeout_add(35,(GSourceFunc)gui_dispatcher,NULL);
	DATA_SET(global_data,"gui_dispatcher_id",GINT_TO_POINTER(id));
	id = g_timeout_add(1000,(GSourceFunc)flush_binary_logs,NULL);
        DATA_SET(global_data,"binlog_flush_id",GINT_TO_POINTER(id));

	/* Kickoff fast interrogation */
	sleep_calib();
	gdk_threads_add_timeout(500,(GSourceFunc)personality_choice,NULL);
	

	DATA_SET(global_data,"ready",GINT_TO_POINTER(TRUE));
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}
