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
#include <dataio.h>
#include <datalogging_gui.h>
#include <datamgmt.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <mode_select.h>
#include <mtxsocket.h>
#include <notifications.h>
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
	Io_Message *message = NULL;
	GHashTable *commands_hash = NULL;
	Command *command = NULL;
	extern GAsyncQueue *io_data_queue;

	commands_hash = DATA_GET(global_data,"commands_hash");

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
	extern GAsyncQueue *io_data_queue;
	extern GAsyncQueue *pf_dispatch_queue;
	extern GCond * io_dispatch_cond;
//	GTimer *clock;

	dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": thread_dispatcher()\n\tThread created!\n"));

	args = DATA_GET(global_data,"args");
	serial_params = DATA_GET(global_data,"serial_params");
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
G_MODULE_EXPORT void send_to_ecu(gint canID, gint page, gint offset, DataSize size, gint value, gboolean queue_update)
{
	OutputData *output = NULL;
	guint8 *data = NULL;
	guint16 u16 = 0;
	gint16 s16 = 0;
	guint32 u32 = 0;
	gint32 s32 = 0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": send_to_ecu()\n\t Sending canID %i, page %i, offset %i, value %i \n",canID,page,offset,value));

	switch (size)
	{
		case MTX_CHAR:
		case MTX_S08:
		case MTX_U08:
	/*		printf("8 bit var %i at offset %i\n",value,offset);*/
			break;
		case MTX_S16:
		case MTX_U16:
	/*		printf("16 bit var %i at offset %i\n",value,offset);*/
			break;
		case MTX_S32:
		case MTX_U32:
	/*		printf("32 bit var %i at offset %i\n",value,offset);*/
			break;
		default:
			printf(_("ERROR!!! Size undefined for variable at canID %i, page %i, offset %i\n"),canID,page,offset);
	}
	output = initialize_outputdata();
	DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
	DATA_SET(output->data,"value", GINT_TO_POINTER(value));
	DATA_SET(output->data,"size", GINT_TO_POINTER(size));
	DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(get_multiplier(size)));
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_SIMPLE_WRITE));
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
			case MTX_U16:
				if (firmware->bigendian)
					u16 = GUINT16_TO_BE((guint16)value);
				else
					u16 = GUINT16_TO_LE((guint16)value);
				data[0] = (guint8)u16;
				data[1] = (guint8)((guint16)u16 >> 8);
				break;
			case MTX_S16:
				if (firmware->bigendian)
					s16 = GINT16_TO_BE((gint16)value);
				else
					s16 = GINT16_TO_LE((gint16)value);
				data[0] = (guint8)s16;
				data[1] = (guint8)((gint16)s16 >> 8);
				break;
			case MTX_S32:
				if (firmware->bigendian)
					s32 = GINT32_TO_BE((gint32)value);
				else
					s32 = GINT32_TO_LE((gint32)value);
				data[0] = (guint8)s32;
				data[1] = (guint8)((gint32)s32 >> 8);
				data[2] = (guint8)((gint32)s32 >> 16);
				data[3] = (guint8)((gint32)s32 >> 24);
				break;
			case MTX_U32:
				if (firmware->bigendian)
					u32 = GUINT32_TO_BE((guint32)value);
				else
					u32 = GUINT32_TO_LE((guint32)value);
				data[0] = (guint8)u32;
				data[1] = (guint8)((guint32)u32 >> 8);
				data[2] = (guint8)((guint32)u32 >> 16);
				data[3] = (guint8)((guint32)u32 >> 24);
				break;
			default:
				break;
		}
		DATA_SET_FULL(output->data,"data", (gpointer)data,g_free);
	}

	/* Set it here otherwise there's a risk of a missed burn due to 
 	 * a potential race condition in the burn checker
 	 */
	set_ecu_data(canID,page,offset,size,value);
	/* If the ecu is multi-page, run the handler to take care of queing
	 * burns and/or page changing
	 */
	if (firmware->multi_page)
		ms_handle_page_change(page,(gint)DATA_GET(global_data,"last_page"));

	output->queue_update = queue_update;
	io_cmd(firmware->write_command,output);
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	return;
}


