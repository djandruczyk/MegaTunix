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
#include <enums.h>
#include <fileio.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <logviewer_gui.h>
#include <ms_structures.h>
#include <notifications.h>
#include <req_fuel.h>
#include <runtime_gui.h>
#include <serialio.h>
#include <stdio.h>
#include <structures.h>
#include <threads.h>
#include <vex_support.h>

extern gboolean interrogated;
extern gboolean dualtable;
extern gboolean iac_variant;
extern gboolean connected;
extern gboolean force_status_update;
extern gboolean raw_reader_running;
extern gchar *delim;
extern gint statuscounts_id;
extern gint max_logables;
extern gint ready;
extern gint req_fuel_popup;
extern gint logging_mode;
extern gint read_wait_time;
extern gfloat ego_pbar_divisor;
extern gfloat map_pbar_divisor;
extern GtkTooltips *tip;
extern GtkWidget *logables_table;
extern GtkWidget *tab_delim_button;
extern GtkWidget *delim_button;
extern GtkWidget *delim_table;
extern GtkWidget *custom_logables;
extern unsigned char *kpa_conversion;
extern unsigned char na_map[];
extern unsigned char turbo_map[];
extern struct Ve_Widgets *ve_widgets;
extern struct DynamicSpinners spinners;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern struct DynamicMisc misc;
extern struct Logables logables;
extern struct Serial_Params *serial_params;

static gint update_rate = 24;
static gint runtime_id = -1;
static gint logviewer_id = -1;
gboolean using_pwm_idle;
gboolean tips_in_use;
gboolean forced_update;
gboolean fahrenheit;
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
	0,0,0,0 
};

const gint mt_full[] = 
{
	1,1,1,1,0,
	0,0,0,0,0,
	0,0,0,0,0,
	1,1,0,1,1,
	1,0,1,1,0,
	1,1,1,1,1,
	1,0,1,1,0,
	0,0,0,0 
}; 

static gboolean paused_handlers = FALSE;
static gboolean constants_loaded = FALSE;
static gint num_squirts_1 = 1;
static gint num_squirts_2 = 1;
gint num_cylinders_1 = 1;
gint num_cylinders_2 = 1;
static gint num_injectors_1 = 1;
static gint num_injectors_2 = 1;
static gfloat req_fuel_total_1 = 0.0;
static gfloat req_fuel_total_2 = 0.0;
static gboolean err_flag = FALSE;
static GList *offsets_1 = NULL;
static gint offset_data_1[5]; /* Only 4 interdependant vars... */
static GList *offsets_2 = NULL;
static gint offset_data_2[5]; /* Only 4 interdependant vars... */

void leave(GtkWidget *widget, gpointer data)
{
	if (statuscounts_id)
		gtk_timeout_remove(statuscounts_id);
	statuscounts_id = 0;

	struct Io_File * iofile = NULL;
	iofile = (struct Io_File *) g_object_get_data(
			G_OBJECT(buttons.close_dlog_but),"data");

	stop_datalogging();
	save_config();
	stop_serial_thread();
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
//	printf("comm_port_change, port %s\n",port);
	if(serial_params->open)
	{
		if (raw_reader_running)
			stop_serial_thread();
		close_serial();
	}
	result = g_file_test(port,G_FILE_TEST_EXISTS);
	if (result)
	{
//		printf("port exists...\n");
		open_serial(port);
		if (serial_params->port_name != NULL)
			g_free(serial_params->port_name);
		serial_params->port_name = g_strdup(port);
		setup_serial_params();
	}
/*	else
	{
		printf("file not found...\n");	
	}
*/

	g_free(port);
	return TRUE;
}

gint toggle_button_handler(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		switch ((ToggleButton)data)
		{
			case TOOLTIPS_STATE:
				gtk_tooltips_enable(tip);
				tips_in_use = TRUE;
				break;
			case FAHRENHEIT:
				fahrenheit = TRUE;
				reset_temps(GINT_TO_POINTER(FAHRENHEIT));
				forced_update = TRUE;
				break;
			case CELSIUS:
				fahrenheit = FALSE;
				reset_temps(GINT_TO_POINTER(CELSIUS));
				forced_update = TRUE;
				break;
			case COMMA:
				delim = g_strdup(",");
				break;
			case TAB:
				delim = g_strdup("\t");
				break;
			case SPACE:
				delim = g_strdup(" ");
				break;
			case REALTIME_VIEW:
				gtk_widget_set_sensitive(buttons.logplay_sel_log_but, FALSE);
				gtk_widget_set_sensitive(buttons.logplay_sel_parm_but, TRUE);
				gtk_widget_set_sensitive(buttons.logplay_start_rt_but, TRUE);
				gtk_widget_set_sensitive(buttons.logplay_stop_rt_but, TRUE);
				break;
			case PLAYBACK_VIEW:
				gtk_widget_set_sensitive(buttons.logplay_sel_parm_but, FALSE);
				gtk_widget_set_sensitive(buttons.logplay_sel_log_but, TRUE);
				gtk_widget_set_sensitive(buttons.logplay_start_rt_but, FALSE);
				gtk_widget_set_sensitive(buttons.logplay_stop_rt_but, FALSE);
				break;
		}
	}
	else
	{	/* not pressed */
		switch ((gint)data)
		{
			case TOOLTIPS_STATE:
				gtk_tooltips_disable(tip);
				tips_in_use = FALSE;
				break;
		}
	}
	return TRUE;
}

