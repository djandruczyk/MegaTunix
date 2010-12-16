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
#include <3d_vetable.h>
#include <args.h>
#include <config.h>
#include <ctype.h>
#include <combo_loader.h>
#include <conversions.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
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
#include <plugin.h>
#include <rtv_processor.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringmatch.h>
#include <tabloader.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <vetable_gui.h>
#include <widgetmgmt.h>



static gboolean grab_single_cell = FALSE;
static gboolean grab_multi_cell = FALSE;
extern gconstpointer *global_data;

extern GdkColor red;
extern GdkColor green;
extern GdkColor blue;
extern GdkColor black;
extern GdkColor white;


/*!
 \brief leave() is the main shutdown function for MegaTunix. It shuts down
 whatever runnign handlers are still going, deallocates memory and quits
 \param widget (GtkWidget *) unused
 \param data (gpointer) quiet or not quiet, leave mode .quiet doesn't prompt 
 to save anything
 */
G_MODULE_EXPORT gboolean leave(GtkWidget *widget, gpointer data)
{
	extern gint pf_dispatcher_id;
	extern gint gui_dispatcher_id;
	/*
	   extern GThread * ascii_socket_id;
	   extern GThread * binary_socket_id;
	   extern GThread * control_socket_id;
	 */
	GMutex *serio_mutex = NULL;
	GMutex *rtv_mutex = NULL;
	GAsyncQueue *io_repair_queue = NULL;
	gboolean tmp = TRUE;
	GIOChannel * iochannel = NULL;
	GTimeVal now;
	static GStaticMutex leave_mutex = G_STATIC_MUTEX_INIT;
	CmdLineArgs *args = DATA_GET(global_data,"args");
	GMutex *mutex = g_mutex_new();
	GCond *pf_dispatch_cond = NULL;
	GCond *gui_dispatch_cond = NULL;
	GCond *io_dispatch_cond = NULL;
	GCond *statuscounts_cond = NULL;
	Firmware_Details *firmware = NULL;


	firmware = DATA_GET(global_data,"firmware");

	if (DATA_GET(global_data,"leaving"))
		return TRUE;

	if ((!args->be_quiet) && (DATA_GET(global_data,"interrogated")))
	{
		DATA_SET(global_data,"might_be_leaving",GINT_TO_POINTER(TRUE));
		if(!prompt_r_u_sure())
		{
			DATA_SET(global_data,"might_be_leaving",GINT_TO_POINTER(FALSE));
			return TRUE;
		}
		prompt_to_save();
	}

	DATA_SET(global_data,"leaving",GINT_TO_POINTER(TRUE));
	/* Stop timeout functions */

	plugins_shutdown();
	stop_tickler(RTV_TICKLER);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_realtime\n"));
	stop_tickler(LV_PLAYBACK_TICKLER);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_lv_playback\n"));

	stop_datalogging();
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after stop_datalogging\n"));

	/* Message to trigger serial repair queue to exit immediately */
	io_repair_queue = DATA_GET(global_data,"io_repair_queue");
	if (io_repair_queue)
		g_async_queue_push(io_repair_queue,&tmp);

	/* Commits any pending data to ECU flash */
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() before burn\n"));
	if ((DATA_GET(global_data,"connected")) && 
			(DATA_GET(global_data,"interrogated")) && 
			(!DATA_GET(global_data,"offline")))
		io_cmd(firmware->burn_all_command,NULL);
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after burn\n"));

	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() configuration saved\n"));
	g_static_mutex_lock(&leave_mutex);


	g_mutex_lock(mutex);

	/* IO dispatch queue */
	g_get_current_time(&now);
	g_time_val_add(&now,250000);
	io_dispatch_cond = DATA_GET(global_data,"io_dispatch_cond");
	g_cond_timed_wait(io_dispatch_cond,mutex,&now);

	/* PF dispatch queue */
	if (pf_dispatcher_id)
		g_source_remove(pf_dispatcher_id);
	g_get_current_time(&now);
	g_time_val_add(&now,250000);
	pf_dispatch_cond = DATA_GET(global_data,"pf_dispatch_cond");
	g_cond_timed_wait(pf_dispatch_cond,mutex,&now);

	/* Statuscounts timeout */
	if (DATA_GET(global_data,"statuscounts_id"))
		g_source_remove((gint)DATA_GET(global_data,"statuscounts_id"));
	DATA_SET(global_data,"statuscounts_id",GINT_TO_POINTER(0));
	g_get_current_time(&now);
	g_time_val_add(&now,250000);
	statuscounts_cond = DATA_GET(global_data,"statuscounts_cond");
	g_cond_timed_wait(statuscounts_cond,mutex,&now);

	/* GUI Dispatch timeout */
	if (gui_dispatcher_id)
		g_source_remove(gui_dispatcher_id);
	gui_dispatcher_id = 0;
	g_get_current_time(&now);
	g_time_val_add(&now,250000);
	gui_dispatch_cond = DATA_GET(global_data,"gui_dispatch_cond");
	g_cond_timed_wait(gui_dispatch_cond,mutex,&now);

	g_mutex_unlock(mutex);

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

	if (lookup_widget("dlog_select_log_button"))
		iochannel = (GIOChannel *) OBJ_GET(lookup_widget("dlog_select_log_button"),"data");

	if (iochannel)	
	{
		g_io_channel_shutdown(iochannel,TRUE,NULL);
		g_io_channel_unref(iochannel);
	}
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() after iochannel\n"));

	rtv_mutex = DATA_GET(global_data,"rtv_mutex");
	g_mutex_lock(rtv_mutex);  /* <-- this makes us wait */
	g_mutex_unlock(rtv_mutex); /* now unlock */

	close_serial();
	unlock_serial();

	/* Grab and release all mutexes to get them to relinquish
	 */
	serio_mutex = DATA_GET(global_data,"serio_mutex");
	g_mutex_lock(serio_mutex);
	g_mutex_unlock(serio_mutex);
	/* Free all buffers */
	mem_dealloc();
	dbg_func(CRITICAL,g_strdup_printf(__FILE__": LEAVE() mem deallocated, closing log and exiting\n"));
	close_debug();
	exit(0);
	return TRUE;
}


