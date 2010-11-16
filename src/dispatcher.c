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

#include <comms.h>
#include <comms_gui.h>
#include <config.h>
#include <conversions.h>
#include <dashboard.h>
#include <dataio.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <dispatcher.h>
#include <enums.h>
#include <gui_handlers.h>
#include <helpers.h>
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
#include <lookuptables.h>
#include <logviewer_gui.h>
#include <menu_handlers.h>
#include <mode_select.h>
#include <notifications.h>
#include <post_process.h>
#include <runtime_gui.h>
#include <runtime_sliders.h>
#include <runtime_text.h>
#include <runtime_status.h>
#include <rtv_map_loader.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <tabloader.h>
#include <timeout_handlers.h>
#include <threads.h>
#include <ms1-t-logger.h>
#include <unistd.h>
#include <widgetmgmt.h>


extern GAsyncQueue *pf_dispatch_queue;
extern GAsyncQueue *gui_dispatch_queue;
extern GCond *pf_dispatch_cond;
extern GCond *gui_dispatch_cond;
static GTimer *ticker;

/*!
 \brief pf_dispatcher() is a GTK+ timeout that runs 10 times per second checking
 for message on the dispatch queue which handles gui operations after a thread
 function runs, This will attempt to handle multiple messages at a time if the
 queue has multiple message queued up.
 \param data (gpointer) unused
 \returns TRUE 
 */
gboolean pf_dispatcher(gpointer data)
{
	gint len=0;
	gint i=0;
	PostFunction *pf=NULL;
	Io_Message *message = NULL;
	extern volatile gboolean leaving;
	extern volatile gboolean might_be_leaving;
	GTimeVal time;

	if (!ticker)
		ticker = g_timer_new();
	else	
		g_timer_start(ticker);

	if (!pf_dispatch_queue) /*queue not built yet... */
	{
		g_cond_signal(pf_dispatch_cond);
		return TRUE;
	}
	if (might_be_leaving)
		return TRUE;
	if (leaving)
	{
		g_cond_signal(pf_dispatch_cond);
		return TRUE;
	}

	g_get_current_time(&time);
	g_time_val_add(&time,5000);
	//message = g_async_queue_timed_pop(pf_dispatch_queue,&time);
	message = g_async_queue_try_pop(pf_dispatch_queue);
	if (!message)
	{
		/*	printf("no messages waiting, returning\n");*/
		g_cond_signal(pf_dispatch_cond);
		return TRUE;
	}
	if (!message->status)
	{
		/* Message failed at some point, do NOT run post functions
		 * in this case.
		 */
		dealloc_message(message);
		g_cond_signal(pf_dispatch_cond);
		return TRUE;
	}

	if (message->command->post_functions != NULL)
	{
		len = message->command->post_functions->len;
		for (i=0;i<len;i++)
		{
			if (leaving)
			{
				dealloc_message(message);
				g_cond_signal(pf_dispatch_cond);
				return TRUE;
			}

			pf = g_array_index(message->command->post_functions,PostFunction *, i);
			/*printf("dispatching post function %s\n",pf->name);*/
			if (!pf)
			{
				printf(_("ERROR postfunction was NULL, continuing\n"));
				continue;
			}
			if (pf->w_arg)
			{
				if (!pf->function_w_arg)
					printf(_("ERROR, couldn't find function with arg \"%s\"\n"),pf->name);
				else
					pf->function_w_arg(message);
			}
			else
			{
				if (!pf->function)
					printf(_("ERROR, couldn't find function \"%s\"\n"),pf->name);
				else
					pf->function();
			}

		}
	}
	dealloc_message(message);
	/*printf ("deallocation of dispatch message complete\n");*/

	gdk_threads_enter();
	while (gtk_events_pending())
	{
		if (leaving)
			goto fast_exit;
		gtk_main_iteration();
	}
	//gdk_flush();
fast_exit:
	gdk_threads_leave();
	g_cond_signal(pf_dispatch_cond);
	g_timer_stop(ticker);
	return TRUE;
}


/*!
 \brief gui_dispatcher() is a GTK+ timeout that runs 30 tiems per second checking
 for message on the dispatch queue which handles gui operations after a thread
 function runs, This will attempt to handle multiple messages at a time if the
 queue has multiple message queued up.
 \param data (gpointer) unused
 \returns TRUE 
 */