gint set_logging_mode(GtkWidget * widget, gpointer *data)
{
	gint i = 0;
	if (!ready)
		return FALSE;
	if (GTK_TOGGLE_BUTTON(widget)->active) /* its pressed */
	{
		switch((LoggingMode)data)
		{
			case MT_CLASSIC_LOG:
				logging_mode = MT_CLASSIC_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,FALSE);
				gtk_toggle_button_set_active(
						GTK_TOGGLE_BUTTON
						(tab_delim_button),
						TRUE);
				gtk_widget_set_sensitive(
						delim_table,FALSE);

				for (i=0;i<max_logables;i++)
				{
					if (mt_classic[i] == 1)
					{
						gtk_toggle_button_set_active(
							GTK_TOGGLE_BUTTON
							(logables.widgets[i]),
							TRUE);
					}
				}
				break;
			case MT_FULL_LOG:
				logging_mode = MT_FULL_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,FALSE);
				gtk_toggle_button_set_active(
						GTK_TOGGLE_BUTTON
						(tab_delim_button),
						TRUE);
				gtk_widget_set_sensitive(
						delim_table,FALSE);

				for (i=0;i<max_logables;i++)
				{
					if (mt_full[i] == 1)
					{
					gtk_toggle_button_set_active(
							GTK_TOGGLE_BUTTON
							(logables.widgets[i]),
							TRUE);
					}
				}
				break;
			case CUSTOM_LOG:
				logging_mode = CUSTOM_LOG;
				clear_logables();
				gtk_widget_set_sensitive(
						logables_table,TRUE);
				gtk_widget_set_sensitive(
						delim_table,TRUE);
				break;
		}
	}
	return TRUE;
}

gint bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	gint config_num = 0;
	gint bit_pos = 0;
	gint bit_val = 0;
	gint bitmask = 0;
	gint dload_val = 0;
	unsigned char tmp = 0;
	gint offset = 0;
	gint dl_type = 0;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_const = NULL;
	ve_const = (struct Ve_Const_Std *) ms_data;

	if (paused_handlers)
		return TRUE;

	config_num = (gint)g_object_get_data(G_OBJECT(widget),"config_num");
	dl_type = (gint)g_object_get_data(G_OBJECT(widget),"dl_type");
	bit_pos = (gint)g_object_get_data(G_OBJECT(widget),"bit_pos");
	bit_val = (gint)g_object_get_data(G_OBJECT(widget),"bit_val");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");
	

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{
		/* If control reaches here, the toggle button is down */
		switch (config_num)
		{
			case 11:
				tmp = ve_const->config11.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << bit_pos);
				ve_const->config11.value = tmp;
				dload_val = tmp;
				offset = 116;
				check_config11(dload_val);
				break;
			case 12:
				tmp = ve_const->config12.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << bit_pos);
				ve_const->config12.value = tmp;
				dload_val = tmp;
				offset = 117;
				break;
			case 13:
				tmp = ve_const->config13.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << bit_pos);
					ve_const->config13.value = tmp;
				dload_val = tmp;
				offset = 118;
				check_config13(dload_val);
				break;
			case 14:
				/*SPECIAL*/
				offset = 92;
				if (bit_val)
				{
					ve_const->alternate=1;
					dload_val = 1;
				}
				else
				{
					ve_const->alternate=0;
					dload_val = 0;
				}
				if (g_list_find(offsets_1,
							GINT_TO_POINTER(offset))==NULL)
				{
					offsets_1 = g_list_append(offsets_1,
							GINT_TO_POINTER(offset));
					offset_data_1[g_list_index(offsets_1,
							GINT_TO_POINTER(offset))] 
						= dload_val;	
				}
				else
				{
					offset_data_1[g_list_index(offsets_1,
							GINT_TO_POINTER(offset))] 
						= dload_val;	
				}
				if (!err_flag)
					check_req_fuel_limits();
				break;
			default:
				printf(" Toggle button NOT handled ERROR!!, contact author\n");
				return FALSE;
				break;

		}
		if (dl_type == IMMEDIATE)
			write_ve_const(dload_val, offset);
	}
	return TRUE;
}

