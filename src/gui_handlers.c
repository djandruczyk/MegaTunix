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
#include <bitfield_handlers.h>
#include <config.h>
#include <conversions.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <fileio.h>
#include <gui_handlers.h>
#include <glib.h>
#include <init.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <offline.h>
#include <mode_select.h>
#include <ms_structures.h>
#include <notifications.h>
#include <post_process.h>
#include <req_fuel.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <structures.h>
#include <tabloader.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <user_outputs.h>
#include <vetable_gui.h>
#include <vex_support.h>



static gboolean grab_allowed = FALSE;
extern gboolean interrogated;
extern gboolean playback_mode;
extern gchar *delimiter;
extern gint ready;
extern GtkTooltips *tip;
extern GList ***ve_widgets;
extern struct Serial_Params *serial_params;
extern GHashTable *interdep_vars_1;
extern GHashTable *interdep_vars_2;

gboolean tips_in_use;
gint temp_units;
gint active_page = -1;
GdkColor red = { 0, 65535, 0, 0};
GdkColor green = { 0, 0, 65535, 0};
GdkColor black = { 0, 0, 0, 0};

gboolean paused_handlers = FALSE;
static gboolean constants_loaded = FALSE;
extern gint num_squirts_1;
extern gint num_squirts_2;
extern gint num_cyls_1;
extern gint num_cyls_2;
extern gint num_inj_1;
extern gint num_inj_2;
extern gint divider_1;
extern gint divider_2;
extern gint alternate_1;
extern gint last_num_squirts_1;
extern gint last_num_squirts_2;
extern gint last_num_cyls_1;
extern gint last_num_cyls_2;
extern gint last_num_inj_1;
extern gint last_num_inj_2;
extern gint last_divider_1;
extern gint last_divider_2;
extern gint last_alternate_1;
extern gfloat req_fuel_total_1;
extern gfloat req_fuel_total_2;
extern gfloat last_req_fuel_total_1;
extern gfloat last_req_fuel_total_2;
static gboolean err_flag = FALSE;
gboolean leaving = FALSE;

void leave(GtkWidget *widget, gpointer data)
{
	extern GHashTable *dynamic_widgets;
	extern gint realtime_id;
	extern gint dispatcher_id;
	extern gint statuscounts_id;
	struct Io_File * iofile = NULL;

	/* Stop timeout functions */
	leaving = TRUE;

	if (statuscounts_id)
		gtk_timeout_remove(statuscounts_id);
	statuscounts_id = 0;

	if (dispatcher_id)
		gtk_timeout_remove(dispatcher_id);
	dispatcher_id = 0;

	if(realtime_id)
		stop_realtime_tickler();
	realtime_id = 0;

	if (dynamic_widgets)
	{
		if (g_hash_table_lookup(dynamic_widgets,"dlog_close_log_button"))
			iofile = (struct Io_File *) g_object_get_data(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_close_log_button")),"data");
	}

	stop_datalogging();
	save_config();
	close_serial();
	if (iofile)	
		close_file(iofile);
	/* Free all buffers */
	mem_dealloc();
	gtk_main_quit();
	return;
}

gint comm_port_change(GtkEditable *editable)
{
	gchar *port;
	gboolean result;

	port = gtk_editable_get_chars(editable,0,-1);
	if(serial_params->open)
	{
		close_serial();
	}
	result = g_file_test(port,G_FILE_TEST_EXISTS);
	if (result)
	{
		open_serial(port);
		setup_serial_params();
	}
	else
	{
		update_logbar("comms_view","warning",g_strdup_printf("\"%s\" File not found\n",port),TRUE,FALSE);
	}


	g_free(port);
	return TRUE;
}

EXPORT gboolean toggle_button_handler(GtkWidget *widget, gpointer data)
{
	void *obj_data = NULL;
	gint handler = 0; 
	extern gint preferred_delimiter;
	extern GHashTable *dynamic_widgets;
	extern gchar *offline_firmware_choice;

	if (GTK_IS_OBJECT(widget))
	{
		obj_data = (void *)g_object_get_data(G_OBJECT(widget),"data");
		handler = (ToggleButton)g_object_get_data(G_OBJECT(widget),"handler");
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		switch ((ToggleButton)handler)
		{
			case OFFLINE_FIRMWARE_CHOICE:
				if(offline_firmware_choice)
					g_free(offline_firmware_choice);
				offline_firmware_choice = g_strdup(g_object_get_data(G_OBJECT(widget),"filename"));	
				break;
			case TOOLTIPS_STATE:
				gtk_tooltips_enable(tip);
				tips_in_use = TRUE;
				break;
			case FAHRENHEIT:
				temp_units = FAHRENHEIT;
				reset_temps(GINT_TO_POINTER(temp_units));
				force_an_update();
				break;
			case CELSIUS:
				temp_units = CELSIUS;
				reset_temps(GINT_TO_POINTER(temp_units));
				force_an_update();
				break;
			case COMMA:
				preferred_delimiter = COMMA;
				update_logbar("dlog_view", NULL,"Setting Log delimiter to a \"Comma\"\n",TRUE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup(",");
				break;
			case TAB:
				preferred_delimiter = TAB;
				update_logbar("dlog_view", NULL,"Setting Log delimiter to a \"Tab\"\n",TRUE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup("\t");
				break;
			case SPACE:
				preferred_delimiter = SPACE;
				update_logbar("dlog_view", NULL,"Setting Log delimiter to a \"Space\"\n",TRUE,FALSE);
				if (delimiter)
					g_free(delimiter);
				delimiter = g_strdup(" ");
				break;
			case REALTIME_VIEW:
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), FALSE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), TRUE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_start_button"), TRUE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_stop_button"), TRUE);
				reset_logviewer_state();
				playback_mode = FALSE;
//				if(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea"))
					//g_signal_emit_by_name(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea")),"configure_event",NULL);
				break;
			case PLAYBACK_VIEW:
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), FALSE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), TRUE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_start_button"), FALSE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_stop_button"), FALSE);
				reset_logviewer_state();
				playback_mode = TRUE;
