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
  \file src/threads.c
  \ingroup CoreMtx
  \brief Thread functions mainly revolving around I/O or message passing
  \author David Andruczyk
  */
#ifdef GTK_DISABLE_SINGLE_INCLUDES
#undef GTK_DISABLE_SINGLE_INCLUDES
#endif

#include <args.h>
#include <3d_vetable.h>
#include <dataio.h>
#include <debugging.h>
#include <dispatcher.h>
#include <init.h>
#include <plugin.h>
#include <serialio.h>

extern gconstpointer *global_data;

/*!
  \brief io_cmd() is called from all over the gui to kick off a threaded I/O
  command.  A command enumeration and an option block of data is passed and
  this function allocates an Io_Message and shoves it down an GAsyncQueue
  to the main thread dispatcher which runs things and then passes any 
  needed information back to the gui via another GAsyncQueue which takes care
  of any post thread GUI updates. (which can NOT be done in a thread context
  due to reentrancy and deadlock conditions)
  \param cmd_name is the name of the command to look for in the commands_hash
  \param data is the additional data for fringe cases..
  */
G_MODULE_EXPORT void io_cmd(const gchar *cmd_name, void *data)
{
	static GAsyncQueue *io_data_queue = NULL;
	static void (*build_output_message_f)(Io_Message *, Command *, gpointer);
	static GHashTable *commands_hash = NULL;
	Io_Message *message = NULL;
	Command *command = NULL;

	ENTER();
	if (!commands_hash)
		commands_hash = (GHashTable *)DATA_GET(global_data,"commands_hash");
	if (!io_data_queue)
		io_data_queue = (GAsyncQueue *)DATA_GET(global_data,"io_data_queue");
	if (!build_output_message_f)
		get_symbol("build_output_message",(void **)&build_output_message_f);

	g_return_if_fail(build_output_message_f);
	g_return_if_fail(commands_hash);
	g_return_if_fail(io_data_queue);
	/* Fringe case for FUNC_CALL helpers that need to trigger 
	 * post_functions AFTER all their subhandlers have ran.  We
	 * call io_cmd with no cmd name and pack in the post functions into
	 * the void pointer part.
	 */
	if (!cmd_name)
	{
		message = initialize_io_message();
		message->command = g_new0(Command, 1);
		message->command->defer_post_functions = FALSE;
		message->command->post_functions = (GArray *)data;
		message->command->type = NULL_CMD;
		message->command->dynamic = TRUE;
	}
	/* Std io_message passed by string name */
	else
	{
		command = (Command *)g_hash_table_lookup(commands_hash,cmd_name);
		if (!command)
		{
			MTXDBG(CRITICAL|IO_MSG|THREADS,_("Command %s is INVALID, aborting call\n"),cmd_name);
			EXIT();
			return;;
		}
		message = initialize_io_message();
		message->command = command;
		message->command->dynamic = FALSE;
		if (data)
			message->payload = data;
		if (command->type == WRITE_CMD)
			build_output_message_f(message,command,data);
	}

	g_async_queue_ref(io_data_queue);
	g_async_queue_push(io_data_queue,(gpointer)message);
	g_async_queue_unref(io_data_queue);
	EXIT();
	return;
}


/*!
  \brief thread_dispatcher() runs continuously as a thread listening to the 
  io_data_queue and running handlers as messages come in. After they are done it
  passes the message back to the gui via the dispatch_queue for further
  gui handling (for things that can't run in a thread context)
  \param data is unused
  */