gint std_button_handler(GtkWidget *widget, gpointer data)
{
	/* get any datastructures attached to the widget */
	void *w_data = NULL;
	if (GTK_IS_OBJECT(widget))
		w_data = (void *)g_object_get_data(G_OBJECT(widget),"data");
	switch ((StdButton)data)
	{
		case START_REALTIME:
			if (!connected)
				check_ecu_comms(NULL,NULL);
			if (!interrogated)
				interrogate_ecu();
			if (!constants_loaded)
			{	/*Read constants first at least once */
				paused_handlers = TRUE;
				read_ve_const();
				update_ve_const();
				set_store_buttons_state(BLACK);
				paused_handlers = FALSE;
				constants_loaded = TRUE;
			}
			start_runtime_display();
			start_serial_thread();
			break;
		case STOP_REALTIME:
			stop_serial_thread();
			reset_runtime_status();
			force_status_update = TRUE;
			stop_runtime_display();
			stop_datalogging();
			break;
		case READ_VE_CONST:
			if (!interrogated)
				interrogate_ecu();
			if (!connected)
				no_ms_connection();
			else
			{
				paused_handlers = TRUE;
				read_ve_const();
				update_ve_const();
				set_store_buttons_state(BLACK);
				paused_handlers = FALSE;
				constants_loaded = TRUE;
			}
			break;
		case BURN_MS_FLASH:
			burn_flash();
			break;
		case SELECT_DLOG_EXP:
			present_filesavebox(DATALOG_EXPORT);
			break;
		case SELECT_DLOG_IMP:
			present_filesavebox(DATALOG_IMPORT);
			break;
		case CLOSE_LOGFILE:
			stop_datalogging();
			close_file(w_data);
			break;
		case START_DATALOGGING:
			start_datalogging();
			break;
		case STOP_DATALOGGING:
			stop_datalogging();
			break;
		case EXPORT_VETABLE:
			present_filesavebox(VE_EXPORT);
			break;
		case IMPORT_VETABLE:
			present_filesavebox(VE_IMPORT);
			break;
		case REVERT_TO_BACKUP:
			revert_to_previous_data();
			break;
		case BACKUP_ALL:
			present_filesavebox(FULL_BACKUP);
			break;
		case RESTORE_ALL:
			present_filesavebox(FULL_RESTORE);
			break;
		case SELECT_PARAMS:
			present_viewer_choices(w_data);
			break;
	}
	return TRUE;
}

