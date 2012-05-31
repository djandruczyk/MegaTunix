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
  \file src/dispatcher.c
  \ingroup CoreMtx
  \brief Handles calling gui functions from thread contexts via msg passing
  
  There are two dispatcher functions, one for "Post Functions". These are 
  special operations that need to take place after some form of I/O with an
  ECU has happened,  In most cases this may do additional I/O and or gui
  operations that can't be done in a threaded context as only the main
  thread context can do Gui operations safely
  The second dispatcher is the gui_dispatcher and handles things that need to
  be isolated and/or delayed from a thread context, and this is more for things
  like queued widget update/redraws, label changers and so on.
  \author David Andruczyk
  */

#include <conversions.h>
#include <debugging.h>
#include <init.h>
#include <notifications.h>
#include <plugin.h>
#include <tabloader.h>
#include <widgetmgmt.h>

/*!
  \brief pf_dispatcher() is a GTK+ timeout that runs 10 times per second 
  checking for message on the dispatch queue which handles gui operations
  after a thread function runs, This will attempt to handle multiple 
  messages at a time if the queue has multiple message queued up.
  \param data is unused
  \returns TRUE 
  */
G_MODULE_EXPORT gboolean pf_dispatcher(gpointer data)
{
	static GAsyncQueue *pf_dispatch_queue = NULL;
	PostFunction *pf=NULL;
	Io_Message *message = NULL;
	/*GTimeVal time;*/
	extern gconstpointer *global_data;

	MTXDBG(DISPATCHER,_("Entered!\n"));
	if (!pf_dispatch_queue)
		pf_dispatch_queue = (GAsyncQueue *)DATA_GET(global_data,"pf_dispatch_queue");

	g_return_val_if_fail(global_data,FALSE);
	g_async_queue_ref(pf_dispatch_queue);

	if (DATA_GET(global_data,"pf_dispatcher_exit"))
	{
		while (NULL != (message = (Io_Message *)g_async_queue_try_pop(pf_dispatch_queue)))
			dealloc_io_message(message);
		g_async_queue_unref(pf_dispatch_queue);
		return FALSE;
	}

	/*
	g_get_current_time(&time);
	g_time_val_add(&time,50000);
	message = g_async_queue_timed_pop(pf_dispatch_queue,&time);
	*/
	message = (Io_Message *)g_async_queue_try_pop(pf_dispatch_queue);
	if (!message)
	{
		MTXDBG(DISPATCHER,_("No messages waiting, signalling!\n"));
		MTXDBG(DISPATCHER,_("Leaving PF Dispatcher\n"));
		return TRUE;
	}
	if (!message->status)
	{
		/* Message failed at some point, do NOT run post functions
		 * in this case.
		 */
		dealloc_io_message(message);
		return TRUE;
	}

	if (message->command->post_functions != NULL)
	{
		gint len = message->command->post_functions->len;
		for (int i=0;i<len;i++)
		{
			pf = g_array_index(message->command->post_functions,PostFunction *, i);
			if (!pf)
			{
				MTXDBG(DISPATCHER,_("ERROR postfunction was NULL, continuing\n"));
				continue;
			}
			if (pf->name)
				MTXDBG(DISPATCHER,_("dispatching post function %s\n"),pf->name);
			if (pf->w_arg)
			{
				if (!pf->function_w_arg)
					MTXDBG(DISPATCHER,_("ERROR, couldn't find function with arg \"%s\"\n"),pf->name);
				else
					pf->function_w_arg(message);
			}
			else
			{
				if (!pf->function)
					MTXDBG(DISPATCHER,_("ERROR, couldn't find function \"%s\"\n"),pf->name);
				else
					pf->function();
			}

		}
	}
	dealloc_io_message(message);
	MTXDBG(DISPATCHER,_("deallocation of dispatch message complete\n"));

	/* Causes issues on shit machines
	gdk_threads_enter();
	while (gtk_events_pending())
	{
		loops++;
		if (DATA_GET(global_data,"leaving"))
			goto fast_exit;
		gtk_main_iteration();
		printf("loopcount %i\n",loops);
		if (loops > 100)
		{
			goto fast_exit;
		}
		printf("pf_dispatcher, main iteration!\n");
	}
	//gdk_flush();
fast_exit:
	gdk_threads_leave();
	*/
	gdk_threads_enter();
	gdk_flush();
	gdk_threads_leave();
	g_async_queue_unref(pf_dispatch_queue);
	MTXDBG(DISPATCHER,_("Leaving PF Dispatcher\n"));
	return TRUE;
}


/*!
  \brief gui_dispatcher() is a GTK+ timeout that runs 30 tiems per second 
  checking for message on the dispatch queue which handles gui operations 
  after a thread function runs, This will attempt to handle multiple 
  messages at a time if the queue has multiple message queued up.
  \param data is unused
  \returns TRUE 
  */
