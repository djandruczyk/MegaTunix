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
#include <defines.h>
#include <args.h>
#include <3d_vetable.h>
#include <comms.h>
#include <comms_gui.h>
#include <conversions.h>
#include <datalogging_gui.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <gui_handlers.h>
#include <init.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <mode_select.h>
#include <notifications.h>
#include <plugin.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <tabloader.h>
#include <xmlcomm.h>
#include <unistd.h>
#include <widgetmgmt.h>


extern gconstpointer *global_data;
gchar *handler_types[]={"Realtime Vars","VE-Block","Raw Memory Dump","Comms Test","Get ECU Error", "NULL Handler"};


/*!
 \brief io_cmd() is called from all over the gui to kick off a threaded I/O
 command.  A command enumeration and an option block of data is passed and
 this function allocates an Io_Message and shoves it down an GAsyncQueue
 to the main thread dispatcher which runs things and then passes any 
 needed information back to the gui via another GAsyncQueue which takes care
 of any post thread GUI updates. (which can NOT be done in a thread context
 due to reentrancy and deadlock conditions)
 \param cmd (gchar *) and enumerated representation of a command to execute
 \param data (void *) additional data for fringe cases..
 */
G_MODULE_EXPORT void io_cmd(gchar *io_cmd_name, void *data)
{
	static GAsyncQueue *io_data_queue = NULL;
	Io_Message *message = NULL;
	GHashTable *commands_hash = NULL;
	Command *command = NULL;

	commands_hash = DATA_GET(global_data,"commands_hash");
	if (!io_data_queue)
		io_data_queue = DATA_GET(global_data,"io_data_queue");

	/* Fringe case for FUNC_CALL helpers that need to trigger 
	 * post_functions AFTER all their subhandlers have ran.  We
	 * call io_cmd with no cmd name and pack in the post functions into
	 * the void pointer part.
	 */
	if (!io_cmd_name)
	{
		message = initialize_io_message();
		message->command = g_new0(Command, 1);
		message->command->defer_post_functions = FALSE;
		message->command->post_functions = (GArray *)data;
		message->command->type = NULL_CMD;
	}
	/* Std io_message passed by string name */
	else
	{
		command = g_hash_table_lookup(commands_hash,io_cmd_name);
		if (!command)
		{
			printf(_("Command %s is INVALID, aborting call\n"),io_cmd_name);
			return;;
		}
		message = initialize_io_message();
		message->command = command;
		if (data)
		{
			message->payload = data;
		}
		if (command->type != FUNC_CALL)
			build_output_string(message,command,data);
	}

	g_async_queue_ref(io_data_queue);
	g_async_queue_push(io_data_queue,(gpointer)message);
	g_async_queue_unref(io_data_queue);

}


/*!
 \brief thread_dispatcher() runs continuously as a thread listening to the 
 io_data_queue and running handlers as messages come in. After they are done it
 passes the message back to the gui via the dispatch_queue for further
 gui handling (for things that can't run in a thread context)
 \param data (gpointer) unused
 */
