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

#define _ISOC99_SOURCE
#include <2d_table_editor.h>
#include <3d_vetable.h>
#include <args.h>
#include <config.h>
#include <combo_loader.h>
#include <conversions.h>
#include <datamgmt.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fileio.h>
#include <firmware.h>
#include "../widgets/gauge.h"
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <init.h>
#include <keyparser.h>
#include <listmgmt.h>
#include <locking.h>
#include <logviewer_core.h>
#include <logviewer_events.h>
#include <logviewer_gui.h>
#include <math.h>
#include <mtxmatheval.h>
#include <offline.h>
#include <mode_select.h>
#include <notifications.h>
#include <post_process.h>
#include <req_fuel.h>
#include <rtv_processor.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <tabloader.h>
#include <ms1-t-logger.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <user_outputs.h>
#include <vetable_gui.h>
#include <vex_support.h>
#include <widgetmgmt.h>


gboolean search_model(GtkTreeModel *, GtkWidget *, GtkTreeIter *);

static gint upd_count = 0;
static gboolean grab_single_cell = FALSE;
static gboolean grab_multi_cell = FALSE;
extern gboolean interrogated;
extern gboolean playback_mode;
extern gchar *delimiter;
extern gint ready;
extern GtkTooltips *tip;
extern GList ***ve_widgets;
extern Serial_Params *serial_params;
extern gint dbg_lvl;
extern GObject *global_data;

gint active_page = -1;
gint active_table = -1;
extern GdkColor red;
extern GdkColor green;
extern GdkColor blue;
extern GdkColor black;
extern GdkColor white;
gboolean paused_handlers = FALSE;
static gboolean err_flag = FALSE;
volatile gboolean leaving = FALSE;


/*!
 \brief leave() is the main shutdown function for MegaTunix. It shuts down
 whatever runnign handlers are still going, deallocates memory and quits
 \param widget (GtkWidget *) unused
 \param data (gpointer) quiet or not quiet, leave mode .quiet doesn't prompt 
 to save anything
 */
EXPORT gboolean leave(GtkWidget *widget, gpointer data)
{
	extern gint pf_dispatcher_id;
	extern gint gui_dispatcher_id;
	extern gint statuscounts_id;
	/*
	extern GThread * ascii_socket_id;
	extern GThread * binary_socket_id;
	extern GThread * control_socket_id;
	*/
	extern GStaticMutex serio_mutex;
	extern GStaticMutex rtv_mutex;
	extern gboolean connected;
	extern gboolean interrogated;
	extern GAsyncQueue *pf_dispatch_queue;
	extern GAsyncQueue *gui_dispatch_queue;
	extern GAsyncQueue *io_data_queue;
	extern GAsyncQueue *io_repair_queue;
	extern Firmware_Details *firmware;
	extern volatile gboolean offline;
	gboolean tmp = TRUE;
	GIOChannel * iochannel = NULL;
	GTimeVal now;
	static GStaticMutex leave_mutex = G_STATIC_MUTEX_INIT;
	gint count = 0;
	CmdLineArgs *args = OBJ_GET(global_data,"args");
	GMutex *mutex = g_mutex_new();
	extern GCond *pf_dispatch_cond;
	extern GCond *gui_dispatch_cond;

	if (!args->be_quiet)
	{
		if(!prompt_r_u_sure())
			return TRUE;
		prompt_to_save();
	}
	if (leaving)
		return TRUE;

	leaving = TRUE;
	/* Stop timeout functions */

	stop_tickler(RTV_TICKLER);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_realtime\n"));
	stop_tickler(TOOTHMON_TICKLER);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_toothmon\n"));
	stop_tickler(TRIGMON_TICKLER);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_trigmon\n"));
	stop_tickler(LV_PLAYBACK_TICKLER);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_lv_playback\n"));

	stop_datalogging();
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_datalogging\n"));

	/* Message to trigger serial repair queue to exit immediately */
	if (io_repair_queue)
		g_async_queue_push(io_repair_queue,&tmp);

	/* Commits any pending data to ECU flash */
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() before burn\n"));
	if ((connected) && (interrogated) && (!offline))
		io_cmd(firmware->burn_all_command,NULL);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after burn\n"));

	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() configuration saved\n"));
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() before leave_mutex\n"));

	g_static_mutex_lock(&leave_mutex);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after leave_mutex\n"));

	save_config();

	/*
	 * Can';t do these until I can get nonblocking socket to behave. 
	 * not sure what I'm doing wrong,  but select loop doesn't detect the 
	 * connection for some reason, so had to go back to blocking mode, thus
	 * the threads sit permanently blocked and can't catch the notify.
	 *
	if (ascii_socket_id)
		g_thread_join(ascii_socket_id);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after ascii socket thread shutdown\n"));
	if (binary_socket_id)
		g_thread_join(binary_socket_id);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after binary socket thread shutdown\n"));
	if (control_socket_id)
		g_thread_join(control_socket_id);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after control socket thread shutdown\n"));
	*/
	if (statuscounts_id)
		g_source_remove(statuscounts_id);
	statuscounts_id = 0;

	if (lookup_widget("dlog_select_log_button"))
		iochannel = (GIOChannel *) OBJ_GET(lookup_widget("dlog_select_log_button"),"data");

	if (iochannel)	
	{
		g_io_channel_shutdown(iochannel,TRUE,NULL);
		g_io_channel_unref(iochannel);
	}
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after iochannel\n"));


	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() before rtv_mutex lock\n"));
	g_static_mutex_lock(&rtv_mutex);  /* <-- this makes us wait */
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after rtv_mutex lock\n"));
	g_static_mutex_unlock(&rtv_mutex); /* now unlock */
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after rtv_mutex UNlock\n"));

	/* This makes us wait until the io queue finishes */
	while ((g_async_queue_length(io_data_queue) > 0) && (count < 30))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() draining I/O Queue,  current length %i\n",g_async_queue_length(io_data_queue)));
		while (gtk_events_pending())
			gtk_main_iteration();
		count++;
	}
	count = 0;
	while ((g_async_queue_length(gui_dispatch_queue) > 0) && (count < 10))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() draining gui Dispatch Queue, current length %i\n",g_async_queue_length(gui_dispatch_queue)));
		g_async_queue_try_pop(gui_dispatch_queue);
		while (gtk_events_pending())
			gtk_main_iteration();
		count++;
	}
	while ((g_async_queue_length(pf_dispatch_queue) > 0) && (count < 10))
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() draining postfunction Dispatch Queue, current length %i\n",g_async_queue_length(pf_dispatch_queue)));
		g_async_queue_try_pop(pf_dispatch_queue);
		while (gtk_events_pending())
			gtk_main_iteration();
		count++;
	}
	g_mutex_lock(mutex);
	if (pf_dispatcher_id)
		g_source_remove(pf_dispatcher_id);
	pf_dispatcher_id = 0;
	g_get_current_time(&now);
	g_time_val_add(&now,250000);
	g_cond_timed_wait(pf_dispatch_cond,mutex,&now);

	if (gui_dispatcher_id)
		g_source_remove(gui_dispatcher_id);
	gui_dispatcher_id = 0;
	g_get_current_time(&now);
	g_time_val_add(&now,250000);
	g_cond_timed_wait(gui_dispatch_cond,mutex,&now);
	g_mutex_unlock(mutex);

	close_serial();
	unlock_serial();

	/* Grab and release all mutexes to get them to relinquish
	*/
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() before serio_mutex lock\n"));
	g_static_mutex_lock(&serio_mutex);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after serio_mutex lock\n"));
	g_static_mutex_unlock(&serio_mutex);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after serio_mutex UNlock\n"));
	/* Free all buffers */
	mem_dealloc();
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() mem deallocated, closing log and exiting\n"));
	close_debug();
	gtk_main_quit();
	return TRUE;
}


/*!
 \brief comm_port_override() is called every time the comm port text entry
 changes. it'll try to open the port and if it does it'll setup the serial 
 parameters
 \param editable (GtkEditable *) pointer to editable widget to extract text from
 \returns TRUE
 */
gboolean comm_port_override(GtkEditable *editable)
{
	gchar *port;

	port = gtk_editable_get_chars(editable,0,-1);
	gtk_widget_modify_text(GTK_WIDGET(editable),GTK_STATE_NORMAL,&black);
	g_free(OBJ_GET(global_data,"override_port"));
	OBJ_SET(global_data,"override_port",g_strdup(port));
	g_free(port);
	close_serial();
	unlock_serial();
	/* I/O thread should detect the closure and spawn the serial
	 * repair thread which should take care of flipping to the 
	 * overridden port
	 */
	return TRUE;
}


/*!
 \brief toggle_button_handler() handles all toggle buttons in MegaTunix
 \param widget (GtkWidget *) the toggle button that changes
 \param data (gpointer) unused in most cases
 \returns TRUE
 */
EXPORT gboolean toggle_button_handler(GtkWidget *widget, gpointer data)
{
	void *obj_data = NULL;
	gint handler = 0; 
	gchar * tmpbuf = NULL;
	extern gint preferred_delimiter;
	extern gchar *offline_firmware_choice;
	extern gboolean forced_update;
	extern gboolean *tracking_focus;

	if (GTK_IS_OBJECT(widget))
	{
		obj_data = (void *)OBJ_GET(widget,"data");
		handler = (ToggleButton)OBJ_GET(widget,"handler");
	}
	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);
	/*printf("toggle_handler for %s\n",(gchar *)glade_get_widget_name(widget));*/

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		switch ((ToggleButton)handler)
		{
			case TOGGLE_NETMODE:
				OBJ_SET(global_data,"network_access",GINT_TO_POINTER(TRUE));
				open_tcpip_sockets();
				break;
			case COMM_AUTODETECT:
				OBJ_SET(global_data,"autodetect_port", GINT_TO_POINTER(TRUE));
				gtk_entry_set_editable(GTK_ENTRY(lookup_widget("active_port_entry")),FALSE);
				break;
			case OFFLINE_FIRMWARE_CHOICE:
				if(offline_firmware_choice)
					g_free(offline_firmware_choice);
				offline_firmware_choice = g_strdup(OBJ_GET(widget,"filename"));	
				break;
			case TRACKING_FOCUS:
				tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
				tracking_focus[(gint)strtol(tmpbuf,NULL,10)] = TRUE;
				break;
			case TOOLTIPS_STATE:
				gtk_tooltips_enable(tip);
				OBJ_SET(global_data,"tips_in_use",GINT_TO_POINTER(TRUE));
				break;
			case FAHRENHEIT:
				OBJ_SET(global_data,"temp_units",GINT_TO_POINTER(FAHRENHEIT));
				reset_temps(GINT_TO_POINTER(FAHRENHEIT));
				forced_update = TRUE;
				break;
			case CELSIUS:
				OBJ_SET(global_data,"temp_units",GINT_TO_POINTER(CELSIUS));
				reset_temps(GINT_TO_POINTER(CELSIUS));
				forced_update = TRUE;
				break;
			case COMMA:
				preferred_delimiter = COMMA;
				update_logbar("dlog_view", NULL,_("Setting Log delimiter to a \"Comma\"\n"),FALSE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup(",");
				break;
			case TAB:
				preferred_delimiter = TAB;
				update_logbar("dlog_view", NULL,_("Setting Log delimiter to a \"Tab\"\n"),FALSE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup("\t");
				break;
			case REALTIME_VIEW:
				set_logviewer_mode(LV_REALTIME);
				break;
			case PLAYBACK_VIEW:
				set_logviewer_mode(LV_PLAYBACK);
				break;
			case HEX_VIEW:
			case DECIMAL_VIEW:
			case BINARY_VIEW:
				update_raw_memory_view((ToggleButton)handler,(GINT)obj_data);
				break;	
			default:
				break;
		}
	}
	else
	{	/* not pressed */
		switch ((ToggleButton)handler)
		{
			case TOGGLE_NETMODE:
				OBJ_SET(global_data,"network_access",GINT_TO_POINTER(FALSE));
				break;
			case COMM_AUTODETECT:
				OBJ_SET(global_data,"autodetect_port", GINT_TO_POINTER(FALSE));
				gtk_entry_set_editable(GTK_ENTRY(lookup_widget("active_port_entry")),TRUE);
				gtk_entry_set_text(GTK_ENTRY(lookup_widget("active_port_entry")),OBJ_GET(global_data,"override_port"));
				break;
			case TRACKING_FOCUS:
				tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
				tracking_focus[(gint)strtol(tmpbuf,NULL,10)] = FALSE;
				break;
			case TOOLTIPS_STATE:
				gtk_tooltips_disable(tip);
				OBJ_SET(global_data,"tips_in_use",GINT_TO_POINTER(FALSE));
				break;
			default:
				break;
		}
	}
	return TRUE;
}


/*!
 \brief bitmask_button_handler() handles all controls that refer to a group
 of bits in a variable (i.e. a var shared by multiple controls),  most commonly
 used for check/radio buttons referring to features that control single
 bits in the firmware
 \param widget (Gtkwidget *) widget being changed
 \param data (gpointer) not really used 
 \returns TRUE
 */