gint spinner_changed(GtkWidget *widget, gpointer data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	gfloat value = 0.0;
	gint offset;
	gint tmpi = 0;
	gint tmp = 0;
	gint dload_val = -1;
	gint dl_type = 0;
	gboolean temp_dep = FALSE;
	extern unsigned char * ms_data;
	struct Reqd_Fuel *reqd_fuel;
	struct Ve_Const_Std * ve_const = (struct Ve_Const_Std *) ms_data;
	struct Ve_Const_DT_2 * ve_const_dt2 = 
			(struct Ve_Const_DT_2 *) (ms_data+MS_PAGE_SIZE);
	reqd_fuel = (struct Reqd_Fuel *) g_object_get_data(G_OBJECT(widget),
			"data");

	if ((paused_handlers) || (!ready))
		return TRUE;
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	dl_type = (gint) g_object_get_data(G_OBJECT(widget),"dl_type");
	tmpi = (int)(value+.001);

	switch ((SpinButton)data)
	{
		case SER_POLL_TIMEO:
			serial_params->poll_timeout = (gint)value;
			break;
		case SER_INTERVAL_DELAY:
			serial_params->read_wait = (gint)value;
			break;
		case REQ_FUEL_DISP:
			reqd_fuel->disp = (gint)value;
			req_fuel_change(reqd_fuel);
			break;
		case REQ_FUEL_CYLS:
			reqd_fuel->cyls = (gint)value;
			req_fuel_change(reqd_fuel);
			break;
		case REQ_FUEL_RATED_INJ_FLOW:
			reqd_fuel->rated_inj_flow = (gint)value;
			req_fuel_change(reqd_fuel);
			break;
		case REQ_FUEL_RATED_PRESSURE:
			reqd_fuel->rated_pressure = (gfloat)value;
			req_fuel_change(reqd_fuel);
			break;
		case REQ_FUEL_ACTUAL_PRESSURE:
			reqd_fuel->actual_pressure = (gfloat)value;
			req_fuel_change(reqd_fuel);
			break;
		case REQ_FUEL_AFR:
			reqd_fuel->target_afr = value;
			req_fuel_change(reqd_fuel);
			break;
		case REQ_FUEL_1:
			req_fuel_total_1 = value;
			req_fuel_change(reqd_fuel);
			check_req_fuel_limits();
			break;
		case REQ_FUEL_2:
			req_fuel_total_2 = value;
			req_fuel_change(reqd_fuel);
			check_req_fuel_limits();
			break;
		case NUM_SQUIRTS_1:
			/* This actuall effects another variable */
			num_squirts_1 = tmpi;
			ve_const->divider = 
				(gint)(((float)num_cylinders_1/
					(float)num_squirts_1)+0.001);
			dload_val = ve_const->divider;
			if (g_list_find(offsets_1,GINT_TO_POINTER(offset))==NULL)
			{
				offsets_1 = g_list_append(offsets_1,
						GINT_TO_POINTER(offset));
				offset_data_1[g_list_index(offsets_1,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data_1[g_list_index(offsets_1,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			if (num_cylinders_1 % num_squirts_1)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,1);
			}
			else
			{
				err_flag = FALSE;
				set_reqfuel_state(BLACK,1);
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS_1:
			/* Updates a shared bitfield */
			num_cylinders_1 = tmpi;
			tmp = ve_const->config11.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_const->config11.value = tmp;
			ve_const->divider = 
				(gint)(((float)num_cylinders_1/
					(float)num_squirts_1)+0.001);
			dload_val = tmp;
			if (g_list_find(offsets_1,GINT_TO_POINTER(offset))==NULL)
			{
				offsets_1 = g_list_append(offsets_1,
						GINT_TO_POINTER(offset));
				offset_data_1[g_list_index(offsets_1,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data_1[g_list_index(offsets_1,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			req_fuel_change(reqd_fuel);
			if (num_cylinders_1 % num_squirts_1)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,1);	
			}
			else
			{
				err_flag = FALSE;
				set_reqfuel_state(BLACK,1);	
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS_1:
			/* Updates a shared bitfield */
			num_injectors_1 = tmpi;
			tmp = ve_const->config12.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_const->config12.value = tmp;
			dload_val = tmp;
			if (g_list_find(offsets_1,GINT_TO_POINTER(offset))==NULL)
			{
				offsets_1 = g_list_append(offsets_1,
						GINT_TO_POINTER(offset));
				offset_data_1[g_list_index(offsets_1,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data_1[g_list_index(offsets_1,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			check_req_fuel_limits();
			break;
		case NUM_SQUIRTS_2:
			/* This actuall effects another variable */
			num_squirts_2 = tmpi;
			ve_const_dt2->divider = 
				(gint)(((float)num_cylinders_2/
					(float)num_squirts_2)+0.001);
			dload_val = ve_const_dt2->divider;
			if (g_list_find(offsets_2,GINT_TO_POINTER(offset))==NULL)
			{
				offsets_2 = g_list_append(offsets_2,
						GINT_TO_POINTER(offset));
				offset_data_2[g_list_index(offsets_2,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data_2[g_list_index(offsets_2,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			if (num_cylinders_2 % num_squirts_2)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,2);
			}
			else
			{
				err_flag = FALSE;
				set_reqfuel_state(BLACK,2);
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS_2:
			/* Updates a shared bitfield */
			num_cylinders_2 = tmpi;
			tmp = ve_const_dt2->config11.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_const_dt2->config11.value = tmp;
			ve_const_dt2->divider = 
				(gint)(((float)num_cylinders_2/
					(float)num_squirts_2)+0.001);
			dload_val = tmp;
			printf("offset %i\n",offset);
			if (g_list_find(offsets_2,GINT_TO_POINTER(offset))==NULL)
			{
				offsets_2 = g_list_append(offsets_2,
						GINT_TO_POINTER(offset));
				offset_data_2[g_list_index(offsets_2,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data_2[g_list_index(offsets_2,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			req_fuel_change(reqd_fuel);
			if (num_cylinders_2 % num_squirts_2)
			{
				err_flag = TRUE;
				set_reqfuel_state(RED,2);	
			}
			else
			{
				err_flag = FALSE;
				set_reqfuel_state(BLACK,2);	
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS_2:
			/* Updates a shared bitfield */
			num_injectors_2 = tmpi;
			tmp = ve_const_dt2->config12.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_const_dt2->config12.value = tmp;
			dload_val = tmp;
			if (g_list_find(offsets_2,GINT_TO_POINTER(offset))==NULL)
			{
				offsets_2 = g_list_append(offsets_2,
						GINT_TO_POINTER(offset));
				offset_data_2[g_list_index(offsets_2,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data_2[g_list_index(offsets_2,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			check_req_fuel_limits();
			break;
		case GENERIC:	/* Handles almost ALL other variables */
			temp_dep = (gboolean)g_object_get_data(
					G_OBJECT(ve_widgets->widget[offset]),
						"temp_dep");

			if (temp_dep)
			{
				if (!fahrenheit) /* using celsius, convert it */
					value = (value*(9.0/5.0))+32;
			}
			dload_val = convert_before_download(offset,value);
					
			break;
		default:
			/* Prevents MS corruption for a SW bug */
			printf("ERROR spinbutton not handled\b\n");
			dl_type = 0;  
			break;
	}
	if (dl_type == IMMEDIATE) 
		write_ve_const(dload_val, offset);
	return TRUE;

}

void update_ve_const()
{
	gint i = 0;
	gint dl_type = 0;
	gfloat tmp = 0.0;
	gfloat value = 0.0;
	gboolean temp_dep = FALSE;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_const = NULL;
	struct Ve_Const_DT_1 *ve_const_dt1 = NULL;
	struct Ve_Const_DT_2 *ve_const_dt2 = NULL;

	/* Point to Table0 (stock MS ) data... */
	ve_const = (struct Ve_Const_Std *) ms_data;
	if (dualtable)
	{
		ve_const_dt2 = (struct Ve_Const_DT_2 *) (ms_data+MS_PAGE_SIZE);
		ve_const_dt1 = (struct Ve_Const_DT_1 *) ms_data;
		check_tblcnf(ve_const_dt1->tblcnf.value);
	}

	check_config11(ve_const->config11.value);
	check_config13(ve_const->config13.value);
	

	/* This formula is ONLY VALID for NON-Dualtable variants..
	 * Dualtable no longer has an "Alternate" variable thus all this
	 * shit breaks...  New version is in progress and is buggy 
	 * 02-16-2004
	 */
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
	if (dualtable)
	{
		/*printf("raw_req_fuel from ecu %i, inj %i, cyls %i, div %i\n",
				ve_const_dt1->req_fuel,
				ve_const_dt1->config12.bit.injectors+1,
				ve_const_dt1->config11.bit.cylinders+1,
				ve_const_dt1->divider);
		*/
		/* Table 1 */
		tmp =	(float)(ve_const_dt1->config12.bit.injectors+1) /
			(float)(ve_const_dt1->divider);
		tmp *= (float)ve_const_dt1->req_fuel;
		tmp /= 10.0;
		req_fuel_total_1 = tmp;

		/* Req Fuel total per cycle */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(spinners.req_fuel_total_1_spin),
				tmp);

		/* Req Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(spinners.req_fuel_per_squirt_1_spin),
				ve_const_dt1->req_fuel/10.0);

		/* Config11 bits */
		/* Cylinders */
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.cylinders_1_spin),
				ve_const_dt1->config11.bit.cylinders+1);
		num_cylinders_1 = ve_const_dt1->config11.bit.cylinders+1;

		/* Injections per cycle */
		tmp =	(float)(ve_const_dt1->config11.bit.cylinders+1) /
			(float)(ve_const_dt1->divider);
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.inj_per_cycle_1_spin),
				tmp);
		num_squirts_1 = (gint)tmp;
		if (num_squirts_1 <1 )
			num_squirts_1 = 1;

		/* Config12 bits */
		/* Number of injectors */
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.injectors_1_spin),
				ve_const_dt1->config12.bit.injectors+1);
		num_injectors_1 = ve_const_dt1->config12.bit.injectors+1;


		/* Table 2 */
		tmp =	(float)(ve_const_dt2->config12.bit.injectors+1) /
			(float)(ve_const_dt2->divider);
		tmp *= (float)ve_const_dt2->req_fuel;
		tmp /= 10.0;
		req_fuel_total_2 = tmp;

		/* Total Req Fuel per CYCLE */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(spinners.req_fuel_total_2_spin),
				tmp);

		/* Req Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(spinners.req_fuel_per_squirt_2_spin),
				ve_const_dt2->req_fuel/10.0);

		/* Config11 bits */
		/* Cylinders */
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.cylinders_2_spin),
				ve_const_dt2->config11.bit.cylinders+1);
		num_cylinders_2 = ve_const_dt2->config11.bit.cylinders+1;

		/* Injections per cycle */
		tmp =	(float)(ve_const_dt2->config11.bit.cylinders+1) /
			(float)(ve_const_dt2->divider);
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.inj_per_cycle_2_spin),
				tmp);
		num_squirts_2 = (gint)tmp;
		if (num_squirts_2 <1 )
			num_squirts_2 = 1;

		/* Config12 bits */
		/* Number of injectors */
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.injectors_2_spin),
				ve_const_dt2->config12.bit.injectors+1);
		num_injectors_2 = ve_const_dt2->config12.bit.injectors+1;

	}
	else
	{
		/*  B&G, MSnS, MSnEDIS req Fuel calc *
		 * req-fuel 
		 *                                        /     num_injectors     \
		 *         	   req_fuel_per_squirt * (-------------------------)
		 *                                        \ divider*(alternate+1) /
		 * req_fuel_total = --------------------------------------------------
		 *				10
		 *
		 * where divider = num_cylinders/num_squirts;
		 *
		 * The req_fuel_per_squirt is the part stored in the MegaSquirt ECU as 
		 * the req_fuel variable.  Take note when doing conversions.  On screen
		 * the value is divided by ten from what is in the MS.  
		 * 
		 */
		tmp =	(float)(ve_const->config12.bit.injectors+1) /
			(float)(ve_const->divider*(ve_const->alternate+1));
		tmp *= (float)ve_const->req_fuel;
		tmp /= 10.0;
		req_fuel_total_1 = tmp;

		/* printf("raw_req_fuel from ecu %i, inj %i, cyls %i, div %i, alt %i\n",
				ve_const->req_fuel,
				ve_const->config12.bit.injectors+1,
				ve_const->config11.bit.cylinders+1,
				ve_const->divider, 
				ve_const->alternate);
		*/
		/* Total Req Fuel per CYCLE */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(spinners.req_fuel_total_1_spin),
				tmp);

		/* Req Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON
				(spinners.req_fuel_per_squirt_1_spin),
				ve_const->req_fuel/10.0);

		/* Config11 bits */
		/* Cylinders */
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.cylinders_1_spin),
				ve_const->config11.bit.cylinders+1);
		num_cylinders_1 = ve_const->config11.bit.cylinders+1;

		/* Injections per cycle */
		tmp =	(float)(ve_const->config11.bit.cylinders+1) /
			(float)(ve_const->divider);
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.inj_per_cycle_1_spin),
				tmp);
		num_squirts_1 = (gint)tmp;

		/* Config12 bits */
		/* Number of injectors */
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(spinners.injectors_1_spin),
				ve_const->config12.bit.injectors+1);
		num_injectors_1 = ve_const->config12.bit.injectors+1;

	}	// End of B&G specific code...

	/* Speed Density or Alpha-N */
	if (ve_const->config13.bit.inj_strat)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.alpha_n_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.speed_den_but),
				TRUE);
	/* Even Fire of Odd Fire */
	if (ve_const->config13.bit.firing)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.odd_fire_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.even_fire_but),
				TRUE);
	/* NarrowBand O2 or WideBand O2 */
	if (ve_const->config13.bit.ego_type)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.wbo2_but),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.nbo2_but),TRUE);
	}
	/* Baro Correction, enabled or disabled */
	if (ve_const->config13.bit.baro_corr)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.baro_ena_but),TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.baro_disa_but),
				TRUE);
	/* CONFIG11 related buttons */
	/* Map sensor 115kPA or 250 kPA */
	if (ve_const->config11.bit.map_type)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.map_250_but),TRUE);
		kpa_conversion = turbo_map;
		map_pbar_divisor = 255.0;
	}
	else
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.map_115_but),TRUE);
		kpa_conversion = na_map;
		map_pbar_divisor = 115.0;
	}
	/* 2 stroke or 4 Stroke */
	if (ve_const->config11.bit.eng_type)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.two_stroke_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.four_stroke_but),
				TRUE);
	/* Multi-Port or TBI */
	if (ve_const->config11.bit.inj_type)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.tbi_but),TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.multi_port_but),
				TRUE);

	/* B&G idle or PWM */
	if ((dualtable) || (iac_variant))
	{
		if (ve_const->config13.bit.idle_policy)
			gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(buttons.pwm_idle_but),TRUE);
		else
			gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(buttons.onoff_idle_but),
					TRUE);
	}


	if (!dualtable)
	{
		if (ve_const->alternate > 0)
			gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(buttons.alternate_but),
					TRUE);
		else
			gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(buttons.simul_but),
					TRUE);
	}


	/* Update all on screen controls (except bitfields (done above)*/
	for (i=0;i<2*MS_PAGE_SIZE;i++)
	{
		temp_dep = FALSE;
		dl_type = -1;
		if (GTK_IS_OBJECT(ve_widgets->widget[i]))
		{
			dl_type = (gint)g_object_get_data(
					G_OBJECT(ve_widgets->widget[i]),
					"dl_type");
			temp_dep = (gboolean)g_object_get_data(
					G_OBJECT(ve_widgets->widget[i]),
					"temp_dep");
			if (dl_type == IMMEDIATE)
				value = convert_after_upload(i);  
			if (temp_dep)
			{
				if (!fahrenheit)
					value = (value-32)*(5.0/9.0);
			}

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(
						ve_widgets->widget[i]),value);
		}
	}
}

