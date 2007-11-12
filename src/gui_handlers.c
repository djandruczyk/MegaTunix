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
#include <config.h>
#include <conversions.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fileio.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <glib.h>
#include <init.h>
#include <keyparser.h>
#include <listmgmt.h>
#include <logviewer_core.h>
#include <logviewer_events.h>
#include <logviewer_gui.h>
//#include <multitherm.h>
#include <offline.h>
#include <mode_select.h>
#include <notifications.h>
#include <post_process.h>
#include <req_fuel.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <structures.h>
#include <tabloader.h>
#include <t-logger.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <user_outputs.h>
#include <vetable_gui.h>
#include <vex_support.h>
#include <widgetmgmt.h>



static gint upd_count = 0;
static gboolean grab_allowed = FALSE;
extern gboolean interrogated;
extern gboolean playback_mode;
extern gchar *delimiter;
extern gint ready;
extern GtkTooltips *tip;
extern GList ***ve_widgets;
extern Serial_Params *serial_params;
extern gchar * default_serial_port;
extern gint dbg_lvl;

gboolean tips_in_use;
gint temp_units;
gint active_page = -1;
gint active_table = -1;
GdkColor red = { 0, 65535, 0, 0};
GdkColor green = { 0, 0, 65535, 0};
GdkColor blue = { 0, 0, 0, 65535};
GdkColor black = { 0, 0, 0, 0};

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
EXPORT void leave(GtkWidget *widget, gpointer data)
{
	extern GHashTable *dynamic_widgets;
	extern gint dispatcher_id;
	extern gint statuscounts_id;
	extern GStaticMutex serio_mutex;
	extern GStaticMutex rtv_mutex;
	extern gboolean connected;
	extern gboolean interrogated;
	extern GAsyncQueue *dispatch_queue;
	extern GAsyncQueue *io_queue;
	extern GAsyncQueue *serial_repair_queue;
	gboolean tmp = TRUE;
	gboolean be_quiet = (gboolean) data;
	GIOChannel * iochannel = NULL;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
	gint count = 0;


	if (!be_quiet)
		prompt_to_save();

	if (leaving)
		return;
	leaving = TRUE;
	/* Message to trigger serial repair queue to exit immediately */
	g_async_queue_push(serial_repair_queue,&tmp);

	if (dbg_lvl & CRITICAL)
	{
		dbg_func(g_strdup_printf(__FILE__": LEAVE() configuration saved\n"));

		dbg_func(g_strdup_printf(__FILE__": LEAVE() before mutex\n"));
	}

	g_static_mutex_lock(&mutex);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after mutex\n"));

	save_config();

	if (statuscounts_id)
		g_source_remove(statuscounts_id);
	statuscounts_id = 0;

	/* Stop timeout functions */

	stop_tickler(RTV_TICKLER);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after stop_realtime\n"));
	stop_tickler(TOOTHMON_TICKLER);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after stop_toothmon\n"));
	stop_tickler(TRIGMON_TICKLER);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after stop_trigmon\n"));
	stop_tickler(LV_PLAYBACK_TICKLER);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after stop_lv_playback\n"));

	stop_datalogging();
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after stop_datalogging\n"));
	if (dynamic_widgets)
	{
		if (g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button"))
			iochannel = (GIOChannel *) g_object_get_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_select_log_button")),"data");
	}

	if (iochannel)	
		g_io_channel_shutdown(iochannel,TRUE,NULL);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after iochannel\n"));


	/* Commits any pending data to ECU flash */
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() before burn\n"));
	if ((connected) && (interrogated))
		io_cmd(IO_BURN_MS_FLASH,NULL);
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() after burn\n"));

	if (dispatcher_id)
		g_source_remove(dispatcher_id);
	dispatcher_id = 0;

	g_static_mutex_lock(&rtv_mutex);  // <-- this  makes us wait 
	g_static_mutex_unlock(&rtv_mutex); // now unlock


	/* This makes us wait until the io queue finishes */
	while ((g_async_queue_length(io_queue) > 0) && (count < 30))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": LEAVE() draining I/O Queue,  current length %i\n",g_async_queue_length(io_queue)));
		while (gtk_events_pending())
			gtk_main_iteration();
		count++;
	}
	count = 0;
	while ((g_async_queue_length(dispatch_queue) > 0) && (count < 10))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": LEAVE() draining Dispatch Queue,  current length %i\n",g_async_queue_length(dispatch_queue)));
		g_async_queue_try_pop(dispatch_queue);
		while (gtk_events_pending())
			gtk_main_iteration();
		count++;
	}


	/* Grab and release all mutexes to get them to relinquish
	 */
	g_static_mutex_lock(&serio_mutex);
	g_static_mutex_unlock(&serio_mutex);
	/* Free all buffers */
	mem_dealloc();
	if (dbg_lvl & CRITICAL)
		dbg_func(g_strdup_printf(__FILE__": LEAVE() mem deallocated, closing log and exiting\n"));
	close_debug();
	g_static_mutex_unlock(&mutex);
	gtk_main_quit();
	return;
}


/*!
 \brief comm_port_change() is called every time the comm port text entry
 changes. it'll try to open the port and if it does it'll setup the serial 
 parameters
 \param editable (GtkWditable *) pointer to editable widget to extract text from
 \returns TRUE
 */