G_MODULE_EXPORT void *thread_dispatcher(gpointer data)
{
	GThread * repair_thread = NULL;
	Serial_Params *serial_params = NULL;
	Io_Message *message = NULL;	
	GAsyncQueue *io_data_queue = NULL;
	CmdLineArgs *args  = NULL;
	void *(*network_repair_thread)(gpointer data) = NULL;
	void *(*serial_repair_thread)(gpointer data) = NULL;
	/*	GTimer *clock;*/

	ENTER();
	io_data_queue = (GAsyncQueue *)DATA_GET(global_data,"io_data_queue");
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	get_symbol("serial_repair_thread",(void **)&serial_repair_thread);
	args = (CmdLineArgs *)DATA_GET(global_data,"args");
	if (args->network_mode)
		get_symbol("network_repair_thread",(void **)&network_repair_thread);

	g_return_val_if_fail(args,NULL);
	g_return_val_if_fail(io_data_queue,NULL);
	g_return_val_if_fail(serial_params,NULL);
	g_async_queue_ref(io_data_queue);
	/*	clock = g_timer_new();*/
	/* Endless Loop, wait for message, processs and repeat... */
	while (TRUE)
	{
		if (DATA_GET(global_data,"thread_dispatcher_exit"))
		{
fast_exit:
			/* drain queue and exit thread */
			while ((message = (Io_Message *)g_async_queue_try_pop(io_data_queue)) != NULL)
				dealloc_io_message(message);
			g_async_queue_unref(io_data_queue);

			EXIT();
			g_thread_exit(0);
		}
		message = (Io_Message *)g_async_queue_timeout_pop(io_data_queue,1000000);
		if (!message) /* NULL message */
		{
			MTXDBG(THREADS|IO_MSG,_("No message received...\n"));
			continue;
		}
		else
			MTXDBG(THREADS|IO_MSG,_("MESSAGE ARRIVED on IO queue...\n"));

		if ((!DATA_GET(global_data,"offline")) && 
				(((!DATA_GET(global_data,"connected")) && 
				  (serial_params->open)) || 
				 (!(serial_params->open))))
		{
			/*printf("somehow somethign went wrong,  connected is %i, offline is %i, serial_params->open is %i\n",DATA_GET(global_data,"connected"),DATA_GET(global_data,"offline"),serial_params->open);*/
			if (args->network_mode)
			{
				MTXDBG(THREADS,_("LINK DOWN, Initiating NETWORK repair thread!\n"));
				repair_thread = g_thread_new("Network Repair thread",network_repair_thread,NULL);
			}
			else
			{
				MTXDBG(THREADS,_("LINK DOWN, Initiating serial repair thread!\n"));
				repair_thread = g_thread_new("Serial Repair thread",serial_repair_thread,NULL);
			}
			g_thread_join(repair_thread);
		}
		if ((!serial_params->open) && (!DATA_GET(global_data,"offline")))
		{
			MTXDBG(THREADS,_("LINK DOWN, Can't process requested command, aborting call\n"));
			thread_update_logbar("comm_view","warning",g_strdup("Disconnected Serial Link. Check Communications link/cable...\n"),FALSE,FALSE);
			thread_update_widget("titlebar",MTX_TITLE,g_strdup("Disconnected link, check Communications tab..."));
			message->status = FALSE;
			continue;
		}
		switch ((CmdType)message->command->type)
		{
			case FUNC_CALL:
				if (!message->command->function)
					MTXDBG(CRITICAL|THREADS,_("CRITICAL ERROR, function \"%s()\" is not found!!\n"),message->command->func_call_name);
				else
				{
					/*printf("Calling FUNC_CALL, function \"%s()\" \n",message->command->func_call_name);*/
					message->status = message->command->function(message->command,message->command->func_call_arg);
					/*
					   if (!result)
					   message->command->defer_post_functions=TRUE;
					   */
				}
				break;
			case WRITE_CMD:
				/*g_timer_start(clock);*/
				message->status = write_data(message);
				if (!message->status)
					DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));

				/*printf("Write command elapsed time %f\n",g_timer_elapsed(clock,NULL));*/
				if (message)
				{
					if (message->command)
					{
						if (message->command->helper_function)
						{
							message->command->helper_function(message, message->command->helper_func_arg);
						}
					}
				}

				/*printf("Write command with post function time %f\n",g_timer_elapsed(clock,NULL));*/
				break;
			case NULL_CMD:
				/*printf("null_cmd, just passing thru\n");*/
				break;

			default:
				MTXDBG(THREADS|CRITICAL,_("Hit default case, this SHOULD NOT HAPPEN it's a bug, notify author! \n"));

				break;

		}
		/* If set to defer post functions, it means they were passed 
		   via a function fall, thus dealloc it here., Otherwise
		   push up the queue to the postfunction dispatcher
		   */
		if (message->command->defer_post_functions)
			dealloc_io_message(message);
		else
			g_idle_add(process_pf_message,message);
	}
	EXIT();
	return 0;
}