G_MODULE_EXPORT gboolean gui_dispatcher(gpointer data)
{
	static GAsyncQueue *gui_dispatch_queue = NULL;
	static void (*update_widget_f)(gpointer,gpointer) = NULL;
	static GList ***ecu_widgets = NULL;
	gint count = 0;
	GtkWidget *widget = NULL;
	Gui_Message *message = NULL;
	Text_Message *t_message = NULL;
	Widget_Update *w_update = NULL;
	Widget_Range *range = NULL;
	QFunction *qfunc = NULL;
	extern gconstpointer *global_data;

	if (!gui_dispatch_queue)
		gui_dispatch_queue = (GAsyncQueue *)DATA_GET(global_data,"gui_dispatch_queue");

	g_return_val_if_fail(gui_dispatch_queue,FALSE);
	g_async_queue_ref(gui_dispatch_queue);
trypop:
	if (DATA_GET(global_data,"gui_dispatcher_exit"))
	{
		while (NULL != (message = (Gui_Message *)g_async_queue_try_pop(gui_dispatch_queue)))
			dealloc_gui_message(message);
		g_async_queue_unref(gui_dispatch_queue);
		return FALSE;
	}
	message = (Gui_Message *)g_async_queue_try_pop(gui_dispatch_queue);
	if (!message)
	{
		MTXDBG(DISPATCHER,_("no messages waiting, signalling\n"));
		return TRUE;
	}

	if (message->functions != NULL)
	{
		gint len = message->functions->len;
		for (gint i=0;i<len;i++)
		{
			UpdateFunction val = g_array_index(message->functions,UpdateFunction, i);
			switch ((UpdateFunction)val)
			{
				case UPD_REFRESH:
					MTXDBG(DISPATCHER,_("Single widget update\n"));
					widget = (GtkWidget *)message->payload;
					if (GTK_IS_WIDGET(widget))
					{
						DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(TRUE));
						if (!update_widget_f)
							get_symbol("update_widget",(void **)&update_widget_f);
						gdk_threads_enter();
						update_widget_f(widget,NULL);
						gdk_threads_leave();
						DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(FALSE));
						message->payload = NULL;
					}
					break;
				case UPD_REFRESH_RANGE:
					MTXDBG(DISPATCHER,_("Widget range update\n"));
					range = (Widget_Range *)message->payload;
					if (!range)
						break;
					if (!update_widget_f)
						get_symbol("update_widget",(void **)&update_widget_f);
					if (!ecu_widgets)
						ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
					DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(TRUE));
					gdk_threads_enter();
					for (i=range->offset;i<range->offset +range->len;i++)
					{
						for (gint j=0;j<g_list_length(ecu_widgets[range->page][i]);j++)
							update_widget_f(g_list_nth_data(ecu_widgets[range->page][i],j),NULL);
					}
					gdk_threads_leave();
					DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(FALSE));
					cleanup(range);
					message->payload = NULL;
					break;
				case UPD_LOGBAR:
					MTXDBG(DISPATCHER,_("Logbar update\n"));
					t_message = (Text_Message *)message->payload;
					gdk_threads_enter();
					update_logbar(t_message->view_name,t_message->tagname,t_message->msg,t_message->count,t_message->clear,FALSE);
					gdk_threads_leave();
					dealloc_textmessage(t_message);
					message->payload = NULL;
					break;
				case UPD_RUN_FUNCTION:
					MTXDBG(DISPATCHER,_("Run function\n"));
					qfunc = (QFunction *)message->payload;
					gdk_threads_enter();
					run_post_functions(qfunc->func_name);
					gdk_threads_leave();
					dealloc_qfunction(qfunc);
					message->payload = NULL;
					break;

				case UPD_WIDGET:
					MTXDBG(DISPATCHER,_("Widget update\n"));
					widget = NULL;
					w_update = (Widget_Update *)message->payload;
					switch (w_update->type)
					{
						case MTX_GROUP_COLOR:
							QUIET_MTXDBG(DISPATCHER,_("group color\n"));
							gdk_threads_enter();
							set_group_color(w_update->color,w_update->group_name);
							gdk_threads_leave();
							break;

						case MTX_ENTRY:
							QUIET_MTXDBG(DISPATCHER,_("entry\n"));
							if (NULL == (widget = lookup_widget(w_update->widget_name)))
								break;
							gdk_threads_enter();
							gtk_entry_set_text(GTK_ENTRY(widget),w_update->msg);
							gdk_threads_leave();
							break;
						case MTX_LABEL:
							QUIET_MTXDBG(DISPATCHER,_("label\n"));
							if (NULL == (widget = lookup_widget(w_update->widget_name)))
								break;
							gdk_threads_enter();
							gtk_label_set_markup(GTK_LABEL(widget),w_update->msg);
							gdk_threads_leave();
							break;
						case MTX_TITLE:
							QUIET_MTXDBG(DISPATCHER,_("title\n"));
							gdk_threads_enter();
							set_title(g_strdup(w_update->msg));
							gdk_threads_leave();
							break;
						case MTX_SENSITIVE:
							QUIET_MTXDBG(DISPATCHER,_("sensitivity change\n"));
							if (NULL == (widget = lookup_widget(w_update->widget_name)))
								break;
							gdk_threads_enter();
							gtk_widget_set_sensitive(GTK_WIDGET(widget),w_update->state);
							gdk_threads_leave();
							break;
						default:
							break;
					}
					dealloc_w_update(w_update);
					message->payload = NULL;
					break;
					gdk_threads_enter();
					reset_temps(DATA_GET(global_data,"mtx_temp_units"));
					gdk_threads_leave();
			}

			gdk_threads_enter();
			while (gtk_events_pending())
			{
				if (DATA_GET(global_data,"leaving"))
				{
					/*QUIET_MTXDBG(CRITICAL,_("events_pending_loop, Gui handler, \"leaving\" flag was set, aborting\n"));*/
					goto dealloc;
				}
				gtk_main_iteration();
			}
			gdk_threads_leave();
		}
	}
dealloc:
	dealloc_gui_message(message);
	MTXDBG(DISPATCHER,_("deallocation of dispatch message complete\n"));
	count++;
	/* try to handle up to 4 messages at a time.  If this is 
	 * set too high, we can cause the timeout to hog the gui if it's
	 * too low, things can fall behind. (GL redraw ;( )
	 * */
	if (count < 3)
	{
		MTXDBG(DISPATCHER,_("trying to handle another message\n"));
		goto trypop;
	}
	gdk_threads_enter();
	gdk_flush();
	gdk_threads_leave();
	MTXDBG(DISPATCHER,_("Leaving Gui Dispatcher\n"));
	return TRUE;
}
