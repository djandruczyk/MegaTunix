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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <protos.h>
#include <defines.h>
#include <globals.h>
#include <constants.h>
#include <datalogging.h>


extern gint req_fuel_popup;
extern unsigned char *kpa_conversion;
extern unsigned char na_map[];
extern unsigned char turbo_map[];
extern GtkTooltips *tip;
gboolean tips_in_use;
gboolean forced_update;
gboolean fahrenheit;
static gboolean paused_handlers = FALSE;
static gboolean constants_loaded = FALSE;
extern gboolean raw_reader_running;
extern gboolean raw_reader_stopped;
extern gint read_wait_time;
extern struct ve_const_std *ve_constants;
extern struct v1_2_Constants constants;
extern struct Reqd_Fuel reqd_fuel;
extern struct Labels labels;
extern struct Logables logables;
extern gboolean connected;
extern gboolean force_status_update;
extern gfloat ego_pbar_divisor;
extern gfloat map_pbar_divisor;
extern GtkWidget *map_tps_frame;
extern GtkWidget *map_tps_label;
extern GtkWidget *custom_logables;
static gint num_squirts = 1;
gint num_cylinders = 1;
static gint num_injectors = 1;
static gfloat req_fuel_total = 0.0;
static gboolean err_flag = FALSE;
GdkColor red = { 0, 65535, 0, 0};
GdkColor black = { 0, 0, 0, 0};
static GList *offsets = NULL;
static gint offset_data[5]; /* Only 4 interdependant vars... */
static gint page_data[5]; /* Only 4 interdependant vars... */
GtkWidget *veconst_widgets_1[VEBLOCK_SIZE];
GtkWidget *veconst_widgets_2[VEBLOCK_SIZE];

void leave(GtkWidget *widget, gpointer data)
{
	save_config();
	stop_serial_thread();
	/* Free all buffers */
	close_serial();
	stop_datalogging();
	close_logfile();
	usleep(100000); /* make sure thread dies cleanly.. */
	mem_dealloc();
	gtk_main_quit();
}

int toggle_button_handler(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{	/* It's pressed (or checked) */
		switch ((gint)data)
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
int bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	gint config_num = 0;
	gint bit_pos = 0;
	gint bit_val = 0;
	gint bitmask = 0;
	gint page = -1;
	gint dload_val = 0;
	unsigned char tmp = 0;
	gint offset = 0;
	gint dl_type = 0;

        if (paused_handlers)
                return TRUE;

	config_num = (gint)g_object_get_data(G_OBJECT(widget),"config_num");
	dl_type = (gint)g_object_get_data(G_OBJECT(widget),"dl_type");
	bit_pos = (gint)g_object_get_data(G_OBJECT(widget),"bit_pos");
	bit_val = (gint)g_object_get_data(G_OBJECT(widget),"bit_val");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");
	page = (gint)g_object_get_data(G_OBJECT(widget),"page");
	if (page == -1)
		printf("Page not defined for bitmask button config_num %i\n",config_num);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{
		/* If control reaches here, the toggle button is down */
		switch (config_num)
		{
			case 11:
				tmp = ve_constants->config11.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << bit_pos);
				ve_constants->config11.value = tmp;
				dload_val = tmp;
				offset = 116;
				check_config11(dload_val);
				break;
			case 12:
				tmp = ve_constants->config12.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << bit_pos);
				ve_constants->config12.value = tmp;
				dload_val = tmp;
				offset = 117;
				break;
			case 13:
				tmp = ve_constants->config13.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << bit_pos);
				ve_constants->config13.value = tmp;
				dload_val = tmp;
				offset = 118;
				check_config13(dload_val);

				break;
			case 14:
				/*SPECIAL*/
				offset = 92;
				if (bit_val)
				{
					ve_constants->alternate=1;
					dload_val = 1;
				}
				else
				{
					ve_constants->alternate=0;
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
					page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
				}
				else
				{
					offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= dload_val;	
					page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
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
			write_ve_const(dload_val, offset, page);
	}
	return TRUE;
}