/*!
  \brief thread_update_logbar() is a function to be called from within threads
  to update a logbar (textview). It's not safe to update a widget from a 
  threaded context in win32, hence this fucntion is created to pass the 
  information to the main thread via an GAsyncQueue to a dispatcher that will
  take care of the message. Since the functions that call this ALWAYS send
  dynamically allocated test in the msg field we DEALLOCATE it HERE...
  \param view_name is the textual name fothe textview to update (required)
  \param tag_name is the textual name ofthe tag to be applied to the text 
  sent.  This can be NULL is no tag is desired
  \param msg is the message to be sent (required)
  \param count is the Flag to display a counter
  \param clear is the Flag to clear the display before writing the text
  */
G_MODULE_EXPORT void  thread_update_logbar(
		const gchar * view_name, 
		const gchar * tag_name, 
		gchar * msg,
		gboolean count,
		gboolean clear)
{
	Gui_Message *message = NULL;
	Text_Message *t_message = NULL;
	gint tmp = 0;

	ENTER();
	g_return_if_fail(view_name);
	g_return_if_fail(msg);

	message = initialize_gui_message();

	t_message = g_new0(Text_Message, 1);
	t_message->view_name = view_name;
	t_message->tag_name = tag_name;
	t_message->msg = msg;
	t_message->count = count;
	t_message->clear = clear;

	message->payload = (void *)t_message;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_LOGBAR;
	g_array_append_val(message->functions,tmp);
	//printf("thread_update_logbar(view %s, tag %s, msg \"%s\") about to send message\n",view_name,tag_name,msg);
	g_idle_add(process_gui_message,message);
	//printf("sent message!\n");
	EXIT();
	return;
}


/*!
  \brief queue_function() is used to run ANY global function in the context
  of the main GUI from within any thread.  It does it by passing a message 
  up an AsyncQueue to the gui process which will lookup the function name
  and run it with no arguments (currently inflexible and can only run "void"
  functions (ones that take no params)
  \param data is the name of function to lookup and run in the main 
  gui context vis the gui dispatcher.
  */
G_MODULE_EXPORT gboolean queue_function(const gchar *name)
{
	Gui_Message *message = NULL;
	QFunction *qfunc = NULL;
	gint tmp = 0;

	ENTER();
	message = initialize_gui_message();

	qfunc = g_new0(QFunction, 1);
	qfunc->func_name = name;

	message->payload = (void *)qfunc;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_RUN_FUNCTION;
	g_array_append_val(message->functions,tmp);

	//printf("queue_functions(function name %s) about to send message\n",name);
	g_idle_add(process_gui_message,message);
	//printf("sent message!\n");
	EXIT();
	return FALSE;
}


/*!
  \brief thread_update_widget() is a function to be called from within threads
  to update a widget (spinner/entry/label). It's not safe to update a 
  widget from a threaded context in win32, hence this fucntion is created 
  to pass the information to the main thread via an GAsyncQueue to a 
  dispatcher that will take care of the message. 
  \param widget_name is the textual name of the widget to update
  \param type is the type of widget to update
  \param msg is the message to be sent (required)
  */
G_MODULE_EXPORT void  thread_update_widget(
		const gchar * widget_name,
		WidgetType type,
		gchar * msg)
{
	Gui_Message *message = NULL;
	Widget_Update *w_update = NULL;
	gint tmp = 0;

	ENTER();
	g_return_if_fail(widget_name);

	message = initialize_gui_message();

	w_update = g_new0(Widget_Update, 1);
	w_update->widget_name = widget_name;
	w_update->type = type;
	w_update->msg = msg;

	message->payload = (void *)w_update;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_WIDGET;
	g_array_append_val(message->functions,tmp);

	//printf("thread_update_widget(widget %s, messages \"%s\") about to send message\n",widget_name,msg);
	g_idle_add(process_gui_message,message);
	//printf("sent message!\n");
	EXIT();
	return;
}


/*!
  \brief thread_set_sensitive() is a function to be called from within threads
  to update a widgets sensitivity state.
  \param widget_name is the textual name of the widget to update
  \param state is the state to set
  */
