/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/main.c
  \ingroup CoreMtx
  \brief The holder of the main() fucntion which gets the ball rolling...
  \author David Andruczyk
  */

#ifdef GTK_DISABLE_SINGLE_INCLUDES
#undef GTK_DISABLE_SINGLE_INCLUDES
#endif


#include <args.h>
#include <binlogger.h>
#include <core_gui.h>
#include <debugging.h>
#include <dispatcher.h>
#include <gtk/gtkgl.h>
#include <locale.h>
#include <locking.h>
#include <init.h>
#include <serialio.h>
#include <stdio.h>
#include <sleep_calib.h>
#include <stringmatch.h>
#include <timeout_handlers.h>
#include <version.h>

gboolean gl_ability = FALSE;
gconstpointer *global_data = NULL;

/*!
  \brief main() is the typical main function in a C program, it performs
  all core initialization, loading of all main parameters, initializing handlers
  and entering gtk_main to process events until program close
  \param argc is the count of command line arguments
  \param argv is the array of command line args
  \returns TRUE
  */
gint main(gint argc, gchar ** argv)
{
	Serial_Params *serial_params = NULL;
	GAsyncQueue *queue = NULL;
	GCond *cond = NULL;
	GMutex *mutex = NULL;
	GMutex *pf_dispatch_mutex = NULL;
	GMutex *gui_dispatch_mutex = NULL;
	GTimer *timer = NULL;
	gint id = 0;
	setlocale(LC_ALL,"");
#ifdef __WIN32__
	bindtextdomain(PACKAGE, "C:\\Program Files\\MegaTunix\\dist\\locale");
#else
	bindtextdomain(PACKAGE, LOCALEDIR);
#endif
	textdomain (PACKAGE);

#ifdef DEBUG
	printf("This is a debug release, Git hash: %s\n",MTX_GIT_HASH);
#endif
	if(!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	glade_init();

	gdk_gl_init_check(&argc, &argv);
	gl_ability = gtk_gl_init_check(&argc, &argv);

	global_data = g_new0(gconstpointer, 1);

	/* Condition Variables */
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
	mutex = g_mutex_new();
	DATA_SET(global_data,"rtv_thread_mutex",mutex);
	mutex = g_mutex_new();
	DATA_SET(global_data,"statuscounts_mutex",mutex);
	gui_dispatch_mutex = g_mutex_new();
	DATA_SET(global_data,"gui_dispatch_mutex",gui_dispatch_mutex);
	pf_dispatch_mutex = g_mutex_new();
	DATA_SET(global_data,"pf_dispatch_mutex",pf_dispatch_mutex);

	/* For testing if gettext works
	   printf(_("Hello World!\n"));
	 */

	/* Build table of strings to enum values */
	build_string_2_enum_table();
	serial_params = (Serial_Params *)g_malloc0(sizeof(Serial_Params));
	DATA_SET(global_data,"serial_params",serial_params);

	handle_args(argc,argv);	/* handle CLI arguments */

	/* This will exit mtx if the locking fails! */
	/* Prevents multiple instances  but stops esoteric usess too 
	 * create_mtx_lock();
	 * */
	open_debug();		/* Open debug log */
	/* Allocate memory  */
	init();			/* Initialize global vars */
	make_mtx_dirs();	/* Create config file dirs if missing */

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

	id = gdk_threads_add_timeout_full(-50,16,(GSourceFunc)pf_dispatcher,pf_dispatch_mutex,timeout_done);
	DATA_SET(global_data,"pf_dispatcher_id",GINT_TO_POINTER(id));
	id = gdk_threads_add_timeout_full(-35,35,(GSourceFunc)gui_dispatcher,gui_dispatch_mutex,timeout_done);
	DATA_SET(global_data,"gui_dispatcher_id",GINT_TO_POINTER(id));
	id = gdk_threads_add_timeout(2000,(GSourceFunc)flush_binary_logs,NULL);
    DATA_SET(global_data,"binlog_flush_id",GINT_TO_POINTER(id));

	sleep_calib();
	/* Check for first_time flag, if so, run first time wizard, otherwise
	   load personality choice
	   */
	timer = g_timer_new();
	DATA_SET_FULL(global_data,"mtx_uptime_timer",timer,g_timer_destroy);
	g_idle_add((GSourceFunc)check_for_first_time,NULL);
	

	DATA_SET(global_data,"ready",GINT_TO_POINTER(TRUE));
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}