//				if(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea"))
					//g_signal_emit_by_name(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea")),"configure_event",NULL);
				break;
			case HEX_VIEW:
			case DECIMAL_VIEW:
			case BINARY_VIEW:
				update_raw_memory_view((ToggleButton)handler,(gint)obj_data);
				break;	
		}
	}
	else
	{	/* not pressed */
		switch ((ToggleButton)handler)
		{
			case TOOLTIPS_STATE:
				gtk_tooltips_disable(tip);
				tips_in_use = FALSE;
				break;
			default:
				break;
		}
	}
	return TRUE;
}

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
	gchar * toggle_group = NULL;
	gchar * swap_label = NULL;
	gboolean invert_state = FALSE;
	gboolean state = FALSE;
	extern gint dbg_lvl;
	extern gint ecu_caps;
	extern gint **ms_data;

	if (paused_handlers)
		return TRUE;

	if (GTK_IS_OBJECT(widget))
	{
		ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
		page = (gint)g_object_get_data(G_OBJECT(widget),"page");
		offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
		dl_type = (gint)g_object_get_data(G_OBJECT(widget),"dl_type");
		bitshift = (gint)g_object_get_data(G_OBJECT(widget),"bitshift");
		bitval = (gint)g_object_get_data(G_OBJECT(widget),"bitval");
		bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");
		handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
		toggle_group = (gchar *)g_object_get_data(G_OBJECT(widget),
				"toggle_group");
		invert_state = (gboolean )g_object_get_data(G_OBJECT(widget),
				"invert_state");
		swap_label = (gchar *)g_object_get_data(G_OBJECT(widget),
				"swap_label");
	}

	// If it's a check button then it's state is dependant on the button's state
	if (!GTK_IS_RADIO_BUTTON(widget))
		bitval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

	/* Toggles a group ON/OFF based on a widgets state.... */
	if (toggle_group)
	{
		state = invert_state == FALSE ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)):!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
		g_list_foreach(get_list(toggle_group),set_widget_sensitive,(gpointer)state);
	}
	/* Swaps the label of another control based on widget state... */
	if (swap_label)
		switch_labels(swap_label,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));

	switch ((SpinButton)handler)
	{
		case GENERIC:
			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	//clears bits 
			tmp = tmp | (bitval << bitshift);
			ms_data[page][offset] = tmp;
			dload_val = tmp;
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
			if (ecu_caps & DUALTABLE)
			{
				tmp = ms_data[page][offset];
				tmp = tmp & ~bitmask;// clears bits 
				tmp = tmp | (bitval << bitshift);
				ms_data[page][offset] = tmp;
				dload_val = tmp;
			}
			else
			{
				last_alternate_1 = alternate_1;
				alternate_1 = bitval;
				ms_data[page][offset] = bitval;
				dload_val = bitval;
				g_hash_table_insert(interdep_vars_1,
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));
				check_req_fuel_limits();
			}
			break;
		default:
			dbg_func(g_strdup_printf(__FILE__": bitmask_button_handler()\n\tbitmask button at page: %i, offset %i, NOT handled\n\tERROR!!, contact author\n",page,offset),CRITICAL);
			return FALSE;
			break;

	}
	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download(widget,dload_val);
		write_ve_const(page, offset, dload_val, ign_parm);
	}
	return TRUE;
}