G_MODULE_EXPORT void *thread_dispatcher(gpointer data)
{
	GThread * repair_thread = NULL;
	Serial_Params *serial_params = NULL;
	CmdLineArgs *args = NULL;
	GTimeVal cur;
	Io_Message *message = NULL;	
	GAsyncQueue *io_data_queue = NULL;
	GAsyncQueue *pf_dispatch_queue = NULL;
	GCond * io_dispatch_cond = NULL;
	void *(*network_repair_thread)(gpointer data) = NULL;
	void *(*serial_repair_thread)(gpointer data) = NULL;
//	GTimer *clock;

	dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\tThread created!\n"));

	io_data_queue = DATA_GET(global_data,"io_data_queue");
	pf_dispatch_queue = DATA_GET(global_data,"pf_dispatch_queue");
	io_dispatch_cond = DATA_GET(global_data,"io_dispatch_cond");
	args = DATA_GET(global_data,"args");
	serial_params = DATA_GET(global_data,"serial_params");
	get_symbol("network_repair_thread",(void*)&network_repair_thread);
	get_symbol("serial_repair_thread",(void*)&serial_repair_thread);
//	clock = g_timer_new();
	/* Endless Loop, wait for message, processs and repeat... */
	while (TRUE)
	{
		g_get_current_time(&cur);
		g_time_val_add(&cur,100000); /* 100 ms timeout */
		message = g_async_queue_timed_pop(io_data_queue,&cur);

		if (DATA_GET(global_data,"leaving"))
		{
			/* drain queue and exit thread */
			while (g_async_queue_try_pop(io_data_queue) != NULL)
			{}
			dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\tMegaTunix is closing, Thread exiting !!\n"));
			g_cond_signal(io_dispatch_cond);
			g_thread_exit(0);
		}
		if (!message) /* NULL message */
			continue;

		if ((!DATA_GET(global_data,"offline")) && 
				(((!DATA_GET(global_data,"connected")) && 
				  (serial_params->open)) || 
				 (!(serial_params->open))))
		{
			/*printf("somehow somethign went wrong,  connected is %i, offline is %i, serial_params->open is %i\n",connected,DATA_GET(global_data,"offline"),serial_params->open);*/
			if (args->network_mode)
			{
				dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\tLINK DOWN, Initiating NETWORK repair thread!\n"));
				repair_thread = g_thread_create(network_repair_thread,NULL,TRUE,NULL);
			}
			else
			{
				dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\tLINK DOWN, Initiating serial repair thread!\n"));
				repair_thread = g_thread_create(serial_repair_thread,NULL,TRUE,NULL);
			}
			g_thread_join(repair_thread);
		}
		if ((!serial_params->open) && (!DATA_GET(global_data,"offline")))
		{
			dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\tLINK DOWN, Can't process requested command, aborting call\n"));
			thread_update_logbar("comm_view","warning",g_strdup("Disconnected Serial Link. Check Communications link/cable...\n"),FALSE,FALSE);
			thread_update_widget("titlebar",MTX_TITLE,g_strdup("Disconnected link, check Communications tab..."));
			continue;
		}

		switch ((CmdType)message->command->type)
		{
			case FUNC_CALL:
				if (!message->command->function)
					printf(_("CRITICAL ERROR, function \"%s()\" is not found!!\n"),message->command->func_call_name);
				else
				{
					/*printf("Calling FUNC_CALL, function \"%s()\" \n",message->command->func_call_name);*/

					gdk_threads_enter();
					message->status = message->command->function(
							message->command,
							message->command->func_call_arg);
					gdk_threads_leave();

					/*
					   if (!result)
					   message->command->defer_post_functions=TRUE;
					 */
				}
				break;
			case WRITE_CMD:
//				g_timer_start(clock);
				message->status = write_data(message);
				//		printf("Write command elapsed time %f\n",g_timer_elapsed(clock,NULL));
				gdk_threads_enter();
				if (message->command->helper_function)
					message->command->helper_function(message, message->command->helper_func_arg);
				gdk_threads_leave();
				//		printf("Write command with post function time %f\n",g_timer_elapsed(clock,NULL));
				break;
			case NULL_CMD:
				/*printf("null_cmd, just passing thru\n");*/
				break;

			default:
				dbg_func(THREADS|CRITICAL,g_strdup_printf(__FILE__": thread_dispatcher()\n\t Hit default case, this SHOULD NOT HAPPEN it's a bug, notify author! \n"));

				break;

		}
		/* Send rest of message back up to main context for gui
		 * updates via asyncqueue
		 */
		if (!message->command->defer_post_functions)
		{
			g_async_queue_ref(pf_dispatch_queue);
			g_async_queue_push(pf_dispatch_queue,(gpointer)message);
			g_async_queue_unref(pf_dispatch_queue);
		}
		else
			dealloc_message(message);
	}
	dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\texiting!!\n"));
}


/*!
 \brief thread_update_logbar() is a function to be called from within threads
 to update a logbar (textview). It's not safe to update a widget from a 
 threaded context in win32, hence this fucntion is created to pass the 
 information to the main thread via an GAsyncQueue to a dispatcher that will
 take care of the message. Since the functions that call this ALWAYS send
 dynamically allocated test in the msg field we DEALLOCATE it HERE...
 \param view_name (gchar *) textual name fothe textview to update (required)
 \param tagname (gchar *) textual name ofthe tag to be applied to the text 
 sent.  This can be NULL is no tag is desired
 \param msg (gchar *) the message to be sent (required)
 \param count (gboolean) Flag to display a counter
 \param clear (gboolean) Flag to clear the display before writing the text
 */