int std_button_handler(GtkWidget *widget, gpointer data)
{
	switch ((gint)data)
	{
		case START_REALTIME:
			if (!raw_reader_running)
				check_ecu_comms(NULL,NULL);
			if (!connected)
				no_ms_connection();
			else if (!constants_loaded)
			{	/*Read constants first at least once */
				paused_handlers = TRUE;
				read_ve_const();
				update_ve_const();
				set_store_black();
				paused_handlers = FALSE;
				constants_loaded = TRUE;
			}
			start_serial_thread();
			break;
		case STOP_REALTIME:
			stop_serial_thread();
			reset_runtime_status();
			force_status_update = TRUE;
			break;
		case REQD_FUEL_POPUP:
			if (!req_fuel_popup)
				reqd_fuel_popup();
			break;
		case READ_FROM_MS:
			if (!raw_reader_running)
				check_ecu_comms(NULL,NULL);
			if (!connected)
				no_ms_connection();
			else
			{
				paused_handlers = TRUE;
				read_ve_const();
				update_ve_const();
				set_store_black();
				paused_handlers = FALSE;
				constants_loaded = TRUE;
			}
			break;
		case WRITE_TO_MS:
			burn_flash();
			break;
		case SELECT_LOGFILE:
			create_dlog_filesel();
			break;
		case TRUNCATE_LOGFILE:
			truncate_log();
			break;
		case CLOSE_LOGFILE:
			stop_datalogging();
			close_logfile();
			break;
		case START_DATALOGGING:
			start_datalogging();
			break;
		case STOP_DATALOGGING:
			stop_datalogging();
			break;
	}
	return TRUE;
}