/*!
 \brief comm_port_override() is called every time the comm port text entry
 changes. it'll try to open the port and if it does it'll setup the serial 
 parameters
 \param editable (GtkEditable *) pointer to editable widget to extract text from
 \returns TRUE
 */
G_MODULE_EXPORT gboolean comm_port_override(GtkEditable *editable)
{
	gchar *port;

	port = gtk_editable_get_chars(editable,0,-1);
	gtk_widget_modify_text(GTK_WIDGET(editable),GTK_STATE_NORMAL,&black);
	g_free(DATA_GET(global_data,"override_port"));
	DATA_SET_FULL(global_data,"override_port",g_strdup(port),g_free);
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
G_MODULE_EXPORT gboolean toggle_button_handler(GtkWidget *widget, gpointer data)
{
	static GtkSettings *settings = NULL;
	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;
	void *obj_data = NULL;
	void (*function)(void);
	gint handler = 0; 
	gchar * tmpbuf = NULL;
	gboolean *tracking_focus = NULL;
	extern gconstpointer * global_data;
	tracking_focus = (gboolean *)DATA_GET(global_data,"tracking_focus");

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
				DATA_SET(global_data,"network_access",GINT_TO_POINTER(TRUE));
				get_symbol("open_tcpip_sockets",(void*)&function);
				function();
				break;
			case COMM_AUTODETECT:
				DATA_SET(global_data,"autodetect_port", GINT_TO_POINTER(TRUE));
				gtk_entry_set_editable(GTK_ENTRY(lookup_widget("active_port_entry")),FALSE);
				break;
			case OFFLINE_FIRMWARE_CHOICE:
				DATA_SET(global_data,"offline_firmware_choice", g_strdup(OBJ_GET(widget,"filename")));
				break;
			case TRACKING_FOCUS:
				tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
				tracking_focus[(gint)strtol(tmpbuf,NULL,10)] = TRUE;
				break;
			case TOOLTIPS_STATE:
				if (!settings)
					settings = gtk_settings_get_default();
				if (gtk_minor_version >= 14)
					g_object_set(settings,"gtk-enable-tooltips",TRUE,NULL);
				DATA_SET(global_data,"tips_in_use",GINT_TO_POINTER(TRUE));
				break;
			case FAHRENHEIT:
				DATA_SET(global_data,"temp_units",GINT_TO_POINTER(FAHRENHEIT));
				reset_temps(GINT_TO_POINTER(FAHRENHEIT));
				DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
				break;
			case CELSIUS:
				DATA_SET(global_data,"temp_units",GINT_TO_POINTER(CELSIUS));
				reset_temps(GINT_TO_POINTER(CELSIUS));
				DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
				break;
			case COMMA:
				DATA_SET(global_data,"preferred_delimiter",GINT_TO_POINTER(COMMA));
				update_logbar("dlog_view", NULL,_("Setting Log delimiter to a \"Comma\"\n"),FALSE,FALSE,FALSE);
				DATA_SET_FULL(global_data,"delimiter",g_strdup(","),cleanup);
				break;
			case TAB:
				DATA_SET(global_data,"preferred_delimiter",GINT_TO_POINTER(TAB));
				update_logbar("dlog_view", NULL,_("Setting Log delimiter to a \"Tab\"\n"),FALSE,FALSE,FALSE);
				DATA_SET_FULL(global_data,"delimiter",g_strdup("\t"),cleanup);
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
				if (!common_handler)
				{
					if (get_symbol("common_toggle_button_handler",(void *)&common_handler))
						return common_handler(widget,data);
					else
					{
						dbg_func(CRITICAL,g_strdup_printf(__FILE__": toggle_button_handler()\n\tToggle button handler in common plugin is MISSING, BUG!\n"));
						return FALSE;
					}
				}
				else
					return common_handler(widget,data);
				break;
		}
	}
	else
	{	/* not pressed */
		switch ((ToggleButton)handler)
		{
			case TOGGLE_NETMODE:
				DATA_SET(global_data,"network_access",GINT_TO_POINTER(FALSE));
				break;
			case COMM_AUTODETECT:
				DATA_SET(global_data,"autodetect_port", GINT_TO_POINTER(FALSE));
				gtk_entry_set_editable(GTK_ENTRY(lookup_widget("active_port_entry")),TRUE);
				gtk_entry_set_text(GTK_ENTRY(lookup_widget("active_port_entry")),DATA_GET(global_data,"override_port"));
				break;
			case TRACKING_FOCUS:
				tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
				tracking_focus[(gint)strtol(tmpbuf,NULL,10)] = FALSE;
				break;
			case TOOLTIPS_STATE:
				if (!settings)
					settings = gtk_settings_get_default();
				if (gtk_minor_version >= 14)
					g_object_set(settings,"gtk-enable-tooltips",FALSE,NULL);
				DATA_SET(global_data,"tips_in_use",GINT_TO_POINTER(FALSE));
				break;
			case FAHRENHEIT:
			case CELSIUS:
			case COMMA:
			case TAB:
			case OFFLINE_FIRMWARE_CHOICE:
			case HEX_VIEW:
			case DECIMAL_VIEW:
			case BINARY_VIEW:
			case REALTIME_VIEW:
			case PLAYBACK_VIEW:
				/* Not pressed, just break */
				break;
			default:
				if (!common_handler)
				{
					if (get_symbol("common_toggle_button_handler",(void *)&common_handler))
						return common_handler(widget,data);
					else
					{
						dbg_func(CRITICAL,g_strdup_printf(__FILE__": toggle_button_handler()\n\tToggle button handler in common plugin is MISSING, BUG!\n"));
						return FALSE;
					}
				}
				else
					return common_handler(widget,data);
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
G_MODULE_EXPORT gboolean bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!common_handler)
	{
		if (get_symbol("common_bitmask_button_handler",(void *)&common_handler))
			return common_handler(widget,data);
		else
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": bitmask_button_handler()\n\tiBitmask button handler in common plugin is MISSING, BUG!\n"));
			return FALSE;
		}
	}
	else
		return common_handler(widget,data);
}