G_MODULE_EXPORT void  thread_update_logbar(
		const gchar * view_name, 
		const gchar * tagname, 
		gchar * msg,
		gboolean count,
		gboolean clear)
{
	static GAsyncQueue *gui_dispatch_queue = NULL;
	Io_Message *message = NULL;
	Text_Message *t_message = NULL;
	gint tmp = 0;

	if (!gui_dispatch_queue)
		gui_dispatch_queue = DATA_GET(global_data,"gui_dispatch_queue");
	message = initialize_io_message();

	t_message = g_new0(Text_Message, 1);
	t_message->view_name = view_name;
	t_message->tagname = tagname;
	t_message->msg = msg;
	t_message->count = count;
	t_message->clear = clear;

	message->payload = t_message;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_LOGBAR;
	g_array_append_val(message->functions,tmp);

	g_async_queue_ref(gui_dispatch_queue);
	g_async_queue_push(gui_dispatch_queue,(gpointer)message);
	g_async_queue_unref(gui_dispatch_queue);
	return;
}


/*!
 \brief queue_function() is used to run ANY global function in the context
 of the main GUI from within any thread.  It does it by passing a message 
 up an AsyncQueue to the gui process which will lookup the function name
 and run it with no arguments (currently inflexible and can only run "void"
 functions (ones that take no params)
 \param name name of function to lookup and run in the main gui context..
 */
G_MODULE_EXPORT gboolean queue_function(const gchar * name)
{
	static GAsyncQueue *gui_dispatch_queue = NULL;
	Io_Message *message = NULL;
	QFunction *qfunc = NULL;
	gint tmp = 0;

	if (!gui_dispatch_queue)
		gui_dispatch_queue = DATA_GET(global_data,"gui_dispatch_queue");
	message = initialize_io_message();

	qfunc = g_new0(QFunction, 1);
	qfunc->func_name = name;

	message->payload = qfunc;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_RUN_FUNCTION;
	g_array_append_val(message->functions,tmp);

	g_async_queue_ref(gui_dispatch_queue);
	g_async_queue_push(gui_dispatch_queue,(gpointer)message);
	g_async_queue_unref(gui_dispatch_queue);
	return FALSE;
}


/*!
 \brief thread_update_widget() is a function to be called from within threads
 to update a widget (spinner/entry/label). It's not safe to update a 
 widget from a threaded context in win32, hence this fucntion is created 
 to pass the information to the main thread via an GAsyncQueue to a 
 dispatcher that will take care of the message. 
 \param widget_name (gchar *) textual name of the widget to update
 \param type (WidgetType enumeration) type of widget to update
 \param msg (gchar *) the message to be sent (required)
 */
G_MODULE_EXPORT void  thread_update_widget(
		const gchar * widget_name,
		WidgetType type,
		gchar * msg)
{
	static GAsyncQueue *gui_dispatch_queue = NULL;
	Io_Message *message = NULL;
	Widget_Update *w_update = NULL;
	gint tmp = 0;

	if (!gui_dispatch_queue)
		gui_dispatch_queue = DATA_GET(global_data,"gui_dispatch_queue");
	message = initialize_io_message();

	w_update = g_new0(Widget_Update, 1);
	w_update->widget_name = widget_name;
	w_update->type = type;
	w_update->msg = msg;

	message->payload = w_update;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_WIDGET;
	g_array_append_val(message->functions,tmp);

	g_async_queue_ref(gui_dispatch_queue);
	g_async_queue_push(gui_dispatch_queue,(gpointer)message);
	g_async_queue_unref(gui_dispatch_queue);
	return;
}


/*!
 \brief thread_set_sensitive() is a function to be called from within threads
 to update a widgets sensitivity state.
 \param widget_name (gchar *) textual name of the widget to update
 \param stats (gboolean) the state to set
 */
G_MODULE_EXPORT void thread_widget_set_sensitive(const gchar * widget_name, gboolean state)
{
	static GAsyncQueue *gui_dispatch_queue = NULL;
	Io_Message *message = NULL;
	Widget_Update *w_update = NULL;
	gint tmp = 0;

	if (!gui_dispatch_queue)
		gui_dispatch_queue = DATA_GET(global_data,"gui_dispatch_queue");
	message = initialize_io_message();

	w_update = g_new0(Widget_Update, 1);
	w_update->widget_name = widget_name;
	w_update->type = MTX_SENSITIVE;
	w_update->state = state;
	w_update->msg = NULL;

	message->payload = w_update;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_WIDGET;
	g_array_append_val(message->functions,tmp);

	g_async_queue_ref(gui_dispatch_queue);
	g_async_queue_push(gui_dispatch_queue,(gpointer)message);
	g_async_queue_unref(gui_dispatch_queue);
	return;
}