gboolean comm_port_change(GtkEditable *editable)
{
	gchar *port;
	gboolean result;

	port = gtk_editable_get_chars(editable,0,-1);
	gtk_widget_modify_text(GTK_WIDGET(editable),GTK_STATE_NORMAL,&black);
	result = g_file_test(port,G_FILE_TEST_EXISTS);
	if (result)
	{
		/* This should append this new string to the vector of serial
		 * ports to search,  but it doesn't do anything yet...
		 */
	}
	else
	{
		update_logbar("comms_view","warning",g_strdup_printf("\"%s\" File not found\n",port),TRUE,FALSE);
	}

	g_free(port);
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
	extern GHashTable *dynamic_widgets;

	if (GTK_IS_OBJECT(widget))
	{
		obj_data = (void *)g_object_get_data(G_OBJECT(widget),"data");
		handler = (ToggleButton)g_object_get_data(G_OBJECT(widget),"handler");
	}
	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);
	//printf("toggle_handler for %s\n",(gchar *)glade_get_widget_name(widget));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		switch ((ToggleButton)handler)
		{

			case OFFLINE_FIRMWARE_CHOICE:
				if(offline_firmware_choice)
					g_free(offline_firmware_choice);
				offline_firmware_choice = g_strdup(g_object_get_data(G_OBJECT(widget),"filename"));	
				break;
			case TRACKING_FOCUS:
				tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"table_num");
				tracking_focus[(gint)g_ascii_strtod(tmpbuf,NULL)] = TRUE;
				break;
			case TOOLTIPS_STATE:
				gtk_tooltips_enable(tip);
				tips_in_use = TRUE;
				break;
			case FAHRENHEIT:
				temp_units = FAHRENHEIT;
				reset_temps(GINT_TO_POINTER(temp_units));
				forced_update = TRUE;
				break;
			case CELSIUS:
				temp_units = CELSIUS;
				reset_temps(GINT_TO_POINTER(temp_units));
				forced_update = TRUE;
				break;
			case USE_ALT_IAT:
				//use_alt_iat = TRUE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"enter_iat_sensor_button")),TRUE);
				break;
			case USE_ALT_CLT:
				//use_alt_clt = TRUE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"enter_clt_sensor_button")),TRUE);
				break;
			case COMMA:
				preferred_delimiter = COMMA;
				update_logbar("dlog_view", NULL,g_strdup("Setting Log delimiter to a \"Comma\"\n"),TRUE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup(",");
				break;
			case TAB:
				preferred_delimiter = TAB;
				update_logbar("dlog_view", NULL,g_strdup("Setting Log delimiter to a \"Tab\"\n"),TRUE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup("\t");
				break;
			case REALTIME_VIEW:
				set_realtime_mode();
				break;
			case PLAYBACK_VIEW:
				set_playback_mode();
				break;
			case HEX_VIEW:
			case DECIMAL_VIEW:
			case BINARY_VIEW:
				update_raw_memory_view((ToggleButton)handler,(gint)obj_data);
				break;	
			case START_TOOTHMON_LOGGER:
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"triggerlogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)g_object_get_data(G_OBJECT(widget),"page"));
				start_tickler(TOOTHMON_TICKLER);
				break;
			case START_TRIGMON_LOGGER:
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"toothlogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)g_object_get_data(G_OBJECT(widget),"page"));
				start_tickler(TRIGMON_TICKLER);
				break;
			case STOP_TOOTHMON_LOGGER:
				stop_tickler(TOOTHMON_TICKLER);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"triggerlogger_buttons_table")),TRUE);
				break;
			case STOP_TRIGMON_LOGGER:
				stop_tickler(TRIGMON_TICKLER);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"toothlogger_buttons_table")),TRUE);
				break;
			default:
				break;
		}
	}
	else
	{	/* not pressed */
		switch ((ToggleButton)handler)
		{
			case TRACKING_FOCUS:
				tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"table_num");
				tracking_focus[(gint)g_ascii_strtod(tmpbuf,NULL)] = FALSE;
				break;
			case TOOLTIPS_STATE:
				gtk_tooltips_disable(tip);
				tips_in_use = FALSE;
				break;
			case USE_ALT_IAT:
				//use_alt_iat = FALSE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"enter_iat_sensor_button")),FALSE);
				break;
			case USE_ALT_CLT:
				//use_alt_clt = FALSE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"enter_clt_sensor_button")),FALSE);
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
	gint page = -1;
	gint tmp = 0;
	gint tmp32 = 0;
	gint offset = -1;
	gboolean ign_parm = FALSE;
	gint dl_type = -1;
	gint handler = 0;
	gint table_num = -1;
	gchar * swap_list = NULL;
	gchar * set_labels = NULL;
	gchar * table_2_update = NULL;
	gchar * group_2_update = NULL;
	gchar * tmpbuf = NULL;
	extern gint dbg_lvl;
	extern gint **ms_data;
	extern GHashTable **interdep_vars;
	extern Firmware_Details *firmware;
	extern GHashTable *sources_hash;

	if ((paused_handlers) || (!ready))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);

	ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	dl_type = (gint)g_object_get_data(G_OBJECT(widget),"dl_type");
	bitshift = (gint)g_object_get_data(G_OBJECT(widget),"bitshift");
	bitval = (gint)g_object_get_data(G_OBJECT(widget),"bitval");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");
	handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	swap_list = (gchar *)g_object_get_data(G_OBJECT(widget),"swap_labels");
	set_labels = (gchar *)g_object_get_data(G_OBJECT(widget),"set_widgets_label");
	group_2_update = (gchar *)g_object_get_data(G_OBJECT(widget),"group_2_update");
	table_2_update = (gchar *)g_object_get_data(G_OBJECT(widget),"update_table");

	// If it's a check button then it's state is dependant on the button's state
	if (!GTK_IS_RADIO_BUTTON(widget))
		bitval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	switch ((SpinButton)handler)
	{
			break;
		case MAP_SENSOR_TYPE:
			//printf("MAP SENSOR CHANGE\n");
			if ((g_object_get_data(G_OBJECT(widget),"source_key")) && (g_object_get_data(G_OBJECT(widget),"source_value")))
			{
		//		printf("key %s value %s\n",(gchar *)g_object_get_data(G_OBJECT(widget),"source_key"),(gchar *)g_object_get_data(G_OBJECT(widget),"source_value"));
				g_hash_table_replace(sources_hash,g_strdup(g_object_get_data(G_OBJECT(widget),"source_key")),g_strdup(g_object_get_data(G_OBJECT(widget),"source_value")));
			}
			/* FAll Through */
		case GENERIC:
			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	//clears bits 
			tmp = tmp | (bitval << bitshift);
			dload_val = tmp;
			if (dload_val == ms_data[page][offset])
				return FALSE;
			break;
		case DEBUG_LEVEL:
			// Debugging selection buttons 
			tmp32 = dbg_lvl;
			tmp32 = tmp32 & ~bitmask;
			tmp32 = tmp32 | (bitval << bitshift);
			dbg_lvl = tmp32;
			break;

		case ALT_SIMUL:
			/* Alternate or simultaneous */
			if (firmware->capabilities & MSNS_E)
			{
				tmpbuf = (gchar *)g_object_get_data(
						G_OBJECT(widget),"table_num");
				table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
				tmp = ms_data[page][offset];
				tmp = tmp & ~bitmask;// clears bits 
				tmp = tmp | (bitval << bitshift);
				dload_val = tmp;
				if (dload_val == ms_data[page][offset])
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				g_hash_table_insert(interdep_vars[page],
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));
				check_req_fuel_limits(table_num);
			}
			else
			{
				tmpbuf = (gchar *)g_object_get_data(
						G_OBJECT(widget),"table_num");
				table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
				dload_val = bitval;
				if (dload_val == ms_data[page][offset])
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				g_hash_table_insert(interdep_vars[page],
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));
				check_req_fuel_limits(table_num);
			}
			break;
		default:
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": bitmask_button_handler()\n\tbitmask button at page: %i, offset %i, NOT handled\n\tERROR!!, contact author\n",page,offset));
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
		write_ve_const(widget, page, offset, dload_val, ign_parm, TRUE);
	}
	return TRUE;
}


/*!
 \brief entry_changed_handler() gets called anytiem a text entries is changed
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
	g_object_set_data(G_OBJECT(widget),"not_sent",GINT_TO_POINTER(TRUE));

	return TRUE;
}


/*!
 \brief focus_out_handler() auto-sends data IF IT IS CHANGED to the ecu thus
 hopefully ending hte user confusion about why data isn't sent.
 \param widget (GtkWidget *) the widget being modified
 \param event (GdkEvent *) not used
 \param data (gpointer) not used
 \returns FALSE
 */