/*!
 \brief entry_changed_handler() gets called anytime a text entries is changed
 (i.e. during edit) it's main purpose is to turn the entry red to signify
 to the user it's being modified but not yet SENT to the ecu
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
G_MODULE_EXPORT gboolean entry_changed_handler(GtkWidget *widget, gpointer data)
{
	if ((DATA_GET(global_data,"paused_handlers")) || 
			(!DATA_GET(global_data,"ready")))
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
G_MODULE_EXPORT gboolean focus_out_handler(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	if (OBJ_GET(widget,"not_sent"))
	{
		OBJ_SET(widget,"not_sent",NULL);
		if (GTK_IS_SPIN_BUTTON(widget))
			spin_button_handler(widget, data);
		else if (GTK_IS_ENTRY(widget))
			std_entry_handler(widget, data);
		else if (GTK_IS_COMBO_BOX(widget))
			std_combo_handler(widget, data);
	}
	return FALSE;
}


/*!
 * \brief slider_value_changed() handles controls based upon a slider
 * sort of like spinbutton controls
 */
G_MODULE_EXPORT gboolean slider_value_changed(GtkWidget *widget, gpointer data)
{
	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}
	if (!common_handler)
	{
		if (get_symbol("common_slider_handler",(void *)&common_handler))
			return common_handler(widget,data);
		else
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": slider_value_changes()\n\tSlider handler in common plugin is MISSING, BUG!\n"));
			return FALSE;
		}
	}
	else
		return common_handler(widget,data);

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
G_MODULE_EXPORT gboolean std_entry_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}
	if (!common_handler)
	{
		if (get_symbol("common_entry_handler",(void *)&common_handler))
			return common_handler(widget,data);
		else
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": std_entry_handler()\n\tEntry handler in common plugin is MISSING, BUG!\n"));
			return FALSE;
		}
	}
	else
		return common_handler(widget,data);
}