EXPORT gboolean std_button_handler(GtkWidget *widget, gpointer data)
{
	/* get any datastructures attached to the widget */
	void *obj_data = NULL;
	gint handler = -1;
	static gboolean queue_referenced =  FALSE;
	extern GAsyncQueue *io_queue;
	extern gboolean no_update;
	extern gboolean offline;
	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	obj_data = (void *)g_object_get_data(G_OBJECT(widget),"data");
	handler = (StdButton)g_object_get_data(G_OBJECT(widget),"handler");

	if (queue_referenced == FALSE)
		g_async_queue_ref(io_queue);
	if (handler == 0)
	{
		dbg_func(__FILE__": std_button_handler()\n\thandler not bound to object, CRITICAL ERROR, aborting\n",CRITICAL);
		return FALSE;
	}

	switch ((StdButton)handler)
	{
		case RESCALE_TABLE:
			rescale_table(obj_data);
			break;
		case INTERROGATE_ECU:
			io_cmd(IO_INTERROGATE_ECU, NULL);
			break;

		case START_REALTIME:
			if (offline)
				break;
			no_update = FALSE;
			if (!interrogated)
				io_cmd(IO_INTERROGATE_ECU, NULL);
			if (!constants_loaded)
				io_cmd(IO_READ_VE_CONST, NULL);
			start_realtime_tickler();
			force_an_update();
			break;
		case STOP_REALTIME:
			if (offline)
				break;
			stop_realtime_tickler();
			no_update = TRUE;
			break;
		case READ_VE_CONST:
			if (offline)
				break;
			io_cmd(IO_READ_VE_CONST, NULL);
			break;
		case READ_RAW_MEMORY:
			if (offline)
				break;
			io_cmd(IO_READ_RAW_MEMORY,(gpointer)obj_data);
			break;
		case CHECK_ECU_COMMS:
			if (offline)
				break;
			io_cmd(IO_COMMS_TEST,NULL);
			break;
		case BURN_MS_FLASH:
			io_cmd(IO_BURN_MS_FLASH,NULL);
			break;
		case SELECT_DLOG_EXP:
			if (offline)
				break;
			present_filesavebox(DATALOG_EXPORT,(gpointer)widget);
			break;
		case SELECT_DLOG_IMP:
			present_filesavebox(DATALOG_IMPORT,(gpointer)widget);
			break;
		case CLOSE_LOGFILE:
			if (offline)
				break;
			stop_datalogging();
			close_file(obj_data);
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
		case EXPORT_VETABLE:
			if (!interrogated)
				break;
			present_filesavebox(VE_EXPORT,(gpointer)widget);
			break;
		case IMPORT_VETABLE:
			if (!interrogated)
				break;
			present_filesavebox(VE_IMPORT,(gpointer)widget);
			break;
		case REVERT_TO_BACKUP:
			revert_to_previous_data();
			break;
		case BACKUP_ALL:
			if (!interrogated)
				break;
			present_filesavebox(FULL_BACKUP,(gpointer)widget);
			break;
		case RESTORE_ALL:
			if (!interrogated)
				break;
			present_filesavebox(FULL_RESTORE,(gpointer)widget);
			break;
		case SELECT_PARAMS:
			if (!interrogated)
				break;
			present_viewer_choices();
			break;
		case REQ_FUEL_POPUP:
			reqd_fuel_popup(widget);
			req_fuel_change(widget);
			break;
		case OFFLINE_MODE:
			set_offline_mode();
			break;
		default:
			dbg_func(__FILE__": std_button_handler()\n\t Standard button not handled properly, BUG detected\n",CRITICAL);
	}		
	return TRUE;
}

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
	gint spconfig = 0;
	gboolean ign_parm = FALSE;
	gboolean temp_dep = FALSE;
	gint tmpi = 0;
	gint tmp = 0;
	gint handler = -1;
	gint divider_offset = 0;
	gfloat value = 0.0;
	GtkWidget * tmpwidget = NULL;
	extern gint realtime_id;
	extern gint **ms_data;
	extern gint lv_scroll;
	struct Reqd_Fuel *reqd_fuel = NULL;
	extern GHashTable *dynamic_widgets;
	extern struct Firmware_Details *firmware;

	if ((paused_handlers) || (!ready))
		return TRUE;

	if (!GTK_IS_WIDGET(widget))
	{
		dbg_func(__FILE__": spin_button_handler()\n\twidget pointer is NOT valid\n",CRITICAL);
		return FALSE;
	}

	reqd_fuel = (struct Reqd_Fuel *)g_object_get_data(
			G_OBJECT(widget),"reqd_fuel");
	handler = (SpinButton)g_object_get_data(G_OBJECT(widget),"handler");
	ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
	spconfig = (gint) g_object_get_data(G_OBJECT(widget),"spconfig");
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
//		case RESCALE_TABLE:
//			rescale_table(widget);
//			break;
		case SER_INTERVAL_DELAY:
			serial_params->read_wait = (gint)value;
			if (realtime_id > 0)
			{
				stop_realtime_tickler();
				start_realtime_tickler();
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
			last_req_fuel_total_1 = req_fuel_total_1;
			req_fuel_total_1 = value;
			check_req_fuel_limits();
			break;
		case REQ_FUEL_2:
			last_req_fuel_total_2 = req_fuel_total_2;
			req_fuel_total_2 = value;
			check_req_fuel_limits();
			break;
		case LOGVIEW_ZOOM:
			lv_scroll = tmpi;
			tmpwidget = g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea");	
			if (tmpwidget)
				g_signal_emit_by_name(tmpwidget,"configure_event",NULL);
			break;

		case NUM_SQUIRTS_1:
			/* This actuall effects another variable */
			divider_offset = firmware->page_params[page]->divider_offset;
			last_num_squirts_1 = num_squirts_1;
			last_divider_1 = ms_data[page][divider_offset];

			num_squirts_1 = tmpi;
			if (num_cyls_1 % num_squirts_1)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);
			}
			else
			{
				dload_val = 
					(gint)(((float)num_cyls_1/
						(float)num_squirts_1)+0.001);
				ms_data[page][divider_offset] = dload_val;
				divider_1 = dload_val;
				g_hash_table_insert(interdep_vars_1,
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));
				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS_1:
			/* Updates a shared bitfield */
			divider_offset = firmware->page_params[page]->divider_offset;
			last_divider_1 = ms_data[page][divider_offset];
			last_num_cyls_1 = num_cyls_1;

			num_cyls_1 = tmpi;
			if (num_cyls_1 % num_squirts_1)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);	
			}
			else
			{
				tmp = ms_data[page][offset];
				tmp = tmp & ~bitmask;	/*clears top 4 bits */
				tmp = tmp | ((tmpi-1) << bitshift);
				ms_data[page][offset] = tmp;
				dload_val = tmp;
				g_hash_table_insert(interdep_vars_1,
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));

				dload_val = 
					(gint)(((float)num_cyls_1/
						(float)num_squirts_1)+0.001);
				ms_data[page][divider_offset] = dload_val;
				divider_1 = dload_val;
				g_hash_table_insert(interdep_vars_1,
						GINT_TO_POINTER(divider_offset),
						GINT_TO_POINTER(dload_val));

				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);	
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS_1:
			/* Updates a shared bitfield */
			last_num_inj_1 = num_inj_1;
			num_inj_1 = tmpi;

			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << bitshift);
			ms_data[page][offset] = tmp;
			dload_val = tmp;
			g_hash_table_insert(interdep_vars_1,
					GINT_TO_POINTER(offset),
					GINT_TO_POINTER(dload_val));

			check_req_fuel_limits();
			break;
		case NUM_SQUIRTS_2:
			divider_offset = firmware->page_params[page]->divider_offset;
			/* This actuall effects another variable */
			last_num_squirts_2 = num_squirts_2;
			last_divider_2 = ms_data[page][divider_offset];

			num_squirts_2 = tmpi;
			if (num_cyls_2 % num_squirts_2)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);
			}
			else
			{
				dload_val = 
					(gint)(((float)num_cyls_2/
						(float)num_squirts_2)+0.001);
				ms_data[page][divider_offset] = dload_val;
				divider_2 = dload_val;
				g_hash_table_insert(interdep_vars_2,
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));
				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS_2:
			divider_offset = firmware->page_params[page]->divider_offset;
			/* Updates a shared bitfield */
			last_divider_2 = ms_data[page][divider_offset];
			last_num_cyls_2 = num_cyls_2;

			num_cyls_2 = tmpi;
			if (num_cyls_2 % num_squirts_2)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);	
			}
			else
			{
				tmp = ms_data[page][offset];
				tmp = tmp & ~bitmask;	/*clears top 4 bits */
				tmp = tmp | ((tmpi-1) << bitshift);
				ms_data[page][offset] = tmp;
				dload_val = tmp;
				g_hash_table_insert(interdep_vars_2,
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));

				dload_val = 
					(gint)(((float)num_cyls_2/
						(float)num_squirts_2)+0.001);
				ms_data[page][divider_offset] = dload_val;
				divider_2 = dload_val;
				g_hash_table_insert(interdep_vars_2,
						GINT_TO_POINTER(divider_offset),
						GINT_TO_POINTER(dload_val));

				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);	
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS_2:
			/* Updates a shared bitfield */

			last_num_inj_2 = num_inj_2;
			num_inj_2 = tmpi;
			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << bitshift);
			ms_data[page][offset] = tmp;
			dload_val = tmp;
			g_hash_table_insert(interdep_vars_2,
					GINT_TO_POINTER(offset),
					GINT_TO_POINTER(dload_val));

			check_req_fuel_limits();
			break;
		case TRIGGER_ANGLE:
			if (spconfig == 0)
			{
				dbg_func(__FILE__": spin_button_handler()\n\tERROR Triggler Angle spinbutton call, but spconfig variable is unset, Aborting handler!!!\n",CRITICAL);
				dl_type = 0;  
				break;
			
			}
			if (value > 112.15)	/* Extra long trigger needed */	
			{
				tmp = ms_data[page][spconfig];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);	/* Set xlong_trig */
				ms_data[page][spconfig] = tmp;
				write_ve_const(page, spconfig, tmp, ign_parm);
				value -= 45.0;
				dload_val = convert_before_download(widget,value);
			}
			else if (value > 89.65) /* Long trigger needed */
			{
				tmp = ms_data[page][spconfig];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);	/* Set long_trig */
				ms_data[page][spconfig] = tmp;
				write_ve_const(page, spconfig, tmp, ign_parm);
				value -= 22.5;
				dload_val = convert_before_download(widget,value);
			}
			else	// value <= 89.65 degrees, no long trigger
			{
				tmp = ms_data[page][spconfig];
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				ms_data[page][spconfig] = tmp;
				write_ve_const(page, spconfig, tmp, ign_parm);
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
			dbg_func(g_strdup_printf(__FILE__": spin_button_handler()\n\tERROR spinbutton not handled, handler = %i\n",handler),CRITICAL);
			dl_type = 0;  
			break;
	}
	if (dl_type == IMMEDIATE) 
		write_ve_const(page, offset, dload_val, ign_parm);
	return TRUE;

}