G_MODULE_EXPORT void ms_handle_page_change(gint page, gint last)
{
	guint8 **ecu_data = NULL;
	guint8 **ecu_data_last = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ecu_data = firmware->ecu_data;
	ecu_data_last = firmware->ecu_data_last;

	/*printf("handle page change!, page %i, last %i\n",page,last);
	 */

	if (last == -1)  /* First Write of the day, burn not needed... */
	{
		queue_ms1_page_change(page);
		return;
	}
	if ((page == last) && (!DATA_GET(global_data,"force_page_change")))
	{
		/*printf("page == last and force_page_change is not set\n");
 		 */
		return;
	}
	/* If current page is NOT a dl_by_default page, but the last one WAS
	 * then a burn is required otherwise settings will be lost in the
	 * last
	 */
	if ((!firmware->page_params[page]->dl_by_default) && (firmware->page_params[last]->dl_by_default))
	{
		/*printf("current was not dl by default  but last was,  burning\n");
		*/
		queue_burn_ecu_flash(last);
		if (firmware->capabilities & MS1)
			queue_ms1_page_change(page);
		return;
	}
	/* If current page is NOT a dl_by_default page, OR the last one was
	 * not then a burn is NOT required.
	 */
	if ((!firmware->page_params[page]->dl_by_default) || (!firmware->page_params[last]->dl_by_default))
	{
		/*printf("current is not dl by default or last was not as well\n");
		*/
		if ((page != last) && (firmware->capabilities & MS1))
		{
			/*printf("page diff and MS1, changing page\n");
			*/
			queue_ms1_page_change(page);
		}
		return;
	}
	/* If current and last pages are DIFFERENT,  do a memory buffer scan
	 * to see if previous and last match,  if so return, otherwise burn
	 * then change page
	 */
	if (((page != last) && (((memcmp(ecu_data_last[last],ecu_data[last],firmware->page_params[last]->length) != 0)) || ((memcmp(ecu_data_last[page],ecu_data[page],firmware->page_params[page]->length) != 0)))))
	{
		/*printf("page and last don't match AND there's a ram difference, burning, before changing\n");
		*/
		queue_burn_ecu_flash(last);
		if (firmware->capabilities & MS1)
			queue_ms1_page_change(page);
	}
	else if ((page != last) && (firmware->capabilities & MS1))
	{
		/*printf("page and last don't match AND there's a NOT a RAM difference, changing page\n");
		 */
		queue_ms1_page_change(page);
	}
}


G_MODULE_EXPORT void queue_ms1_page_change(gint page)
{
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	if (DATA_GET(global_data,"offline"))
		return;

	output = initialize_outputdata();
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CMD_WRITE));
	io_cmd(firmware->page_command,output);
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
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
G_MODULE_EXPORT void chunk_write(gint canID, gint page, gint offset, gint num_bytes, guint8 * data)
{
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": chunk_write()\n\t Sending canID %i, page %i, offset %i, num_bytes %i, data %p\n",canID,page,offset,num_bytes,data));
	output = initialize_outputdata();
	DATA_SET(output->data,"canID", GINT_TO_POINTER(canID));
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"offset", GINT_TO_POINTER(offset));
	DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(num_bytes));
	DATA_SET_FULL(output->data,"data", (gpointer)data, g_free);
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CHUNK_WRITE));

	/* save it otherwise the burn checker can miss it due to a potential
 	 * race condition
 	 */
	store_new_block(canID,page,offset,data,num_bytes);

	if (firmware->multi_page)
		ms_handle_page_change(page,(gint)DATA_GET(global_data,"last_page"));
	output->queue_update = TRUE;
	io_cmd(firmware->chunk_write_command,output);
	DATA_SET(global_data,"last_page",GINT_TO_POINTER(page));
	return;
}


/*!
 \brief table_write() gets called to send a block of lookuptable values to the ECU
 \param page (tableID) (gint) page in which the value refers to.
 \param len (gint) length of block to sent
 \param data (guint8) the block of data to be sent which better damn well be
 int ECU byte order if there is an endianness thing..
 a horrible stall when doing an ECU restore or batch load...
 */