EXPORT gboolean bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	gint bitshift = -1;
	gint bitval = -1;
	gint bitmask = -1;
	gint dload_val = -1;
	gint canID = 0;
	gint page = -1;
	gint tmp = 0;
	gint tmp32 = 0;
	gint offset = -1;
	DataSize size = 0;
	gint dl_type = -1;
	gint handler = 0;
	gint table_num = -1;
	Deferred_Data *d_data = NULL;
	gchar * swap_list = NULL;
	gchar * set_labels = NULL;
	gchar * table_2_update = NULL;
	gchar * group_2_update = NULL;
	extern gint dbg_lvl;
	extern GHashTable **interdep_vars;
	extern GHashTable *sources_hash;
	extern Firmware_Details *firmware;

	if ((paused_handlers) || (!ready))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);

	canID = (GINT)OBJ_GET(widget,"canID");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	size = (DataSize)OBJ_GET(widget,"size");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	bitval = (GINT)OBJ_GET(widget,"bitval");
	bitmask = (GINT)OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift(bitmask);
	handler = (GINT)OBJ_GET(widget,"handler");
	swap_list = (gchar *)OBJ_GET(widget,"swap_labels");
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	group_2_update = (gchar *)OBJ_GET(widget,"group_2_update");
	table_2_update = (gchar *)OBJ_GET(widget,"table_2_update");


	/* If it's a check button then it's state is dependant on the button's state*/
	if (!GTK_IS_RADIO_BUTTON(widget))
		bitval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	switch ((MtxButton)handler)
	{
		case MULTI_EXPRESSION:
			/*printf("MULTI_EXPRESSION CHANGE\n");*/
			if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_value")))
			{
		/*		printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),(gchar *)OBJ_GET(widget,"source_value"));*/
				g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(OBJ_GET(widget,"source_value")));
			}
			/* FAll Through */
		case GENERIC:
			tmp = get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;	/*clears bits */
			tmp = tmp | (bitval << bitshift);
			dload_val = tmp;
			if (dload_val == get_ecu_data(canID,page,offset,size))
				return FALSE;
			break;
		case DEBUG_LEVEL:
			/* Debugging selection buttons */
			tmp32 = dbg_lvl;
			tmp32 = tmp32 & ~bitmask;
			tmp32 = tmp32 | (bitval << bitshift);
			dbg_lvl = tmp32;
			break;

		case ALT_SIMUL:
			/* Alternate or simultaneous */
			if (firmware->capabilities & MSNS_E)
			{
				table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
				tmp = get_ecu_data(canID,page,offset,size);
				tmp = tmp & ~bitmask;/* clears bits */
				tmp = tmp | (bitval << bitshift);
				dload_val = tmp;
				/*printf("ALT_SIMUL, MSnS-E, table num %i, dload_val %i, curr ecu val %i\n",table_num,dload_val, get_ecu_data(canID,page,offset,size));*/
				if (dload_val == get_ecu_data(canID,page,offset,size))
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				/*printf("last alt %i, cur alt %i\n",firmware->rf_params[table_num]->last_alternate,firmware->rf_params[table_num]->alternate);*/

				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);
				check_req_fuel_limits(table_num);
			}
			else
			{
				table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
				dload_val = bitval;
				if (dload_val == get_ecu_data(canID,page,offset,size))
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);
				check_req_fuel_limits(table_num);
			}
			break;
		default:
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": bitmask_button_handler()\n\tbitmask button at page: %i, offset %i, NOT handled\n\tERROR!!, contact author\n",page,offset));
			return FALSE;
			break;

	}

	/* Swaps the label of another control based on widget state... */
	if ((set_labels) && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		set_widget_labels(set_labels);
	if (swap_list)
		swap_labels(swap_list,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	/* MUST use dispatcher, as the update functions run outside of the
	 * normal GTK+ context, so if we were to call it direct we'd get a 
	 * deadlock due to gtk_threads_enter/leave() calls,  so we use the
	 * dispatch queue to let it run in the correct "state"....
	 */
	if (table_2_update)
		g_timeout_add(2000,force_update_table,table_2_update);
	
	/* Update controls that are dependant on a controls state...
	 * In this case, MAP sensor related ctrls */
	if (group_2_update)
	{
		g_timeout_add(2000,force_view_recompute,NULL);
		g_timeout_add(2000,trigger_group_update,group_2_update);
	}

	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download(widget,dload_val);
		send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	return TRUE;
}


/*!
 \brief entry_changed_handler() gets called anytime a text entries is changed
 (i.e. during edit) it's main purpose is to turn the entry red to signify
 to the user it's being modified but not yet SENT to the ecu
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
EXPORT gboolean entry_changed_handler(GtkWidget *widget, gpointer data)
{
	if ((paused_handlers) || (!ready))
		return TRUE;

	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	OBJ_SET(widget,"not_sent",GINT_TO_POINTER(TRUE));

	return TRUE;
}


/*!
 \brief focus_out_handler() auto-sends data IF IT IS CHANGED to the ecu thus
 hopefully ending the user confusion about why data isn't sent.
 \param widget (GtkWidget *) the widget being modified
 \param event (GdkEvent *) not used
 \param data (gpointer) not used
 \returns FALSE
 */
EXPORT gboolean focus_out_handler(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	if (OBJ_GET(widget,"not_sent"))
	{
		OBJ_SET(widget,"not_sent",NULL);
		std_entry_handler(widget, data);
	}
	return FALSE;
}


/*!
 * \brief slider_value_changed() handles controls based upon a slider
 * sort of like spinbutton controls
 */
EXPORT gboolean slider_value_changed(GtkWidget *widget, gpointer data)
{
	gint page = 0;
	gint offset = 0;
	DataSize size = 0;
	gint canID = 0;
	MtxButton handler = -1;
	gint dl_type = -1;
	gfloat value = 0.0;
	gint dload_val = 0;

	handler = (MtxButton)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	size = (DataSize)OBJ_GET(widget,"size");
	canID = (GINT)OBJ_GET(widget,"canID");
	
	value = gtk_range_get_value(GTK_RANGE(widget));
	dload_val = convert_before_download(widget,value);

	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != get_ecu_data(canID,page,offset,size))
			send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	return FALSE; /* Let other handlers run! */
}


/*!
 \brief std_entry_handler() gets called when a text entries is "activated"
 i.e. when the user hits enter. This function extracts the data, converts it
 to a number (checking for base10 or base16) performs the proper conversion
 and send it to the ECU and updates the gui to the closest value if the user
 didn't enter in an exact value.
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
EXPORT gboolean std_entry_handler(GtkWidget *widget, gpointer data)
{
	gint handler = -1;
	gchar *text = NULL;
	gchar *tmpbuf = NULL;
	gfloat tmpf = -1;
	gfloat value = -1;
	gint table_num = -1;
	gint tmpi = -1;
	gint tmp = -1;
	gint page = -1;
	gint canID = 0;
	gint base = -1;
	gint old = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint dl_type = -1;
	gint precision = -1;
	gint spconfig_offset = -1;
	gint oddfire_bit_offset = -1;
	gint temp_units = 0;
	gfloat scaler = 0.0;
	gboolean temp_dep = FALSE;
	gfloat real_value = 0.0;
	gboolean use_color = FALSE;
	DataSize size = 0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	GdkColor color;
	extern Firmware_Details *firmware;

	if ((paused_handlers) || (!ready))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	temp_units = (GINT)OBJ_GET(global_data,"temp_units");
	temp_dep = (GBOOLEAN)OBJ_GET(widget,"temp_dep");
	handler = (MtxButton)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	canID = (GINT)OBJ_GET(widget,"canID");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	if (!OBJ_GET(widget,"size"))
		size = MTX_U08 ; 	/* default! */
	else
		size = (DataSize)OBJ_GET(widget,"size");
	if (OBJ_GET(widget,"raw_lower"))
		raw_lower = (gint)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		raw_lower = get_extreme_from_size(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		raw_upper = (gint)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		raw_upper = get_extreme_from_size(size,UPPER);
	if (!OBJ_GET(widget,"base"))
		base = 10;
	else
		base = (GINT)OBJ_GET(widget,"base");
	precision = (GINT)OBJ_GET(widget,"precision");
	use_color = (GBOOLEAN)OBJ_GET(widget,"use_color");
	if (use_color)
		if (OBJ_GET(widget,"table_num"))
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);

	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpi = (gint)strtol(text,NULL,base);
	tmpf = (gfloat)g_ascii_strtod(g_strdelimit(text,",.",'.'),NULL);
	/*
	 * printf("base \"%i\", text \"%s\" int val \"%i\", float val \"%f\" precision %i \n",base,text,tmpi,tmpf,precision);
	 */
	
	g_free(text);
	/* This isn't quite correct, as the base can either be base10 
	 * or base16, the problem is the limits are in base10
	 */

	if ((tmpf != (gfloat)tmpi) && (precision == 0))
	{
		/* Pause signals while we change the value */
		/*		printf("resetting\n");*/
		g_signal_handlers_block_by_func (widget,(gpointer)std_entry_handler, data);
		g_signal_handlers_block_by_func (widget,(gpointer)entry_changed_handler, data);
		tmpbuf = g_strdup_printf("%i",tmpi);
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		g_free(tmpbuf);
		g_signal_handlers_unblock_by_func (widget,(gpointer)entry_changed_handler, data);
		g_signal_handlers_unblock_by_func (widget,(gpointer)std_entry_handler, data);
	}
	switch ((MtxButton)handler)
	{
		case GENERIC:
			if (temp_dep)
			{
				if (temp_units == CELSIUS)
					value = (tmpf*(9.0/5.0))+32;
				else
					value = tmpf;
			}
			else
				value = tmpf;
			if (base == 10)
			{
				dload_val = convert_before_download(widget,value);
			}
			else if (base == 16)
				dload_val = convert_before_download(widget,tmpi);
			else
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": std_entry_handler()\n\tBase of textentry \"%i\" is invalid!!!\n",base));
				return TRUE;
			}
			/* What we are doing is doing the forward/reverse 
			 * conversion which will give us an exact value 
			 * if the user inputs something in between,  thus 
			 * we can reset the display to a sane value...
			 */
			old = get_ecu_data(canID,page,offset,size);
			set_ecu_data(canID,page,offset,size,dload_val);

			real_value = convert_after_upload(widget);
			set_ecu_data(canID,page,offset,size,old);

			g_signal_handlers_block_by_func (widget,(gpointer) std_entry_handler, data);
			g_signal_handlers_block_by_func (widget,(gpointer) entry_changed_handler, data);
			
				if (base == 10)
				{
					tmpbuf = g_strdup_printf("%1$.*2$f",real_value,precision);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);
				}
				else
				{
					tmpbuf = g_strdup_printf("%.2X",(gint)real_value);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);
				}
			g_signal_handlers_unblock_by_func (widget,(gpointer) entry_changed_handler, data);
			g_signal_handlers_unblock_by_func (widget,(gpointer) std_entry_handler, data);
			break;

		case TRIGGER_ANGLE:
			spconfig_offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			if (spconfig_offset == 0)
			{
				dbg_func(CRITICAL,g_strdup(__FILE__": std_entry_handler()\n\tERROR Trigger Angle entry call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (tmpf > 112.15)	/* Extra long trigger needed */	
			{
				tmp = get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);	/* Set xlong_trig */
				send_to_ecu(canID, page, spconfig_offset, size, tmp,  TRUE);
				tmpf -= 45.0;
				dload_val = convert_before_download(widget,tmpf);
			}
			else if (tmpf > 89.65) /* Long trigger needed */
			{
				tmp = get_ecu_data(canID,page,spconfig_offset,size);;
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);	/* Set long_trig */
				send_to_ecu(canID, page, spconfig_offset, size, tmp, TRUE);
				tmpf -= 22.5;
				dload_val = convert_before_download(widget,tmpf);
			}
			else	/* tmpf <= 89.65 degrees, no long trigger*/
			{
				tmp = get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				send_to_ecu(canID, page, spconfig_offset, size, tmp, TRUE);
				dload_val = convert_before_download(widget,tmpf);
			}

			break;

		case ODDFIRE_ANGLE:
			oddfire_bit_offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
			{
				dbg_func(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\tERROR Offset Angle spinbutton call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (tmpf > 90)	/*  */	
			{
				tmp = get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);	/* Set +90 */
				send_to_ecu(canID, page, oddfire_bit_offset, size, tmp, TRUE);
				tmpf -= 90.0;
				dload_val = convert_before_download(widget,tmpf);
			}
			else if (tmpf > 45) /* */
			{
				tmp = get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);	/* Set +45 */
				send_to_ecu(canID, page, oddfire_bit_offset, size, tmp, TRUE);
				tmpf -= 45.0;
				dload_val = convert_before_download(widget,tmpf);
			}
			else	/* tmpf <= 45 degrees, */
			{
				tmp = get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				send_to_ecu(canID, page, oddfire_bit_offset, size, tmp,  TRUE);
				dload_val = convert_before_download(widget,tmpf);
			}

			break;
		default:
			/* We don't care about anything else for now... */
			break;

	}


	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != get_ecu_data(canID,page,offset,size))
			send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}

	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);

	if (use_color)
	{
		if (table_num >= 0)
		{
			if (firmware->table_params[table_num]->color_update == FALSE)
			{
				recalc_table_limits(canID,table_num);
				if ((firmware->table_params[table_num]->last_z_maxval != firmware->table_params[table_num]->z_maxval) || (firmware->table_params[table_num]->last_z_minval != firmware->table_params[table_num]->z_minval))
					firmware->table_params[table_num]->color_update = TRUE;
				else
					firmware->table_params[table_num]->color_update = FALSE;
			}

			scaler = 256.0/((firmware->table_params[table_num]->z_maxval - firmware->table_params[table_num]->z_minval)*1.05);
			color = get_colors_from_hue(256 - (dload_val - firmware->table_params[table_num]->z_minval)*scaler, 0.50, 1.0);
		}
		else
		{
			color = get_colors_from_hue(((gfloat)(dload_val-raw_lower)/raw_upper)*-300.0+180, 0.50, 1.0);
		}
		gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	
	}

	OBJ_SET(widget,"not_sent",GINT_TO_POINTER(FALSE));
	return TRUE;
}