/*!
 \brief std_button_handler() handles all standard non toggle/check/radio
 buttons. 
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
G_MODULE_EXPORT gboolean std_button_handler(GtkWidget *widget, gpointer data)
{
	/* get any datastructures attached to the widget */

	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;
	void *obj_data = NULL;
	gint handler = -1;
	Firmware_Details *firmware = NULL;
	void (*select_for)(gint) = NULL;
	void (*revert)(void) = NULL;
	gboolean (*create_2d_table_editor)(gint,GtkWidget *) = NULL;
	gboolean (*create_2d_table_editor_group)(GtkWidget *) = NULL;

	firmware = DATA_GET(global_data,"firmware");

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
		case EXPORT_SINGLE_TABLE:
			if (OBJ_GET(widget,"table_num"))
				if(get_symbol("select_table_for_export",(void*)&select_for))
					select_for((gint)strtol(OBJ_GET(widget,"table_num"),NULL,10));
			break;
		case IMPORT_SINGLE_TABLE:
			if (OBJ_GET(widget,"table_num"))
				if(get_symbol("select_table_for_import",(void*)&select_for))
					select_for((gint)strtol(OBJ_GET(widget,"table_num"),NULL,10));
			break;
		case RESCALE_TABLE:
			rescale_table(widget);
			break;
		case INTERROGATE_ECU:
			set_title(g_strdup(_("User initiated interrogation...")));
			update_logbar("interr_view","warning",_("USER Initiated ECU interrogation...\n"),FALSE,FALSE,FALSE);
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
			if (DATA_GET(global_data,"offline"))
				break;
			if (!DATA_GET(global_data,"interrogated"))
				io_cmd("interrogation", NULL);
			start_tickler(RTV_TICKLER);
			DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
			break;
		case STOP_REALTIME:
			if (DATA_GET(global_data,"offline"))
				break;
			stop_tickler(RTV_TICKLER);
			break;
		case READ_VE_CONST:
			set_title(g_strdup(_("Reading VE/Constants...")));
			io_cmd(firmware->get_all_command, NULL);
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
			if (DATA_GET(global_data,"offline"))
				break;
			stop_datalogging();
			break;
		case START_DATALOGGING:
			if (DATA_GET(global_data,"offline"))
				break;
			start_datalogging();
			break;
		case STOP_DATALOGGING:
			if (DATA_GET(global_data,"offline"))
				break;
			stop_datalogging();
			break;
		case REVERT_TO_BACKUP:
			if (get_symbol("revert_to_previous_data",(void*)&revert))
				revert();
			break;
		case SELECT_PARAMS:
			if (!DATA_GET(global_data,"interrogated"))
				break;
			gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("logviewer_select_logfile_button")),FALSE);
			present_viewer_choices();
			break;
		case OFFLINE_MODE:
			set_title(g_strdup(_("Offline Mode...")));
			g_timeout_add(100,(GSourceFunc)set_offline_mode,NULL);
			break;
		case TE_TABLE:
			if (OBJ_GET(widget,"te_table_num"))
				if (get_symbol("create_2d_table_editor",(void *)&create_2d_table_editor))
					create_2d_table_editor((gint)strtol(OBJ_GET(widget,"te_table_num"),NULL,10), NULL);
			break;
		case TE_TABLE_GROUP:
			if (get_symbol("create_2d_table_editor_group",(void *)&create_2d_table_editor_group))
				create_2d_table_editor_group(widget);
			break;

		default:
			if (!common_handler)
			{
				if (get_symbol("common_button_handler",(void *)&common_handler))
					return common_handler(widget,data);
				else
					dbg_func(CRITICAL,g_strdup(__FILE__": std_button_handler()\n\tDefault case, common handler NOT found in plugins, BUG!\n"));
			}
			else
				return common_handler(widget,data);
			break;
	}		
	return TRUE;
}