EXPORT gboolean focus_out_handler(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	if (g_object_get_data(G_OBJECT(widget),"not_sent"))
	{
		g_object_set_data(G_OBJECT(widget),"not_sent",NULL);
		std_entry_handler(widget, data);
	}
	return FALSE;
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
	gint tmpi = -1;
	gint tmp = -1;
	gint page = -1;
	gint base = -1;
	gint old = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint dl_type = -1;
	gint precision = -1;
	gint spconfig_offset = -1;
	gint oddfire_bit_offset = -1;
	gfloat real_value = 0.0;
	gboolean is_float = FALSE;
	gboolean ign_parm = FALSE;
	gboolean use_color = FALSE;
	GdkColor color;
	extern gint **ms_data;

	if ((paused_handlers) || (!ready))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	handler = (SpinButton)g_object_get_data(G_OBJECT(widget),"handler");
	dl_type = (gint) g_object_get_data(G_OBJECT(widget),"dl_type");
	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	if (!g_object_get_data(G_OBJECT(widget),"base"))
		base = 10;
	else
		base = (gint)g_object_get_data(G_OBJECT(widget),"base");
	ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
	precision = (gint)g_object_get_data(G_OBJECT(widget),"precision");
	is_float = (gboolean)g_object_get_data(G_OBJECT(widget),"is_float");
	use_color = (gboolean)g_object_get_data(G_OBJECT(widget),"use_color");

	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpi = (gint)strtol(text,NULL,base);
	tmpf = (gfloat)g_strtod(text,NULL);
	//	printf("base \"%i\", text \"%s\" int val \"%i\", float val \"%f\" \n",base,text,tmpi,tmpf);
	g_free(text);
	/* This isn't quite correct, as the base can either be base10 
	 * or base16, the problem is the limits are in base10
	 */

	if ((tmpf != (gfloat)tmpi) && (!is_float))
	{
		/* Pause signals while we change the value */
		//		printf("resetting\n");
		g_signal_handlers_block_by_func (widget,(gpointer) std_entry_handler, data);
		tmpbuf = g_strdup_printf("%i",tmpi);
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		g_free(tmpbuf);
		g_signal_handlers_unblock_by_func (widget,(gpointer) std_entry_handler, data);
	}
	switch ((SpinButton)handler)
	{
		case GENERIC:
			if (base == 10)
			{
				if (is_float)
					dload_val = convert_before_download(widget,tmpf);
				else
					dload_val = convert_before_download(widget,tmpi);
			}
			else if (base == 16)
				dload_val = convert_before_download(widget,tmpi);
			else
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": std_entry_handler()\n\tBase of textentry \"%i\" is invalid!!!\n",base));
				return TRUE;
			}
			/* What we are doing is doing the forward/reverse 
			 * conversion which will give us an exact value 
			 * if the user inputs something in between,  thus 
			 * we can reset the display to a sane value...
			 */
			old = ms_data[page][offset];
			ms_data[page][offset] = dload_val;

			real_value = convert_after_upload(widget);
			ms_data[page][offset] = old;

			//			printf("real_value %f\n",real_value);
			g_signal_handlers_block_by_func (widget,(gpointer) std_entry_handler, data);
			if (is_float)
			{
				tmpbuf = g_strdup_printf("%1$.*2$f",real_value,precision);
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				g_free(tmpbuf);
			}
			else
			{
				if (base == 10)
				{
					tmpbuf = g_strdup_printf("%i",(gint)real_value);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);
				}
				else
				{
					tmpbuf = g_strdup_printf("%.2X",(gint)real_value);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);
				}
			}
			g_signal_handlers_unblock_by_func (widget,(gpointer) std_entry_handler, data);
			break;

		case TRIGGER_ANGLE:
			spconfig_offset = (gint)g_object_get_data(G_OBJECT(widget),"spconfig_offset");
			if (spconfig_offset == 0)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": std_entry_handler()\n\tERROR Trigger Angle entry call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (tmpf > 112.15)	/* Extra long trigger needed */	
			{
				tmp = ms_data[page][spconfig_offset];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);	/* Set xlong_trig */
				//ms_data[page][spconfig_offset] = tmp;
				write_ve_const(widget, page, spconfig_offset, tmp, ign_parm, TRUE);
				tmpf -= 45.0;
				dload_val = convert_before_download(widget,tmpf);
			}
			else if (tmpf > 89.65) /* Long trigger needed */
			{
				tmp = ms_data[page][spconfig_offset];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);	/* Set long_trig */
				//ms_data[page][spconfig_offset] = tmp;
				write_ve_const(widget, page, spconfig_offset, tmp, ign_parm, TRUE);
				tmpf -= 22.5;
				dload_val = convert_before_download(widget,tmpf);
			}
			else	// tmpf <= 89.65 degrees, no long trigger
			{
				tmp = ms_data[page][spconfig_offset];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				//ms_data[page][spconfig_offset] = tmp;
				write_ve_const(widget, page, spconfig_offset, tmp, ign_parm, TRUE);
				dload_val = convert_before_download(widget,tmpf);
			}

			break;

		case ODDFIRE_ANGLE:
			oddfire_bit_offset = (gint)g_object_get_data(G_OBJECT(widget),"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": spin_button_handler()\n\tERROR Offset Angle spinbutton call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (tmpf > 90)	/*  */	
			{
				tmp = ms_data[page][oddfire_bit_offset];
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);	/* Set +90 */
				write_ve_const(widget, page, oddfire_bit_offset, tmp, ign_parm, TRUE);
				tmpf -= 90.0;
				dload_val = convert_before_download(widget,tmpf);
			}
			else if (tmpf > 45) /* */
			{
				tmp = ms_data[page][oddfire_bit_offset];
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);	/* Set +45 */
				write_ve_const(widget, page, oddfire_bit_offset, tmp, ign_parm, TRUE);
				tmpf -= 45.0;
				dload_val = convert_before_download(widget,tmpf);
			}
			else	// tmpf <= 45 degrees, 
			{
				tmp = ms_data[page][oddfire_bit_offset];
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				write_ve_const(widget, page, oddfire_bit_offset, tmp, ign_parm, TRUE);
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
		 *                  * and wasting time.
		 *                                   */
		if (dload_val != ms_data[page][offset])
			write_ve_const(widget, page, offset, dload_val, ign_parm, TRUE);
	}

	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);

	if (use_color)
	{
		color = get_colors_from_hue(((gfloat)dload_val/256.0)*360.0,0.33, 1.0);
		gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	
	}

	g_object_set_data(G_OBJECT(widget),"not_sent",GINT_TO_POINTER(FALSE));
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
	gchar * tmpbuf = NULL;
	gboolean restart = FALSE;
	extern gint realtime_id;
	extern gboolean no_update;
	extern volatile gboolean offline;
	extern gboolean forced_update;
	extern GHashTable *dynamic_widgets;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	obj_data = (void *)g_object_get_data(G_OBJECT(widget),"data");
	handler = (StdButton)g_object_get_data(G_OBJECT(widget),"handler");

	if (handler == 0)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": std_button_handler()\n\thandler not bound to object, CRITICAL ERROR, aborting\n"));
		return FALSE;
	}

	switch ((StdButton)handler)
	{
		case EXPORT_SINGLE_TABLE:
			tmpbuf = g_object_get_data(G_OBJECT(widget),"table_num");
			if (tmpbuf)
			{
				tmpi = (gint)strtol(tmpbuf,NULL,10);
				select_table_for_export(tmpi);
			}
			break;
		case IMPORT_SINGLE_TABLE:
			tmpbuf = g_object_get_data(G_OBJECT(widget),"table_num");
			if (tmpbuf)
			{
				tmpi = (gint)strtol(tmpbuf,NULL,10);
				select_table_for_import(tmpi);
			}
			break;
		case RESCALE_TABLE:
			rescale_table(widget);
			break;
		case REQFUEL_RESCALE_TABLE:
			reqfuel_rescale_table(widget);
			break;
		case INTERROGATE_ECU:
			set_title(g_strdup("User initiated interrogation..."));
			update_logbar("interr_view","warning",g_strdup("USER Initiated ECU interrogation...\n"),FALSE,FALSE);
			widget = g_hash_table_lookup(dynamic_widgets,"interrogate_button");
			if (GTK_IS_WIDGET(widget))
				gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
			io_cmd(IO_INTERROGATE_ECU, NULL);
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
			no_update = FALSE;
			if (!interrogated)
				io_cmd(IO_INTERROGATE_ECU, NULL);
			start_tickler(RTV_TICKLER);
			forced_update = TRUE;
			break;
		case STOP_REALTIME:
			if (offline)
				break;
			stop_tickler(RTV_TICKLER);
			no_update = TRUE;
			break;
		case REBOOT_GETERR:
			if (offline)
				break;
			if (realtime_id > 0)
			{
				stop_tickler(RTV_TICKLER);
				restart = TRUE;
			}
			io_cmd(IO_REBOOT_GET_ERROR,NULL);
			if (restart)
				start_tickler(RTV_TICKLER);
			break;
		case READ_VE_CONST:
			set_title(g_strdup("Reading VE/Constants..."));
			io_cmd(IO_READ_VE_CONST, NULL);
			break;
		case READ_RAW_MEMORY:
			if (offline)
				break;
			io_cmd(IO_READ_RAW_MEMORY,(gpointer)obj_data);
			break;
		case BURN_MS_FLASH:
			io_cmd(IO_BURN_MS_FLASH,NULL);
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
		case SELECT_FIRMWARE_LOAD:
			gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"multitherm_table")),TRUE);
			gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"download_fw_button")),TRUE);
			break;
		case DOWNLOAD_FIRMWARE:
			printf("not implemented yet\n");
			gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"multitherm_table")),FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"enter_iat_sensor_button")),FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"enter_clt_sensor_button")),FALSE);
			break;
		case ENTER_SENSOR_INFO:
			printf("not implemented yet\n");
			//multitherm_request_info((gpointer)widget);
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
			gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button")),FALSE);
			present_viewer_choices();
			break;
		case REQ_FUEL_POPUP:
			reqd_fuel_popup(widget);
			req_fuel_change(widget);
			break;
		case OFFLINE_MODE:
			set_title(g_strdup("Offline Mode..."));
			set_offline_mode();
			break;
		default:
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup(__FILE__": std_button_handler()\n\t Standard button not handled properly, BUG detected\n"));
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
	gint page = -1;
	gint bitmask = -1;
	gint bitshift = -1;
	gint spconfig_offset = 0;
	gint oddfire_bit_offset = 0;
	gboolean ign_parm = FALSE;
	gboolean temp_dep = FALSE;
	gint tmpi = 0;
	gint tmp = 0;
	gint handler = -1;
	gint divider_offset = 0;
	gint table_num = -1;
	gfloat value = 0.0;
	gchar *tmpbuf = NULL;
	GtkWidget * tmpwidget = NULL;
	extern gint realtime_id;
	extern gint **ms_data;
	extern gint lv_zoom;
	Reqd_Fuel *reqd_fuel = NULL;
	extern GHashTable *dynamic_widgets;
	extern Firmware_Details *firmware;
	extern gboolean forced_update;
	extern GHashTable **interdep_vars;

	if ((paused_handlers) || (!ready))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	if (!GTK_IS_WIDGET(widget))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": spin_button_handler()\n\twidget pointer is NOT valid\n"));
		return FALSE;
	}

	reqd_fuel = (Reqd_Fuel *)g_object_get_data(
			G_OBJECT(widget),"reqd_fuel");
	handler = (SpinButton)g_object_get_data(G_OBJECT(widget),"handler");
	ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
	dl_type = (gint) g_object_get_data(G_OBJECT(widget),"dl_type");
	page = (gint) g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	bitmask = (gint) g_object_get_data(G_OBJECT(widget),"bitmask");
	bitshift = (gint) g_object_get_data(G_OBJECT(widget),"bitshift");
	temp_dep = (gboolean)g_object_get_data(G_OBJECT(widget),"temp_dep");
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);

	tmpi = (int)(value+.001);


	switch ((SpinButton)handler)
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
			tmpbuf = (gchar *)g_object_get_data(
					G_OBJECT(widget),"table_num");
			table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
			firmware->rf_params[table_num]->last_req_fuel_total = firmware->rf_params[table_num]->req_fuel_total;
			firmware->rf_params[table_num]->req_fuel_total = value;
			check_req_fuel_limits(table_num);
			break;
		case LOCKED_REQ_FUEL:
			tmpbuf = (gchar *)g_object_get_data(
					G_OBJECT(widget),"table_num");
			table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),firmware->rf_params[table_num]->req_fuel_total);
			break;

		case LOGVIEW_ZOOM:
			lv_zoom = tmpi;
			tmpwidget = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");	
			if (tmpwidget)
				lv_configure_event(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea"),NULL,NULL);
			//	g_signal_emit_by_name(tmpwidget,"configure_event",NULL);
			break;

		case NUM_SQUIRTS_1:
		case NUM_SQUIRTS_2:
			/* This actuall effects another variable */
			tmpbuf = (gchar *)g_object_get_data(
					G_OBJECT(widget),"table_num");
			table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
			divider_offset = firmware->table_params[table_num]->divider_offset;
			firmware->rf_params[table_num]->last_num_squirts = firmware->rf_params[table_num]->num_squirts;
			firmware->rf_params[table_num]->last_divider = ms_data[page][divider_offset];

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
				g_hash_table_insert(interdep_vars[page],
						GINT_TO_POINTER(divider_offset),
						GINT_TO_POINTER(dload_val));
				err_flag = FALSE;
				set_reqfuel_color(BLACK,table_num);
				check_req_fuel_limits(table_num);
			}
			break;
		case NUM_CYLINDERS_1:
		case NUM_CYLINDERS_2:
			/* Updates a shared bitfield */
			tmpbuf = (gchar *)g_object_get_data(
					G_OBJECT(widget),"table_num");
			table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
			//printf("table num %i\n",table_num);
			divider_offset = firmware->table_params[table_num]->divider_offset;
			firmware->rf_params[table_num]->last_divider = ms_data[page][divider_offset];
			firmware->rf_params[table_num]->last_num_cyls = firmware->rf_params[table_num]->num_cyls;

			firmware->rf_params[table_num]->num_cyls = tmpi;
			if (firmware->rf_params[table_num]->num_cyls % firmware->rf_params[table_num]->num_squirts)
			{
				err_flag = TRUE;
				set_reqfuel_color(RED,table_num);	
			}
			else
			{
				tmp = ms_data[page][offset];
				tmp = tmp & ~bitmask;	/*clears top 4 bits */
				tmp = tmp | ((tmpi-1) << bitshift);
				dload_val = tmp;
				//printf("new num_cyls is %i, new data is %i\n",tmpi,tmp);
				g_hash_table_insert(interdep_vars[page],
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));

				dload_val = 
					(gint)(((float)firmware->rf_params[table_num]->num_cyls/(float)firmware->rf_params[table_num]->num_squirts)+0.001);
						
				firmware->rf_params[table_num]->divider = dload_val;
				g_hash_table_insert(interdep_vars[page],
						GINT_TO_POINTER(divider_offset),
						GINT_TO_POINTER(dload_val));

				err_flag = FALSE;
				set_reqfuel_color(BLACK,table_num);	
				check_req_fuel_limits(table_num);
			}
			break;
		case NUM_INJECTORS_1:
		case NUM_INJECTORS_2:
			/* Updates a shared bitfield */
			tmpbuf = (gchar *)g_object_get_data(
					G_OBJECT(widget),"table_num");
			table_num = (gint)g_ascii_strtod(tmpbuf,NULL);
			firmware->rf_params[table_num]->last_num_inj = firmware->rf_params[table_num]->num_inj;
			firmware->rf_params[table_num]->num_inj = tmpi;

			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << bitshift);
			//ms_data[page][offset] = tmp;
			dload_val = tmp;
			g_hash_table_insert(interdep_vars[page],
					GINT_TO_POINTER(offset),
					GINT_TO_POINTER(dload_val));

			check_req_fuel_limits(table_num);
			break;
		case TRIGGER_ANGLE:
			spconfig_offset = (gint)g_object_get_data(G_OBJECT(widget),"spconfig_offset");
			if (spconfig_offset == 0)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": spin_button_handler()\n\tERROR Trigger Angle spinbutton call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (value > 112.15)	/* Extra long trigger needed */	
			{
				tmp = ms_data[page][spconfig_offset];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);	/* Set xlong_trig */
				//ms_data[page][spconfig_offset] = tmp;
				write_ve_const(widget, page, spconfig_offset, tmp, ign_parm, TRUE);
				value -= 45.0;
				dload_val = convert_before_download(widget,value);
			}
			else if (value > 89.65) /* Long trigger needed */
			{
				tmp = ms_data[page][spconfig_offset];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);	/* Set long_trig */
				//ms_data[page][spconfig_offset] = tmp;
				write_ve_const(widget, page, spconfig_offset, tmp, ign_parm, TRUE);
				value -= 22.5;
				dload_val = convert_before_download(widget,value);
			}
			else	// value <= 89.65 degrees, no long trigger
			{
				tmp = ms_data[page][spconfig_offset];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				//ms_data[page][spconfig_offset] = tmp;
				write_ve_const(widget, page, spconfig_offset, tmp, ign_parm, TRUE);
				dload_val = convert_before_download(widget,value);
			}

			break;

		case ODDFIRE_ANGLE:
			oddfire_bit_offset = (gint)g_object_get_data(G_OBJECT(widget),"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": spin_button_handler()\n\tERROR Offset Angle spinbutton call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;  
				break;

			}
			if (value > 90)	/*  */	
			{
				tmp = ms_data[page][oddfire_bit_offset];
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);	/* Set +90 */
				write_ve_const(widget, page, oddfire_bit_offset, tmp, ign_parm, TRUE);
				value -= 90.0;
				dload_val = convert_before_download(widget,value);
			}
			else if (value > 45) /* */
			{
				tmp = ms_data[page][oddfire_bit_offset];
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);	/* Set +45 */
				write_ve_const(widget, page, oddfire_bit_offset, tmp, ign_parm, TRUE);
				value -= 45.0;
				dload_val = convert_before_download(widget,value);
			}
			else	// value <= 45 degrees, 
			{
				tmp = ms_data[page][oddfire_bit_offset];
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				write_ve_const(widget, page, oddfire_bit_offset, tmp, ign_parm, TRUE);
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
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": spin_button_handler()\n\tERROR spinbutton not handled, handler = %i\n",handler));
			dl_type = 0;  
			break;
	}
	if (dl_type == IMMEDIATE) 
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != ms_data[page][offset])
			write_ve_const(widget, page, offset, dload_val, ign_parm, TRUE);
	}
	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	return TRUE;

}