void update_ve_const()
{
	gint page = 0;
	gint offset = 0;
	gfloat tmp = 0.0;
	gint reqfuel = 0;
	union config11 cfg11;
	union config12 cfg12;
	extern gint **ms_data;
	extern gint ecu_caps;
	extern struct Firmware_Details *firmware;

	/* Point to Table0 (stock MS ) data... */

	check_config13(ms_data[page][firmware->page_params[page]->cfg13_offset]);

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

	 */
	if (ecu_caps & DUALTABLE)
	{
		/* Table 1 */
		page = 0;
		cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];	
		cfg12.value = ms_data[page][firmware->page_params[page]->cfg12_offset];	
		num_cyls_1 = cfg11.bit.cylinders+1;
		last_num_cyls_1 = cfg11.bit.cylinders+1;
                num_inj_1 = cfg12.bit.injectors+1;
                last_num_inj_1 = cfg12.bit.injectors+1;

		divider_1 = ms_data[page][firmware->page_params[page]->divider_offset];
		last_divider_1 = divider_1;
		reqfuel = ms_data[page][firmware->page_params[page]->reqfuel_offset];

		/* ReqFuel Total */
                tmp = (float)(num_inj_1)/(float)(divider_1);
                tmp *= (float)reqfuel;
                tmp /= 10.0;
                req_fuel_total_1 = tmp;
                last_req_fuel_total_1 = tmp;

		/* Injections per cycle */
		num_squirts_1 =	(float)(num_cyls_1)/(float)(divider_1);
		if (num_squirts_1 < 1 )
			num_squirts_1 = 1;
		last_num_squirts_1 = num_squirts_1;


		/* Table 2 */
		page = 1;
		cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];	
		cfg12.value = ms_data[page][firmware->page_params[page]->cfg12_offset];	
		num_cyls_2 = cfg11.bit.cylinders+1;
		last_num_cyls_2 = cfg11.bit.cylinders+1;
                num_inj_2 = cfg12.bit.injectors+1;
                last_num_inj_2 = cfg12.bit.injectors+1;

		divider_2 = ms_data[page][firmware->page_params[page]->divider_offset];
		last_divider_2 = divider_2;
		reqfuel = ms_data[page][firmware->page_params[page]->reqfuel_offset];

		/* ReqFuel Total */
                tmp = (float)(num_inj_2)/(float)(divider_2);
                tmp *= (float)reqfuel;
                tmp /= 10.0;
                req_fuel_total_2 = tmp;
                last_req_fuel_total_2 = tmp;

		/* Injections per cycle */
		num_squirts_2 =	(float)(num_cyls_2)/(float)(divider_2);
		if (num_squirts_2 < 1 )
			num_squirts_2 = 1;
		last_num_squirts_2 = num_squirts_2;

		set_reqfuel_state(BLACK,page);
	}
	else
	{
		/*  B&G, MSnS, MSnEDIS req Fuel calc *
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
		/* Table 1 */
		page = 0;
		cfg11.value = ms_data[page][firmware->page_params[page]->cfg11_offset];	
		cfg12.value = ms_data[page][firmware->page_params[page]->cfg12_offset];	
		num_cyls_1 = cfg11.bit.cylinders+1;
		last_num_cyls_1 = cfg11.bit.cylinders+1;
                num_inj_1 = cfg12.bit.injectors+1;
                last_num_inj_1 = cfg12.bit.injectors+1;
		divider_1 = ms_data[page][firmware->page_params[page]->divider_offset];
		last_divider_1 = divider_1;
		reqfuel = ms_data[page][firmware->page_params[page]->reqfuel_offset];
		alternate_1 = ms_data[page][firmware->page_params[page]->alternate_offset];
		last_alternate_1 = alternate_1;

		/* ReqFuel Total */
                tmp = (float)(num_inj_1)/
			((float)(divider_1)*((float)(alternate_1)+1.0));
                tmp *= (float)reqfuel;
                tmp /= 10.0;
                req_fuel_total_1 = tmp;
                last_req_fuel_total_1 = tmp;

		/* Injections per cycle */
		num_squirts_1 =	(float)(num_cyls_1)/(float)(divider_1);
		if (num_squirts_1 < 1 )
			num_squirts_1 = 1;
		last_num_squirts_1 = num_squirts_1;

		set_reqfuel_state(BLACK,page);
	}	// End of B&G specific code...


	/* Update all on screen controls (except bitfields (done above)*/
	for (page=0;page<firmware->total_pages;page++)
	{
		for (offset=0;offset<firmware->page_params[page]->size;offset++)
		{
			if (ve_widgets[page][offset] != NULL)
			{
				//				printf("there is a list at %i,%i with %i elements\n",page,offset,g_list_length(ve_widgets[page][offset]));
				g_list_foreach(ve_widgets[page][offset],
						update_widget,NULL);
			}
		}
	}
}