/*!
 \brief std_button_handler() handles all standard non toggle/check/radio
 buttons. 
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
EXPORT gboolean std_button_handler(GtkWidget *widget, gpointer data)
{
	/* get any datastructures attached to the widget */
	void *obj_data = NULL;
	gint handler = -1;
	gint tmpi = 0;
	gint tmp2 = 0;
	gint page = 0;
	gint offset = 0;
	gint canID = 0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	DataSize size = 0;
	gfloat tmpf = 0.0;
	gchar * tmpbuf = NULL;
	gchar * dest = NULL;
	gboolean restart = FALSE;
	extern gint realtime_id;
	extern volatile gboolean offline;
	extern gboolean forced_update;
	extern Firmware_Details *firmware;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	obj_data = (void *)OBJ_GET(widget,"data");
	handler = (StdButton)OBJ_GET(widget,"handler");

	if (handler == 0)
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": std_button_handler()\n\thandler not bound to object, CRITICAL ERROR, aborting\n"));
		return FALSE;
	}

	switch ((StdButton)handler)
	{
		case PHONE_HOME:
			printf("Phone home (callback TCP) is not yet implemented, ask nicely\n");
			break;
		case INCREMENT_VALUE:
		case DECREMENT_VALUE:
			dest = OBJ_GET(widget,"partner_widget");
			tmp2 = (GINT)OBJ_GET(widget,"amount");
			if (OBJ_GET(dest,"raw_lower"))
				raw_lower = (gint)strtol(OBJ_GET(dest,"raw_lower"),NULL,10);
			else
				raw_lower = get_extreme_from_size(size,LOWER);
			if (OBJ_GET(dest,"raw_upper"))
				raw_upper = (gint)strtol(OBJ_GET(dest,"raw_upper"),NULL,10);
			else
				raw_upper = get_extreme_from_size(size,UPPER);
			canID = (GINT)OBJ_GET(dest,"canID");
			page = (GINT)OBJ_GET(dest,"page");
			size = (DataSize)OBJ_GET(dest,"size");
			offset = (GINT)OBJ_GET(dest,"offset");
			tmpi = get_ecu_data(canID,page,offset,size);
			if (handler == INCREMENT_VALUE)
				tmpi = tmpi+tmp2 > raw_upper? raw_upper:tmpi+tmp2;
			else 
				tmpi = tmpi-tmp2 < raw_lower? raw_lower:tmpi-tmp2;
			send_to_ecu(canID, page, offset, size, tmpi, TRUE);
			break;
		case GET_CURR_TPS:
			tmpbuf = OBJ_GET(widget,"source");
			lookup_current_value(tmpbuf,&tmpf);
			dest = OBJ_GET(widget,"dest_widget");
			tmpbuf = g_strdup_printf("%.0f",tmpf);
			gtk_entry_set_text(GTK_ENTRY(lookup_widget(dest)),tmpbuf);
			g_signal_emit_by_name(lookup_widget(dest),"activate",NULL);

			g_free(tmpbuf);
			break;

		case EXPORT_SINGLE_TABLE:
			if (OBJ_GET(widget,"table_num"))
				select_table_for_export((gint)strtol(OBJ_GET(widget,"table_num"),NULL,10));
			break;
		case IMPORT_SINGLE_TABLE:
			if (OBJ_GET(widget,"table_num"))
				select_table_for_import((gint)strtol(OBJ_GET(widget,"table_num"),NULL,10));
			break;
		case RESCALE_TABLE:
			rescale_table(widget);
			break;
		case REQFUEL_RESCALE_TABLE:
			reqfuel_rescale_table(widget);
			break;
		case INTERROGATE_ECU:
			set_title(g_strdup(_("User initiated interrogation...")));
			update_logbar("interr_view","warning",_("USER Initiated ECU interrogation...\n"),FALSE,FALSE);
			widget = lookup_widget("interrogate_button");
			if (GTK_IS_WIDGET(widget))
				gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
			io_cmd("interrogation", NULL);
			break;
		case START_PLAYBACK:
			start_tickler(LV_PLAYBACK_TICKLER);
			break;
		case STOP_PLAYBACK:
			stop_tickler(LV_PLAYBACK_TICKLER);
			break;

		case START_REALTIME:
			if (offline)
				break;
			if (!interrogated)
				io_cmd("interrogation", NULL);
			start_tickler(RTV_TICKLER);
			forced_update = TRUE;
			break;
		case STOP_REALTIME:
			if (offline)
				break;
			stop_tickler(RTV_TICKLER);
			break;
		case REBOOT_GETERR:
			if (offline)
				break;
			if (realtime_id > 0)
			{
				stop_tickler(RTV_TICKLER);
				restart = TRUE;
			}
			gtk_widget_set_sensitive(widget,FALSE);
			io_cmd("ms1_extra_reboot_get_error",NULL);
			if (restart)
				start_tickler(RTV_TICKLER);
			break;
		case READ_VE_CONST:
			set_title(g_strdup(_("Reading VE/Constants...")));
			io_cmd(firmware->get_all_command, NULL);
			break;
		case READ_RAW_MEMORY:
			if (offline)
				break;
			/*io_cmd(firmware->raw_mem_command,(gpointer)obj_data);*/
			break;
		case BURN_MS_FLASH:
			io_cmd(firmware->burn_all_command,NULL);
			break;
		case DLOG_SELECT_ALL:
			dlog_select_all();
			break;
		case DLOG_DESELECT_ALL:
			dlog_deselect_all();
			break;
		case DLOG_SELECT_DEFAULTS:
			dlog_select_defaults();
			break;
		case CLOSE_LOGFILE:
			if (offline)
				break;
			stop_datalogging();
			break;
		case START_DATALOGGING:
			if (offline)
				break;
			start_datalogging();
			break;
		case STOP_DATALOGGING:
			if (offline)
				break;
			stop_datalogging();
			break;
		case REVERT_TO_BACKUP:
			revert_to_previous_data();
			break;
		case SELECT_PARAMS:
			if (!interrogated)
				break;
			gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("logviewer_select_logfile_button")),FALSE);
			present_viewer_choices();
			break;
		case REQ_FUEL_POPUP:
			reqd_fuel_popup(widget);
			req_fuel_change(widget);
			break;
		case OFFLINE_MODE:
			set_title(g_strdup(_("Offline Mode...")));
			set_offline_mode();
			break;
		case TE_TABLE:
			if (OBJ_GET(widget,"te_table_num"))
				create_2d_table_editor((gint)strtol(OBJ_GET(widget,"te_table_num"),NULL,10), NULL);
			break;
		case TE_TABLE_GROUP:
			create_2d_table_editor_group(widget);
			break;

		default:
			dbg_func(CRITICAL,g_strdup(__FILE__": std_button_handler()\n\t Standard button not handled properly, BUG detected\n"));
	}		
	return TRUE;
}


/*!
 \brief std_combo_handler() handles all combo boxes
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
EXPORT gboolean std_combo_handler(GtkWidget *widget, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	gboolean state = FALSE;
	gint handler = 0; 
	gint bitmask = 0;
	gint bitshift = 0;
	gint total = 0;
	guchar bitval = 0;
	gchar * set_labels = NULL;
	gchar * swap_list = NULL;
	gchar * tmpbuf = NULL;
	gchar * table_2_update = NULL;
	gchar * group_2_update = NULL;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * dl_conv = NULL;
	gchar * ul_conv = NULL;
	gint precision = 0;
	gchar ** vector = NULL;
	guint i = 0;
	gint tmpi = 0;
	gint page = 0;
	gint offset = 0;
	gint canID = 0;
	gint table_num = 0;
	gchar * range = NULL;
	DataSize size = MTX_U08;
	gchar * choice = NULL;
	guint8 tmp = 0;
	gint dload_val = 0;
	gint dl_type = 0;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	Deferred_Data *d_data = NULL;
	GtkWidget *tmpwidget = NULL;
	void *eval = NULL;
	extern Firmware_Details *firmware;
	extern GHashTable **interdep_vars;
	extern GHashTable *sources_hash;

	if ((paused_handlers) || (!ready))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	page = (GINT) OBJ_GET(widget,"page");
	offset = (GINT) OBJ_GET(widget,"offset");
	bitmask = (GINT) OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift(bitmask);
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	handler = (GINT) OBJ_GET(widget,"handler");
	canID = (GINT)OBJ_GET(widget,"canID");
	size = (DataSize)OBJ_GET(widget,"size");
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	swap_list = (gchar *)OBJ_GET(widget,"swap_labels");
	table_2_update = (gchar *)OBJ_GET(widget,"table_2_update");
	group_2_update = (gchar *)OBJ_GET(widget,"group_2_update");

	state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (state == 0)	
	{
		/* Not selected by combo popdown button, thus is being edited. 
		 * Do a model scan to see if we actually hit the jackpot or 
		 * not, and get the iter for it...
		 */
		if (!search_model(model,widget,&iter))
			return FALSE;
	}
	gtk_tree_model_get(model,&iter,CHOICE_COL,&choice, \
			BITVAL_COL,&bitval,-1);

	/*printf("choice %s, bitmask %i, bitshift %i bitval %i\n",choice,bitmask,bitshift, bitval );*/
	switch ((MtxButton)handler)
	{
		case MULTI_EXPRESSION:
			printf("combo MULTI EXPRESSION\n");
			if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_values")))
			{
				tmpbuf = OBJ_GET(widget,"source_values");
				vector = g_strsplit(tmpbuf,",",-1);
				if ((guint)gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) >= g_strv_length(vector))
				{
					printf("combo size doesn't match source_values for multi_expression\n");
					return FALSE;
				}
				printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]);
				g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]));
				g_timeout_add(2000,update_multi_expression,NULL);
			}
		case GENERIC:
			tmp = get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;	/*clears bits */
			tmp = tmp | (bitval << bitshift);
			dload_val = tmp;
			if (dload_val == get_ecu_data(canID,page,offset,size))
				return FALSE;
			break;
		case ALT_SIMUL:
			/* Alternate or simultaneous */
			if (firmware->capabilities & MSNS_E)
			{
				table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
				tmp = get_ecu_data(canID,page,offset,size);
				tmp = tmp & ~bitmask;/* clears bits */
				tmp = tmp | (bitval << bitshift);
				dload_val = tmp;
				/*printf("ALT_SIMUL, MSnS-E, table num %i, dload_val %i, curr ecu val %i\n",table_num,dload_val, get_ecu_data(canID,page,offset,size));*/
				if (dload_val == get_ecu_data(canID,page,offset,size))
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				/*printf("last alt %i, cur alt %i\n",firmware->rf_params[table_num]->last_alternate,firmware->rf_params[table_num]->alternate);*/

				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);
				check_req_fuel_limits(table_num);
			}
			else
			{
				table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
				dload_val = bitval;
				if (dload_val == get_ecu_data(canID,page,offset,size))
				{
					return FALSE;
				}
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);
				check_req_fuel_limits(table_num);
			}
			break;
		case MS2_USER_OUTPUTS:
			/* Send the offset */
			tmp = get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;	/*clears bits */
			tmp = tmp | (bitval << bitshift);
			send_to_ecu(canID, page, offset, size, tmp, TRUE);
			/* Get the rest of the data from the combo */
			gtk_tree_model_get(model,&iter,UO_SIZE_COL,&size,UO_LOWER_COL,&lower,UO_UPPER_COL,&upper,UO_RANGE_COL,&range,UO_PRECISION_COL,&precision,UO_DL_CONV_COL,&dl_conv,UO_UL_CONV_COL,&ul_conv,-1);

			/* Send the "size" of the offset to the ecu */
			offset = (gint)strtol(OBJ_GET(widget,"size_offset"),NULL,10);
			send_to_ecu(canID, page, offset, MTX_U08,size, TRUE);

			tmpbuf = (gchar *)OBJ_GET(widget,"range_label");
			if (tmpbuf)
				tmpwidget = lookup_widget(tmpbuf);
			if (GTK_IS_LABEL(tmpwidget))
				gtk_label_set_text(GTK_LABEL(tmpwidget),range);

			tmpbuf = (gchar *)OBJ_GET(widget,"thresh_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				eval = NULL;
				eval = OBJ_GET(tmpwidget,"dl_evaluator");
				if (eval)
				{
					evaluator_destroy(eval);
					OBJ_SET(tmpwidget,"dl_evaluator",NULL);
					eval = NULL;
				}
				if (dl_conv)
				{
					eval = evaluator_create(dl_conv);
					OBJ_SET(tmpwidget,"dl_evaluator",eval);
					if (upper)
					{
						tmpf2 = g_ascii_strtod(upper,NULL);
						tmpf = evaluator_evaluate_x(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_upper");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET(tmpwidget,"raw_upper",g_strdup_printf("%f",tmpf));
						/*printf("combo_handler thresh has dl conv expr and upper limit of %f\n",tmpf);*/
					}
					if (lower)
					{
						tmpf2 = g_ascii_strtod(lower,NULL);
						tmpf = evaluator_evaluate_x(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_lower");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET(tmpwidget,"raw_lower",g_strdup_printf("%f",tmpf));
						/*printf("combo_handler thresh has dl conv expr and lower limit of %f\n",tmpf);*/
					}
				}
				else
					OBJ_SET(tmpwidget,"raw_upper",upper);

				eval = NULL;
				eval = OBJ_GET(tmpwidget,"ul_evaluator");
				if (eval)
				{
					evaluator_destroy(eval);
					OBJ_SET(tmpwidget,"ul_evaluator",NULL);
					eval = NULL;
				}
				if (ul_conv)
				{
					eval = evaluator_create(ul_conv);
					OBJ_SET(tmpwidget,"ul_evaluator",eval);
				}
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"dl_conv_expr",dl_conv);
				OBJ_SET(tmpwidget,"ul_conv_expr",ul_conv);
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				/*printf ("combo_handler thresh widget to size '%i', dl_conv '%s' ul_conv '%s' precision '%i'\n",size,dl_conv,ul_conv,precision);*/
				update_widget(tmpwidget,NULL);
			}
			tmpbuf = (gchar *)OBJ_GET(widget,"hyst_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				eval = NULL;
				eval = OBJ_GET(tmpwidget,"dl_evaluator");
				if (eval)
				{
					evaluator_destroy(eval);
					OBJ_SET(tmpwidget,"dl_evaluator",NULL);
					eval = NULL;
				}
				if (dl_conv)
				{
					eval = evaluator_create(dl_conv);
					OBJ_SET(tmpwidget,"dl_evaluator",eval);
					if (upper)
					{
						tmpf2 = g_ascii_strtod(upper,NULL);
						tmpf = evaluator_evaluate_x(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_upper");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET(tmpwidget,"raw_upper",g_strdup_printf("%f",tmpf));
						/*printf("combo_handler hyst has dl conv expr and upper limit of %f\n",tmpf);*/
					}
					if (lower)
					{
						tmpf2 = g_ascii_strtod(lower,NULL);
						tmpf = evaluator_evaluate_x(eval,tmpf2);
						tmpbuf = OBJ_GET(tmpwidget,"raw_lower");
						if (tmpbuf)
							g_free(tmpbuf);
						OBJ_SET(tmpwidget,"raw_lower",g_strdup_printf("%f",tmpf));
						/*printf("combo_handler hyst has dl conv expr and lower limit of %f\n",tmpf);*/
					}
				}
				else
					OBJ_SET(tmpwidget,"raw_upper",upper);

				eval = NULL;
				eval = OBJ_GET(tmpwidget,"ul_evaluator");
				if (eval)
				{
					evaluator_destroy(eval);
					OBJ_SET(tmpwidget,"ul_evaluator",NULL);
					eval = NULL;
				}
				if (ul_conv)
				{
					eval = evaluator_create(ul_conv);
					OBJ_SET(tmpwidget,"ul_evaluator",eval);
				}
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"dl_conv_expr",dl_conv);
				OBJ_SET(tmpwidget,"ul_conv_expr",ul_conv);
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				/*printf ("combo_handler hyst widget to size '%i', dl_conv '%s' ul_conv '%s' precision '%i'\n",size,dl_conv,ul_conv,precision);*/
				update_widget(tmpwidget,NULL);
			}
			return TRUE;
			break;
		default:
			printf(_("std_combo_handler, default case!!! wrong wrong wrong!!\n"));
			break;
	}

	if (swap_list)
		swap_labels(swap_list,bitval);
	if (table_2_update)
		g_timeout_add(2000,force_update_table,table_2_update);
	if (set_labels)
	{
		total = get_choice_count(model);
		tmpi = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
		vector = g_strsplit(set_labels,",",-1);
		if ((g_strv_length(vector)%(total+1)) != 0)
		{
			dbg_func(CRITICAL,g_strdup(__FILE__": std_combo_handler()\n\tProblem with set_widget_labels, counts don't match up\n"));
			goto combo_download;
		}
		for (i=0;i<(g_strv_length(vector)/(total+1));i++)
		{
			tmpbuf = g_strconcat(vector[i*(total+1)],",",vector[(i*(total+1))+1+tmpi],NULL);
			set_widget_labels(tmpbuf);
			g_free(tmpbuf);
		}
		g_strfreev(vector);
	}

combo_download:
	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download(widget,dload_val);
		send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	return TRUE;
}


