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
#include <globals.h>
#include <gui_handlers.h>
#include <init.h>
#include <interrogate.h>
#include <logviewer_gui.h>
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
extern gboolean connected;
extern gboolean force_status_update;
extern gboolean raw_reader_running;
extern gchar *delim;
extern gint statuscounts_id;
extern gint max_logables;
extern gint ready;
static gint num_squirts = 1;
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
extern struct Reqd_Fuel reqd_fuel;
extern struct DynamicLabels labels;
extern struct DynamicMisc misc;
extern struct Logables logables;
extern struct Serial_Params *serial_params;

static gint update_rate = 33;
static gint runtime_id = -1;
static gint logviewer_id = -1;
gboolean tips_in_use;
gboolean forced_update;
gboolean fahrenheit;
gint num_cylinders = 1;
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
static gint num_injectors = 1;
static gfloat req_fuel_total = 0.0;
static gboolean err_flag = FALSE;
static GList *offsets = NULL;
static gint offset_data[5]; /* Only 4 interdependant vars... */

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
	struct Ve_Const_Std *ve_const = (struct Ve_Const_Std *) ms_data;

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
				if (g_list_find(offsets,
							GINT_TO_POINTER(offset))==NULL)
				{
					offsets = g_list_append(offsets,
							GINT_TO_POINTER(offset));
					offset_data[g_list_index(offsets,
							GINT_TO_POINTER(offset))] 
						= dload_val;	
				}
				else
				{
					offset_data[g_list_index(offsets,
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
		case REQD_FUEL_POPUP:
			if (!req_fuel_popup)
				reqd_fuel_popup();
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
	struct Ve_Const_Std * ve_const = (struct Ve_Const_Std *) ms_data;

	if ((paused_handlers) || (!ready))
		return TRUE;
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	dl_type = (gint) g_object_get_data(G_OBJECT(widget),"dl_type");
	tmpi = (int)(value+.001);

	switch ((SpinButton)data)
	{
		case SET_SER_PORT:
			if(serial_params->open)
			{
				if (raw_reader_running)
					stop_serial_thread();
				close_serial();
			}
			open_serial((int)value);
			setup_serial_params();
			break;
		case SER_POLL_TIMEO:
			serial_params->poll_timeout = (gint)value;
			break;
		case SER_INTERVAL_DELAY:
			serial_params->read_wait = (gint)value;
			break;
		case REQ_FUEL_DISP:
			reqd_fuel.disp = (gint)value;
			break;
		case REQ_FUEL_CYLS:
			reqd_fuel.cyls = (gint)value;
			break;
		case REQ_FUEL_RATED_INJ_FLOW:
			reqd_fuel.rated_inj_flow = (gint)value;
			break;
		case REQ_FUEL_RATED_PRESSURE:
			reqd_fuel.rated_pressure = (gfloat)value;
			break;
		case REQ_FUEL_ACTUAL_PRESSURE:
			reqd_fuel.actual_pressure = (gfloat)value;
			break;
		case REQ_FUEL_AFR:
			reqd_fuel.afr = value;
			break;
		case REQ_FUEL:
			req_fuel_total = value;
			check_req_fuel_limits();
			break;
		case NUM_SQUIRTS:
			/* This actuall effects another variable */
			num_squirts = tmpi;
			ve_const->divider = 
				(gint)(((float)num_cylinders/
					(float)num_squirts)+0.001);
			dload_val = ve_const->divider;
			if (g_list_find(offsets,GINT_TO_POINTER(offset))==NULL)
			{
				offsets = g_list_append(offsets,
						GINT_TO_POINTER(offset));
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			if (num_cylinders % num_squirts)
			{
				err_flag = TRUE;
				squirt_cyl_inj_set_state(RED);
			}
			else
			{
				err_flag = FALSE;
				squirt_cyl_inj_set_state(BLACK);
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS:
			/* Updates a shared bitfield */
			num_cylinders = tmpi;
			tmp = ve_const->config11.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_const->config11.value = tmp;
			ve_const->divider = 
				(gint)(((float)num_cylinders/
					(float)num_squirts)+0.001);
			dload_val = tmp;
			if (g_list_find(offsets,GINT_TO_POINTER(offset))==NULL)
			{
				offsets = g_list_append(offsets,
						GINT_TO_POINTER(offset));
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			if (num_cylinders % num_squirts)
			{
				err_flag = TRUE;
				squirt_cyl_inj_set_state(RED);
			}
			else
			{
				err_flag = FALSE;
				squirt_cyl_inj_set_state(BLACK);
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS:
			/* Updates a shared bitfield */
			num_injectors = tmpi;
			tmp = ve_const->config12.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_const->config12.value = tmp;
			dload_val = tmp;
			if (g_list_find(offsets,GINT_TO_POINTER(offset))==NULL)
			{
				offsets = g_list_append(offsets,
						GINT_TO_POINTER(offset));
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
					= dload_val;	
			}
			else
			{
				offset_data[g_list_index(offsets,
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
	struct Ve_Const_Std *ve_const;

	/* Point to Table0 (stock MS ) data... */
	ve_const = (struct Ve_Const_Std *) ms_data;

	check_config11(ve_const->config11.value);
	check_config13(ve_const->config13.value);

	/* req-fuel 
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
	req_fuel_total = tmp;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinners.req_fuel_total_spin),
			tmp);

	/* req-fuel info box  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinners.req_fuel_per_squirt_spin),
			ve_const->req_fuel/10.0);

	/* CONFIG11-13 related buttons */
	/* Cylinders */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(spinners.cylinders_spin),
			ve_const->config11.bit.cylinders+1);
	num_cylinders = ve_const->config11.bit.cylinders+1;

	/* Number of injectors */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(spinners.injectors_spin),
			ve_const->config12.bit.injectors+1);
	num_injectors = ve_const->config12.bit.injectors+1;

	/* Injections per cycle */
	tmp =	(float)(ve_const->config11.bit.cylinders+1) /
		(float)(ve_const->divider);
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(spinners.inj_per_cycle_spin),
			tmp);
	num_squirts = (gint)tmp;

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

	/* THIS is NOT compatible with DualTable code,  must be changed */
	if (ve_const->alternate > 0)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.alternate_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(buttons.simul_but),
				TRUE);


	/* Table 1 for dualtable, and all std megasquirt units */
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
	gfloat req_fuel_per_squirt_dl = 0.0;
	gint lim_flag = 0;
	gint tuning_zone_flag = 0;  
	gint index = 0;
	gint dload_val = 0;
	gint offset = 0;
	gint page = -1;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_const = NULL;
	/* Set it to point to page 0 (stock MS) */
	ve_const = (struct Ve_Const_Std *)ms_data;

	/* req-fuel 
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

	tmp =	(float)(ve_const->divider*(float)(ve_const->alternate+1))/(float)(num_injectors);

	/* This is 1 tenth the value as the one screen stuff is 1/10th 
	 * for the ms variable,  it gets converted farther down, just 
	 * before download to the MS
	 */
	req_fuel_per_squirt = tmp * req_fuel_total;
	req_fuel_per_squirt_dl = (gint)((req_fuel_per_squirt*10.0)+0.001);

	if (req_fuel_per_squirt_dl != ve_const->req_fuel)
	{
		if (req_fuel_per_squirt_dl > 255)
			lim_flag = 1;
		if (req_fuel_per_squirt_dl < 0)
			lim_flag = 1;
		if ((req_fuel_per_squirt_dl > 80) & (req_fuel_per_squirt_dl < 120))
			tuning_zone_flag = 1;      
	}
	/* req-fuel info box  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(
				spinners.req_fuel_per_squirt_spin),
			req_fuel_per_squirt);
	page = 0;
	if (tuning_zone_flag)
	{ /*
	   * This means each injector squirt is within sane limits, and
	   * should be tunable and operate over a full rev-range
	   */
		//		gtk_widget_modify_text(GTK_WIDGET(								spinners.req_fuel_per_squirt_spin),
		//				GTK_STATE_INSENSITIVE,&green);
	}
	if (lim_flag)
	{	/*
		 * ERROR, something is out of bounds 
		 * change the color of the potential offenders onscreen to
		 * let the user know something is wrong..
		 */
		interdep_state(RED, page);
	}
	else
	{	/*
		 * Everything is OK with all inter-dependant variables.
		 * settings all previous gui enties to normal state...
		 */
		interdep_state(BLACK, page);

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
		dload_val = convert_before_download(offset,req_fuel_per_squirt);
		write_ve_const(dload_val, offset);
		for (index=0;index<g_list_length(offsets);index++)
		{
			gint offset;
			gint data;
			offset = GPOINTER_TO_INT(g_list_nth_data(offsets,index));
			data = offset_data[g_list_index(offsets,
					g_list_nth_data(offsets,index))];
			write_ve_const(data, offset);
		}
		g_list_free(offsets);
		offsets = NULL;
		for (index=0;index<5;index++)
			offset_data[index]=0;
	}
	return ;

}

void check_config11(int tmp)
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

void check_config13(int tmp)
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
}

void set_dualtable_mode(gboolean state)
{
	if (state)
	{
		dualtable = TRUE;
		gtk_widget_set_sensitive(misc.vetable2,TRUE);
		printf("enabling dualtable mode controls\n");
	}
	else
	{
		dualtable = FALSE;
		gtk_widget_set_sensitive(misc.vetable2,FALSE);
		printf("disabling dualtable mode controls\n");
	}
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
	/* A trick used in main() to startup MEgaTunix faster..
	 * the problem is that calling the READ_VE_CONST stuff before 
	 *gtk_main is that it makes the gui have to go through interrogation
	 * of the ecu before the gui appears, giving the appearance that
	 * MegaTunix is slow.  By creating this simple wrapper and kicking
	 * it off as a timeout, it'll run just after the gui is ready, and
	 * since it returns FALSE, the timeout will be canceled and deleted
	 * i.e. it acts like a one-shot behind a time delay. (the delay lets
	 * the gui get "ready" and then this kicks off the interrogator and
	 * populated the gui controls (if hte ECU is detected... 
	 */

	std_button_handler(NULL,GINT_TO_POINTER(READ_VE_CONST));
	return FALSE;
}