/*!
 \brief std_combo_handler() handles all combo boxes
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
G_MODULE_EXPORT gboolean std_combo_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}
	if (!common_handler)
	{
		if (get_symbol("common_combo_handler",(void *)&common_handler))
			return common_handler(widget,data);
		else
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": std_combo_handler()\n\tCombo handler in common plugin is MISSING, BUG!\n"));
			return FALSE;
		}
	}
	else
		return common_handler(widget,data);
	return TRUE;
}


/*!
 \brief spin_button_handler() handles ALL spinbuttons in MegaTunix and does
 whatever is needed for the data before sending it to the ECU
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
G_MODULE_EXPORT gboolean spin_button_handler(GtkWidget *widget, gpointer data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	static gboolean (*common_handler)(GtkWidget *, gpointer) = NULL;
	gint tmpi = 0;
	gint handler = -1;
	gint source = 0;
	gfloat value = 0.0;
	GtkWidget * tmpwidget = NULL;
	GHashTable **interdep_vars = NULL;
	Serial_Params *serial_params = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	interdep_vars = DATA_GET(global_data,"interdep_vars");
	serial_params = DATA_GET(global_data,"serial_params");

	if (!GTK_IS_WIDGET(widget))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\twidget pointer is NOT valid\n"));
		return FALSE;
	}
	if ((DATA_GET(global_data,"paused_handlers")) || 
			(!DATA_GET(global_data,"ready")))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	handler = (MtxButton)OBJ_GET(widget,"handler");
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);

	tmpi = (int)(value+.001);


	switch ((MtxButton)handler)
	{
		case SER_INTERVAL_DELAY:
			serial_params->read_wait = (gint)value;
			break;
		case SER_READ_TIMEOUT:
			DATA_SET(global_data,"read_timeout",GINT_TO_POINTER((gint)value));
			break;
		case RTSLIDER_FPS:
			DATA_SET(global_data,"rtslider_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)DATA_GET(global_data,"rtslider_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000/(float)tmpi),(GSourceFunc)update_rtsliders,NULL);
			DATA_SET(global_data,"rtslider_id",GINT_TO_POINTER(tmpi));
			break;
		case RTTEXT_FPS:
			DATA_SET(global_data,"rttext_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)DATA_GET(global_data,"rttext_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000.0/(float)tmpi),(GSourceFunc)update_rttext,NULL);
			DATA_SET(global_data,"rttext_id",GINT_TO_POINTER(tmpi));
			break;
		case DASHBOARD_FPS:
			DATA_SET(global_data,"dashboard_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)DATA_GET(global_data,"dashboard_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000.0/(float)tmpi),(GSourceFunc)update_dashboards,NULL);
			DATA_SET(global_data,"dashboard_id",GINT_TO_POINTER(tmpi));
			break;
		case VE3D_FPS:
			DATA_SET(global_data,"ve3d_fps",GINT_TO_POINTER(tmpi));
			source = (GINT)DATA_GET(global_data,"ve3d_id");
			if (source)
				g_source_remove(source);
			tmpi = g_timeout_add((gint)(1000.0/(float)tmpi),(GSourceFunc)update_ve3ds,NULL);
			DATA_SET(global_data,"ve3d_id",GINT_TO_POINTER(tmpi));
			break;
		case LOGVIEW_ZOOM:
			DATA_SET(global_data,"lv_zoom",GINT_TO_POINTER(tmpi));
			tmpwidget = lookup_widget("logviewer_trace_darea");	
			if (tmpwidget)
				lv_configure_event(lookup_widget("logviewer_trace_darea"),NULL,NULL);
			/*	g_signal_emit_by_name(tmpwidget,"configure_event",NULL);*/
			break;
		default:
			if (!common_handler)
			{
				if (get_symbol("common_spin_button_handler",(void *)&common_handler))
					return common_handler(widget,data);
				else
				{
					dbg_func(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\tDefault case, common handler NOT found in plugins, BUG!\n"));
					return TRUE;
				}
			}
			else
				return common_handler(widget,data);
			break;
	}
	return TRUE;
}