/*!
 \brief spin_button_handler() handles ALL spinbuttons in MegaTunix and does
 whatever is needed for the data before sending it to the ECU
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
EXPORT gboolean spin_button_handler(GtkWidget *widget, gpointer data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	gint dl_type = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint canID = 0;
	gint page = -1;
	DataSize size = -1;
	gint bitmask = -1;
	gint bitshift = -1;
	gint spconfig_offset = 0;
	gint oddfire_bit_offset = 0;
	gint tmpi = 0;
	gint tmp = 0;
	gint handler = -1;
	gint divider_offset = 0;
	gint table_num = -1;
	gint temp_units = 0;
	gint source = 0;
	gboolean temp_dep = FALSE;
	gfloat value = 0.0;
	GtkWidget * tmpwidget = NULL;
	Deferred_Data *d_data = NULL;
	extern gint realtime_id;
	Reqd_Fuel *reqd_fuel = NULL;
	extern gboolean forced_update;
	extern GHashTable **interdep_vars;
	extern Firmware_Details *firmware;

	if ((paused_handlers) || (!ready))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	if (!GTK_IS_WIDGET(widget))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\twidget pointer is NOT valid\n"));
		return FALSE;
	}

	reqd_fuel = (Reqd_Fuel *)OBJ_GET(
			widget,"reqd_fuel");
	handler = (MtxButton)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	canID = (GINT) OBJ_GET(widget,"canID");
	page = (GINT) OBJ_GET(widget,"page");
	offset = (GINT) OBJ_GET(widget,"offset");
	size = (DataSize) OBJ_GET(widget,"size");
	bitmask = (GINT) OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift(bitmask);
	temp_units = (GINT)OBJ_GET(global_data,"temp_units");
	temp_dep = (GBOOLEAN)OBJ_GET(widget,"temp_dep");
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);

	tmpi = (int)(value+.001);


	switch ((MtxButton)handler)
	{
		case SER_INTERVAL_DELAY:
			serial_params->read_wait = (gint)value;
			if (realtime_id > 0)
			{
				stop_tickler(RTV_TICKLER);
				start_tickler(RTV_TICKLER);
				forced_update=TRUE;
			}
			break;
		case SER_READ_TIMEOUT:
			OBJ_SET(global_data,"read_timeout",GINT_TO_POINTER((gint)value));
			break;
		case RTSLIDER_FPS:
			OBJ_SET(global_data,"rtslider_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)OBJ_GET(global_data,"rtslider_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000/(float)tmpi),(GtkFunction)update_rtsliders,NULL);
			OBJ_SET(global_data,"rtslider_id",GINT_TO_POINTER(tmpi));
			break;
		case RTTEXT_FPS:
			OBJ_SET(global_data,"rttext_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)OBJ_GET(global_data,"rttext_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000.0/(float)tmpi),(GtkFunction)update_rttext,NULL);
			OBJ_SET(global_data,"rttext_id",GINT_TO_POINTER(tmpi));
			break;
		case DASHBOARD_FPS:
			OBJ_SET(global_data,"dashboard_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)OBJ_GET(global_data,"dashboard_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000.0/(float)tmpi),(GtkFunction)update_dashboards,NULL);
			OBJ_SET(global_data,"dashboard_id",GINT_TO_POINTER(tmpi));
			break;
		case VE3D_FPS:
			OBJ_SET(global_data,"ve3d_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)OBJ_GET(global_data,"ve3d_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000.0/(float)tmpi),(GtkFunction)update_ve3ds,NULL);
			OBJ_SET(global_data,"ve3d_id",GINT_TO_POINTER(tmpi));
			break;
		case REQ_FUEL_DISP:
			reqd_fuel->disp = (gint)value;
			req_fuel_change(widget);
			break;
		case REQ_FUEL_CYLS:
			reqd_fuel->cyls = (gint)value;
			req_fuel_change(widget);
			break;
		case REQ_FUEL_RATED_INJ_FLOW:
			reqd_fuel->rated_inj_flow = (gfloat)value;
			req_fuel_change(widget);
			break;
		case REQ_FUEL_RATED_PRESSURE:
			reqd_fuel->rated_pressure = (gfloat)value;
			req_fuel_change(widget);
			break;
		case REQ_FUEL_ACTUAL_PRESSURE:
			reqd_fuel->actual_pressure = (gfloat)value;
			req_fuel_change(widget);
			break;
		case REQ_FUEL_AFR:
			reqd_fuel->target_afr = value;
			req_fuel_change(widget);
			break;
		case REQ_FUEL_1:
		case REQ_FUEL_2:
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			firmware->rf_params[table_num]->last_req_fuel_total = firmware->rf_params[table_num]->req_fuel_total;
			firmware->rf_params[table_num]->req_fuel_total = value;
			check_req_fuel_limits(table_num);
			break;
		case LOCKED_REQ_FUEL:
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),firmware->rf_params[table_num]->req_fuel_total);
			break;

		case LOGVIEW_ZOOM:
			OBJ_SET(global_data,"lv_zoom",GINT_TO_POINTER(tmpi));
			tmpwidget = lookup_widget("logviewer_trace_darea");	
			if (tmpwidget)
				lv_configure_event(lookup_widget("logviewer_trace_darea"),NULL,NULL);
			/*	g_signal_emit_by_name(tmpwidget,"configure_event",NULL);*/
			break;

		case NUM_SQUIRTS_1:
		case NUM_SQUIRTS_2:
			/* This actually affects another variable */
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			divider_offset = firmware->table_params[table_num]->divider_offset;
			firmware->rf_params[table_num]->last_num_squirts = firmware->rf_params[table_num]->num_squirts;
			firmware->rf_params[table_num]->last_divider = get_ecu_data(canID,page,divider_offset,size);

			firmware->rf_params[table_num]->num_squirts = tmpi;
			if (firmware->rf_params[table_num]->num_cyls % firmware->rf_params[table_num]->num_squirts)
			{
				err_flag = TRUE;
				set_reqfuel_color(RED,table_num);
			}
			else
			{
				dload_val = (gint)(((float)firmware->rf_params[table_num]->num_cyls/(float)firmware->rf_params[table_num]->num_squirts)+0.001);

				firmware->rf_params[table_num]->divider = dload_val;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = divider_offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(divider_offset),
						d_data);
				err_flag = FALSE;
				set_reqfuel_color(BLACK,table_num);
				check_req_fuel_limits(table_num);
			}
			break;
		case NUM_CYLINDERS_1:
		case NUM_CYLINDERS_2:
			/* Updates a shared bitfield */
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			divider_offset = firmware->table_params[table_num]->divider_offset;
			firmware->rf_params[table_num]->last_divider = get_ecu_data(canID,page,divider_offset,size);
			firmware->rf_params[table_num]->last_num_cyls = firmware->rf_params[table_num]->num_cyls;

			firmware->rf_params[table_num]->num_cyls = tmpi;
			if (firmware->rf_params[table_num]->num_cyls % firmware->rf_params[table_num]->num_squirts)
			{
				err_flag = TRUE;
				set_reqfuel_color(RED,table_num);	
			}
			else
			{
				tmp = get_ecu_data(canID,page,offset,size);
				tmp = tmp & ~bitmask;	/*clears bits */
				if (firmware->capabilities & MS2)
					tmp = tmp | ((tmpi) << bitshift);
				else
					tmp = tmp | ((tmpi-1) << bitshift);
				dload_val = tmp;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);

				dload_val = 
					(gint)(((float)firmware->rf_params[table_num]->num_cyls/(float)firmware->rf_params[table_num]->num_squirts)+0.001);

				firmware->rf_params[table_num]->divider = dload_val;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = divider_offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(divider_offset),
						d_data);

				err_flag = FALSE;
				set_reqfuel_color(BLACK,table_num);	
				check_req_fuel_limits(table_num);
			}
			break;
		case NUM_INJECTORS_1:
		case NUM_INJECTORS_2:
			/* Updates a shared bitfield */
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			firmware->rf_params[table_num]->last_num_inj = firmware->rf_params[table_num]->num_inj;
			firmware->rf_params[table_num]->num_inj = tmpi;

			tmp = get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;	/*clears bits */
			if (firmware->capabilities & MS2)
				tmp = tmp | ((tmpi) << bitshift);
			else
				tmp = tmp | ((tmpi-1) << bitshift);
			dload_val = tmp;

			d_data = g_new0(Deferred_Data, 1);
			d_data->canID = canID;
			d_data->page = page;
			d_data->offset = offset;
			d_data->value = dload_val;
			d_data->size = MTX_U08;
			g_hash_table_replace(interdep_vars[table_num],
					GINT_TO_POINTER(offset),
					d_data);

			check_req_fuel_limits(table_num);
			break;
		case TRIGGER_ANGLE:
			spconfig_offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			if (spconfig_offset == 0)
			{
				dbg_func(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\tERROR Trigger Angle spinbutton call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (value > 112.15)	/* Extra long trigger needed */	
			{
				tmp = get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);	/* Set xlong_trig */
				send_to_ecu(canID, page, spconfig_offset, size, tmp,  TRUE);
				value -= 45.0;
				dload_val = convert_before_download(widget,value);
			}
			else if (value > 89.65) /* Long trigger needed */
			{
				tmp = get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);	/* Set long_trig */
				send_to_ecu(canID, page, spconfig_offset, size, tmp, TRUE);
				value -= 22.5;
				dload_val = convert_before_download(widget,value);
			}
			else	/* value <= 89.65 degrees, no long trigger*/
			{
				tmp = get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				send_to_ecu(canID, page, spconfig_offset, size, tmp, TRUE);
				dload_val = convert_before_download(widget,value);
			}

			break;

		case ODDFIRE_ANGLE:
			oddfire_bit_offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
			{
				dbg_func(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\tERROR Offset Angle spinbutton call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (value > 90)	/*  */	
			{
				tmp = get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);	/* Set +90 */
				send_to_ecu(canID, page, oddfire_bit_offset, size, tmp, TRUE);
				value -= 90.0;
				dload_val = convert_before_download(widget,value);
			}
			else if (value > 45) /* */
			{
				tmp = get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);	/* Set +45 */
				send_to_ecu(canID, page, oddfire_bit_offset, size, tmp, TRUE);
				value -= 45.0;
				dload_val = convert_before_download(widget,value);
			}
			else	/* value <= 45 degrees, */
			{
				tmp = get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				send_to_ecu(canID, page, oddfire_bit_offset, size, tmp,  TRUE);
				dload_val = convert_before_download(widget,value);
			}

			break;

		case GENERIC:	/* Handles almost ALL other variables */
			if (temp_dep)
			{
				if (temp_units == CELSIUS)
					value = (value*(9.0/5.0))+32;
			}

			dload_val = convert_before_download(widget,value);
			break;
		default:
			/* Prevents MS corruption for a SW bug */
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": spin_button_handler()\n\tERROR spinbutton not handled, handler = %i\n",handler));
			dl_type = 0;  
			break;
	}
	if (dl_type == IMMEDIATE) 
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != get_ecu_data(canID,page,offset,size))
			send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	return TRUE;

}


/*!
 \brief update_ve_const_pf() is called after a read of the VE/Const block of 
 data from the ECU.  It takes care of updating evey control that relates to
 an ECU variable on screen
 */
