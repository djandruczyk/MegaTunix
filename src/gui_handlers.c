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
#include <logviewer_gui.h>
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
#include <vetable_gui.h>
#include <vex_support.h>

static gboolean grab_allowed = FALSE;
extern gboolean interrogated;
extern gboolean connected;
extern gboolean logviewer_mode;
extern gchar *delimiter;
extern gint statuscounts_id;
extern gint ready;
extern GtkTooltips *tip;
extern GList *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];
struct DynamicButtons buttons;
extern struct Serial_Params *serial_params;
extern GHashTable *interdep_vars_1;
extern GHashTable *interdep_vars_2;

gboolean tips_in_use;
gint temp_units;
gint active_page = -1;
GdkColor red = { 0, 65535, 0, 0};
GdkColor green = { 0, 0, 65535, 0};
GdkColor black = { 0, 0, 0, 0};

/* mt_classic[] and mt_full[] are arrays laid out like the datalogging
 * screen, insert a "1" where you want the button selected for that mode
 * otherwise use a zero...
 */
const gint mt_classic[] =
{
	0,1,1,1,0,
	0,0,0,0,0,
	0,0,0,0,0,
	0,1,0,0,0,
	1,0,1,0,0,
	1,1,1,1,0,
	1,0,0,0,0,
	0,0,0,0,0,
	0,0 
};

const gint mt_full[] = 
{
	1,1,1,1,0,
	0,0,0,0,0,
	0,0,0,0,0,
	1,1,0,1,1,
	1,0,1,0,0,
	1,1,1,1,1,
	1,1,1,1,0,
	0,0,0,0,0,
	0,0 
}; 

gboolean paused_handlers = FALSE;
static gboolean constants_loaded = FALSE;
extern gint num_squirts_1;
extern gint num_squirts_2;
extern gint num_cylinders_1;
extern gint num_cylinders_2;
extern gint num_injectors_1;
extern gint num_injectors_2;
extern gfloat req_fuel_total_1;
extern gfloat req_fuel_total_2;
static gboolean err_flag = FALSE;