/*!
 \brief key_event() is triggered when a key is pressed on a widget that
 has a key_press/release_event handler registered.
 \param widget (GtkWidget *) widget where the event occurred
 \param event (GdkEventKey) event struct, (contains key that was pressed)
 \param data (gpointer) unused
 */
G_MODULE_EXPORT gboolean key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	static gint (*get_ecu_data_f)(gpointer) = NULL;
	static void (*send_to_ecu_f)(gpointer, gint, gboolean) = NULL;
	/*static void (*ms_send_to_ecu_f)(gint, gint, gint, DataSize, gint, gboolean) = NULL;*/
	static void (*update_widget_f)(gpointer, gpointer);
	static GList ***ecu_widgets = NULL;
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
	gboolean *tracking_focus = NULL;
	Firmware_Details *firmware = NULL;

	if (!ecu_widgets)
		ecu_widgets = DATA_GET(global_data,"ecu_widgets");
	if (!get_ecu_data_f)
		get_symbol("get_ecu_data",(void *)&get_ecu_data_f);
	if (!send_to_ecu_f)
		get_symbol("send_to_ecu",(void *)&send_to_ecu_f);
	/*
	if (!ms_send_to_ecu_f)
		get_symbol("ms_send_to_ecu",(void *)&ms_send_to_ecu_f);
		*/
	if (!update_widget_f)
		get_symbol("update_widget",(void *)&update_widget_f);

	firmware = DATA_GET(global_data,"firmware");
	tracking_focus = (gboolean *)DATA_GET(global_data,"tracking_focus");


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
	value = get_ecu_data_f(widget);
	DATA_SET(global_data,"active_table",GINT_TO_POINTER(active_table));

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
			if (tracking_focus[active_table])
				tracking_focus[active_table] = FALSE;
			else
				tracking_focus[active_table] = TRUE;
			retval = TRUE;
			break;
		case GDK_Escape:
			update_widget_f(widget,NULL);
			retval = FALSE;
			break;
		case GDK_Return:
		case GDK_KP_Enter:
			if (GTK_IS_SPIN_BUTTON(widget))
				spin_button_handler(widget, NULL);
			else
				std_entry_handler(widget,NULL);
			retval = FALSE;
			break;
		default:	
			retval = FALSE;
			break;
	}
	if (send)
		send_to_ecu_f(widget,dload_val,TRUE);
	return retval;
}