EXPORT void update_ve_const_pf()
{
	gint canID = 0;  
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gfloat tmpf = 0.0;
	gint reqfuel = 0;
	gint i = 0;
	guint mask = 0;
	guint shift = 0;
	guint tmpi = 0;
	guint8 addon = 0;
	gint mult = 0;
	
	extern Firmware_Details *firmware;
	extern volatile gboolean leaving;
	extern volatile gboolean offline;
	extern gboolean connected;
	canID = firmware->canID;

	if (leaving)
		return;
	if (!((connected) || (offline)))
		return;

	set_title(g_strdup(_("Updating Controls...")));
	paused_handlers = TRUE;

	/* DualTable Fuel Calculations
	 * DT code no longer uses the "alternate" firing mode as each table
	 * is pretty much independant from the other,  so the calcs are a 
	 * little simpler...
	 *
	 *                                        /        num_inj_1      \
	 *         	   req_fuel_per_squirt * (-------------------------)
	 *                                        \ 	    divider       /
	 * req_fuel_total = --------------------------------------------------
	 *				10
	 *
	 * where divider = num_cyls/num_squirts;
	 *
	 *
	 *  B&G, MSnS, MSnEDIS req Fuel calc *
	 * req-fuel 
	 *                                /        num_inj        \
	 *    	   req_fuel_per_squirt * (-------------------------)
	 *                                \ divider*(alternate+1) /
	 * req_fuel_total = ------------------------------------------
	 *				10
	 *
	 * where divider = num_cyls/num_squirts;
	 *
	 * The req_fuel_per_squirt is the part stored in the MS ECU as 
	 * the req_fuel variable.  Take note when doing conversions.  
	 * On screen the value is divided by ten from what is 
	 * in the MS.  
	 * 
	 */

	/* All Tables */
	if (firmware->capabilities & MS2)
	{
		addon = 0;
		mult = 100;
	}
	else
	{
		addon = 1;
		mult = 1;
	}

	for (i=0;i<firmware->total_tables;i++)
	{
		if (firmware->table_params[i]->color_update == FALSE)
		{
			recalc_table_limits(0,i);
			if ((firmware->table_params[i]->last_z_maxval != firmware->table_params[i]->z_maxval) || (firmware->table_params[i]->last_z_minval != firmware->table_params[i]->z_minval))
				firmware->table_params[i]->color_update = TRUE;
			else
				firmware->table_params[i]->color_update = FALSE;
		}

		if (firmware->table_params[i]->reqfuel_offset < 0)
			continue;

		tmpi = get_ecu_data(canID,firmware->table_params[i]->num_cyl_page,firmware->table_params[i]->num_cyl_offset,size);	
		mask = firmware->table_params[i]->num_cyl_mask;
		shift = get_bitshift(firmware->table_params[i]->num_cyl_mask);
		firmware->rf_params[i]->num_cyls = ((tmpi & mask) >> shift)+addon;
		firmware->rf_params[i]->last_num_cyls = ((tmpi & mask) >> shift)+addon;
		/*printf("num_cyls for table %i in the firmware is %i\n",i,firmware->rf_params[i]->num_cyls);*/

		tmpi = get_ecu_data(canID,firmware->table_params[i]->num_inj_page,firmware->table_params[i]->num_inj_offset,size);	
		mask = firmware->table_params[i]->num_cyl_mask;
		shift = get_bitshift(firmware->table_params[i]->num_cyl_mask);

		firmware->rf_params[i]->num_inj = ((tmpi & mask) >> shift)+addon;
		firmware->rf_params[i]->last_num_inj = ((tmpi & mask) >> shift)+addon;
		/*printf("num_inj for table %i in the firmware is %i\n",i,firmware->rf_params[i]->num_inj);*/

		firmware->rf_params[i]->divider = get_ecu_data(canID,firmware->table_params[i]->divider_page,firmware->table_params[i]->divider_offset,size);
		firmware->rf_params[i]->last_divider = firmware->rf_params[i]->divider;
		firmware->rf_params[i]->alternate = get_ecu_data(canID,firmware->table_params[i]->alternate_page,firmware->table_params[i]->alternate_offset,size);
		firmware->rf_params[i]->last_alternate = firmware->rf_params[i]->alternate;
		/*printf("alternate for table %i in the firmware is %i\n",i,firmware->rf_params[i]->alternate);*/
		reqfuel = get_ecu_data(canID,firmware->table_params[i]->reqfuel_page,firmware->table_params[i]->reqfuel_offset,firmware->table_params[i]->reqfuel_size);
		/*printf("reqfuel for table %i in the firmware is %i\n",i,reqfuel);*/

		/*
		printf("reqfuel_page %i, reqfuel_offset %i\n",firmware->table_params[i]->reqfuel_page,firmware->table_params[i]->reqfuel_offset);
	
		printf("num_inj %i, divider %i\n",firmware->rf_params[i]->num_inj,firmware->rf_params[i]->divider);
		printf("num_cyls %i, alternate %i\n",firmware->rf_params[i]->num_cyls,firmware->rf_params[i]->alternate);
		printf("req_fuel_per_1_squirt is %i\n",reqfuel);
		*/
		 
		/* Calcs vary based on firmware. 
		 * DT uses num_inj/divider
		 * MSnS-E uses the SAME in DT mode only
		 * MSnS-E uses B&G form in single table mode
		 */
		if (firmware->capabilities & MS1_DT)
		{
			/*
			 * printf("DT\n");
			 */
			tmpf = (float)(firmware->rf_params[i]->num_inj)/(float)(firmware->rf_params[i]->divider);
		}
		else if (firmware->capabilities & MSNS_E)
		{
			shift = get_bitshift(firmware->table_params[i]->dtmode_mask);
			if ((get_ecu_data(canID,firmware->table_params[i]->dtmode_page,firmware->table_params[i]->dtmode_offset,size) & firmware->table_params[i]->dtmode_mask) >> shift)
			{
				/*
				 * printf("MSnS-E DT\n"); 
				 */
				tmpf = (float)(firmware->rf_params[i]->num_inj)/(float)(firmware->rf_params[i]->divider);
			}
			else
			{
				/*
				 * printf("MSnS-E non-DT\n"); 
				 */
				tmpf = (float)(firmware->rf_params[i]->num_inj)/((float)(firmware->rf_params[i]->divider)*((float)(firmware->rf_params[i]->alternate)+1.0));
			}
		}
		else
		{
			/*
			 * printf("B&G\n"); 
			 */
			tmpf = (float)(firmware->rf_params[i]->num_inj)/((float)(firmware->rf_params[i]->divider)*((float)(firmware->rf_params[i]->alternate)+1.0));
		}

		/* ReqFuel Total */
		/*
		 * printf("intermediate tmpf is %f\n",tmpf);
		 */
		tmpf *= (float)reqfuel;
		tmpf /= (10.0*mult);
		firmware->rf_params[i]->req_fuel_total = tmpf;
		firmware->rf_params[i]->last_req_fuel_total = tmpf;
		/*
		 * printf("req_fuel_total for table number %i is %f\n",i,tmpf);
		 */

		/* Injections per cycle */
		firmware->rf_params[i]->num_squirts = (float)(firmware->rf_params[i]->num_cyls)/(float)(firmware->rf_params[i]->divider);
		/*
		 * printf("num_squirts for table number %i is %i\n",i,firmware->rf_params[i]->num_squirts);
		 */
		if (firmware->rf_params[i]->num_squirts < 1 )
			firmware->rf_params[i]->num_squirts = 1;
		firmware->rf_params[i]->last_num_squirts = firmware->rf_params[i]->num_squirts;

		set_reqfuel_color(BLACK,i);
	}


	/* Update all on screen controls (except bitfields (done above)*/
	upd_count = 0;

	for (page=0;page<firmware->total_pages;page++)
	{
		if ((leaving) || (!firmware))
			return;
		if (!firmware->page_params[page]->dl_by_default)
			continue;
		thread_update_widget("info_label",MTX_LABEL,g_strdup_printf(_("Updating Controls on Page %i"),page));
		for (offset=0;offset<firmware->page_params[page]->length;offset++)
		{
			if ((leaving) || (!firmware))
				return;
			if (ve_widgets[page][offset] != NULL)
				g_list_foreach(ve_widgets[page][offset],
						update_widget,NULL);
		}
	}
	for (i=0;i<firmware->total_tables;i++)
		firmware->table_params[i]->color_update = FALSE;

	paused_handlers = FALSE;
	thread_update_widget("info_label",MTX_LABEL,g_strdup_printf(_("Ready...")));
	set_title(g_strdup(_("Ready...")));
	return;
}


/*!
 \brief trigger_group_update() updates a subset of widgets (any widgets in
 the group name passed. This runs as a timeout delayed asynchronously from
 when the ctrl is modified, to prevent a deadlock.
 \param data, string name of list of controls
 */
gboolean trigger_group_update(gpointer data)
{
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;

	g_list_foreach(get_list((gchar *)data),update_widget,NULL);
	return FALSE;/* Make it cancel and not run again till called */
}

/*!
 \brief force_update_table() updates a subset of widgets (specifically ONLY
 the Z axis widgets) of a table on screen.
 \param table_num, integer number of the table in question
 */
gboolean force_update_table(gpointer data)
{
	gint offset = -1;
	gint page = -1;
	gint table_num = -1;
	extern Firmware_Details *firmware;
	extern volatile gboolean leaving;
	extern gboolean forced_update;
	extern GList ***ve_widgets;
	gint base = 0;
	gint length = 0;

	if (leaving)
		return FALSE;
	if (page > firmware->total_pages)
	       return FALSE;
	table_num = (gint)strtol((gchar *)data,NULL,10);
	if ((table_num < 0) || (table_num > (firmware->total_tables-1)))
		return FALSE;
	base = firmware->table_params[table_num]->z_base;
	length = firmware->table_params[table_num]->x_bincount *
		firmware->table_params[table_num]->y_bincount;
	page =  firmware->table_params[table_num]->z_page;
	for (offset=base;offset<base+length;offset++)
	{
		if ((leaving) || (!firmware))
			return FALSE;
		if (ve_widgets[page][offset] != NULL)
			g_list_foreach(ve_widgets[page][offset],update_widget,NULL);
	}
	forced_update = TRUE;
	return FALSE;
}


/*!
 \brief update_widget() updates a widget on screen.  All parameters re the
 conversions and where the raw value is stored is embedded within the widget 
 itself.
 \param object (gpointer) pointer to the widget object
 \param user_data (gpointer) pointer to a widget to compare against to 
 prevent a race
 */
void update_widget(gpointer object, gpointer user_data)
{
	GtkWidget * widget = object;
	gint dl_type = -1;
	gboolean temp_dep = FALSE;
	gboolean use_color = FALSE;
	gboolean changed = FALSE;
	guint i = 0;
	gint j = 0;
	gint tmpi = -1;
	gint page = -1;
	gint offset = -1;
	DataSize size = 0;
	gint canID = 0;
	gdouble value = 0.0;
	gboolean valid = FALSE;
	guchar t_bitval = -1;
	guchar bitval = -1;
	guchar bitshift = -1;
	guchar bitmask = -1;
	gint table_num = -1;
	gint base = -1;
	gint precision = -1;
	gint spconfig_offset = 0;
	gint oddfire_bit_offset = 0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	gfloat scaler = 0.0;
	gboolean cur_state = FALSE;
	gboolean new_state = FALSE;
	gint algo = 0;
	gint total = 0;
	gchar * range = NULL;
	gchar * toggle_labels = NULL;
	gchar * toggle_groups = NULL;
	gchar * swap_list = NULL;
	gchar * set_labels = NULL;
	gchar * tmpbuf = NULL;
	gchar **vector = NULL;
	gchar * widget_text = NULL;
	gchar * group_2_update = NULL;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * dl_conv = NULL;
	gchar * ul_conv = NULL;
	GtkWidget *tmpwidget = NULL;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	void *eval = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	gdouble spin_value = 0.0; 
	GdkColor color;
	extern Firmware_Details *firmware;
	extern gint *algorithm;
	extern volatile gboolean leaving;
	extern GHashTable *sources_hash;

	if (leaving)
		return;

	upd_count++;
	if ((upd_count%64) == 0)
	{
		while (gtk_events_pending())
		{
			if (leaving)
			{
				return;
			}
			gtk_main_iteration();
		}
	}
	if (!GTK_IS_OBJECT(widget))
		return;

	/* If passed widget and user data are identical,  break out as
	 * we already updated the widget.
	 */
	if ((GTK_IS_OBJECT(user_data)) && (widget == user_data))
		return;

	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	canID = (GINT)OBJ_GET(widget,"canID");
	if (!OBJ_GET(widget,"size"))
		size = MTX_U08 ; 	/* default! */
	else
		size = (DataSize)OBJ_GET(widget,"size");
	if (OBJ_GET(widget,"raw_lower"))
		raw_lower = (gint)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		raw_lower = get_extreme_from_size(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		raw_upper = (gint)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		raw_upper = get_extreme_from_size(size,UPPER);
	bitval = (GINT)OBJ_GET(widget,"bitval");
	bitmask = (GINT)OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift(bitmask);
	if (!OBJ_GET(widget,"base"))
		base = 10;
	else
		base = (GINT)OBJ_GET(widget,"base");

	precision = (GINT)OBJ_GET(widget,"precision");
	temp_dep = (GBOOLEAN)OBJ_GET(widget,"temp_dep");
	toggle_labels = (gchar *)OBJ_GET(widget,"toggle_labels");
	toggle_groups = (gchar *)OBJ_GET(widget,"toggle_groups");
	use_color = (GBOOLEAN)OBJ_GET(widget,"use_color");
	if (use_color)
		if (OBJ_GET(widget,"table_num"))
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
	swap_list = (gchar *)OBJ_GET(widget,"swap_labels");
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	group_2_update = (gchar *)OBJ_GET(widget,"group_2_update");

	value = convert_after_upload(widget);  
	/*printf("value is %f for widget %s at page %i, offset %i\n",value,glade_get_widget_name(widget),page,offset);*/

	if (temp_dep)
	{
		if ((GINT)OBJ_GET(global_data,"temp_units") == CELSIUS)
			value = (value-32)*(5.0/9.0);
	}

	/*printf("update_widget %s, page %i, offset %i bitval %i, mask %i, shift %i\n",(gchar *)glade_get_widget_name(widget), page,offset,bitval,bitmask,bitshift);*/
	/* update widget whether spin,radio or checkbutton  
	 * (checkbutton encompases radio)
	 */
	if ((GTK_IS_ENTRY(widget)) && (!GTK_IS_SPIN_BUTTON(widget)))
	{
		if ((GINT)OBJ_GET(widget,"handler") == ODDFIRE_ANGLE)
		{
			oddfire_bit_offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
				return;
			switch (get_ecu_data(canID,page,oddfire_bit_offset,size))
			{
				case 4:
					tmpbuf = g_strdup_printf("%1$.*2$f",value+90,precision);
					break;
				case 2:
					tmpbuf = g_strdup_printf("%1$.*2$f",value+45,precision);
					break;
				case 0:
					tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
					break;
				default:
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t ODDFIRE_ANGLE_UPDATE invalid value for oddfire_bit_offset at ecu_data[%i][%i], ERROR\n",page,oddfire_bit_offset));

			}
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
		}
		else if ((GINT)OBJ_GET(widget,"handler") == TRIGGER_ANGLE)
		{
			spconfig_offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			switch ((get_ecu_data(canID,page,spconfig_offset,size) & 0x03))
			{
				case 2:
					tmpbuf = g_strdup_printf("%1$.*2$f",value+45,precision);
					break;
				case 1:
					tmpbuf = g_strdup_printf("%1$.*2$f",value+22.5,precision);
					break;
				case 0:
					tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
					break;
				default:
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t TRIGGER_ANGLE_UPDATE invalid value for spconfig_offset at ecu_data[%i][%i], ERROR\n",page,spconfig_offset));

			}
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
		}
		else
		{
			widget_text = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
			if (base == 10)
			{
				tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
				if (g_ascii_strcasecmp(widget_text,tmpbuf) != 0)
				{
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					changed = TRUE;
				}
				g_free(tmpbuf);
			}
			else if (base == 16)
			{
				tmpbuf = g_strdup_printf("%.2X",(gint)value);
				if (g_ascii_strcasecmp(widget_text,tmpbuf) != 0)
				{
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					changed = TRUE;
				}
				g_free(tmpbuf);
			}
			else
				dbg_func(CRITICAL,g_strdup(__FILE__": update_widget()\n\t base for nemeric textentry is not 10 or 16, ERROR\n"));

			if (use_color)
			{
				if ((table_num >= 0) && (firmware->table_params[table_num]->color_update))
				{
					scaler = 256.0/((firmware->table_params[table_num]->z_maxval - firmware->table_params[table_num]->z_minval)*1.05);
					color = get_colors_from_hue(256.0 - (get_ecu_data(canID,page,offset,size)-firmware->table_params[table_num]->z_minval)*scaler, 0.50, 1.0);
					gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	

				}
				else
				{
					if ((changed) || (value == 0))
					{
						color = get_colors_from_hue(((gfloat)(get_ecu_data(canID,page,offset,size)-raw_lower)/raw_upper)*-300.0+180, 0.50, 1.0);
						gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	
					}
				}

			}
		}
		if (OBJ_GET(widget,"not_sent"))
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	}
	else if (GTK_IS_SPIN_BUTTON(widget))
	{
		if ((GINT)OBJ_GET(widget,"handler") == ODDFIRE_ANGLE)
		{
			oddfire_bit_offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
				return;
			switch (get_ecu_data(canID,page,oddfire_bit_offset,size))
			{
				case 4:
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value+90);
					break;
				case 2:
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value+45);
					break;
				case 0:
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
					break;
				default:
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t ODDFIRE_ANGLE_UPDATE invalid value for oddfire_bit_offset at ecu_data[%i][%i], ERROR\n",page,oddfire_bit_offset));


			}
		}
		else if ((GINT)OBJ_GET(widget,"handler") == TRIGGER_ANGLE)
		{
			spconfig_offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			switch ((get_ecu_data(canID,page,spconfig_offset,size) & 0x03))
			{
				case 2:
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value+45);
					break;
				case 1:
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value+22.5);
					break;
				case 0:
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
					break;
				default:
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t TRIGGER_ANGLE_UPDATE invalid value for spconfig_offset at ecu_data[%i][%i], ERROR\n",page,spconfig_offset));


			}
		}
		else
		{
			spin_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			if (value != spin_value)
			{
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
				if (use_color)
				{
					if (table_num >= 0)
					{
						scaler = 256.0/((firmware->table_params[table_num]->z_maxval - firmware->table_params[table_num]->z_minval)*1.05);
						color = get_colors_from_hue(256 - (get_ecu_data(canID,page,offset,size)-firmware->table_params[table_num]->z_minval)*scaler, 0.50, 1.0);
					}
					else
						color = get_colors_from_hue(((gfloat)(get_ecu_data(canID,page,offset,size)-raw_lower)/raw_upper)*-300.0+180, 0.50, 1.0);
					gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	
				}
			}
		}
		if (OBJ_GET(widget,"not_sent"))
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	}
	else if (GTK_IS_COMBO_BOX(widget))
	{
		//printf("Combo at page %i, offset %i, bitmask %i, bitshift %i, value %i\n",page,offset,bitmask,bitshift,(gint)value);

		tmpi = ((gint)value & bitmask) >> bitshift;
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
		if (!GTK_IS_TREE_MODEL(model))
			printf(_("ERROR no model for Combo at page %i, offset %i, bitmask %i, bitshift %i, value %i\n"),page,offset,bitmask,bitshift,(gint)value);
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model),&iter);
		i = 0;
		while (valid)
		{
			gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,BITVAL_COL,&t_bitval,-1);
			if (tmpi == t_bitval)
			{
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
				gtk_widget_modify_base(GTK_BIN (widget)->child,GTK_STATE_NORMAL,&white);
				if ((GINT)OBJ_GET(widget,"handler") == MS2_USER_OUTPUTS)
				{
					/* Get the rest of the data from the combo */
					gtk_tree_model_get(model,&iter,UO_SIZE_COL,&size,UO_LOWER_COL,&lower,UO_UPPER_COL,&upper,UO_RANGE_COL,&range,UO_PRECISION_COL,&precision,UO_DL_CONV_COL,&dl_conv,UO_UL_CONV_COL,&ul_conv,-1);

					tmpbuf = (gchar *)OBJ_GET(widget,"range_label");
					if (tmpbuf)
						tmpwidget = lookup_widget(tmpbuf);
					if (GTK_IS_LABEL(tmpwidget))
						gtk_label_set_text(GTK_LABEL(tmpwidget),range);
					tmpbuf = (gchar *)OBJ_GET(widget,"thresh_widget");
					if (tmpbuf)
						tmpwidget = lookup_widget(tmpbuf);
					if (GTK_IS_WIDGET(tmpwidget))
					{
						eval = NULL;
						eval = OBJ_GET(tmpwidget,"dl_evaluator");
						if (eval)
						{
							evaluator_destroy(eval);
							OBJ_SET(tmpwidget,"dl_evaluator",NULL);
							eval = NULL;
						}
						if (dl_conv)
						{
							eval = evaluator_create(dl_conv);
							OBJ_SET(tmpwidget,"dl_evaluator",eval);
							if (upper)
							{
								tmpf2 = g_ascii_strtod(upper,NULL);
								tmpf = evaluator_evaluate_x(eval,tmpf2);
								tmpbuf = OBJ_GET(tmpwidget,"raw_upper");
								if (tmpbuf)
									g_free(tmpbuf);
								OBJ_SET(tmpwidget,"raw_upper",g_strdup_printf("%f",tmpf));
								/*printf("update_widget thresh has dl conv expr and upper limit of %f\n",tmpf);*/
							}
							if (lower)
							{
								tmpf2 = g_ascii_strtod(lower,NULL);
								tmpf = evaluator_evaluate_x(eval,tmpf2);
								tmpbuf = OBJ_GET(tmpwidget,"raw_lower");
								if (tmpbuf)
									g_free(tmpbuf);
								OBJ_SET(tmpwidget,"raw_lower",g_strdup_printf("%f",tmpf));
								/*printf("update_widget thresh has dl conv expr and lower limit of %f\n",tmpf);*/
							}
						}
						else
							OBJ_SET(tmpwidget,"raw_upper",upper);

						eval = NULL;
						eval = OBJ_GET(tmpwidget,"ul_evaluator");
						if (eval)
						{
							evaluator_destroy(eval);
							OBJ_SET(tmpwidget,"ul_evaluator",NULL);
							eval = NULL;
						}
						if (ul_conv)
						{
							eval = evaluator_create(ul_conv);
							OBJ_SET(tmpwidget,"ul_evaluator",eval);
						}
						OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
						OBJ_SET(tmpwidget,"dl_conv_expr",dl_conv);
						OBJ_SET(tmpwidget,"ul_conv_expr",ul_conv);
						OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
						/*printf ("update widgets setting thresh widget to size '%i', dl_conv '%s' ul_conv '%s' precision '%i'\n",size,dl_conv,ul_conv,precision);*/
						update_widget(tmpwidget,NULL);
					}
					tmpbuf = (gchar *)OBJ_GET(widget,"hyst_widget");
					if (tmpbuf)
						tmpwidget = lookup_widget(tmpbuf);
					if (GTK_IS_WIDGET(tmpwidget))
					{
						eval = NULL;
						eval = OBJ_GET(tmpwidget,"dl_evaluator");
						if (eval)
						{
							evaluator_destroy(eval);
							OBJ_SET(tmpwidget,"dl_evaluator",NULL);
							eval = NULL;
						}
						if (dl_conv)
						{
							eval = evaluator_create(dl_conv);
							OBJ_SET(tmpwidget,"dl_evaluator",eval);
							if (upper)
							{
								tmpf2 = g_ascii_strtod(upper,NULL);
								tmpf = evaluator_evaluate_x(eval,tmpf2);
								tmpbuf = OBJ_GET(tmpwidget,"raw_upper");
								if (tmpbuf)
									g_free(tmpbuf);
								OBJ_SET(tmpwidget,"raw_upper",g_strdup_printf("%f",tmpf));
								/*printf("update_widget hyst has dl conv expr and upper limit of %f\n",tmpf);*/
							}
							if (lower)
							{
								tmpf2 = g_ascii_strtod(lower,NULL);
								tmpf = evaluator_evaluate_x(eval,tmpf2);
								tmpbuf = OBJ_GET(tmpwidget,"raw_lower");
								if (tmpbuf)
									g_free(tmpbuf);
								OBJ_SET(tmpwidget,"raw_lower",g_strdup_printf("%f",tmpf));
								/*printf("update_widget hyst has dl conv expr and lower limit of %f\n",tmpf);*/
							}
						}
						else
							OBJ_SET(tmpwidget,"raw_upper",upper);

						eval = NULL;
						eval = OBJ_GET(tmpwidget,"ul_evaluator");
						if (eval)
						{
							evaluator_destroy(eval);
							OBJ_SET(tmpwidget,"ul_evaluator",NULL);
							eval = NULL;
						}
						if (ul_conv)
						{
							eval = evaluator_create(ul_conv);
							OBJ_SET(tmpwidget,"ul_evaluator",eval);
						}
						OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
						OBJ_SET(tmpwidget,"dl_conv_expr",dl_conv);
						OBJ_SET(tmpwidget,"ul_conv_expr",ul_conv);
						OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
						/*printf ("update widgets setting hyst widget to size '%i', dl_conv '%s' ul_conv '%s' precision '%i'\n",size,dl_conv,ul_conv,precision);*/
						update_widget(tmpwidget,NULL);
					}
				}
				if (group_2_update)
				{
					if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_values")))
					{
						tmpbuf = OBJ_GET(widget,"source_values");
						vector = g_strsplit(tmpbuf,",",-1);
						if ((guint)gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) >= g_strv_length(vector))
						{
							dbg_func(CRITICAL,g_strdup(__FILE__": update_widget()\n\tCOMBOBOX Problem with source_values,  length mismatch, check datamap\n"));
							return ;
						}
						/*printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]);*/
						g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]));
						g_list_foreach(get_list("multi_expression"),update_widget,NULL);
						g_strfreev(vector);
					}
				}
				tmpbuf = (gchar *)OBJ_GET(widget,"algorithms");
				if (tmpbuf)
				{
					vector = g_strsplit(tmpbuf,",",-1);
					if ((guint)gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) >= g_strv_length(vector))
					{
						dbg_func(CRITICAL,g_strdup(__FILE__": update_widget()\n\tCOMBOBOX Problem with algorithms, length mismatch, check datamap\n"));
						return ;
					}
					algo = translate_string(vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]);
					g_strfreev(vector);

					tmpbuf = (gchar *)OBJ_GET(widget,"applicable_tables");
					if (!tmpbuf)
					{
						dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t Check/Radio button  %s has algorithm defines but no applicable tables, BUG!\n",(gchar *)glade_get_widget_name(widget)));
						goto noalgo;
					}

					vector = g_strsplit(tmpbuf,",",-1);
					j = 0;
					while (vector[j])
					{
						algorithm[(gint)strtol(vector[j],NULL,10)]=(Algorithm)algo;
						j++;
					}
					g_strfreev(vector);
				}
				goto combo_toggle;
			}
			valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &iter);
			i++;

		}
		/*printf("COULD NOT FIND MATCH for data for combo %p, data %i!!\n",widget,tmpi);*/
		gtk_widget_modify_base(GTK_BIN(widget)->child,GTK_STATE_NORMAL,&red);
		return;