int spinner_changed(GtkWidget *widget, gpointer data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	gfloat value = 0.0;
	gint offset;
	gint page = -1;
	gint tmpi = 0;
	gint tmp = 0;
	gint dload_val = -1;
	gint dl_type = 0;
	gboolean temp_dep = FALSE;
	if (paused_handlers)
		return TRUE;
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	page = (gint) g_object_get_data(G_OBJECT(widget),"page");
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	dl_type = (gint) g_object_get_data(G_OBJECT(widget),"dl_type");
	tmpi = (int)(value+.001);
	if (page == -1)
		printf("ERROR, page not defined for variable at offset %i\n",offset);

	switch ((gint)data)
	{
		case SET_SER_PORT:
			if(serial_params.open)
			{
				if (raw_reader_running)
					stop_serial_thread();
				close_serial();
				usleep(100000);
			}
			open_serial((int)value);
			setup_serial_params();
			break;
		case SER_POLL_TIMEO:
			serial_params.poll_timeout = (gint)value;
			break;
		case SER_INTERVAL_DELAY:
			serial_params.read_wait = (gint)value;
			break;
		case REQ_FUEL_DISP:
			reqd_fuel.disp = (gint)value;
			break;
		case REQ_FUEL_CYLS:
			reqd_fuel.cyls = (gint)value;
			break;
		case REQ_FUEL_INJ_RATE:
			reqd_fuel.inj_rate = (gint)value;
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
			ve_constants->divider = 
				(gint)(((float)num_cylinders/
					(float)num_squirts)+0.001);
			dload_val = ve_constants->divider;
			if (g_list_find(offsets,GINT_TO_POINTER(offset))==NULL)
			{
				offsets = g_list_append(offsets,
						GINT_TO_POINTER(offset));
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= dload_val;	
				page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
			}
			else
			{
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= dload_val;	
				page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
			}
			if (num_cylinders % num_squirts)
			{
				err_flag = TRUE;
				squirt_cyl_inj_red();
			}
			else
			{
				err_flag = FALSE;
				squirt_cyl_inj_black();
				check_req_fuel_limits();
			}
			break;
		case NUM_CYLINDERS:
			/* Updates a shared bitfield */
			num_cylinders = tmpi;
			tmp = ve_constants->config11.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_constants->config11.value = tmp;
			ve_constants->divider = 
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
				page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
			}
			else
			{
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= dload_val;	
				page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
			}
			if (num_cylinders % num_squirts)
			{
				err_flag = TRUE;
				squirt_cyl_inj_red();
			}
			else
			{
				err_flag = FALSE;
				squirt_cyl_inj_black();
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS:
			/* Updates a shared bitfield */
			printf("number of injectors changed\n");
			num_injectors = tmpi;
			tmp = ve_constants->config12.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_constants->config12.value = tmp;
			dload_val = tmp;
			if (g_list_find(offsets,GINT_TO_POINTER(offset))==NULL)
			{
				offsets = g_list_append(offsets,
						GINT_TO_POINTER(offset));
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= dload_val;	
				page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
			}
			else
			{
				offset_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= dload_val;	
				page_data[g_list_index(offsets,
						GINT_TO_POINTER(offset))] 
						= page;	
			}
			check_req_fuel_limits();
			break;
		case GENERIC:	/* Handles almost ALL other variables */
			temp_dep = (gboolean)g_object_get_data(
					G_OBJECT(veconst_widgets_1[offset]),
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
		write_ve_const(dload_val, offset, page);
	return TRUE;

}

void update_ve_const()
{
	gint i = 0;
	gint dl_type = 0;
	gfloat tmp = 0.0;
	gfloat value = 0.0;
	gboolean temp_dep = FALSE;

	check_config11(ve_constants->config11.value);
	check_config13(ve_constants->config13.value);

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
	tmp =	(float)(ve_constants->config12.bit.injectors+1) /
		(float)(ve_constants->divider*(ve_constants->alternate+1));
	tmp *= (float)ve_constants->req_fuel;
	tmp /= 10.0;
	req_fuel_total = tmp;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_total_spin),
			tmp);

	/* req-fuel info box  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_per_squirt_spin),
			ve_constants->req_fuel/10.0);

	/* CONFIG11-13 related buttons */
	/* Cylinders */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.cylinders_spin),
			ve_constants->config11.bit.cylinders+1);
	num_cylinders = ve_constants->config11.bit.cylinders+1;

	/* Number of injectors */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.injectors_spin),
			ve_constants->config12.bit.injectors+1);
	num_injectors = ve_constants->config12.bit.injectors+1;

	/* Injections per cycle */
	tmp =	(float)(ve_constants->config11.bit.cylinders+1) /
		(float)(ve_constants->divider);
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.inj_per_cycle_spin),
			tmp);
	num_squirts = (gint)tmp;

	/* Speed Density or Alpha-N */
	if (ve_constants->config13.bit.inj_strat)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.alpha_n_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.speed_den_but),
				TRUE);
	/* Even Fire of Odd Fire */
	if (ve_constants->config13.bit.firing)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.odd_fire_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.even_fire_but),
				TRUE);
	/* NarrowBand O2 or WideBand O2 */
	if (ve_constants->config13.bit.ego_type)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.wbo2_but),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.nbo2_but),TRUE);
	}
	/* Baro Correction, enabled or disabled */
	if (ve_constants->config13.bit.baro_corr)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.baro_ena_but),TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.baro_disa_but),
				TRUE);
	/* CONFIG11 related buttons */
	/* Map sensor 115kPA or 250 kPA */
	if (ve_constants->config11.bit.map_type)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.map_250_but),TRUE);
		kpa_conversion = turbo_map;
		map_pbar_divisor = 255.0;
	}
	else
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.map_115_but),TRUE);
		kpa_conversion = na_map;
		map_pbar_divisor = 115.0;
	}
	/* 2 stroke or 4 Stroke */
	if (ve_constants->config11.bit.eng_type)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.two_stroke_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.four_stroke_but),
				TRUE);
	/* Multi-Port or TBI */
	if (ve_constants->config11.bit.inj_type)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.tbi_but),TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.multi_port_but),
				TRUE);

	/* THIS is NOT compatible with DualTable code,  must be changed */
	if (ve_constants->alternate > 0)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.alternate_but),
				TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.simul_but),
				TRUE);


	/* Table 1 for dualtable, and all std megasquirt units */
	for (i=0;i<VEBLOCK_SIZE;i++)
	{
		temp_dep = FALSE;
		if (GTK_IS_OBJECT(veconst_widgets_1[i]))
		{
			dl_type = (gint)g_object_get_data(
					G_OBJECT(veconst_widgets_1[i]),
					"dl_type");
			temp_dep = (gboolean)g_object_get_data(
					G_OBJECT(veconst_widgets_1[i]),
					"temp_dep");
			if (dl_type == IMMEDIATE)
				value = convert_after_upload(i);  
			if (temp_dep)
			{
				if (!fahrenheit)
					value = (value-32)*(5.0/9.0);
			}

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(
						veconst_widgets_1[i]),value);
					
		}

	}
}
void check_req_fuel_limits()
{
	gfloat tmp = 0.0;
	gfloat req_fuel_per_squirt = 0.0;
	gfloat req_fuel_per_squirt_dl = 0.0;
	gint lim_flag = 0;
	gint index = 0;
	gint dload_val = 0;
	gint offset = 0;
	gint page = -1;

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

	tmp =	(float)(ve_constants->divider*(float)(ve_constants->alternate+1))/(float)(num_injectors);
	
	/* This is 1 tenth the value as the one screen stuff is 1/10th 
	 * for the ms variable,  it gets converted farther down, just 
	 * before download to the MS
	 */
	req_fuel_per_squirt = tmp * req_fuel_total;
	req_fuel_per_squirt_dl = (gint)((req_fuel_per_squirt*10.0)+0.001);

	if (req_fuel_per_squirt_dl != ve_constants->req_fuel)
	{
		if (req_fuel_per_squirt_dl > 255)
			lim_flag = 1;
		if (req_fuel_per_squirt_dl < 0)
			lim_flag = 1;
	}
		/* req-fuel info box  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_per_squirt_spin),
			req_fuel_per_squirt);
	if (lim_flag)
	{	/*
		 * ERROR, something is out of bounds 
		 * change the color of the potential offenders onscreen to
		 * let the user know something is wrong..
		 */
		gtk_widget_modify_fg(labels.squirts_lab,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_fg(labels.injectors_lab,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_fg(labels.cylinders_lab,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.req_fuel_total_spin,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.inj_per_cycle_spin,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.cylinders_spin,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.injectors_spin,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.req_fuel_per_squirt_spin,
				GTK_STATE_INSENSITIVE,&red);

	}
	else
	{	/*
		 * Everything is OK with all inter-dependant variables.
		 * settings all previous gui enties to normal state...
		 */

		gtk_widget_modify_fg(labels.squirts_lab,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_fg(labels.injectors_lab,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_fg(labels.cylinders_lab,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.req_fuel_total_spin,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.inj_per_cycle_spin,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.cylinders_spin,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.injectors_spin,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.req_fuel_per_squirt_spin,
				GTK_STATE_INSENSITIVE,&black);

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
		page = 0;
		write_ve_const(dload_val, offset, page);
		for (index=0;index<g_list_length(offsets);index++)
		{
			gint offset;
			gint data;
			page = -1;
			offset = GPOINTER_TO_INT(g_list_nth_data(offsets,index));
			data = offset_data[g_list_index(offsets,
					g_list_nth_data(offsets,index))];
			page = page_data[g_list_index(offsets,
					g_list_nth_data(offsets,index))];
			write_ve_const(data, offset, page);
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
		ego_pbar_divisor=5.0;
	else
		ego_pbar_divisor=1.2;

	/* Check SD/Alpha-N button and adjust VEtable labels
	 * to suit
	 */
	if (((tmp >> 2)&0x1) == 1)
	{
		label = gtk_frame_get_label_widget(
				GTK_FRAME
				(map_tps_frame));
		gtk_label_set_text(GTK_LABEL
				(label),"TPS Bins");
		gtk_label_set_text(GTK_LABEL
				(map_tps_label),
				"TPS %");
	}
	else
	{
		label = gtk_frame_get_label_widget(
				GTK_FRAME
				(map_tps_frame));
		gtk_label_set_text(GTK_LABEL
				(label),"MAP Bins");
		gtk_label_set_text(GTK_LABEL
				(map_tps_label),
				"Kpa");
	}
}