G_MODULE_EXPORT void thread_widget_set_sensitive(const gchar * widget_name, gboolean state)
{
	Gui_Message *message = NULL;
	Widget_Update *w_update = NULL;
	gint tmp = 0;

	ENTER();
	message = initialize_gui_message();

	w_update = g_new0(Widget_Update, 1);
	w_update->widget_name = widget_name;
	w_update->type = MTX_SENSITIVE;
	w_update->state = state;
	w_update->msg = NULL;

	message->payload = (void *)w_update;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_WIDGET;
	g_array_append_val(message->functions,tmp);

	//printf("thread_widget_set_sensitive(%s), about to send message\n",widget_name);
	g_idle_add(process_gui_message,message);
	//printf("sent message!\n");
	EXIT();
	return;
}


/*!
  \brief thread_refresh_widget() is a function to be called from within threads
  to force a widget rerender
  \param widget is the widget pointer to be updated
  */
G_MODULE_EXPORT void thread_refresh_widget(GtkWidget * widget)
{
	Gui_Message *message = NULL;
	gint tmp = 0;

	ENTER();
	message = initialize_gui_message();

	message->payload = (void *)widget;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_REFRESH;
	g_array_append_val(message->functions,tmp);

	//printf("thread_refresh_widget(widget pointer %p) about to send message\n",(gpointer)widget);
	g_idle_add(process_gui_message,message);
	//printf("sent message!\n");
	EXIT();
	return;
}


/*!
  \brief refreshs all widgets as a particular page/offset
  \param page is the mtx-ecu page number
  \param offset is the offset from that page's origin
  */
G_MODULE_EXPORT void thread_refresh_widgets_at_offset(gint page, gint offset)
{
	guint i = 0;
	Firmware_Details *firmware = NULL;
	GList ***ecu_widgets = NULL;

	ENTER();
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	for (i=0;i<g_list_length(ecu_widgets[page][offset]);i++)
		thread_refresh_widget((GtkWidget *)g_list_nth_data(ecu_widgets[page][offset],i));
	update_ve3d_if_necessary(page,offset);
	EXIT();
	return;
}


/*!
  \brief To be called from within threads to force a widget rerender
  \param page is the MTX-ecu page
  \param offset is the offset from beginning of the page
  \param len How many bytes worth of widgets to search for an update
  */
G_MODULE_EXPORT void thread_refresh_widget_range(gint page, gint offset, gint len)
{
	gint tmp = 0;
	Gui_Message *message = NULL;
	Widget_Range *range = NULL;

	ENTER();
	message = initialize_gui_message();
	range = g_new0(Widget_Range,1);

	range->page = page;
	range->offset = offset;
	range->len = len;
	message->payload = (void *)range;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_REFRESH_RANGE;
	g_array_append_val(message->functions,tmp);

	message = initialize_gui_message();
	range = g_new0(Widget_Range,1);

	range->page = page;
	range->offset = offset;
	range->len = len;
	message->payload = (void *)range;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_VE3D;
	g_array_append_val(message->functions,tmp);
	
	//printf("thread_refresh_widget_range( page %i, offset %i, len %i) about to send message\n",page,offset,len);
	g_idle_add(process_gui_message,message);
	EXIT();
	return;
}

/*!
  \brief Sends a message to the gui dispatcher to set a group of widgets to
  a specific color enumeration
  */
G_MODULE_EXPORT void thread_set_group_color(GuiColor color,const gchar *group)
{
	Gui_Message *message = NULL;
	Widget_Update *w_update = NULL;
	gint tmp = 0;

	ENTER();
	message = initialize_gui_message();

	w_update = g_new0(Widget_Update, 1);
	w_update->group_name = g_strdup(group);
	w_update->color = color;
	w_update->type = MTX_GROUP_COLOR;
	w_update->msg = NULL;

	message->payload = (void *)w_update;
	message->functions = g_array_new(FALSE,TRUE,sizeof(gint));
	tmp = UPD_WIDGET;
	g_array_append_val(message->functions,tmp);

	//printf("thread_set_group_color(group %s) about to send message\n",group);
	g_idle_add(process_gui_message,message);
	//printf("sent message!\n");
	EXIT();
	return;
}