combo_toggle:
		if (toggle_labels)
			combo_toggle_labels_linked(widget,i);
		if (toggle_groups)
			combo_toggle_groups_linked(widget,i);
		if (swap_list)
			swap_labels(swap_list,tmpi);
		if (set_labels)
		{
			total = get_choice_count(model);
			tmpi = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
			vector = g_strsplit(set_labels,",",-1);
			if ((g_strv_length(vector)%(total+1)) != 0)
			{
				dbg_func(CRITICAL,g_strdup(__FILE__": std_combo_handler()\n\tProblem wiht set_widget_labels, counts don't match up\n"));
				return;
			}
			for (i=0;i<(g_strv_length(vector)/(total+1));i++)
			{
				tmpbuf = g_strconcat(vector[i*(total+1)],",",vector[(i*(total+1))+1+tmpi],NULL);
				set_widget_labels(tmpbuf);
				g_free(tmpbuf);
			}
			g_strfreev(vector);
		}

	}
	else if (GTK_IS_CHECK_BUTTON(widget))
	{
		if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
			gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);
		/* Swaps the label of another control based on widget state... */
		/* If value masked by bitmask, shifted right by bitshift = bitval
		 * then set button state to on...
		 */
		cur_state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
		tmpi = (gint)value;
		/* Avoid unnecessary widget setting and signal propogation 
		 * First if.  If current bit is SET but button is NOT, set it
		 * Second if, If currrent bit is NOT set but button IS  then
		 * un-set it.
		 */
		if (((tmpi & bitmask) >> bitshift) == bitval)
			new_state = TRUE;
		else if (((tmpi & bitmask) >> bitshift) != bitval)
			new_state = FALSE;

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),new_state);
		if ((set_labels) && (new_state))
			set_widget_labels(set_labels);
		if (swap_list)
			swap_labels(swap_list,new_state);
		if ((new_state) && (group_2_update))
		{
			if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_value")))
			{
				/*	printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),(gchar *)OBJ_GET(widget,"source_value"));*/
				g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(OBJ_GET(widget,"source_value")));

			}
			g_timeout_add(2000,force_view_recompute,NULL);
			g_timeout_add(2000,trigger_group_update,group_2_update);
		}

		if (new_state)
		{
			algo = (Algorithm)OBJ_GET(widget,"algorithm");
			if (algo > 0)
			{
				tmpbuf = (gchar *)OBJ_GET(widget,"applicable_tables");
				if (!tmpbuf)
				{
					dbg_func(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t Check/Radio button  %s has algorithm defines but no applicable tables, BUG!\n",(gchar *)glade_get_widget_name(widget)));
					goto noalgo;
				}

				vector = g_strsplit(tmpbuf,",",-1);
				i = 0;
				while (vector[i])
				{
					algorithm[(gint)strtol(vector[i],NULL,10)]=(Algorithm)algo;
					i++;
				}
				g_strfreev(vector);
			}
		}
noalgo:
		if (toggle_groups)
			toggle_groups_linked(widget,new_state);

	}
	else if (GTK_IS_RANGE(widget))
	{
		gtk_range_set_value(GTK_RANGE(widget),value);
	}
	else if (GTK_IS_SCROLLED_WINDOW(widget))
	{
		/* This will looks really weird, but is used in the 
		 * special case of a treeview widget which is always
		 * packed into a scrolled window. Since the treeview
		 * depends on ECU variables, we call a handler here
		 * passing in a pointer to the treeview(the scrolled
		 * window's child widget)
		 */
		update_model_from_view(gtk_bin_get_child(GTK_BIN(widget)));
	}
	/* IF control has groups linked to it's state, adjust */

}


/*!
 \brief key_event() is triggered when a key is pressed on a widget that
 has a key_press/release_event handler registered.
 \param widget (GtkWidget *) widget where the event occurred
 \param event (GdkEventKey) event struct, (contains key that was pressed)
 \param data (gpointer) unused
 */