/*!
 \brief update_ve_const() is called after a read of the VE/Const block of 
 data from the ECU.  It takes care of updating evey control that relates to
 an ECU variable on screen
 */
void update_ve_const()
{
	gint page = 0;
	gint offset = 0;
	gfloat tmpf = 0.0;
	gint reqfuel = 0;
	gint i = 0;
	union config11 cfg11;
	union config12 cfg12;
	extern gint **ms_data;
	extern volatile gboolean leaving;
	extern Firmware_Details *firmware;

	if (leaving)
		return;

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

	for (i=0;i<firmware->total_tables;i++)
	{
		//printf("\n");
		page = firmware->table_params[i]->z_page;
		if (firmware->table_params[i]->reqfuel_offset < 0)
			continue;

		cfg11.value = ms_data[page][firmware->table_params[i]->cfg11_offset];	
		cfg12.value = ms_data[page][firmware->table_params[i]->cfg12_offset];	
		firmware->rf_params[i]->num_cyls = cfg11.bit.cylinders+1;
		firmware->rf_params[i]->last_num_cyls = cfg11.bit.cylinders+1;
		firmware->rf_params[i]->num_inj = cfg12.bit.injectors+1;
		firmware->rf_params[i]->last_num_inj = cfg12.bit.injectors+1;

		firmware->rf_params[i]->divider = ms_data[page][firmware->table_params[i]->divider_offset];
		firmware->rf_params[i]->last_divider = firmware->rf_params[i]->divider;
		firmware->rf_params[i]->alternate = ms_data[page][firmware->table_params[i]->alternate_offset];
		firmware->rf_params[i]->last_alternate = firmware->rf_params[i]->alternate;
		reqfuel = ms_data[page][firmware->table_params[i]->reqfuel_offset];

		//printf("num_inj %i, divider %i\n",firmware->rf_params[i]->num_inj,firmware->rf_params[i]->divider);
		//printf("num_cyls %i, alternate %i\n",firmware->rf_params[i]->num_cyls,firmware->rf_params[i]->alternate);
		//printf("req_fuel_per_lsquirt is %i\n",reqfuel);


		/* Calcs vary based on firmware. 
		 * DT uses nim_inj/divider
		 * MSnS-E use the SAME in DT mode only
		 * MSnS-E uses B&G form in single table mode
		 */
		if (firmware->capabilities & DUALTABLE)
		{
		//	printf("DT\n");
			tmpf = (float)(firmware->rf_params[i]->num_inj)/(float)(firmware->rf_params[i]->divider);
		}
		else if ((firmware->capabilities & MSNS_E) && (((ms_data[firmware->table_params[i]->dtmode_page][firmware->table_params[i]->dtmode_offset] & 0x10) >> 4) == 1))
		{
		//	printf("MSnS-E DT\n");
			tmpf = (float)(firmware->rf_params[i]->num_inj)/(float)(firmware->rf_params[i]->divider);
		}
		else
		{
		//	printf("B&G\n");
			tmpf = (float)(firmware->rf_params[i]->num_inj)/((float)(firmware->rf_params[i]->divider)*((float)(firmware->rf_params[i]->alternate)+1.0));
		}

		/* ReqFuel Total */
		tmpf *= (float)reqfuel;
		tmpf /= 10.0;
		firmware->rf_params[i]->req_fuel_total = tmpf;
		firmware->rf_params[i]->last_req_fuel_total = tmpf;
		//printf("req_fuel_total is %f\n",tmpf);

		/* Injections per cycle */
		firmware->rf_params[i]->num_squirts = (float)(firmware->rf_params[i]->num_cyls)/(float)(firmware->rf_params[i]->divider);
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
		for (offset=0;offset<firmware->page_params[page]->length;offset++)
		{
			if ((leaving) || (!firmware))
				return;
			if (ve_widgets[page][offset] != NULL)
				g_list_foreach(ve_widgets[page][offset],
						update_widget,NULL);
		}
	}
	

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
	extern gint **ms_data;
	extern volatile gboolean leaving;
	extern gboolean forced_update;
	extern GList ***ve_widgets;
	extern Firmware_Details *firmware;
	gint base = 0;
	gint length = 0;

	if (leaving)
		return FALSE;
	if (page > firmware->total_pages)
	       return FALSE;
	table_num = (gint)g_ascii_strtod((gchar *)data,NULL);
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
	gboolean is_float = FALSE;
	gboolean use_color = FALSE;
	gint i = 0;
	gint tmpi = -1;
	gint page = -1;
	gint offset = -1;
	gdouble value = 0.0;
	gint bitval = -1;
	gint bitshift = -1;
	gint bitmask = -1;
	gint base = -1;
	gint precision = -1;
	gint num_groups = 0;
	gint spconfig_offset = 0;
	gint oddfire_bit_offset = 0;
	gchar ** groups = NULL;
	gchar * toggle_groups = NULL;
	gchar * group_states = NULL;
	gboolean cur_state = FALSE;
	gboolean tmp_state = FALSE;
	gboolean new_state = FALSE;
	gboolean state = FALSE;
	gint algo = -0;
	gchar * swap_list = NULL;
	gchar * set_labels = NULL;
	gchar * tmpbuf = NULL;
	gchar **vector = NULL;
	gchar * widget_text = NULL;
	gchar * group_2_update = NULL;
	gdouble spin_value = 0.0; 
	gboolean update_color = TRUE;
	GdkColor color;
	extern gint ** ms_data;
	extern GHashTable *widget_group_states;
	extern gint *algorithm;
	extern volatile gboolean leaving;
	extern GHashTable *sources_hash;

	if (leaving)
		return;

	upd_count++;
	if ((upd_count%64) == 0)
	{
		gdk_threads_enter();
		while (gtk_events_pending())
		{
			if (leaving)
			{
				gdk_threads_leave();
				return;
			}
			gtk_main_iteration();
		}
		gdk_threads_leave();

	}
	if (!GTK_IS_OBJECT(widget))
		return;

	if ((GTK_IS_OBJECT(user_data)) && (widget == user_data))
		return;


	dl_type = (gint)g_object_get_data(G_OBJECT(widget),
			"dl_type");
	page = (gint)g_object_get_data(G_OBJECT(widget),
			"page");
	offset = (gint)g_object_get_data(G_OBJECT(widget),
			"offset");
	bitval = (gint)g_object_get_data(G_OBJECT(widget),
			"bitval");
	bitshift = (gint)g_object_get_data(G_OBJECT(widget),
			"bitshift");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),
			"bitmask");
	if (!g_object_get_data(G_OBJECT(widget),"base"))
		base = 10;
	else
		base = (gint)g_object_get_data(G_OBJECT(widget),
				"base");

	precision = (gint)g_object_get_data(G_OBJECT(widget),
			"precision");
	temp_dep = (gboolean)g_object_get_data(G_OBJECT(widget),
			"temp_dep");
	is_float = (gboolean)g_object_get_data(G_OBJECT(widget),
			"is_float");
	toggle_groups = (gchar *)g_object_get_data(G_OBJECT(widget),
			"toggle_groups");
	group_states = (gchar *)g_object_get_data(G_OBJECT(widget),
			"group_states");
	use_color = (gboolean)g_object_get_data(G_OBJECT(widget),
			"use_color");
	swap_list = (gchar *)g_object_get_data(G_OBJECT(widget),
			"swap_labels");
	set_labels = (gchar *)g_object_get_data(G_OBJECT(widget),
			"set_widgets_label");
	group_2_update = (gchar *)g_object_get_data(G_OBJECT(widget),
			"group_2_update");

	value = convert_after_upload(widget);  

	if (temp_dep)
	{
		if (temp_units == CELSIUS)
			value = (value-32)*(5.0/9.0);
	}

	// update widget whether spin,radio or checkbutton  (check encompases radio)
	if ((GTK_IS_ENTRY(widget)) && (!GTK_IS_SPIN_BUTTON(widget)))
	{
		if ((int)g_object_get_data(G_OBJECT(widget),"handler") == ODDFIRE_ANGLE)
		{
			oddfire_bit_offset = (gint)g_object_get_data(G_OBJECT(widget),"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
				return;
			switch (ms_data[page][oddfire_bit_offset])
			{
				case 4:
					if (is_float)
						tmpbuf = g_strdup_printf("%1$.*2$f",value+90,precision);
					else
						tmpbuf = g_strdup_printf("%i",(gint)value+90);
					break;
				case 2:
					if (is_float)
						tmpbuf = g_strdup_printf("%1$.*2$f",value+45,precision);
					else
						tmpbuf = g_strdup_printf("%i",(gint)value+45);
					break;
				case 0:
					if (is_float)
						tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
					else
						tmpbuf = g_strdup_printf("%i",(gint)value);
					break;
				default:
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": update_widget()\n\t ODDFIRE_ANGLE_UPDATE invalid value for oddfire_bit_offset at ms_data[%i][%i], ERROR\n",page,oddfire_bit_offset));

			}
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
		}
		else if ((int)g_object_get_data(G_OBJECT(widget),"handler") == TRIGGER_ANGLE)
		{
			spconfig_offset = (gint)g_object_get_data(G_OBJECT(widget),"spconfig_offset");
			switch ((ms_data[page][spconfig_offset] & 0x03))
			{
				case 2:
					if (is_float)
						tmpbuf = g_strdup_printf("%1$.*2$f",value+45,precision);
					else
						tmpbuf = g_strdup_printf("%i",(gint)value+45);
					break;
				case 1:
					if (is_float)
						tmpbuf = g_strdup_printf("%1$.*2$f",value+22.5,precision);
					else
						tmpbuf = g_strdup_printf("%i",(gint)(value+22.5));
					break;
				case 0:
					if (is_float)
						tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
					else
						tmpbuf = g_strdup_printf("%i",(gint)value);
					break;
				default:
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": update_widget()\n\t TRIGGER_ANGLE_UPDATE invalid value for spconfig_offset at ms_data[%i][%i], ERROR\n",page,spconfig_offset));

			}
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
		}
		else
		{
			widget_text = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
			update_color = TRUE;
			if (base == 10)
			{
				if (is_float)
				{
					tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
					if (g_ascii_strcasecmp(widget_text,tmpbuf) != 0)
						gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					else
						update_color = FALSE;
					g_free(tmpbuf);
				}
				else
				{
					tmpbuf = g_strdup_printf("%i",(gint)value);
					if (g_ascii_strcasecmp(widget_text,tmpbuf) != 0)
						gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					else
						update_color = FALSE;
					g_free(tmpbuf);
				}
			}
			else if (base == 16)
			{
				tmpbuf = g_strdup_printf("%.2X",(gint)value);
				if (g_ascii_strcasecmp(widget_text,tmpbuf) != 0)
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				else
					update_color = FALSE;
				g_free(tmpbuf);
			}
			else
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup(__FILE__": update_widget()\n\t base for nemeric textentry is not 10 or 16, ERROR\n"));

			if ((use_color) && (update_color))
			{
				color = get_colors_from_hue(((gfloat)ms_data[page][offset]/256.0)*360.0,0.33, 1.0);
				gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	
			}
			if (update_color)
				gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		}
	}
	else if (GTK_IS_SPIN_BUTTON(widget))
	{
		if ((int)g_object_get_data(G_OBJECT(widget),"handler") == ODDFIRE_ANGLE)
		{
			oddfire_bit_offset = (gint)g_object_get_data(G_OBJECT(widget),"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
				return;
			switch (ms_data[page][oddfire_bit_offset])
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
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": update_widget()\n\t ODDFIRE_ANGLE_UPDATE invalid value for oddfire_bit_offset at ms_data[%i][%i], ERROR\n",page,oddfire_bit_offset));


			}
		}
		else if ((int)g_object_get_data(G_OBJECT(widget),"handler") == TRIGGER_ANGLE)
		{
			spconfig_offset = (gint)g_object_get_data(G_OBJECT(widget),"spconfig_offset");
			switch ((ms_data[page][spconfig_offset] & 0x03))
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
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": update_widget()\n\t TRIGGER_ANGLE_UPDATE invalid value for spconfig_offset at ms_data[%i][%i], ERROR\n",page,spconfig_offset));


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
					color = get_colors_from_hue(((gfloat)ms_data[page][offset]/256.0)*360.0,0.33, 1.0);
					gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);	
				}
			}
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
		/*
		   if ((((tmpi & bitmask) >> bitshift) == bitval) && (!cur_state))
		   new_state = TRUE;
		   else if ((((tmpi & bitmask) >> bitshift) != bitval) && (cur_state))
		   new_state = FALSE;
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
			if ((g_object_get_data(G_OBJECT(widget),"source_key")) && (g_object_get_data(G_OBJECT(widget),"source_value")))
			{
			//	printf("key %s value %s\n",(gchar *)g_object_get_data(G_OBJECT(widget),"source_key"),(gchar *)g_object_get_data(G_OBJECT(widget),"source_value"));
				g_hash_table_replace(sources_hash,g_strdup(g_object_get_data(G_OBJECT(widget),"source_key")),g_strdup(g_object_get_data(G_OBJECT(widget),"source_value")));

			}
			g_timeout_add(2000,force_view_recompute,NULL);
			g_timeout_add(2000,trigger_group_update,group_2_update);
		}

		if (new_state)
		{
			algo = (Algorithm)g_object_get_data(G_OBJECT(widget),"algorithm");
			if (algo > 0)
			{
				tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"applicable_tables");
				if (!tmpbuf)
				{
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": update_widget()\n\t Check/Radio button  %s has algorithm defines but no applicable tables, BUG!\n",(gchar *)glade_get_widget_name(widget)));
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
		{
			//	printf("toggling groups\n");
			groups = parse_keys(toggle_groups,&num_groups,",");
			//	printf("toggle groups defined for widget %p at page %i, offset %i\n",widget,page,offset);

			for (i=0;i<num_groups;i++)
			{
				//		printf("UW: This widget has %i groups, checking state of (%s)\n", num_groups, groups[i]);
				tmp_state = get_state(group_states,i);
				//		printf("If this ctrl is active we want state to be %i\n",tmp_state);
				state = tmp_state == TRUE ? new_state:!new_state;
				//		printf("Current state of button is %i\n",new_state),
				//		printf("new group state is %i\n",state);
				g_hash_table_insert(widget_group_states,g_strdup(groups[i]),(gpointer)state);
				//		printf("setting all widgets in that group to state %i\n\n",state);
				g_list_foreach(get_list(groups[i]),alter_widget_state,NULL);
			}
			//	printf ("DONE!\n\n\n");
			g_strfreev(groups);
		}

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
	gint page = 0;
	gint offset = 0;
	gint value = 0;
	gint lower = 0;
	gint upper = 255;
	gint dload_val = 0;
	gboolean ign_parm = FALSE;
	gboolean retval = FALSE;
	gboolean reverse_keys = FALSE;
	extern gint **ms_data;
	extern GList ***ve_widgets;

	if (g_object_get_data(G_OBJECT(widget),"raw_lower") != NULL)
		lower = (gint) g_object_get_data(G_OBJECT(widget),"raw_lower");
	if (g_object_get_data(G_OBJECT(widget),"raw_upper") != NULL)
		upper = (gint) g_object_get_data(G_OBJECT(widget),"raw_upper");
	page = (gint) g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	ign_parm = (gboolean) g_object_get_data(G_OBJECT(widget),"ign_parm");
	reverse_keys = (gboolean) g_object_get_data(G_OBJECT(widget),"reverse_keys");

	value = ms_data[page][offset];
	if (event->keyval == GDK_KP_Space)
	{
		printf("spacebar!\n");
	}
	if (event->keyval == GDK_Shift_L)
	{
		if (event->type == GDK_KEY_PRESS)
			grab_allowed = TRUE;
		else
			grab_allowed = FALSE;
		return FALSE;
	}

	if (event->type == GDK_KEY_RELEASE)
	{
		grab_allowed = FALSE;
		return FALSE;
	}

	switch (event->keyval)
	{
		case GDK_Page_Up:
			if (reverse_keys)
			{
				if (value >= (lower+10))
					dload_val = ms_data[page][offset] - 10;
				else
					return FALSE;
			}
			else 
			{
				if (value <= (upper-10))
					dload_val = ms_data[page][offset] + 10;
				else
					return FALSE;
			}
			retval = TRUE;
			break;
		case GDK_Page_Down:
			if (reverse_keys)
			{
				if (value <= (upper-10))
					dload_val = ms_data[page][offset] + 10;
				else
					return FALSE;
			}
			else 
			{
				if (value >= (lower+10))
					dload_val = ms_data[page][offset] - 10;
				else
					return FALSE;
			}
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
					dload_val = ms_data[page][offset] - 1;
				else
					return FALSE;
			}
			else 
			{
				if (value <= (upper-1))
					dload_val = (ms_data[page][offset]) + 1;
				else
					return FALSE;
			}
			retval = TRUE;
			break;
		case GDK_minus:
		case GDK_W:
		case GDK_w:
		case GDK_KP_Subtract:
			if (reverse_keys)
			{
				if (value <= (upper-1))
					dload_val = ms_data[page][offset] + 1;
				else
					return FALSE;
			}
			else 
			{
				if (value >= (lower+1))
					dload_val = ms_data[page][offset] - 1;
				else
					return FALSE;
			}
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
	if (retval)
		write_ve_const(widget,page,offset,dload_val,ign_parm, TRUE);

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
	extern GdkColor red;
	static GdkColor old_bg;
	static GdkColor text_color;
	static GtkStyle *style;
	static gint total_marked = 0;
	GtkWidget *frame = NULL;
	GtkWidget *parent = NULL;
	gchar * frame_name = NULL;
	extern GHashTable *dynamic_widgets;

	/* Select all chars on click */
