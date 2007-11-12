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
#include <init.h>
#include <interrogate.h>
#include <listmgmt.h>
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
#include <structures.h>
#include <tabloader.h>
#include <timeout_handlers.h>
#include <threads.h>
#include <t-logger.h>
#include <unistd.h>


extern GAsyncQueue *dispatch_queue;
extern gboolean connected;			/* valid connection with MS */
extern volatile gboolean offline;			/* Offline mode */
extern gboolean tabs_loaded;			/* Tabs loaded? */
extern gboolean interrogated;			/* valid detection with MS */
gint statuscounts_id = -1;
gboolean force_page_change = FALSE;


/*!
 \brief dispatcher() is a GTK+ timeout that runs 30 tiems per second checking
 for message on the dispatch queue which handles gui operations after a thread
 function runs, This will attempt to handle multiple messages at a time if the
 queue has multiple message queued up.
 \param data (gpointer) unused
 \returns TRUE 
 */
gboolean dispatcher(gpointer data)
{
	extern Firmware_Details * firmware;
	gint len=0;
	gint i=0;
	gint j=0;
	gint val=-1;
	gint count = 0;
	GtkWidget *widget = NULL;
	Io_Message *message = NULL;
	Text_Message *t_message = NULL;
	Widget_Update *w_update = NULL;
	QFunction *qfunc = NULL;
	extern gint temp_units;
	extern gboolean paused_handlers;
	extern gboolean forced_update;
	extern volatile gboolean leaving;
	extern gint mem_view_style[];
	extern GHashTable *dynamic_widgets;

	if (!dispatch_queue) /*queue not built yet... */
		return TRUE;
	/* Endless Loop, wait for message, processs and repeat... */
trypop:
	//printf("dispatch queue length is %i\n",g_async_queue_length(dispatch_queue));
	if (leaving)
		return TRUE;
	message = g_async_queue_try_pop(dispatch_queue);
	if (!message)
	{
	//	printf("no messages waiting, returning\n");
		return TRUE;
	}

	if (message->funcs != NULL)
	{
		len = message->funcs->len;
		for (i=0;i<len;i++)
		{
			if (leaving)
			{
				dealloc_message(message);
				return TRUE;
			}

			val = g_array_index(message->funcs,UpdateFunction, i);

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
				case UPD_POPULATE_DLOGGER:
					if ((connected) && (interrogated))
					{
						set_title(g_strdup("Populating Datalogger..."));
						populate_dlog_choices();
					}
					break;
				case UPD_LOAD_RT_STATUS:
					if ((connected) && (interrogated))
					{
						set_title(g_strdup("Loading RT Status..."));
						load_status();
						set_title(g_strdup("RT Status Loaded..."));
					}
					break;
				case UPD_LOAD_RT_SLIDERS:
					if ((connected) && (interrogated))
					{
						set_title(g_strdup("Loading RT Sliders..."));
						load_sliders();
						reset_temps(GINT_TO_POINTER(temp_units));
						set_title(g_strdup("RT Sliders Loaded..."));
					}
					break;
				case UPD_LOAD_RT_TEXT:
					if ((connected) && (interrogated))
					{
						set_title(g_strdup("Loading RT Text..."));
						load_rt_text();
						reset_temps(GINT_TO_POINTER(temp_units));
						set_title(g_strdup("RT Text Loaded..."));
					}
					break;
				case UPD_LOAD_REALTIME_MAP:
					if ((interrogated) && ((connected) || (offline)))
					{
						set_title(g_strdup("Loading RT Map..."));
						load_realtime_map();
						set_title(g_strdup("RT Map Loaded..."));
					}
					break;
				case UPD_LOAD_GUI_TABS:
					if ((((connected) || (offline))) && (!tabs_loaded))
					{
						set_title(g_strdup("Loading Gui Tabs..."));
						load_gui_tabs();
						reset_temps(GINT_TO_POINTER(temp_units));

						set_title(g_strdup("Gui Tabs Loaded..."));
					}
					break;
				case UPD_READ_VE_CONST:
					if ((connected) || (offline))
					{
						set_title(g_strdup("Reading VE/Constants..."));
						io_cmd(IO_READ_VE_CONST,NULL);
						set_title(g_strdup("VE/Constants Read..."));
					}
					break;
				case UPD_REENABLE_INTERROGATE_BUTTON:
					if (leaving)
						break;
					if(!offline)
						gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets, "interrogate_button")),TRUE);
					break;
				case UPD_REENABLE_GET_DATA_BUTTONS:
					g_list_foreach(get_list("get_data_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
					break;
				case UPD_START_STATUSCOUNTS:
					if ((connected) && (interrogated))
						statuscounts_id = g_timeout_add(100,(GtkFunction)update_errcounts,NULL);
					break;
				case UPD_START_REALTIME:
					start_tickler(RTV_TICKLER);
					break;
				case UPD_REALTIME:
					if (interrogated)
						update_runtime_vars();
					break;
				case UPD_VE_CONST:
					set_title(g_strdup("Updating Controls..."));
					paused_handlers = TRUE;
					if ((connected) || (offline))
						update_ve_const();

					paused_handlers = FALSE;
					setup_menu_handlers();
					set_title(g_strdup("Ready..."));
					break;
				case UPD_TRIGTOOTHMON:
					crunch_trigtooth_data(message->page);
					update_trigtooth_display(message->page);
					break;
				case UPD_SET_STORE_RED:
					set_group_color(RED,"burners");
					break;
				case UPD_SET_STORE_BLACK:
					set_group_color(BLACK,"burners");
					for (j=0;j<firmware->total_tables;j++)
						set_reqfuel_color(BLACK,j);
					break;
				case UPD_ENABLE_THREE_D_BUTTONS:
					g_list_foreach(get_list("3d_buttons"),set_widget_sensitive,GINT_TO_POINTER(TRUE));
					break;
				case UPD_LOGVIEWER:
					if ((connected) && (interrogated))
						rt_update_logview_traces(FALSE);
					break;
				case UPD_RAW_MEMORY:
					if ((connected) && (interrogated))
						update_raw_memory_view(mem_view_style[message->offset],message->offset);
					break;
				case UPD_DATALOGGER:
					if ((connected) && (interrogated))
						run_datalog();
					break;
				case UPD_COMMS_STATUS:
					update_comms_status();
					break;
				case UPD_WRITE_STATUS:
					update_write_status(message->payload);
					break;
				case UPD_GET_BOOT_PROMPT:
					if (connected)
						io_cmd(IO_GET_BOOT_PROMPT,NULL);
					break;
				case UPD_REBOOT_GET_ERROR:
					if (connected)
						io_cmd(IO_BOOT_READ_ERROR,NULL);
					break;
				case UPD_BURN_MS_FLASH:
					if (connected)
						io_cmd(IO_BURN_MS_FLASH,NULL);
					break;
				case UPD_JUST_BOOT:
					if (connected)
						io_cmd(IO_JUST_BOOT,NULL);
					break;
				case UPD_FORCE_UPDATE:
					forced_update = TRUE;
					break;
				case UPD_FORCE_PAGE_CHANGE:
					force_page_change = TRUE;
					break;
				case UPD_INITIALIZE_DASH:
					initialize_dashboards();
					break;
			}

			gdk_threads_enter();
			while (gtk_events_pending())
			{
				if (leaving)
				{
					gdk_threads_leave();
					goto dealloc;
				}
				gtk_main_iteration();
			}
			gdk_threads_leave();
		}
	}
dealloc:
	dealloc_message(message);
	//printf ("deallocation of dispatch message complete\n");
	count++;
	/* try to handle up to 4 messages at a time.  If this is 
	 * set too high, we can cause the timeout to hog the gui if it's
	 * too low, things can fall behind. (GL redraw ;( )
	 * */
	if(count < 3)
	{
		//printf("trying to handle another message\n");
		goto trypop;
	}
	//printf("returning\n");
	return TRUE;
}