G_MODULE_EXPORT void insert_text_handler(GtkEntry *entry, const gchar *text, gint len, gint *pos, gpointer data)
{
	gint count = 0;
	gint i = 0;
	GtkEditable *editable = GTK_EDITABLE(entry);
	gchar *result = g_new (gchar, len);
	for (i=0; i < len; i++) 
	{
		if ((g_ascii_isdigit(text[i])) || g_ascii_ispunct(text[i]))
			result[count++] = text[i];
	}
	if (count > 0)
	{
		g_signal_handlers_block_by_func (G_OBJECT (editable),
				G_CALLBACK (insert_text_handler),
				data);
		gtk_editable_insert_text (editable, result, count, pos);
		g_signal_handlers_unblock_by_func (G_OBJECT (editable),
				G_CALLBACK (insert_text_handler),
				data);
	}
	g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

	g_free (result);
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
G_MODULE_EXPORT gboolean widget_grab(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gboolean marked = FALSE;
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
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

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
G_MODULE_EXPORT void notebook_page_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	gint tab_ident = 0;
	gint sub_page = 0;
	gint active_table = -1;
	GtkWidget *sub = NULL;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	tab_ident = (TabIdent)OBJ_GET(widget,"tab_ident");
	DATA_SET(global_data,"active_page",GINT_TO_POINTER(tab_ident));

	if (tab_ident == RUNTIME_TAB)
		DATA_SET(global_data,"rt_forced_update",GINT_TO_POINTER(TRUE));

#if GTK_MINOR_VERSION >= 18
	if ((OBJ_GET(widget,"table_num")) && (gtk_widget_get_state(widget) != GTK_STATE_INSENSITIVE))
#else
	if ((OBJ_GET(widget,"table_num")) && (GTK_WIDGET_STATE(widget) != GTK_STATE_INSENSITIVE))
#endif
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
#if GTK_MINOR_VERSION >= 18
			if ((OBJ_GET(widget,"table_num")) && (gtk_widget_get_state(widget) != GTK_STATE_INSENSITIVE))
#else
			if ((OBJ_GET(widget,"table_num")) && (GTK_WIDGET_SENSITIVE(widget) != GTK_STATE_INSENSITIVE))
#endif
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
	DATA_SET(global_data,"active_table",GINT_TO_POINTER(active_table));
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
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
G_MODULE_EXPORT void subtab_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	gint active_table = -1;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	if (OBJ_GET(widget,"table_num"))
	{
		active_table = (gint)strtol((gchar *)OBJ_GET(widget,"table_num"),NULL,10);
		DATA_SET(global_data,"active_table",GINT_TO_POINTER(active_table));
	}
	else
		return;
	/*printf("active table changed to %i\n",active_table); */
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));

	return;
}



/*!
 \brief set_algorithm() handles the buttons that deal with the Fueling
 algorithm, as special things need to be taken care of to enable proper
 display of data.
 \param widget (GtkWidget *) the toggle button that changes
 \param data (gpointer) unused in most cases
 \returns TRUE if handles
 */
G_MODULE_EXPORT gboolean set_algorithm(GtkWidget *widget, gpointer data)
{
	gint algo = 0; 
	gint tmpi = 0;
	gint i = 0;
	gint *algorithm = NULL;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	extern gconstpointer *global_data;

	algorithm = (gint *)DATA_GET(global_data,"algorithm");

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
		DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	}
	g_strfreev(vector);
	return TRUE;
}



/*!
 * \brief dummy handler to prevent window closing
 */
G_MODULE_EXPORT gboolean prevent_close(GtkWidget *widget, gpointer data)
{
	return TRUE;
}


/*!
 * \brief prompts user to save internal datalog and ecu backup
 */
G_MODULE_EXPORT void prompt_to_save(void)
{
	gint result = 0;
	GtkWidget *dialog = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	GdkPixbuf *pixbuf = NULL;
	GtkWidget *image = NULL;
	void (*do_ecu_backup)(GtkWidget *, gpointer) = NULL;


	if (DATA_GET(global_data,"offline"))
		return;
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
	{
		get_symbol("select_file_for_ecu_backup",(void *)&do_ecu_backup);
		do_ecu_backup(NULL,NULL);
	}
	gtk_widget_destroy (dialog);

}


/*!
 * \brief prompts user for yes/no to quit
 */
G_MODULE_EXPORT gboolean prompt_r_u_sure(void)
{
	gint result = 0;
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


G_MODULE_EXPORT guint get_bitshift(guint mask)
{
	gint i = 0;
	for (i=0;i<32;i++)
		if (mask & (1 << i))
			return i;
	return 0;
}

G_MODULE_EXPORT void update_misc_gauge(DataWatch *watch)
{
	if (MTX_IS_GAUGE_FACE(watch->user_data))
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(watch->user_data),watch->val);
	else
		remove_watch(watch->id);
}


G_MODULE_EXPORT glong get_extreme_from_size(DataSize size,Extreme limit)
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
G_MODULE_EXPORT gboolean clamp_value(GtkWidget *widget, gpointer data)
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


G_MODULE_EXPORT void refocus_cell(GtkWidget *widget, Direction dir)
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
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");


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