/*!
 \brief thread_refresh_widget() is a function to be called from within threads
 to force a widget rerender
 \param widget_name (GtkWidget *) widget pointer
 */
G_MODULE_EXPORT void thread_refresh_widget(GtkWidget * widget)
{
	static GAsyncQueue *gui_dispatch_queue = NULL;
	Io_Message *message = NULL;
	gint tmp = 0;

	if (!gui_dispatch_queue)
		gui_dispatch_queue = DATA_GET(global_data,"gui_dispatch_queue");
	message = initialize_io_message();

	message->payload = (void *)widget;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_REFRESH;
	g_array_append_val(message->functions,tmp);

	g_async_queue_ref(gui_dispatch_queue);
	g_async_queue_push(gui_dispatch_queue,(gpointer)message);
	g_async_queue_unref(gui_dispatch_queue);
	return;
}


/*! 
 \brief build_output_string() is called when doing output to the ECU, to 
 append the needed data together into one nice string for sending
 */
G_MODULE_EXPORT void build_output_string(Io_Message *message, Command *command, gpointer data)
{
	guint i = 0;
	gint v = 0;
	gint len = 0;
	OutputData *output = NULL;
	PotentialArg * arg = NULL;
	guint8 *sent_data = NULL;
	DBlock *block = NULL;

	if (data)
		output = (OutputData *)data;

	message->sequence = g_array_new(FALSE,TRUE,sizeof(DBlock *));

	/* Base command */
	block = g_new0(DBlock, 1);
	block->type = DATA;
	block->data = (guint8 *)g_strdup(command->base);
	if (!command->base)
		block->len = 0;
	else
		block->len = strlen(command->base);
	g_array_append_val(message->sequence,block);

	/* Arguments */
	for (i=0;i<command->args->len;i++)
	{
		arg = g_array_index(command->args,PotentialArg *, i);
		block = g_new0(DBlock, 1);
		if (arg->type == ACTION)
		{
			/*printf("build_output_string(): ACTION being created!\n");*/
			block->type = ACTION;
			block->action = arg->action;
			block->arg = arg->action_arg;
			g_array_append_val(message->sequence,block);
			continue;
		}
		if (arg->type == STATIC_STRING)
		{
			block->type = DATA;
			block->data = (guint8 *)g_strdup(arg->static_string);
			block->len = strlen(arg->static_string);
			g_array_append_val(message->sequence,block);
			continue;
		}
		if (!output)
			continue;
		switch (arg->size)
		{
			case MTX_U08:
			case MTX_S08:
			case MTX_CHAR:
				/*printf("8 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("value %i\n",v);*/
				block->data = g_new0(guint8,1);
				block->data[0] = (guint8)v;
				block->len = 1;
				break;
			case MTX_U16:
			case MTX_S16:
				/*printf("16 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (GINT)DATA_GET(output->data,arg->internal_name);
				/*printf("value %i\n",v);*/
				block->data = g_new0(guint8,2);
				block->data[0] = (v & 0xff00) >> 8;
				block->data[1] = (v & 0x00ff);
				block->len = 2;
				break;
			case MTX_U32:
			case MTX_S32:
/*				printf("32 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (GINT)DATA_GET(output->data,arg->internal_name);
/*				printf("value %i\n",v); */
				block->data = g_new0(guint8,4);
				block->data[0] = (v & 0xff000000) >> 24;
				block->data[1] = (v & 0xff0000) >> 16;
				block->data[2] = (v & 0xff00) >> 8;
				block->data[3] = (v & 0x00ff);
				block->len = 4;
				break;
			case MTX_UNDEF:
				/*printf("arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				if (!arg->internal_name)
					printf(_("ERROR, MTX_UNDEF, donno what to do!!\n"));
				sent_data = (guint8 *)DATA_GET(output->data,arg->internal_name);
				len = (GINT)DATA_GET(output->data,"num_bytes");
				block->data = g_memdup(sent_data,len);
				block->len = len;
				/*
				for (j=0;j<len;j++)
				{
					printf("sent_data[%i] is %i\n",j,sent_data[j]);
					printf("block->data[%i] is %i\n",j,block->data[j]);
				}
				*/

		}
		g_array_append_val(message->sequence,block);
	}
}