void update_widget(gpointer object, gpointer user_data)
{
	GtkWidget * widget = object;
	gint dl_type = -1;
	gboolean temp_dep = FALSE;
	gint tmpi = -1;
	gint page = -1;
	gint offset = -1;
	gfloat value = 0.0;
	gint bitval = -1;
	gint bitshift = -1;
	gint bitmask = -1;
	gchar * toggle_group = NULL;
	gboolean invert_state = FALSE;
	gboolean state = FALSE;
	gchar * swap_label = NULL;

	if (GTK_IS_OBJECT(widget))
	{
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
		temp_dep = (gboolean)g_object_get_data(G_OBJECT(widget),
				"temp_dep");
		toggle_group = (gchar *)g_object_get_data(G_OBJECT(widget),
				"toggle_group");
		invert_state = (gboolean)g_object_get_data(G_OBJECT(widget),
				"invert_state");
		swap_label = (gchar *)g_object_get_data(G_OBJECT(widget),
				"swap_label");

		value = convert_after_upload(widget);  

		if (temp_dep)
		{
			if (temp_units == CELSIUS)
				value = (value-32)*(5.0/9.0);
		}

		// update widget whether spin,radio or checkbutton  (check encompases radio)
		if (GTK_IS_SPIN_BUTTON(widget))
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
		else if (GTK_IS_CHECK_BUTTON(widget))
		{
			/* If value masked by bitmask, shifted right by bitshift = bitval
			 * then set button state to on...
			 */
			tmpi = (gint)value;
			if (((tmpi & bitmask) >> bitshift) == bitval)
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),TRUE);
			else
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),FALSE);
		}
		else if (GTK_IS_SCROLLED_WINDOW(widget))
		{
			/* This will looks really weird, but is used in the 
			 * special case of a treeview widget which is always
			 * packed into ascrolled window. Since the treeview
			 * depends on ECU variables, we call a handler here
			 * passing in a pointer to the treeview(the scrolled
			 * window's child widget)
			 */
			update_model_from_view(gtk_bin_get_child(GTK_BIN(widget)));
		}
		if (toggle_group)
		{
			state = invert_state == FALSE ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)):!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			g_list_foreach(get_list(toggle_group),set_widget_sensitive,(gpointer)state);
		}
		/* Swaps the label of another control based on widget state... */
		if (swap_label)
			switch_labels(swap_label,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));

	}
}