void check_req_fuel_limits()
{
	gfloat tmp = 0.0;
	gfloat req_fuel_per_squirt = 0.0;
	gint lim_flag = 0;
	gint index = 0;
	gint dload_val = 0;
	gint offset = 0;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_const = NULL;
	struct Ve_Const_DT_1 *ve_const_dt1 = NULL;
	struct Ve_Const_DT_2 *ve_const_dt2 = NULL;


	if (dualtable)
	{
		ve_const_dt1 = (struct Ve_Const_DT_1 *)ms_data;
		ve_const_dt2 = (struct Ve_Const_DT_2 *) (ms_data+MS_PAGE_SIZE);
		/* F&H Dualtable required Fuel calc
		 *
		 *                                        / num_injectors \
		 *         	   req_fuel_per_squirt * (-----------------)
		 *                                        \    divider    /
		 * req_fuel_total = -------------------------------------------
		 *				10
		 *
		 * where divider = num_cylinders/num_squirts;
		 *
		 * rearranging to solve for req_fuel_per_squirt...
		 *
		 *                        (req_fuel_total * 10)
		 * req_fuel_per_squirt =  ---------------------
		 *			    / num_injectors \
		 *                         (-----------------)
		 *                          \    divider    /
		 */

		/* TABLE 1 */
		tmp = (float)(num_injectors_1)/(float)(ve_const_dt1->divider);
		req_fuel_per_squirt = ((float)req_fuel_total_1 * 10.0)/tmp;

		if (req_fuel_per_squirt != ve_const_dt1->req_fuel)
		{
			if (req_fuel_per_squirt > 255)
				lim_flag = 1;
			if (req_fuel_per_squirt < 0)
				lim_flag = 1;
		}
		/* Required Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(
					spinners.req_fuel_per_squirt_1_spin),
				req_fuel_per_squirt/10.0);

		/* Throw warning if an issue */
		if (lim_flag)
			set_interdep_state(RED,1);
		else
		{
			set_interdep_state(BLACK,1);

			if (paused_handlers)
				return;
			offset = 90;
			dload_val = convert_before_download(offset,(gint)req_fuel_per_squirt);
			write_ve_const(dload_val, offset);
			for (index=0;index<g_list_length(offsets_1);index++)
			{
				gint offset;
				gint data;
				offset = GPOINTER_TO_INT(g_list_nth_data(offsets_1,index));
				data = offset_data_1[g_list_index(offsets_1,
						g_list_nth_data(offsets_1,index))];
				write_ve_const(data, offset);
			}
			g_list_free(offsets_1);
			offsets_1 = NULL;
			for (index=0;index<5;index++)
				offset_data_1[index]=0;
		}

		lim_flag = 0;
		/* TABLE 2 */
		tmp = (float)(num_injectors_2)/(float)(ve_const_dt2->divider);
		req_fuel_per_squirt = ((float)req_fuel_total_2 * 10.0)/tmp;

		if (req_fuel_per_squirt != ve_const_dt2->req_fuel)
		{
			if (req_fuel_per_squirt > 255)
				lim_flag = 1;
			if (req_fuel_per_squirt < 0)
				lim_flag = 1;
		}

		/* Required Fuel per SQUIRT */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(
					spinners.req_fuel_per_squirt_2_spin),
				req_fuel_per_squirt/10);

		/* Throw warning if an issue */
		if (lim_flag)
			set_interdep_state(RED,2);
		else
		{
			set_interdep_state(BLACK,2);

			if (paused_handlers)
				return;
			offset = 90 + MS_PAGE_SIZE;
			dload_val = convert_before_download(offset,(gint)req_fuel_per_squirt);
			write_ve_const(dload_val, offset);
			for (index=0;index<g_list_length(offsets_2);index++)
			{
				gint offset;
				gint data;
				offset = GPOINTER_TO_INT(g_list_nth_data(offsets_2,index));
				data = offset_data_2[g_list_index(offsets_2,
						g_list_nth_data(offsets_2,index))];
				write_ve_const(data, offset);
			}
			g_list_free(offsets_2);
			offsets_2 = NULL;
			for (index=0;index<5;index++)
				offset_data_2[index]=0;
		}
	}// END Dualtable Req fuel checks... */
	else
	{
		ve_const = (struct Ve_Const_Std *)ms_data;

		/* B&G, MSnS, MSnEDIS Required Fuel Calc
		 *
		 *                                        /     num_injectors_1     \
		 *         	   req_fuel_per_squirt * (-------------------------)
		 *                                        \ divider*(alternate+1) /
		 * req_fuel_total = --------------------------------------------------
		 *				10
		 *
		 * where divider = num_cylinders_1/num_squirts_1;
		 *
		 * rearranging to solve for req_fuel_per_squirt...
		 *
		 *                        (req_fuel_total * 10)
		 * req_fuel_per_squirt =  ----------------------
		 *			    /  num_injectors  \
		 *                         (-------------------)
		 *                          \ divider*(alt+1) /
		 *
		 * 
		 */

		tmp =	((float)(num_injectors_1))/((float)ve_const->divider*(float)(ve_const->alternate+1));

		/* This is 1 tenth the value as the one screen stuff is 1/10th 
		 * for the ms variable,  it gets converted farther down, just 
		 * before download to the MS
		 */
		req_fuel_per_squirt = ((float)req_fuel_total_1*10.0)/tmp;

		if (req_fuel_per_squirt != ve_const->req_fuel)
		{
			if (req_fuel_per_squirt > 255)
				lim_flag = 1;
			if (req_fuel_per_squirt < 0)
				lim_flag = 1;
		}
		/* req-fuel info box  */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(
					spinners.req_fuel_per_squirt_1_spin),
				req_fuel_per_squirt/10.0);

		if (lim_flag)
			set_interdep_state(RED,1);
		else
		{
			set_interdep_state(BLACK,1);

			/* All Tested succeeded, download Required fuel, 
			 * then iterate through the list of offsets of changed
			 * inter-dependant variables, extract the data out of 
			 * the companion array, and send to ECU.  Then free
			 * the offset GList, and clear the array...
			 */

			/* Handlers get paused during a read of MS VE/Constants. We
			 * don't need to write anything back during this window. 
			 */
			if (paused_handlers)
				return;
			offset = 90;
			dload_val = convert_before_download(offset,(gint)req_fuel_per_squirt);
			write_ve_const(dload_val, offset);
			for (index=0;index<g_list_length(offsets_1);index++)
			{
				gint offset;
				gint data;
				offset = GPOINTER_TO_INT(g_list_nth_data(offsets_1,index));
				data = offset_data_1[g_list_index(offsets_1,
						g_list_nth_data(offsets_1,index))];
				write_ve_const(data, offset);
			}
			g_list_free(offsets_1);
			offsets_1 = NULL;
			for (index=0;index<5;index++)
				offset_data_1[index]=0;
		}
	} // End B&G style Req Fuel check 
	return ;

}