void leave(GtkWidget *widget, gpointer data)
{
	extern GHashTable *dynamic_widgets;

	if (statuscounts_id)
		gtk_timeout_remove(statuscounts_id);
	statuscounts_id = 0;

	struct Io_File * iofile = NULL;
	iofile = (struct Io_File *) g_object_get_data(
			G_OBJECT(g_hash_table_lookup(dynamic_widgets,"dlog_close_log_button")),"data");

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

gboolean toggle_button_handler(GtkWidget *widget, gpointer data)
{
	void *obj_data = NULL;
	gint handler = 0; 
	extern gint preferred_delimiter;
	extern GHashTable *dynamic_widgets;

	if (GTK_IS_OBJECT(widget))
	{
		obj_data = (void *)g_object_get_data(G_OBJECT(widget),"data");
		handler = (ToggleButton)g_object_get_data(G_OBJECT(widget),"handler");
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		switch ((ToggleButton)handler)
		{
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
				logviewer_mode = FALSE;
				g_signal_emit_by_name(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea")),"configure_event",NULL);
				break;
			case PLAYBACK_VIEW:
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_params_button"), FALSE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_select_logfile_button"), TRUE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_start_button"), FALSE);
				gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"logviewer_stop_button"), FALSE);
				logviewer_mode = TRUE;
				g_signal_emit_by_name(G_OBJECT(g_hash_table_lookup(dynamic_widgets,"logviewer_trace_darea")),"configure_event",NULL);
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

gboolean bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	gint bitshift = -1;
	gint bitval = -1;
	gint bitmask = -1;
	gint dload_val = -1;
	gint page = -1;
	guchar tmp = 0;
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
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];

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
		g_list_foreach(get_list(toggle_group),set_widget_state,(gpointer)state);
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

gboolean std_button_handler(GtkWidget *widget, gpointer data)
{
	/* get any datastructures attached to the widget */
	void *obj_data = NULL;
	gint handler = -1;
	static gboolean queue_referenced =  FALSE;
	extern GAsyncQueue *io_queue;
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
		case INTERROGATE_ECU:
			io_cmd(IO_INTERROGATE_ECU, NULL);
			break;

		case START_REALTIME:
			if (!interrogated)
				io_cmd(IO_INTERROGATE_ECU, NULL);
			if (!constants_loaded)
				io_cmd(IO_READ_VE_CONST, NULL);
			start_realtime_tickler();
			break;
		case STOP_REALTIME:
			stop_realtime_tickler();
			//stop_datalogging();
			reset_runtime_status();
			break;
		case READ_VE_CONST:
			io_cmd(IO_READ_VE_CONST, NULL);
			break;
		case READ_RAW_MEMORY:
			io_cmd(IO_READ_RAW_MEMORY,(gpointer)obj_data);
			break;
		case CHECK_ECU_COMMS:
			io_cmd(IO_COMMS_TEST,NULL);
			break;
		case BURN_MS_FLASH:
			io_cmd(IO_BURN_MS_FLASH,NULL);
			break;
		case SELECT_DLOG_EXP:
			present_filesavebox(DATALOG_EXPORT,(gpointer)widget);
			break;
		case SELECT_DLOG_IMP:
			present_filesavebox(DATALOG_IMPORT,(gpointer)widget);
			break;
		case CLOSE_LOGFILE:
			stop_datalogging();
			close_file(obj_data);
			break;
		case START_DATALOGGING:
			start_datalogging();
			break;
		case STOP_DATALOGGING:
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
			present_viewer_choices(obj_data);
			break;
		case REQ_FUEL_POPUP:
			reqd_fuel_popup(widget);
			req_fuel_change(widget);
			break;
	}		
	return TRUE;
}