EXPORT gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if(event->keyval == GDK_Shift_L)
	{
		if (event->type == GDK_KEY_PRESS)
			grab_allowed = TRUE;
		else
			grab_allowed = FALSE;
	}
	return FALSE;
}

EXPORT gboolean spin_button_grab(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gboolean marked = FALSE;
	gint page = -1;
	extern GdkColor red;
	static GdkColor old_bg;
	static GdkColor text_color;
	static GtkStyle *style;
	static gint total_marked = 0;
	GtkWidget *frame = NULL;
	extern GHashTable *dynamic_widgets;

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

	frame = g_hash_table_lookup(dynamic_widgets, g_strdup_printf("rescale_frame_page%i",page));
	if ((total_marked > 0) && (frame != NULL))
		gtk_widget_set_sensitive(GTK_WIDGET(frame),TRUE);
	else
		gtk_widget_set_sensitive(GTK_WIDGET(frame),FALSE);

	return FALSE;	// Allow other handles to run... 

}

void page_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	gint page_ident = 0;
	GtkWidget *widget = gtk_notebook_get_nth_page(notebook,page_no);

	page_ident = (PageIdent)g_object_get_data(G_OBJECT(widget),"page_ident");
	active_page = page_ident;
	force_an_update();

	return;
}

void switch_labels(gchar * widget_name,gboolean state)
{
	extern GHashTable *dynamic_widgets;
	extern gint temp_units;
	GtkWidget *widget = g_hash_table_lookup(dynamic_widgets,widget_name);
	if (widget)
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
	}
}