void check_config11(unsigned char tmp)
{
	/* checks some of the bits in the config11 variable and 
	 * adjusts some important things as necessary....
	 */
	if ((tmp &0x3) == 0)	
	{
		kpa_conversion = na_map;
		map_pbar_divisor = 115.0;
		//	printf("using 115KPA map sensor\n");
	}
	if ((tmp &0x3) == 1)	
	{
		kpa_conversion = turbo_map;
		map_pbar_divisor = 255.0;
		//	printf("using 250KPA map sensor\n");
	}
}

void check_config13(unsigned char tmp)
{
	GtkWidget *label;
	/* checks bits of the confgi13 bitfield and forces
	 * gui to update/adapt as necessary...
	 */


	/* check O2 sensor bit and adjust factor
	   so runtime display has a sane scale... */
	if (((tmp >> 1)&0x1) == 1)
	{
		ego_pbar_divisor = 5.0;
		force_status_update = TRUE;
	}
	else
	{
		ego_pbar_divisor = 1.2;
		force_status_update = TRUE;
	}

	/* Check SD/Alpha-N button and adjust VEtable labels
	 * to suit
	 */
	if (((tmp >> 2)&0x1) == 1)
	{
		label = gtk_frame_get_label_widget(
				GTK_FRAME
				(misc.p0_map_tps_frame));
		gtk_label_set_text(GTK_LABEL
				(label),"TPS Bins");
		gtk_label_set_text(GTK_LABEL
				(labels.p0_map_tps_lab),
				"TPS %");
	}
	else
	{
		label = gtk_frame_get_label_widget(
				GTK_FRAME
				(misc.p0_map_tps_frame));
		gtk_label_set_text(GTK_LABEL
				(label),"MAP Bins");
		gtk_label_set_text(GTK_LABEL
				(labels.p0_map_tps_lab),
				"Kpa");
	}

	/* Check Idle method */
	if (((tmp >> 4)&0x1) == 1)
	{
		// Brian Fielding PWM method, enable controls
		using_pwm_idle = TRUE;
		set_enhanced_idle_state(TRUE);
		reset_temps(GINT_TO_POINTER(fahrenheit));
	}
	else
	{
		using_pwm_idle = FALSE;
		set_enhanced_idle_state(FALSE);
		reset_temps(GINT_TO_POINTER(fahrenheit));
	}
		
}

