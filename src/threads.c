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

#include <3d_vetable.h>
#include <comms.h>
#include <comms_gui.h>
#include <config.h>
#include <conversions.h>
#include <dataio.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <mode_select.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <tabloader.h>
#include <xmlcomm.h>
#include <unistd.h>


extern gboolean connected;			/* valid connection with MS */
extern volatile gboolean offline;		/* ofline mode with MS */
extern gboolean interrogated;			/* valid connection with MS */
extern gint dbg_lvl;
extern GObject *global_data;
gchar *handler_types[]={"Realtime Vars","VE-Block","Raw Memory Dump","Comms Test","Get ECU Error", "NULL Handler"};


/*!
 \brief io_cmd() is called from all over the gui to kick off a threaded I/O
 command.  A command enumeration and an option block of data is passed and
 this function allocates a Io_Message and shoves it down an GAsyncQueue
 to the main thread dispatcher which runs things and then passes any 
 needed information back to the gui via another GAsyncQueue which takes care
 of any post thread GUI updates. (which can NOT be done in a thread context
 due to reentrancy and deadlock conditions)
 \param cmd (gchar *) and enumerated representation of a command to execute
 \param data (void *) additional data for fringe cases..
 */
void io_cmd(gchar *io_cmd_name, void *data)
{
	Io_Message *message = NULL;
	GHashTable *commands_hash = NULL;
	Command *command = NULL;
	extern GAsyncQueue *io_queue;

	commands_hash = OBJ_GET(global_data,"commands_hash");

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
		message = initialize_io_message();
		message->command = command;
		if (data)
			message->payload = data;
		if (command->type != FUNC_CALL)
			build_output_string(message,command,data);
	}

	g_async_queue_ref(io_queue);
	g_async_queue_push(io_queue,(gpointer)message);
	g_async_queue_unref(io_queue);

}


/*!
 \brief thread_dispatcher() runs continuously as a thread listening to the 
 io_queue and running handlers as messages come in. After they are done it
 passes the message back to the gui via the dispatch_queue for further
 gui handling (for things that can't run in a thread context)
 \param data (gpointer) unused
 */
void *thread_dispatcher(gpointer data)
{
	GThread * repair_thread = NULL;
	extern GAsyncQueue *io_queue;
	extern GAsyncQueue *pf_dispatch_queue;
	extern gboolean port_open;
	extern volatile gboolean leaving;
	gboolean result;
	GTimeVal cur;
	Io_Message *message = NULL;	

	/* Endless Loop, wait for message, processs and repeat... */
	while (1)
	{
		/*printf("thread_dispatch_queue length is %i\n",g_async_queue_length(io_queue));*/
		g_get_current_time(&cur);
		g_time_val_add(&cur,100000); /* 100 ms timeout */
		message = g_async_queue_timed_pop(io_queue,&cur);

		if (leaving)
		{
			/* drain queue and exit thread */
			while (g_async_queue_try_pop(io_queue) != NULL)
			{}
			g_thread_exit(0);
		}
		if (!message) /* NULL message */
			continue;

		if ((!offline) && (((!connected) && (port_open)) || (!port_open)))
		{
			repair_thread = g_thread_create(serial_repair_thread,NULL,TRUE,NULL);
			g_thread_join(repair_thread);
		}
		if ((!port_open) && (!offline))
		{
			if (dbg_lvl & (THREADS|CRITICAL))
				dbg_func(g_strdup(__FILE__": thread_dispatcher()\n\tLINK DOWN, Can't process requested command, aborting call\n"));
			thread_update_logbar("comm_view","warning",g_strdup("Disconnected Serial Link. Check Communications link/cable...\n"),FALSE,FALSE);
			thread_update_widget(g_strdup("titlebar"),MTX_TITLE,g_strdup("Disconnected link, check Communications tab..."));
			continue;
		}

		switch ((CmdType)message->command->type)
		{
			case FUNC_CALL:
				if (!message->command->function)
					printf("CRITICAL ERROR, function \"%s()\" is not found!!\n",message->command->func_call_name);
				else
				{
					/*printf("Calling FUNC_CALL, function \"%s()\" \n",message->command->func_call_name);*/
					result = message->command->function(
							message->command,
							message->command->func_call_arg);

				if (!result)
					message->command->defer_post_functions=TRUE;
				}
				break;
			case WRITE_CMD:
				write_data(message);
				if (message->command->helper_function)
					message->command->helper_function(message, message->command->helper_func_arg);
				break;
			case NULL_CMD:
				/*printf("null_cmd, just passing thru\n");*/
				break;

			default:
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": thread_dispatcher()\n\t Hit default case, this SHOULD NOT HAPPEN it's a bug, notify author! \n"));

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
}