EXPORT gboolean key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = 0;
	gint value = 0;
	gint active_table = -1;
	glong lower = 0;
	glong upper = 0;
	glong hardlower = 0;
	glong hardupper = 0;
	gint dload_val = 0;
	gboolean send = FALSE;
	gboolean retval = FALSE;
	gboolean reverse_keys = FALSE;
	extern Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern gboolean *tracking_focus;


	canID = (GINT) OBJ_GET(widget,"canID");
	page = (GINT) OBJ_GET(widget,"page");
	offset = (GINT) OBJ_GET(widget,"offset");
	size = (DataSize) OBJ_GET(widget,"size");
	reverse_keys = (GBOOLEAN) OBJ_GET(widget,"reverse_keys");
	if (OBJ_GET(widget,"table_num"))
		active_table = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
	if (OBJ_GET(widget,"raw_lower"))
		lower = (gint)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		lower = get_extreme_from_size(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		upper = (gint)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		upper = get_extreme_from_size(size,UPPER);
	hardlower = get_extreme_from_size(size,LOWER);
	hardupper = get_extreme_from_size(size,UPPER);

	upper = upper > hardupper ? hardupper:upper;
	lower = lower < hardlower ? hardlower:lower;
	value = get_ecu_data(canID,page,offset,size);
	if (event->keyval == GDK_Control_L)
	{
		if (event->type == GDK_KEY_PRESS)
			grab_single_cell = TRUE;
		else
			grab_single_cell = FALSE;
		return FALSE;
	}
	if (event->keyval == GDK_Shift_L)
	{
		if (event->type == GDK_KEY_PRESS)
			grab_multi_cell = TRUE;
		else
			grab_multi_cell = FALSE;
		return FALSE;
	}

	if (event->type == GDK_KEY_RELEASE)
	{
/*		grab_single_cell = FALSE;
		grab_multi_cell = FALSE;
		*/
		return FALSE;
	}

	switch (event->keyval)
	{
		case GDK_Page_Up:
			if (reverse_keys)
			{
				if (value >= (lower+10))
					dload_val = value - 10;
				else
					return FALSE;
			}
			else 
			{
				if (value <= (upper-10))
					dload_val = value + 10;
				else
					return FALSE;
			}
			send = TRUE;
			retval = TRUE;
			break;
		case GDK_Page_Down:
			if (reverse_keys)
			{
				if (value <= (upper-10))
					dload_val = value + 10;
				else
					return FALSE;
			}
			else 
			{
				if (value >= (lower+10))
					dload_val = value - 10;
				else
					return FALSE;
			}
			send = TRUE;
			retval = TRUE;
			break;
		case GDK_plus:
		case GDK_KP_Add:
		case GDK_KP_Equal:
		case GDK_equal:
		case GDK_Q:
		case GDK_q:
			if (reverse_keys)
			{
				if (value >= (lower+1))
					dload_val = value - 1;
				else
					return FALSE;
			}
			else 
			{
				if (value <= (upper-1))
					dload_val = value + 1;
				else
					return FALSE;
			}
			send = TRUE;
			retval = TRUE;
			break;
		case GDK_W:
		case GDK_w:
			if (reverse_keys)
			{
				if (value <= (upper-1))
					dload_val = value + 1;
				else
					return FALSE;
			}
			else 
			{
				if (value >= (lower+1))
					dload_val = value - 1;
				else
					return FALSE;
			}
			send = TRUE;
			retval = TRUE;
			break;
		case GDK_minus:
		case GDK_KP_Subtract:
			if (lower >= 0)
			{
				if (reverse_keys)
				{
					if (value <= (upper-1))
						dload_val = value + 1;
					else
						return FALSE;
				}
				else 
				{
					if (value >= (lower+1))
						dload_val = value - 1;
					else
						return FALSE;
				}
				send = TRUE;
				retval = TRUE;
			}
			break;
		case GDK_H:
		case GDK_h:
		case GDK_KP_Left:
			if (active_table >= 0)
			{
				refocus_cell(widget,GO_LEFT);
				if (grab_single_cell)
					widget_grab(widget,(GdkEventButton *)event,GINT_TO_POINTER(TRUE));
			}
			retval = TRUE;
			break;
		case GDK_L:
		case GDK_l:
		case GDK_KP_Right:
			if (active_table >= 0)
			{
				refocus_cell(widget,GO_RIGHT);
				if (grab_single_cell)
					widget_grab(widget,(GdkEventButton *)event,GINT_TO_POINTER(TRUE));
			}
			retval = TRUE;
			break;
		case GDK_K:
		case GDK_k:
		case GDK_KP_Up:
			if (active_table >= 0)
			{
				refocus_cell(widget,GO_UP);
				if (grab_single_cell)
					widget_grab(widget,(GdkEventButton *)event,GINT_TO_POINTER(TRUE));
			}
			retval = TRUE;
			break;
		case GDK_J:
		case GDK_j:
		case GDK_KP_Down:
			if (active_table >= 0)
			{
				refocus_cell(widget,GO_DOWN);
				if (grab_single_cell)
					widget_grab(widget,(GdkEventButton *)event,GINT_TO_POINTER(TRUE));
			}
			retval = TRUE;
			break;
		case GDK_F:
		case GDK_f:
			printf("tracking focus key!\n");
			if (tracking_focus[active_table])
				tracking_focus[active_table] = FALSE;
			else
				tracking_focus[active_table] = TRUE;
			retval = TRUE;
			break;
		case GDK_Escape:
			g_list_foreach(ve_widgets[page][offset],update_widget,NULL);
			gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
			retval = FALSE;
			break;
		default:	
			retval = FALSE;
	}
	if (send)
		send_to_ecu(canID, page, offset, size, dload_val, TRUE);

	return retval;
}


/*!
 \brief widget_grab() used on Tables only to "select" the widget or 
 group of widgets for rescaling . Requires shift key to be held down and click
 on each spinner/entry that you want to select for rescaling
 \param widget (GtkWidget *) widget being selected
 \param event (GdkEventButton *) struct containing details on the event
 \param data (gpointer) unused
 \returns FALSE to allow other handlers to run
 */
EXPORT gboolean widget_grab(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gboolean marked = FALSE;
	gint page = -1;
	/*
	gint table_num = -1;
	const gchar * widget_name = NULL;
	gchar **vector = NULL;
	*/
	extern GdkColor red;
	static gint total_marked = 0;
	GtkWidget *frame = NULL;
	GtkWidget *parent = NULL;
	gchar * frame_name = NULL;
	extern Firmware_Details *firmware;

	/* Select all chars on click */
	/*
	 * printf("selecting all chars, or so I thought....\n");
	 * gtk_editable_select_region(GTK_EDITABLE(widget),0,-1);
	 */

	if ((GBOOLEAN)data == TRUE)
		goto testit;

	if (event->button != 1) /* Left button click  */
		return FALSE;

	if (!grab_single_cell)
		return FALSE;

testit:
	marked = (GBOOLEAN)OBJ_GET(widget,"marked");
	page = (GINT)OBJ_GET(widget,"page");
	/*
	table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
	widget_name = glade_get_widget_name(widget);
	vector = g_strsplit(widget_name,"_",-1);
	printf("Widget %s, table_num %i, x rows %i, y cols %i vector size %i\n",widget_name,table_num,firmware->table_params[table_num]->x_bincount,firmware->table_params[table_num]->y_bincount,g_strv_length(vector));
	printf("Total entries in this table %s, index of this one %s\n",vector[g_strv_length(vector)-1],vector[g_strv_length(vector)-3]);
	printf("Row %i, col %i\n",strtol(vector[g_strv_length(vector)-3],NULL,10)/firmware->table_params[table_num]->x_bincount,strtol(vector[g_strv_length(vector)-3],NULL,10)%firmware->table_params[table_num]->x_bincount);
	g_strfreev(vector);
	*/

	if (marked)
	{
		total_marked--;
		OBJ_SET(widget,"marked",GINT_TO_POINTER(FALSE));
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	}
	else
	{
		total_marked++;
		OBJ_SET(widget,"marked",GINT_TO_POINTER(TRUE));
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	}

	parent = gtk_widget_get_parent(GTK_WIDGET(widget));
	frame_name = (gchar *)OBJ_GET(parent,"rescaler_frame");
	if (!frame_name)
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": widget_grab()\n\t\"rescale_frame\" key could NOT be found\n"));
		return FALSE;
	}

	frame = lookup_widget( frame_name);
	if ((total_marked > 0) && (frame != NULL))
		gtk_widget_set_sensitive(GTK_WIDGET(frame),TRUE);
	else
		gtk_widget_set_sensitive(GTK_WIDGET(frame),FALSE);

	return FALSE;	/* Allow other handles to run...  */

}


/*
 \brief notebook_page_changed() is fired off whenever a new notebook page 
 is chosen.
 This function just sets a variable marking the current page.  this is to
 prevent the runtime sliders from being updated if they aren't visible
 \param notebook (GtkNotebook *) nbotebook that emitted the event
 \param page (GtkNotebookPage *) page
 \param page_no (guint) page number that's now active
 \param data (gpointer) unused
 */
EXPORT void notebook_page_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	gint tab_ident = 0;
	gint sub_page = 0;
	extern gboolean forced_update;
	extern gboolean rt_forced_update;
	GtkWidget *sub = NULL;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	tab_ident = (TabIdent)OBJ_GET(widget,"tab_ident");
	active_page = tab_ident;

	if (active_page == RUNTIME_TAB)
		rt_forced_update = TRUE;

	if ((OBJ_GET(widget,"table_num")) && GTK_WIDGET_SENSITIVE(widget))
		active_table = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
	else
		active_table = -1;

	if (OBJ_GET(widget,"sub-notebook"))
	{
/*		printf(" This tab has a sub-notebook\n"); */
		sub = lookup_widget( (OBJ_GET(widget,"sub-notebook")));
		if (GTK_IS_WIDGET(sub))
		{
			sub_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(sub));
			widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(sub),sub_page);
/*			printf("subtable found, searching for active page\n"); */
			if ((OBJ_GET(widget,"table_num")) && GTK_WIDGET_SENSITIVE(widget))
			{
				active_table = (gint)strtol((gchar *)OBJ_GET(widget,"table_num"),NULL,10);
				/*printf("found it,  active table %i\n",active_table);*/
			}
			else
			{
				active_table = -1;
/*			 	printf("didn't find table_num key on subtable\n"); */
			}
			 
		}
	}


	forced_update = TRUE;

	return;
}


/*
 \brief subtab_changed() is fired off whenever a new sub-notebook page is 
 chosen.
 This fucntion just sets a variable marking the current page.  this is to
 prevent the runtime sliders from being updated if they aren't visible
 \param notebook (GtkNotebook *) nbotebook that emitted the event
 \param page (GtkNotebookPage *) page
 \param page_no (guint) page number that's now active
 \param data (gpointer) unused
 */
EXPORT void subtab_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	extern gboolean forced_update;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	if (OBJ_GET(widget,"table_num"))
		active_table = (gint)strtol((gchar *)OBJ_GET(widget,"table_num"),NULL,10);
	else
		return;
	/*printf("active table changed to %i\n",active_table); */
	forced_update = TRUE;

	return;
}


/*!
 \brief swap_labels() is a utility function that extracts the list passed to 
 it, and kicks off a subfunction to do the swapping on each widget in the list
 \param input (gchar *) name of list to enumeration and run the subfunction
 on.
 \param state (gboolean) passed on to subfunction
 the default label
 */
void swap_labels(gchar * input, gboolean state)
{
	GList *list = NULL;
	GtkWidget *widget = NULL;
	gchar **fields = NULL;
	gint i = 0;
	gint num_widgets = 0;

	fields = parse_keys(input,&num_widgets,",");

	for (i=0;i<num_widgets;i++)
	{
		widget = NULL;
		widget = lookup_widget(fields[i]);
		if (GTK_IS_WIDGET(widget))
			switch_labels((gpointer)widget,GINT_TO_POINTER(state));
		else if ((list = get_list(fields[i])) != NULL)
			g_list_foreach(list,switch_labels,GINT_TO_POINTER(state));
	}
	g_strfreev(fields);

}
/*!
 \brief switch_labels() swaps labels that depend on the state of another 
 control. Handles temp dependant labels as well..
 \param key (gpointer) gpointer encapsulation of the widget
 \param data (gpointer)  gpointer encapsulation of the target state if TRUE 
 we use the alternate label, if FALSE we use
 the default label
 */
void switch_labels(gpointer key, gpointer data)
{
	GtkWidget * widget = (GtkWidget *) key;
	gboolean state = (GBOOLEAN) data;
	gint temp_units;

	temp_units = (GINT)OBJ_GET(global_data,"temp_units");
	if (GTK_IS_WIDGET(widget))
	{
		if ((GBOOLEAN)OBJ_GET(widget,"temp_dep") == TRUE)
		{
			if (state)
			{
				if (temp_units == FAHRENHEIT)
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"alt_f_label"));
				else
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"alt_c_label"));
			}
			else
			{
				if (temp_units == FAHRENHEIT)
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"f_label"));
				else
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"c_label"));
			}
		}
		else
		{
			if (state)
				gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"alt_label"));
			else
				gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"label"));
		}
	}
}


/*!
 \brief set_algorithm() handles the buttons that deal with the Fueling
 algorithm, as special things need to be taken care of to enable proper
 display of data.
 \param widget (GtkWidget *) the toggle button that changes
 \param data (gpointer) unused in most cases
 \returns TRUE if handles
 */
EXPORT gboolean set_algorithm(GtkWidget *widget, gpointer data)
{
	gint algo = 0; 
	gint tmpi = 0;
	gint i = 0;
	extern gint *algorithm;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	extern gboolean forced_update;

	if (GTK_IS_OBJECT(widget))
	{
		algo = (Algorithm)OBJ_GET(widget,"algorithm");
		tmpbuf = (gchar *)OBJ_GET(widget,"applicable_tables");
	}
	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);
	/* Split string to pieces to grab the list of tables this algorithm
	 * applies to
	 */
	vector = g_strsplit(tmpbuf,",",-1);
	if (!vector)
		return FALSE;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		i = 0;
		while (vector[i])
		{
			tmpi = (gint)strtol(vector[i],NULL,10);
			algorithm[tmpi]=(Algorithm)algo;
			i++;
		}
		forced_update = TRUE;
	}
	g_strfreev(vector);
	return TRUE;
}



/*!
 * \brief dummy handler to prevent window closing
 */
EXPORT gboolean prevent_close(GtkWidget *widget, gpointer data)
{
	return TRUE;
}


/*!
 * \brief prompts user to save internal datalog and ecu backup
 */
void prompt_to_save(void)
{
	gint result = 0;
	extern volatile gboolean offline;
	GtkWidget *dialog = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	GdkPixbuf *pixbuf = NULL;
	GtkWidget *image = NULL;

	if (!offline)
	{
		dialog = gtk_dialog_new_with_buttons(_("Save internal log, yes/no ?"),
				GTK_WINDOW(lookup_widget("main_window")),GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_YES,GTK_RESPONSE_YES,
				GTK_STOCK_NO,GTK_RESPONSE_NO,
				NULL);
		hbox = gtk_hbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,10);
		pixbuf = gtk_widget_render_icon (hbox,GTK_STOCK_DIALOG_QUESTION,GTK_ICON_SIZE_DIALOG,NULL);
		image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_box_pack_start(GTK_BOX(hbox),image,TRUE,TRUE,10);
		label = gtk_label_new(_("Would you like to save the internal datalog for this session to disk?  It is a complete log and useful for playback/analysis at a future point in time"));
		gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,10);
		gtk_widget_show_all(hbox);

		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(lookup_widget("main_window")));

		result = gtk_dialog_run(GTK_DIALOG(dialog));
		g_object_unref(pixbuf);
		if (result == GTK_RESPONSE_YES)
			internal_datalog_dump(NULL,NULL);
		gtk_widget_destroy (dialog);

	}

	dialog = gtk_dialog_new_with_buttons(_("Save ECU settings to file?"),
			GTK_WINDOW(lookup_widget("main_window")),GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_YES,GTK_RESPONSE_YES,
			GTK_STOCK_NO,GTK_RESPONSE_NO,
			NULL);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,10);
	pixbuf = gtk_widget_render_icon (hbox,GTK_STOCK_DIALOG_QUESTION,GTK_ICON_SIZE_DIALOG,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(hbox),image,TRUE,TRUE,10);
	label = gtk_label_new(_("Would you like to save the ECU settings to a file so that they can be restored at a future time?"));
	gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,10);
	gtk_widget_show_all(hbox);

	gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(lookup_widget("main_window")));
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_YES)
		select_file_for_ecu_backup(NULL,NULL);
	gtk_widget_destroy (dialog);

}


/*!
 * \brief prompts user for yes/no to quit
 */
gboolean prompt_r_u_sure(void)
{
	gint result = 0;
	extern volatile gboolean offline;
	GtkWidget *dialog = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	GdkPixbuf *pixbuf = NULL;
	GtkWidget *image = NULL;

	dialog = gtk_dialog_new_with_buttons(_("Quit MegaTunix, yes/no ?"),
			GTK_WINDOW(lookup_widget("main_window")),GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_YES,GTK_RESPONSE_YES,
			GTK_STOCK_NO,GTK_RESPONSE_NO,
			NULL);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,10);
	pixbuf = gtk_widget_render_icon (hbox,GTK_STOCK_DIALOG_QUESTION,GTK_ICON_SIZE_DIALOG,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(hbox),image,TRUE,TRUE,10);
	label = gtk_label_new(_("Are you sure you want to quit?"));
	gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,10);
	gtk_widget_show_all(hbox);

	gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(lookup_widget("main_window")));

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	g_object_unref(pixbuf);
	gtk_widget_destroy (dialog);

	if (result == GTK_RESPONSE_YES)
		return TRUE;
	else 
		return FALSE;
	return FALSE;
}