gboolean spin_button_handler(GtkWidget *widget, gpointer data)
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
	gfloat value = 0.0;
	GtkWidget * tmpwidget = NULL;
	extern gint realtime_id;
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];
	extern gint ecu_caps;
	extern gint lv_scroll;
	struct Ve_Const_Std * ve_const = (struct Ve_Const_Std *) ms_data[0];
	struct Ve_Const_DT_2 * ve_const_dt2 = NULL;
	struct Reqd_Fuel *reqd_fuel = NULL;
	extern GHashTable *dynamic_widgets;

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

	if (ecu_caps & DUALTABLE)
		ve_const_dt2 = (struct Ve_Const_DT_2 *) (ms_data[1]);

	tmpi = (int)(value+.001);


	switch ((SpinButton)handler)
	{
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
			req_fuel_total_1 = value;
			check_req_fuel_limits();
			break;
		case REQ_FUEL_2:
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
			num_squirts_1 = tmpi;
			if (num_cylinders_1 % num_squirts_1)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);
			}
			else
			{
				dload_val = 
					(gint)(((float)num_cylinders_1/
						(float)num_squirts_1)+0.001);
				ve_const->divider = dload_val;
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
			num_cylinders_1 = tmpi;
			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << bitshift);
			ms_data[page][offset] = tmp;
			dload_val = tmp;
			g_hash_table_insert(interdep_vars_1,
					GINT_TO_POINTER(offset),
					GINT_TO_POINTER(dload_val));

			dload_val = 
				(gint)(((float)num_cylinders_1/
					(float)num_squirts_1)+0.001);
			ve_const->divider = dload_val;
			g_hash_table_insert(interdep_vars_1,
					GINT_TO_POINTER(DIV_OFFSET_1),
					GINT_TO_POINTER(dload_val));

			if (num_cylinders_1 % num_squirts_1)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);	
			}
			else
			{
				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);	
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS_1:
			/* Updates a shared bitfield */
			num_injectors_1 = tmpi;
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
			/* This actuall effects another variable */
			num_squirts_2 = tmpi;
			if (num_cylinders_2 % num_squirts_2)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);
			}
			else
			{
				dload_val = 
					(gint)(((float)num_cylinders_2/
						(float)num_squirts_2)+0.001);
				ve_const_dt2->divider = dload_val;
				g_hash_table_insert(interdep_vars_2,
						GINT_TO_POINTER(offset),
						GINT_TO_POINTER(dload_val));
				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS_2:
			/* Updates a shared bitfield */
			num_cylinders_2 = tmpi;
			tmp = ms_data[page][offset];
			tmp = tmp & ~bitmask;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << bitshift);
			ms_data[page][offset] = tmp;
			dload_val = tmp;
			g_hash_table_insert(interdep_vars_2,
					GINT_TO_POINTER(offset),
					GINT_TO_POINTER(dload_val));

			dload_val = 
				(gint)(((float)num_cylinders_2/
					(float)num_squirts_2)+0.001);
			ve_const_dt2->divider = dload_val;
			g_hash_table_insert(interdep_vars_2,
					GINT_TO_POINTER(DIV_OFFSET_2),
					GINT_TO_POINTER(dload_val));

			if (num_cylinders_2 % num_squirts_2)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,page);	
			}
			else
			{
				err_flag = FALSE;
				set_reqfuel_state(BLACK,page);	
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS_2:
			/* Updates a shared bitfield */
			num_injectors_2 = tmpi;
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
			if (!(ecu_caps & (S_N_SPARK|S_N_EDIS)))
				dbg_func(__FILE__": spin_button_handler()\n\tERROR, Trigger angle set but not using Ignition Firmware\n",CRITICAL);
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
	extern guchar *ms_data[MAX_SUPPORTED_PAGES];
	extern gint ecu_caps;
	struct Ve_Const_Std *ve_const = NULL;
	struct Ve_Const_DT_1 *ve_const_dt1 = NULL;
	struct Ve_Const_DT_2 *ve_const_dt2 = NULL;

	/* Point to Table0 (stock MS ) data... */
	ve_const = (struct Ve_Const_Std *) ms_data[0];

	check_config11(ve_const->config11.value);
	check_config13(ve_const->config13.value);


	/* DualTable Fuel Calculations
	 * DT code no longer uses the "alternate" firing mode as each table
	 * is pretty much independant from the other,  so the calcs are a 
	 * little simpler...
	 *
	 *                                        /     num_injectors_1   \
	 *         	   req_fuel_per_squirt * (-------------------------)
	 *                                        \ 	    divider       /
	 * req_fuel_total = --------------------------------------------------
	 *				10
	 *
	 * where divider = num_cylinders/num_squirts;
	 *

	 */
	if (ecu_caps & DUALTABLE)
	{
		ve_const_dt1 = (struct Ve_Const_DT_1 *) ms_data[0];
		ve_const_dt2 = (struct Ve_Const_DT_2 *) (ms_data[1]);

		/* Table 1 */
                tmp =   (float)(ve_const_dt1->config12.bit.injectors+1) /
                        (float)(ve_const_dt1->divider);
                tmp *= (float)ve_const_dt1->req_fuel;
                tmp /= 10.0;
                req_fuel_total_1 = tmp;

		/* Table 2 */
                tmp =   (float)(ve_const_dt2->config12.bit.injectors+1) /
                        (float)(ve_const_dt2->divider);
                tmp *= (float)ve_const_dt2->req_fuel;
                tmp /= 10.0;
                req_fuel_total_2 = tmp;

		/* Config11 bits */
		num_cylinders_1 = ve_const_dt1->config11.bit.cylinders+1;
		num_cylinders_2 = ve_const_dt2->config11.bit.cylinders+1;
		/* Config12 bits */
                num_injectors_1 = ve_const_dt1->config12.bit.injectors+1;
                num_injectors_2 = ve_const_dt2->config12.bit.injectors+1;

		/* Injections per cycle */
		tmp =	(float)(ve_const_dt1->config11.bit.cylinders+1) /
			(float)(ve_const_dt1->divider);
		num_squirts_1 = (gint)tmp;
		if (num_squirts_1 < 1 )
			num_squirts_1 = 1;

		tmp =	(float)(ve_const_dt2->config11.bit.cylinders+1) /
			(float)(ve_const_dt2->divider);
		num_squirts_2 = (gint)tmp;
		if (num_squirts_2 < 1 )
			num_squirts_2 = 1;

		set_reqfuel_state(BLACK,page);
	}
	else
	{
		/*  B&G, MSnS, MSnEDIS req Fuel calc *
		 * req-fuel 
		 *                                   /     num_injectors     \
		 *    	   req_fuel_per_squirt * (-------------------------)
		 *                                   \ divider*(alternate+1) /
		 * req_fuel_total = ------------------------------------------
		 *				10
		 *
		 * where divider = num_cylinders/num_squirts;
		 *
		 * The req_fuel_per_squirt is the part stored in the MS ECU as 
		 * the req_fuel variable.  Take note when doing conversions.  
		 * On screen the value is divided by ten from what is 
		 * in the MS.  
		 * 
		 */
		tmp =	(float)(ve_const->config12.bit.injectors+1.0) /
			(float)(ve_const->divider*(ve_const->alternate+1.0));
		tmp *= (float)ve_const->req_fuel;
		tmp /= 10.0;
		req_fuel_total_1 = tmp;

		num_cylinders_1 = ve_const->config11.bit.cylinders+1;
		num_injectors_1 = ve_const->config12.bit.injectors+1;

		/* Injections per cycle */
		tmp =	(float)(ve_const->config11.bit.cylinders+1) /
			(float)(ve_const->divider);
		num_squirts_1 = (gint)tmp;

		set_reqfuel_state(BLACK,page);
	}	// End of B&G specific code...


	/* Update all on screen controls (except bitfields (done above)*/
	for (page=0;page<MAX_SUPPORTED_PAGES;page++)
	{
		for (offset=0;offset<2*MS_PAGE_SIZE;offset++)
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
		if (toggle_group)
		{
			state = invert_state == FALSE ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)):!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			g_list_foreach(get_list(toggle_group),set_widget_state,(gpointer)state);
		}
		/* Swaps the label of another control based on widget state... */
		if (swap_label)
			switch_labels(swap_label,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));

	}
}

gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if(event->keyval == GDK_Shift_L)
	{
		if (event->type == GDK_KEY_PRESS)
			grab_allowed = TRUE;
		else
			grab_allowed = FALSE;
	}
	return TRUE;
}

gboolean spin_button_grab(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	static gboolean marked[MS_PAGE_SIZE];
	gint index = 0;
	extern GdkColor red;
	static GdkColor old_bg;
	static GdkColor text_color;
	static GtkStyle *style;

	if (event->button != 1) // Left button click 
		return FALSE;
	if (!grab_allowed)
		return FALSE;

	index = (gint)g_object_get_data(G_OBJECT(widget),"offset");
	if (index >= MS_PAGE_SIZE)
		index -= (MS_PAGE_SIZE/2);

	if (marked[index])
	{
		marked[index] = FALSE;
		gtk_widget_modify_bg(widget,GTK_STATE_NORMAL,&old_bg);
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&text_color);
	}
	else
	{
		marked[index] = TRUE;
		style = gtk_widget_get_style(widget);
		old_bg = style->bg[GTK_STATE_NORMAL];
		text_color = style->text[GTK_STATE_NORMAL];
		gtk_widget_modify_bg(widget,GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	}

	return FALSE;	// Allow other handles to run... 

}

void page_changed(GtkNotebook *notebook, GtkNotebookPage *page, guint page_no, gpointer data)
{
	//	printf("page changed to %i\n",page_no);
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