void check_tblcnf(unsigned char tmp)
{
	unsigned char val = 0;
	if ((tmp &0x1) == 0)
	{	/* B&G Simul style both channels driven from table 1*/
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj1_table1),
				TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj2_table1),
				TRUE);
		/* all other bits don't matter, quit now... */
		return;
	}
	val = (tmp >> 1)&0x3;  //(interested in bits 1-2) 
	switch (val)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_not_driven),
					TRUE);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_table1),
					TRUE);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj1_table2),
					TRUE);
			break;
		default:
			fprintf(stderr,__FILE__":bits 1-2 in tblcnf invalid\n");
	}
	val = (tmp >> 3)&0x3;  //(interested in bits 3-4) 
	switch (val)
	{
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_not_driven),
					TRUE);
			break;
		case 1:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_table1),
					TRUE);
			break;
		case 2:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
					(buttons.inj2_table2),
					TRUE);
			break;
		default:
			fprintf(stderr,__FILE__":bits 1-2 in tblcnf invalid\n");
	}
	/* Gammae for injection channel 1 */
	if (((tmp >> 5)&0x1) == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj1_gammae_dis),
				TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj1_gammae_ena),
				TRUE);
		
	/* Gammae for injection channel 2 */
	if (((tmp >> 6)&0x1) == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj2_gammae_dis),
				TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				(buttons.inj2_gammae_ena),
				TRUE);
	return;	
}