/*!
 * \brief toggle_groups_linked is used to change the state of controls that
 * are "linked" to various other controls for the purpose of making the 
 * UI more intuitive.  i.e. if u uncheck a feature, this can be used to 
 * grey out a group of related controls.
 * \param toggle_groups, comms sep list of group names
 * \param new_state,  new state of the button linking to these groups
 */
 
void toggle_groups_linked(GtkWidget *widget,gboolean new_state)
{
	gint num_choices = 0;
	gint num_groups = 0;
	gint i = 0;
	gboolean state = FALSE;
	gchar **choices = NULL;
	gchar **groups = NULL;
	gchar * toggle_groups = NULL;
	extern gboolean ready;
	extern GHashTable *widget_group_states;

	if (!ready)
		return;
	toggle_groups = (gchar *)OBJ_GET(widget,"toggle_groups");
/*	printf("groups to toggle %s to state %i\n",toggle_groups,new_state);*/

	choices = parse_keys(toggle_groups,&num_choices,",");
	if (num_choices != 2)
		printf(_("toggle_groups_linked, numeber of choices is out of range, it should be 2, it is %i"),num_choices);

	/*printf("toggle groups defined for widget %p at page %i, offset %i\n",widget,page,offset);*/

	/* Turn off */
	groups = parse_keys(choices[0],&num_groups,":");
/*	printf("Choice %i, has %i groups\n",0,num_groups);*/
	state = FALSE;
	for (i=0;i<num_groups;i++)
	{
/*		printf("setting all widgets in group %s to state %i\n\n",groups[i],state);*/
		g_hash_table_replace(widget_group_states,g_strdup(groups[i]),GINT_TO_POINTER(state));
		g_list_foreach(get_list(groups[i]),alter_widget_state,NULL);
	}
	g_strfreev(groups);
	/* Turn on */
	groups = parse_keys(choices[1],&num_groups,":");
/*	printf("Choice %i, has %i groups\n",1,num_groups);*/
	state = new_state;
	for (i=0;i<num_groups;i++)
	{
/*		printf("setting all widgets in group %s to state %i\n\n",groups[i],state);*/
		g_hash_table_replace(widget_group_states,g_strdup(groups[i]),GINT_TO_POINTER(state));
		g_list_foreach(get_list(groups[i]),alter_widget_state,NULL);
	}
	g_strfreev(groups);
	g_strfreev(choices);
}



/*!
 * \brief combo_toggle_groups_linked is used to change the state of controls that
 * are "linked" to various other controls for the purpose of making the 
 * UI more intuitive.  i.e. if u uncheck a feature, this can be used to 
 * grey out a group of related controls.
 * \param widget, combo button
 * \param active, which entry in list was selected
 */
void combo_toggle_groups_linked(GtkWidget *widget,gint active)
{
	gint num_groups = 0;
	gint num_choices = 0;
	gint i = 0;
	gint j = 0;
	gboolean state = FALSE;
	gint page = 0;
	gint offset = 0;

	gchar **choices = NULL;
	gchar **groups = NULL;
	gchar * toggle_groups = NULL;
	extern gboolean ready;
	extern GHashTable *widget_group_states;

	if (!ready)
		return;
	toggle_groups = (gchar *)OBJ_GET(widget,"toggle_groups");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");

	/*printf("toggling combobox groups\n");*/
	choices = parse_keys(toggle_groups,&num_choices,",");
	if (active >= num_choices)
	{
		printf(_("active is %i, num_choices is %i\n"),active,num_choices);
		printf(_("active is out of bounds for widget %s\n"),glade_get_widget_name(widget));
	}
	/*printf("toggle groups defined for combo box %p at page %i, offset %i\n",widget,page,offset);*/

	/* First TURN OFF all non active groups */
	for (i=0;i<num_choices;i++)
	{
		groups = parse_keys(choices[i],&num_groups,":");
		/*printf("Choice %i, has %i groups\n",i,num_groups);*/
		if (i == active)
			continue;
		state = FALSE;
		for (j=0;j<num_groups;j++)
		{
			/*printf("setting all widgets in group %s to state %i\n\n",groups[j],state);*/
			g_hash_table_replace(widget_group_states,g_strdup(groups[j]),GINT_TO_POINTER(state));
			g_list_foreach(get_list(groups[j]),alter_widget_state,NULL);
		}
		g_strfreev(groups);
	}

	/* Then TURN ON all active groups */
	groups = parse_keys(choices[active],&num_groups,":");
	state = TRUE;
	for (j=0;j<num_groups;j++)
	{
		/*printf("setting all widgets for %s in group %s to state %i\n\n",glade_get_widget_name(widget),groups[j],state);*/
		g_hash_table_replace(widget_group_states,g_strdup(groups[j]),GINT_TO_POINTER(state));
		g_list_foreach(get_list(groups[j]),alter_widget_state,NULL);
	}
	g_strfreev(groups);

	/*printf ("DONE!\n\n\n");*/
	g_strfreev(choices);
}




/*!
 * \brief combo_toggle_labels_linked is used to change the state of controls that
 * are "linked" to various other controls for the purpose of making the 
 * UI more intuitive.  i.e. if u uncheck a feature, this can be used to 
 * grey out a group of related controls.
 * \param widget, combo button
 * \param active, which entry in list was selected
 */
void combo_toggle_labels_linked(GtkWidget *widget,gint active)
{
	gint num_groups = 0;
	gint i = 0;
	gchar **groups = NULL;
	gchar * toggle_labels = NULL;
	extern gboolean ready;

	if (!ready)
		return;
	toggle_labels = (gchar *)OBJ_GET(widget,"toggle_labels");

	groups = parse_keys(toggle_labels,&num_groups,",");
	/* toggle_labels is a list of groups of widgets that need to have their 
	 * label reset to a specific one from an array bound to that widget.
	 * So we get the names of those groups, and call "set_widget_label_from_array"
	 * passing in the index of the one in the array we want
	 */
	for (i=0;i<num_groups;i++)
		g_list_foreach(get_list(groups[i]),set_widget_label_from_array,GINT_TO_POINTER(active));

	g_strfreev(groups);
}


void set_widget_label_from_array(gpointer key, gpointer data)
{
	gchar *labels = NULL;
	gchar **vector = NULL;
	GtkWidget *label = (GtkWidget *)key;
	gint index = (GINT)data;

	if (!GTK_IS_LABEL(label))
		return;
	labels = OBJ_GET(label,"labels");
	if (!labels)
		return;
	vector = g_strsplit(labels,",",-1);
	if (index > g_strv_length(vector))
		return;
	gtk_label_set_text(GTK_LABEL(label),vector[index]);
	g_strfreev(vector);
	return;
}



gboolean search_model(GtkTreeModel *model, GtkWidget *box, GtkTreeIter *iter)
{
	gchar *choice = NULL;
	gboolean valid = TRUE;
	gchar * cur_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(box));
	valid = gtk_tree_model_get_iter_first(model,iter);
	while (valid)
	{
		gtk_tree_model_get(model,iter,CHOICE_COL, &choice, -1);
		if (g_ascii_strcasecmp(cur_text,choice) == 0)
		{
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(box),iter);
			return TRUE;
		}
		valid = gtk_tree_model_iter_next (model, iter);
	}
	return FALSE;
}


gint get_choice_count(GtkTreeModel *model)
{
	gchar *choice = NULL;
	gboolean valid = TRUE;
	GtkTreeIter iter;
	gint i = 0;

	valid = gtk_tree_model_get_iter_first(model,&iter);
	while (valid)
	{
		gtk_tree_model_get(model,&iter,CHOICE_COL, &choice, -1);
		valid = gtk_tree_model_iter_next (model, &iter);
		i++;
	}
	return i;
}

guint get_bitshift(guint mask)
{
	gint i = 0;
	for (i=0;i<32;i++)
		if (mask & (1 << i))
			return i;
	return 0;
}

EXPORT void update_misc_gauge(DataWatch *watch)
{
	if (MTX_IS_GAUGE_FACE(watch->user_data))
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(watch->user_data),watch->val);
	else
		remove_watch(watch->id);
}

void refresh_widgets_at_offset(gint page, gint offset)
{
	guint i = 0;
	extern GList ***ve_widgets;
	extern Firmware_Details *firmware;

	/*printf("Refresh widget at page %i, offset %i\n",page,offset);*/


	for (i=0;i<g_list_length(ve_widgets[page][offset]);i++)
		update_widget(g_list_nth_data(ve_widgets[page][offset],i),NULL);
	update_ve3d_if_necessary(page,offset);
}


glong get_extreme_from_size(DataSize size,Extreme limit)
{
	glong lower_limit = 0;
	glong upper_limit = 0;

	switch (size)
	{
		case MTX_CHAR:
		case MTX_S08:
			lower_limit = G_MININT8;
			upper_limit = G_MAXINT8;
			break;
		case MTX_U08:
			lower_limit = 0;
			upper_limit = G_MAXUINT8;
			break;
		case MTX_S16:
			lower_limit = G_MININT16;
			upper_limit = G_MAXINT16;
			break;
		case MTX_U16:
			lower_limit = 0;
			upper_limit = G_MAXUINT16;
			break;
		case MTX_S32:
			lower_limit = G_MININT32;
			upper_limit = G_MAXINT32;
			break;
		case MTX_U32:
			lower_limit = 0;
			upper_limit = G_MAXUINT32;
			break;
		case MTX_UNDEF:
			break;
	}
	switch (limit)
	{
		case LOWER:
			return lower_limit;
			break;
		case UPPER:
			return upper_limit;
			break;
	}
	return 0;
}


/* Clamps a value to it's limits and updates if needed */
EXPORT gboolean clamp_value(GtkWidget *widget, gpointer data)
{
	gint lower = 0;
	gint upper = 0;
	gint precision = 0;
	gfloat val = 0.0;
	gboolean clamped = FALSE;

	if (OBJ_GET(widget,"raw_lower"))
		lower = (gint)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		lower = get_extreme_from_size((DataSize)OBJ_GET(widget,"size"),LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		upper = (gint)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		upper = get_extreme_from_size((DataSize)OBJ_GET(widget,"size"),UPPER);
	precision = (GINT)OBJ_GET(widget,"precision");

	val = g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);

	if (val > upper)
	{
		val = upper;
		clamped = TRUE;
	}
	if (val < lower)
	{
		val = lower;
		clamped = TRUE;
	}
	if (clamped)
		gtk_entry_set_text(GTK_ENTRY(widget),g_strdup_printf("%1$.*2$f",val,precision));
	return TRUE;
}


/*!
 \brief recalc_table_limits() Finds the minimum and maximum values for a 
 2D table (this will be deprecated when thevetables are a custom widget)
 */
void recalc_table_limits(gint canID, gint table_num)
{
	extern Firmware_Details *firmware;
	gint i = 0;
	gint x_count = 0;
	gint y_count = 0;
	gint z_base = 0;
	gint z_page = 0;
	gint z_size = 0;
	gint z_mult = 0;
	gint tmpi = 0;
	gint max = 0;
	gint min = 0;

	/* Limit check */
	if ((table_num < 0 ) || (table_num > firmware->total_tables-1))
		return;
	firmware->table_params[table_num]->last_z_maxval = firmware->table_params[table_num]->z_maxval;
	firmware->table_params[table_num]->last_z_minval = firmware->table_params[table_num]->z_minval;
	x_count = firmware->table_params[table_num]->x_bincount;
	y_count = firmware->table_params[table_num]->y_bincount;
	z_base = firmware->table_params[table_num]->z_base;
	z_page = firmware->table_params[table_num]->z_page;
	z_size = firmware->table_params[table_num]->z_size;
	z_mult = get_multiplier(z_size);
	min = get_extreme_from_size(z_size,UPPER);
	max = get_extreme_from_size(z_size,LOWER);

	for (i=0;i<x_count*y_count;i++)
	{
		tmpi = get_ecu_data(canID,z_page,z_base+(i*z_mult),z_size);
		if (tmpi > max)
			max = tmpi;
		if (tmpi < min)
			min = tmpi;
	}
	if (min == max)	/* FLAT table, causes black screen */
	{
		min -= 10;
		max += 10;
	}
	firmware->table_params[table_num]->z_maxval = max;
	firmware->table_params[table_num]->z_minval = min;
	return;
}


gboolean update_multi_expression(gpointer data)
{
	g_list_foreach(get_list("multi_expression"),update_widget,NULL);	
	return FALSE;
}


void refocus_cell(GtkWidget *widget, Direction dir)
{
	gchar *widget_name = NULL;
	GtkWidget *widget_2_focus = NULL;
	gchar *ptr = NULL;
	gchar *tmpbuf = NULL;
	gchar *prefix = NULL;
	gboolean return_now = FALSE;
	gint table_num = -1;
	gint row = -1;
	gint col = -1;
	gint index = -1;
	gint count = -1;
	extern Firmware_Details *firmware;


	widget_name = OBJ_GET(widget,"fullname");
	if (!widget_name)
		return;
	if (OBJ_GET(widget,"table_num"))
		table_num = (gint) strtol(OBJ_GET(widget,"table_num"),NULL,10);
	else
		return;
	
	ptr = g_strrstr_len(widget_name,strlen(widget_name),"_of_");
	ptr = g_strrstr_len(widget_name,ptr-widget_name,"_");
	tmpbuf = g_strdelimit(g_strdup(ptr),"_",' ');
	prefix = g_strndup(widget_name,ptr-widget_name);
	sscanf(tmpbuf,"%d of %d",&index, &count);
	g_free(tmpbuf);
	row = index/firmware->table_params[table_num]->x_bincount;
	col = index%firmware->table_params[table_num]->x_bincount;

	switch (dir)
	{
		case GO_LEFT:
			if (col == 0)
				return_now = TRUE;
			else
				col--;
			break;
		case GO_RIGHT:
			if (col > firmware->table_params[table_num]->x_bincount-2)
				return_now = TRUE;
			else
				col++;
			break;
		case GO_UP:
			if (row > firmware->table_params[table_num]->y_bincount-2)
				return_now = TRUE;
			else
				row++;
			break;
		case GO_DOWN:
			if (row == 0)
				return_now = TRUE;
			else
				row--;
			break;
	}
	if (return_now)
		return;
	tmpbuf = g_strdup_printf("%s_%i_of_%i",prefix,col+(row*firmware->table_params[table_num]->x_bincount),count);

	widget_2_focus = lookup_widget(tmpbuf);
	if (GTK_IS_WIDGET(widget_2_focus))
		gtk_widget_grab_focus(widget_2_focus);

	g_free(tmpbuf);
}

