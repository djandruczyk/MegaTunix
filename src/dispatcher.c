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
#include <t-logger.h>
#include <unistd.h>


extern GAsyncQueue *pf_dispatch_queue;
extern GAsyncQueue *gui_dispatch_queue;
extern GObject *global_data;


/*!
 \brief dispatcher() is a GTK+ timeout that runs 30 tiems per second checking
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
	gint count = 0;
	Io_Message *message = NULL;
	extern volatile gboolean leaving;

	if (!pf_dispatch_queue) /*queue not built yet... */
		return TRUE;
	/* Endless Loop, wait for message, processs and repeat... */
trypop:
//	printf("pf_dispatch queue length is %i\n",g_async_queue_length(pf_dispatch_queue));
	if (leaving)
		return TRUE;
	message = g_async_queue_try_pop(pf_dispatch_queue);
	if (!message)
	{
	/*	printf("no messages waiting, returning\n");*/
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
				return TRUE;
			}

			pf = g_array_index(message->command->post_functions,PostFunction *, i);
			if (!pf)
			{
				printf("ERROR postfunction was null, continuing\n");
				continue;
			}
			//printf ("Should run function %s, %p\n",pf->name,pf->function);
			if (pf->w_arg)
			{
				if (!pf->function_w_arg)
					printf("ERROR, couldn't find function w arg \"%s\"\n",pf->name);
				else
					pf->function_w_arg(message);
			}
			else
			{
				if (!pf->function)
					printf("ERROR, couldn't find function \"%s\"\n",pf->name);
				else
					pf->function();
			}

			while (gtk_events_pending())
			{
				if (leaving)
				{
					goto dealloc;
				}
				gtk_main_iteration();
			}
		}
	}
dealloc:
	dealloc_message(message);
	/*printf ("deallocation of dispatch message complete\n");*/
	count++;
	/* try to handle up to 10 messages at a time.  If this is 
	 * set too high, we can cause the timeout to hog the gui if it's
	 * too low, things can fall behind. (GL redraw ;( )
	 * */
	if(count < 10)
	{
		//printf("trying to handle another message\n");
		goto trypop;
	}
	//printf("returning\n");
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
	extern volatile gboolean leaving;
	//extern gint mem_view_style[];
	extern GHashTable *dynamic_widgets;

	if (!gui_dispatch_queue) /*queue not built yet... */
		return TRUE;
	/* Endless Loop, wait for message, processs and repeat... */
trypop:
	//printf("gui_dispatch queue length is %i\n",g_async_queue_length(gui_dispatch_queue));
	if (leaving)
		return TRUE;
	message = g_async_queue_try_pop(gui_dispatch_queue);
	if (!message)
	{
	/*	printf("no messages waiting, returning\n");*/
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
				return TRUE;
			}

			val = g_array_index(message->functions,UpdateFunction, i);

			switch ((UpdateFunction)val)
			{
				case UPD_LOGBAR:
					t_message = (Text_Message *)message->payload;
					update_logbar(t_message->view_name,t_message->tagname,t_message->msg,t_message->count,t_message->clear);
					dealloc_textmessage(t_message);
					message->payload = NULL;
					break;
				case UPD_RUN_FUNCTION:
					qfunc = (QFunction *)message->payload;
					run_post_function(qfunc->func_name);
					dealloc_qfunction(qfunc);
					message->payload = NULL;
					break;

				case UPD_WIDGET:
					widget = NULL;
					w_update = (Widget_Update *)message->payload;
					switch (w_update->type)
					{
						case MTX_ENTRY:
							if (NULL == (widget = g_hash_table_lookup(dynamic_widgets,w_update->widget_name)))
								break;
							gtk_entry_set_text(GTK_ENTRY(widget),w_update->msg);
							break;
						case MTX_LABEL:
							if (NULL == (widget = g_hash_table_lookup(dynamic_widgets,w_update->widget_name)))
								break;
							gtk_label_set_text(GTK_LABEL(widget),w_update->msg);
							break;
						case MTX_TITLE:
							set_title(g_strdup(w_update->msg));
							break;
						default:
							break;
					}
					dealloc_w_update(w_update);
					message->payload = NULL;
					break;
					reset_temps(OBJ_GET(global_data,"temp_units"));
					/*
				case UPD_RAW_MEMORY:
					update_raw_memory_view(mem_view_style[message->offset],message->offset);
					break;
					*/
			}

			while (gtk_events_pending())
			{
				if (leaving)
				{
					goto dealloc;
				}
				gtk_main_iteration();
			}
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
	if(count < 3)
	{
		/*printf("trying to handle another message\n");*/
		goto trypop;
	}
	/*printf("returning\n");*/
	return TRUE;
}