//	printf("selecting all chars, or so I thought....\n");
//	gtk_editable_select_region(GTK_EDITABLE(widget),0,-1);

	if ((gboolean)data == TRUE)
		goto testit;

	if (event->button != 1) // Left button click 
		return FALSE;

	if (!grab_allowed)
		return FALSE;

testit:
	marked = (gboolean)g_object_get_data(G_OBJECT(widget),"marked");
	page = (gint)g_object_get_data(G_OBJECT(widget),"page");

	if (marked)
	{
		total_marked--;
		g_object_set_data(G_OBJECT(widget),"marked",GINT_TO_POINTER(FALSE));
		gtk_widget_modify_bg(widget,GTK_STATE_NORMAL,&old_bg);
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&text_color);
	}
	else
	{
		total_marked++;
		g_object_set_data(G_OBJECT(widget),"marked",GINT_TO_POINTER(TRUE));
		style = gtk_widget_get_style(widget);
		old_bg = style->bg[GTK_STATE_NORMAL];
		text_color = style->text[GTK_STATE_NORMAL];
		gtk_widget_modify_bg(widget,GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	}

	parent = gtk_widget_get_parent(GTK_WIDGET(widget));
	frame_name = (gchar *)g_object_get_data(G_OBJECT(parent),"rescaler_frame");
	if (!frame_name)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": widget_grab()\n\t\"rescale_frame\" key could NOT be found\n"));
		return FALSE;
	}

	frame = g_hash_table_lookup(dynamic_widgets, frame_name);
	if ((total_marked > 0) && (frame != NULL))
		gtk_widget_set_sensitive(GTK_WIDGET(frame),TRUE);
	else
		gtk_widget_set_sensitive(GTK_WIDGET(frame),FALSE);

	return FALSE;	// Allow other handles to run... 

}