gboolean gui_dispatcher(gpointer data)
{
	gint len=0;
	gint i=0;
	UpdateFunction val = 0;
	gint count = 0;
	GtkWidget *widget = NULL;
	Io_Message *message = NULL;
	Text_Message *t_message = NULL;
	Widget_Update *w_update = NULL;
	QFunction *qfunc = NULL;
	extern gconstpointer *global_data;
	extern volatile gboolean leaving;
	extern volatile gboolean might_be_leaving;
	/*extern gint mem_view_style[];*/

	if (!gui_dispatch_queue) /*queue not built yet... */
	{
		g_cond_signal(gui_dispatch_cond);
		return TRUE;
	}
	/* Endless Loop, wait for message, processs and repeat... */
trypop:
	/*printf("gui_dispatch queue length is %i\n",g_async_queue_length(gui_dispatch_queue));*/
	if (leaving)
	{
		g_cond_signal(gui_dispatch_cond);
		return TRUE;
	}
	if (might_be_leaving)
		return TRUE;
	message = g_async_queue_try_pop(gui_dispatch_queue);
	if (!message)
	{
		/*	printf("no messages waiting, returning\n");*/
		g_cond_signal(gui_dispatch_cond);
		return TRUE;
	}

	if (message->functions != NULL)
	{
		len = message->functions->len;
		for (i=0;i<len;i++)
		{
			if (leaving)
			{
				dealloc_message(message);
				g_cond_signal(gui_dispatch_cond);
				return TRUE;
			}

			val = g_array_index(message->functions,UpdateFunction, i);
			/*printf("gui_dispatcher\n");*/
			switch ((UpdateFunction)val)
			{
				case UPD_REFRESH:
					widget = (GtkWidget *)message->payload;
					if (GTK_IS_WIDGET(widget))
					{
						gdk_threads_enter();
						update_widget(widget,NULL);
						gdk_threads_leave();
						message->payload = NULL;
					}
					break;
				case UPD_LOGBAR:
					/*printf("logbar update\n");*/
					t_message = (Text_Message *)message->payload;
					gdk_threads_enter();
					update_logbar(t_message->view_name,t_message->tagname,t_message->msg,t_message->count,t_message->clear,FALSE);
					gdk_threads_leave();
					dealloc_textmessage(t_message);
					message->payload = NULL;
					break;
				case UPD_RUN_FUNCTION:
					/*printf("run function\n");*/
					qfunc = (QFunction *)message->payload;
					gdk_threads_enter();
					run_post_functions(qfunc->func_name);
					gdk_threads_leave();
					dealloc_qfunction(qfunc);
					message->payload = NULL;
					break;

				case UPD_WIDGET:
					/*printf("widget update\n");*/
					widget = NULL;
					w_update = (Widget_Update *)message->payload;
					switch (w_update->type)
					{
						case MTX_ENTRY:
							/*printf("entry\n");*/
							if (NULL == (widget = lookup_widget(w_update->widget_name)))
								break;
							gdk_threads_enter();
							gtk_entry_set_text(GTK_ENTRY(widget),w_update->msg);
							gdk_threads_leave();
							break;
						case MTX_LABEL:
							/*printf("label\n");*/
							if (NULL == (widget = lookup_widget(w_update->widget_name)))
								break;
							gdk_threads_enter();
							gtk_label_set_markup(GTK_LABEL(widget),w_update->msg);
							gdk_threads_leave();
							break;
						case MTX_TITLE:
							/*printf("title\n");*/
							gdk_threads_enter();
							set_title(g_strdup(w_update->msg));
							gdk_threads_leave();
							break;
						case MTX_SENSITIVE:
							/*printf("sensitivity change\n");*/
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
					reset_temps(DATA_GET(global_data,"temp_units"));
					gdk_threads_leave();
					/*
					   case UPD_RAW_MEMORY:
					   update_raw_memory_view(mem_view_style[message->offset],message->offset);
					   break;
					 */
			}

			gdk_threads_enter();
			while (gtk_events_pending())
			{
				if (leaving)
					goto dealloc;
				gtk_main_iteration();
			}
			gdk_flush();
			gdk_threads_leave();
		}
	}
dealloc:
	dealloc_message(message);
	/*printf ("deallocation of dispatch message complete\n");*/
	count++;
	/* try to handle up to 4 messages at a time.  If this is 
	 * set too high, we can cause the timeout to hog the gui if it's
	 * too low, things can fall behind. (GL redraw ;( )
	 * */
	if ((count < 3) && (!leaving))
	{
		/*printf("trying to handle another message\n");*/
		goto trypop;
	}
	/*printf("returning\n");*/
	g_cond_signal(gui_dispatch_cond);
	return TRUE;
}


void *clock_watcher(gpointer data)
{
	while(TRUE)
	{
		g_usleep(500000);
		printf ("Dispatcher time consumed over 0.5 seconds %.2f %%\n",100.0*(g_timer_elapsed(ticker,NULL)/0.5));
		g_timer_reset(ticker);
	}
}