/*!
 \brief send_to_ecu() gets called to send a value to the ECU.  This function
 will check if the value sent is NOT the reqfuel_offset (that has special
 interdependancy issues) and then will check if there are more than 1 widgets
 that are associated with this page/offset and update those widgets before
 sending the value to the ECU.
 \param widget (GtkWidget *) pointer to the widget that was modified or NULL
 \param page (gint) page in which the value refers to.
 \param offset (gint) offset from the beginning of the page that this data
 refers to.
 \param value (gint) the value that should be sent to the ECU At page.offset
 \param queue_update (gboolean), if true queues a gui update, used to prevent
 a horrible stall when doing an ECU restore or batch load...
 */
void send_to_ecu(gint canID, gint page, gint offset, DataSize size, gint value, gboolean queue_update)
{
	extern Firmware_Details *firmware;
	OutputData *output = NULL;
	guint8 *data = NULL;

	if (dbg_lvl & SERIAL_WR)
		dbg_func(g_strdup_printf(__FILE__": send_to_ecu()\n\t Sending canID %i, page %i, offset %i, value %i \n",canID,page,offset,value));

	switch (size)
	{
		case MTX_CHAR:
		case MTX_S08:
		case MTX_U08:
		case MTX_S16:
		case MTX_U16:
		case MTX_S32:
		case MTX_U32:
			break;
		default:
			printf("ERROR!!! Size undefined for var at canID %i, page %i, offset %i\n",canID,page,offset);
	}
	output = initialize_outputdata();
	OBJ_SET(output->object,"canID", GINT_TO_POINTER(canID));
	OBJ_SET(output->object,"page", GINT_TO_POINTER(page));
	OBJ_SET(output->object,"truepgnum", GINT_TO_POINTER(firmware->page_params[page]->truepgnum));
	OBJ_SET(output->object,"offset", GINT_TO_POINTER(offset));
	OBJ_SET(output->object,"value", GINT_TO_POINTER(value));
	OBJ_SET(output->object,"size", GINT_TO_POINTER(size));
	OBJ_SET(output->object,"num_bytes", GINT_TO_POINTER(get_multiplier(size)));
	OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_SIMPLE_WRITE));
	/* SPECIAL case for MS2,  as it's write always assume a "datablock"
	 * and it doesn't have a simple easy write api due to it's use of 
	 * different sized vars,  hence the extra complexity.
	 */
	if (firmware->capabilities & MS2)
	{
		/* Get memory */
		data = g_new0(guint8,get_multiplier(size));
		switch (size)
		{
			case MTX_CHAR:
			case MTX_U08:
				data[0] = (guint8)value;
				break;
			case MTX_S08:
				data[0] = (gint8)value;
				break;
			case MTX_S16:
			case MTX_U16:
				data[1] = (guint8)value;
				data[0] = (guint8)((guint16)value >> 8);
				break;
			case MTX_S32:
			case MTX_U32:
				data[3] = (guint8)value;
				data[2] = (guint8)((guint32)value >> 8);
				data[1] = (guint8)((guint32)value >> 16);
				data[0] = (guint8)((guint32)value >> 24);
				break;
			default:
				break;
		}
		OBJ_SET(output->object,"data", (gpointer)data);
	}
	output->need_page_change = TRUE;
	output->queue_update = queue_update;
	io_cmd(firmware->write_command,output);
	return;
}


/*!
 \brief chunk_write() gets called to send a block of values to the ECU.
 \param canID (gint) can identifier (0-14)
 \param page (gint) page in which the value refers to.
 \param offset (gint) offset from the beginning of the page that this data
 refers to.
 \param len (gint) length of block to sent
 \param data (guint8) the block of data to be sent which better damn well be
 int ECU byte order if there is an endianness thing..
 a horrible stall when doing an ECU restore or batch load...
 */