G_MODULE_EXPORT void table_write(gint page, gint num_bytes, guint8 * data)
{
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	OutputData *output = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	g_static_mutex_lock(&mutex);

	dbg_func(SERIAL_WR,g_strdup_printf(__FILE__": table_write()\n\t Sending page %i, num_bytes %i, data %p\n",page,num_bytes,data));

	output = initialize_outputdata();
	DATA_SET(output->data,"page", GINT_TO_POINTER(page));
	DATA_SET(output->data,"phys_ecu_page", GINT_TO_POINTER(firmware->page_params[page]->phys_ecu_page));
	DATA_SET(output->data,"num_bytes", GINT_TO_POINTER(num_bytes));
	DATA_SET(output->data,"data", (gpointer)data);
	DATA_SET(output->data,"mode", GINT_TO_POINTER(MTX_CHUNK_WRITE));

	/* save it otherwise the burn checker can miss it due to a potential
	 * race condition
	 */
	store_new_block(0,page,0,data,num_bytes);

	if (firmware->multi_page)
		ms_handle_page_change(page,(gint)DATA_GET(global_data,"last_page"));
	output->queue_update = TRUE;
	io_cmd(firmware->table_write_command,output);

	g_static_mutex_unlock(&mutex);
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
G_MODULE_EXPORT void  thread_update_logbar(
		const gchar * view_name, 
		const gchar * tagname, 
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
G_MODULE_EXPORT gboolean queue_function(const gchar * name)
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
G_MODULE_EXPORT void  thread_update_widget(
		const gchar * widget_name,
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
 \brief thread_set_sensitive() is a function to be called from within threads
 to update a widgets sensitivity state.
 \param widget_name (gchar *) textual name of the widget to update
 \param stats (gboolean) the state to set
 */
G_MODULE_EXPORT void thread_widget_set_sensitive(const gchar * widget_name, gboolean state)
{

	Io_Message *message = NULL;
	Widget_Update *w_update = NULL;
	extern GAsyncQueue *gui_dispatch_queue;
	gint tmp = 0;

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

	Io_Message *message = NULL;
	extern GAsyncQueue *gui_dispatch_queue;
	gint tmp = 0;

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
 \brief start_restore_monitor kicks off a thread to update the tools view
 during an ECU restore to provide user feedback since this is a time
 consuming operation.  if uses message passing over asyncqueues to send the 
 gui update messages.
 */
G_MODULE_EXPORT void start_restore_monitor(void)
{
	GThread * restore_update_thread = NULL;
	restore_update_thread = g_thread_create(restore_update,
			NULL, /* Thread args */
			TRUE, /* Joinable */
			NULL); /*GError Pointer */

}

G_MODULE_EXPORT void *restore_update(gpointer data)
{
	extern GAsyncQueue *io_data_queue;
	gint max_xfers = g_async_queue_length(io_data_queue);
	gint remaining_xfers = max_xfers;
	gint last_xferd = max_xfers;

	dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": restore_update()\n\tThread created!\n"));
	thread_update_logbar("tools_view","warning",g_strdup_printf(_("There are %i pending I/O transactions waiting to get to the ECU, please be patient.\n"),max_xfers),FALSE,FALSE);
	while (remaining_xfers > 5)
	{
		remaining_xfers = g_async_queue_length(io_data_queue);
		g_usleep(10000);
		if (remaining_xfers <= (last_xferd-50))
		{
			thread_update_logbar("tools_view",NULL,g_strdup_printf(_("Approximately %i Transactions remaining, please wait\n"),remaining_xfers),FALSE,FALSE);
			last_xferd = remaining_xfers;
		}

	}
	thread_update_logbar("tools_view","info",g_strdup_printf(_("All Transactions complete\n")),FALSE,FALSE);

	dbg_func(THREADS|CRITICAL,g_strdup(__FILE__": restore_update()\n\tThread exiting!\n"));
	return NULL;
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