/*
 \brief page_changed() is fired off whenever a new notebook page is chosen.
 This fucntion just sets a variable marking the current page.  this is to
 prevent the runtime sliders from being updated if they aren't visible
 \param notebook (GtkNotebook *) nbotebook that emitted the event
 \param page (GtkNotebookPage *) page
 \param page_no (guint) page number that's now active
 \param data (gpointer) unused
 */
EXPORT void page_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	gint tab_ident = 0;
	gint sub_page = 0;
	gchar * tmpbuf = NULL;
	extern gboolean forced_update;
	GtkWidget *sub = NULL;
	extern GHashTable *dynamic_widgets;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	tab_ident = (TabIdent)g_object_get_data(G_OBJECT(widget),"tab_ident");
	active_page = tab_ident;

	if (g_object_get_data(G_OBJECT(widget),"table_num"))
	{
		tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"table_num");
		active_table = (gint)g_ascii_strtod(tmpbuf,NULL);
	}

	if (g_object_get_data(G_OBJECT(widget),"sub-notebook"))
	{
		//printf(" This tab has a sub-notebook\n");
		sub = g_hash_table_lookup(dynamic_widgets, (g_object_get_data(G_OBJECT(widget),"sub-notebook")));
		if (GTK_IS_WIDGET(sub))
		{
			sub_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(sub));
			widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(sub),sub_page);
			//printf("subtable found, searching for active page\n");
			if (g_object_get_data(G_OBJECT(widget),"table_num"))
			{
				tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"table_num");
				active_table = (gint)g_ascii_strtod(tmpbuf,NULL);
			//	printf("found it,  active table %i\n",active_table);
			}
			//else
			//	printf("didn't find table_num key on subtable\n");
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
	gchar * tmpbuf = NULL;
	extern gboolean forced_update;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	if (!g_object_get_data(G_OBJECT(widget),"table_num"))
		return;
	else
	{
		tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"table_num");
		active_table = (gint)g_ascii_strtod(tmpbuf,NULL);
	}