void set_enhanced_idle_state(gboolean state)
{
	extern GList *enh_idle_widgets;
        g_list_foreach(enh_idle_widgets, set_widget_state,(gpointer)state);
}

void set_dualtable_mode(gboolean state)
{
	extern GList *dt_widgets;
	extern GList *inv_dt_widgets;
	dualtable = state;
	GtkWidget *label;

	/* get label widget for constants reqd_fuel table 1 */
	label = gtk_frame_get_label_widget(
			GTK_FRAME
			(misc.tbl1_reqd_fuel_frame));
			
	g_list_foreach(inv_dt_widgets, set_widget_state,(gpointer)(!state));
	g_list_foreach(dt_widgets, set_widget_state,(gpointer)state);

	/* fahrenheit is a FLAG... */
	reset_temps(GINT_TO_POINTER(fahrenheit));
	if (state)
	{
		gtk_label_set_text(GTK_LABEL(label),"Required Fuel for Table 1");
		printf("enabling dualtable mode controls\n");
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(label),"Required Fuel");
		printf("disabling dualtable mode controls\n");
	}
}

void set_widget_state(gpointer widget, gpointer state)
{
	gtk_widget_set_sensitive(GTK_WIDGET(widget),(gboolean)state);	
}

void start_runtime_display()
{
	runtime_id = gtk_timeout_add((int)((1.0/(float)update_rate)*1000.0),
			(GtkFunction)update_runtime_vars,NULL);
	logviewer_id = gtk_timeout_add((int)((1.0/(float)update_rate)*1000.0),
			(GtkFunction)update_logview_traces,NULL);
}
void stop_runtime_display()
{
	if (runtime_id)
		gtk_timeout_remove(runtime_id);
	runtime_id = 0;
	if (logviewer_id)
		gtk_timeout_remove(logviewer_id);
	logviewer_id = 0;
}

gboolean populate_gui()
{
	/* A trick used in main() to startup MegaTunix faster..
	 * the problem is that calling the READ_VE_CONST stuff before 
	 * gtk_main is that it makes the gui have to go through interrogation
	 * of the ecu before the gui appears, giving the appearance that
	 * MegaTunix is slow.  By creating this simple wrapper and kicking
	 * it off as a timeout, it'll run just after the gui is ready, and
	 * since it returns FALSE, the timeout will be canceled and deleted
	 * i.e. it acts like a one-shot behind a time delay. (the delay lets
	 * the gui get "ready" and then this kicks off the interrogator and
	 * populates the gui controls if the ECU is detected... 
	 */

	std_button_handler(NULL,GINT_TO_POINTER(READ_VE_CONST));
	return FALSE;
}