void chunk_write(gint canID, gint page, gint offset, gint num_bytes, guint8 * data)
{
	extern Firmware_Details *firmware;
	OutputData *output = NULL;

	if (dbg_lvl & SERIAL_WR)
		dbg_func(g_strdup_printf(__FILE__": chunk_write()\n\t Sending page %i, offset %i, num_bytes %i, data %p\n",page,offset,num_bytes,data));
	output = initialize_outputdata();
	OBJ_SET(output->object,"canID", GINT_TO_POINTER(canID));
	OBJ_SET(output->object,"page", GINT_TO_POINTER(page));
	OBJ_SET(output->object,"truepgnum", GINT_TO_POINTER(firmware->page_params[page]->truepgnum));
	OBJ_SET(output->object,"offset", GINT_TO_POINTER(offset));
	OBJ_SET(output->object,"num_bytes", GINT_TO_POINTER(num_bytes));
	OBJ_SET(output->object,"data", (gpointer)data);
	OBJ_SET(output->object,"mode", GINT_TO_POINTER(MTX_CHUNK_WRITE));
	output->need_page_change = TRUE;
	output->queue_update = TRUE;
	io_cmd(firmware->chunk_write_command,output);
	return;
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
void  thread_update_logbar(
		gchar * view_name, 
		gchar * tagname, 
		gchar * msg,
		gboolean count,
		gboolean clear)
{

	Io_Message *message = NULL;
	Text_Message *t_message = NULL;
	extern GAsyncQueue *gui_dispatch_queue;
	gint tmp = 0;

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
gboolean queue_function(gchar * name)
{
	Io_Message *message = NULL;
	QFunction *qfunc = NULL;
	extern GAsyncQueue *gui_dispatch_queue;
	gint tmp = 0;

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
void  thread_update_widget(
		gchar * widget_name,
		WidgetType type,
		gchar * msg)
{

	Io_Message *message = NULL;
	Widget_Update *w_update = NULL;
	extern GAsyncQueue *gui_dispatch_queue;
	gint tmp = 0;

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
 \brief start_restore_monitor kicks off a thread to update the tools view
 during an ECU restore to provide user feedback since this is a time
 consuming operation.  if uses message passing over asyncqueues to send the 
 gui update messages.
 */
void start_restore_monitor(void)
{
	GThread * restore_update_thread = NULL;
	restore_update_thread = g_thread_create(restore_update,
			NULL, /* Thread args */
			TRUE, /* Joinable */
			NULL); /*GError Pointer */

}

void *restore_update(gpointer data)
{
	extern GAsyncQueue *io_queue;
	gint max_xfers = g_async_queue_length(io_queue);
	gint remaining_xfers = max_xfers;
	gint last_xferd = max_xfers;

	thread_update_logbar("tools_view","warning",g_strdup_printf("There are %i pending I/O transactions waiting to get to the ECU, please be patient.\n",max_xfers),FALSE,FALSE);
	while (remaining_xfers > 5)
	{
		remaining_xfers = g_async_queue_length(io_queue);
		g_usleep(5000);
		if (remaining_xfers <= (last_xferd-50))
		{
			thread_update_logbar("tools_view",NULL,g_strdup_printf("Approximately %i Transactions remaining, please wait\n",remaining_xfers),FALSE,FALSE);
			last_xferd = remaining_xfers;
		}

	}
	thread_update_logbar("tools_view","info",g_strdup_printf("All Transactions complete\n"),FALSE,FALSE);

	return NULL;
}



/*! 
 \brief build_output_string() is called when doing output to the ECU, to 
 append the needed data together into one nice string for sending
 */
void build_output_string(Io_Message *message, Command *command, gpointer data)
{
	gint i = 0;
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
	block->len = strlen(command->base);
	g_array_append_val(message->sequence,block);

	/* Arguments */
	for (i=0;i<command->args->len;i++)
	{
		arg = g_array_index(command->args,PotentialArg *, i);
		block = g_new0(DBlock, 1);
		if (arg->type == ACTION)
		{
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
				v = (gint)OBJ_GET(output->object,arg->internal_name);
				/*printf("value %i\n",v);*/
				block->data = g_new0(guint8,1);
				block->data[0] = (guint8)v;
				block->len = 1;
				break;
			case MTX_U16:
			case MTX_S16:
				/*printf("16 bit arg %i, name \"%s\"\n",i,arg->internal_name);*/
				block->type = DATA;
				v = (gint)OBJ_GET(output->object,arg->internal_name);
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
				v = (gint)OBJ_GET(output->object,arg->internal_name);
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
					printf("ERROR, MTX_UNDEF, donno what to do!!\n");
				sent_data = (guint8 *)OBJ_GET(output->object,arg->internal_name);
				len = (gint)OBJ_GET(output->object,"num_bytes");
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