//	printf("active table changed to %i\n",active_table);
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
	gchar ** fields = NULL;
	gint i = 0;
	gint num_widgets = 0;
	extern GHashTable *dynamic_widgets;

	fields = parse_keys(input,&num_widgets,",");

	for (i=0;i<num_widgets;i++)
	{
		widget = NULL;
		GtkWidget *widget = g_hash_table_lookup(dynamic_widgets,fields[i]);
		if (GTK_IS_WIDGET(widget))
			switch_labels((gpointer)widget,(gpointer)state);
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
	gboolean state = (gboolean) data;
	extern gint temp_units;

	if (GTK_IS_WIDGET(widget))
	{
		if ((gboolean)g_object_get_data(G_OBJECT(widget),"temp_dep") == TRUE)
		{
			if (state)
			{
				if (temp_units == FAHRENHEIT)
					gtk_label_set_text(GTK_LABEL(widget),g_object_get_data(G_OBJECT(widget),"alt_f_label"));
				else
					gtk_label_set_text(GTK_LABEL(widget),g_object_get_data(G_OBJECT(widget),"alt_c_label"));
			}
			else
			{
				if (temp_units == FAHRENHEIT)
					gtk_label_set_text(GTK_LABEL(widget),g_object_get_data(G_OBJECT(widget),"f_label"));
				else
					gtk_label_set_text(GTK_LABEL(widget),g_object_get_data(G_OBJECT(widget),"c_label"));
			}
		}
		else
		{
			if (state)
				gtk_label_set_text(GTK_LABEL(widget),g_object_get_data(G_OBJECT(widget),"alt_label"));
			else
				gtk_label_set_text(GTK_LABEL(widget),g_object_get_data(G_OBJECT(widget),"label"));
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
	extern GHashTable *dynamic_widgets;

	if (GTK_IS_OBJECT(widget))
	{
		algo = (Algorithm)g_object_get_data(G_OBJECT(widget),"algorithm");
		tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"applicable_tables");
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
	extern GtkWidget *main_window;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	GdkPixbuf *pixbuf = NULL;
	GtkWidget *image = NULL;

	if (!offline)
	{
		dialog = gtk_dialog_new_with_buttons("Save internal log, yes/no ?",
				GTK_WINDOW(main_window),GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_YES,GTK_RESPONSE_YES,
				GTK_STOCK_NO,GTK_RESPONSE_NO,
				NULL);
		hbox = gtk_hbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,10);
		pixbuf = gtk_widget_render_icon (hbox,GTK_STOCK_DIALOG_QUESTION,GTK_ICON_SIZE_DIALOG,NULL);
		image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_box_pack_start(GTK_BOX(hbox),image,TRUE,TRUE,10);
		label = gtk_label_new("Would you like to save the internal datalog for this session to disk?  It is a complete log and useful for playback/analysis at a future point in time");
		gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,10);
		gtk_widget_show_all(hbox);

		//		gtk_window_set_title(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),"Save internal log, yes/no ?");
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(main_window));

		result = gtk_dialog_run(GTK_DIALOG(dialog));
		g_object_unref(pixbuf);
		if (result == GTK_RESPONSE_YES)
			internal_datalog_dump(NULL,NULL);
		gtk_widget_destroy (dialog);

	}

	dialog = gtk_dialog_new_with_buttons("Save ECU settings to file?",
			GTK_WINDOW(main_window),GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_YES,GTK_RESPONSE_YES,
			GTK_STOCK_NO,GTK_RESPONSE_NO,
			NULL);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,10);
	pixbuf = gtk_widget_render_icon (hbox,GTK_STOCK_DIALOG_QUESTION,GTK_ICON_SIZE_DIALOG,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(hbox),image,TRUE,TRUE,10);
	label = gtk_label_new("Would you like to save the ECU settings to a file so that they can be restored at a future time?");
	gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,10);
	gtk_widget_show_all(hbox);

	gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(dialog)),GTK_WINDOW(main_window));
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_YES)
		select_file_for_ecu_backup(NULL,NULL);
	gtk_widget_destroy (dialog);

}


