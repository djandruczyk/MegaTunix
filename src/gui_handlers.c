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


extern gint req_fuel_popup;
static gint paused_handlers = FALSE;
extern gint raw_reader_running;
extern gint raw_reader_stopped;
extern gint read_wait_time;
extern struct ms_ve_constants *ve_constants;
extern struct v1_2_Constants constants;
extern struct Reqd_Fuel reqd_fuel;
extern struct Labels labels;
static gint num_squirts;
static gint num_cylinders;
static gint num_injectors;
static GdkColor red = { 0, 65535, 0, 0};
static GdkColor black = { 0, 0, 0, 0};

void leave(GtkWidget *widget, gpointer *data)
{
	save_config();
	stop_serial_thread();
	/* Free all buffers */
	close_serial();
	usleep(100000); /* make sure thread dies cleanly.. */
	mem_dealloc();
	gtk_main_quit();
}

int toggle_button_handler(GtkWidget *widget, gpointer *data)
{
	gint config_num = 0;
	gint bit_pos = 0;
	gint bit_val = 0;
	gint bitmask = 0;
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

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{
		/* If control reaches here, the toggle button is down */
		printf("config_num %i, bit_pos %i, bit_val %i,bitmask %i\n",config_num,bit_pos,bit_val,bitmask);		
		switch (config_num)
		{
			case 11:
				tmp = ve_constants->config11.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << (bit_pos-1));
				ve_constants->config11.value = tmp;
				dload_val = tmp;
				offset = 117;
				break;
			case 12:
				tmp = ve_constants->config12.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << (bit_pos-1));
				dload_val = tmp;
				offset = 118;
				break;
			case 13:
				tmp = ve_constants->config13.value;
				tmp = tmp & ~bitmask;	/*clears bits */
				tmp = tmp | (bit_val << (bit_pos-1));
				dload_val = tmp;
				offset = 119;
				break;
			case 14:
				/*SPECIAL*/
				offset = 93;
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
				check_req_fuel_limits();
				break;

		}
		if (dl_type == IMMEDIATE)
		{
			printf("Immediate download\n");
			write_ve_const(dload_val, offset);
		}
		else if (dl_type == DEFERRED)
		{
			printf("Test if in gList, if not add it\n");
			printf("Deferred download (inter-related variable)\n");
			printf("tests should be performed and downlaod button\nchanged to RED to inform user to force a download\n");
		}
	}
	return TRUE;
}

int std_button_handler(GtkWidget *widget, gpointer *data)
{
	switch ((gint)data)
	{
		case START_REALTIME:
			start_serial_thread();
			break;
		case STOP_REALTIME:
			stop_serial_thread();
			break;
		case REQD_FUEL_POPUP:
			if (!req_fuel_popup)
				reqd_fuel_popup();
			break;
		case READ_FROM_MS:
			paused_handlers = TRUE;
			read_ve_const();
			update_const_ve();
			paused_handlers = FALSE;
			break;
		case WRITE_TO_MS:
			//write_ve_const();
			break;
	}
	return TRUE;
}

void update_statusbar(GtkWidget *status_bar,int context_id, gchar * message)
{
	/* takes 3 args, 
	 * the GtkWidget pointer to the statusbar,
	 * the context_id of the statusbar in arg[0],
	 * and the string to be sent to that bar
	 *
	 * Fairly generic, works for multiple statusbars
	 */

	gtk_statusbar_pop(GTK_STATUSBAR(status_bar),
			context_id);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar),
			context_id,
			message);
}
	
int classed_spinner_changed(GtkWidget *widget, gpointer *data)
{
        gfloat value = 0.0;
        gint offset = 0;
	gint class = 0;
        if (paused_handlers)
                return TRUE;
        value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	/* Class is set to determine the course of action */
	class = (gint) g_object_get_data(G_OBJECT(widget),"class");
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	switch (class)
	{
		case WARMUP:
			ve_constants->warmup_bins[offset-WARMUP_BINS_OFFSET] 
				= (gint)value;
			write_ve_const((gint)value, offset);
			break;
		case RPM:
			ve_constants->rpm_bins[offset-RPM_BINS_OFFSET] 
				= (gint)value/100.0;
			write_ve_const((gint)value/100.0, offset);
			break;
		case KPA:
			ve_constants->kpa_bins[offset-KPA_BINS_OFFSET] 
				= (gint)value;
			write_ve_const((gint)value, offset);
			break;
		case VE:
			ve_constants->ve_bins[offset-VE_TABLE_OFFSET] 
				= (gint)value;
			write_ve_const((gint)value, offset);
			break;
		case ACCEL:
			ve_constants->accel_bins[offset-ACCEL_BINS_OFFSET] 
				= (gint)((value*10.0)+.001);
			write_ve_const((gint)((value*10.0)+.001), offset);
			break;
	}

	return TRUE;
}

int spinner_changed(GtkWidget *widget, gpointer *data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	gfloat value = 0.0;
	gint offset;
	gint tmpi_x10 = 0; /* value*10 converted to an INT */
	gint tmpi = 0;
	gint tmp = 0;
	gint dload_val = 0;
	gint dl_type = 0;
	gint err_flag = 0;
	if (paused_handlers)
		return TRUE;
	printf("spinner changed handler \n");
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	offset = (gint) g_object_get_data(G_OBJECT(widget),"offset");
	dl_type = (gint) g_object_get_data(G_OBJECT(widget),"dl_type");
	tmpi_x10 = (int)((value*10.0)+.001);
	tmpi = (int)(value+.001);

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
//			ve_constants->req_fuel = tmpi_x10;
			break;
		case INJ_OPEN_TIME:
			/* This funny conversion is needed cause of 
			 * some weird quirk when multiplying the float
			 * by ten and then converting to int (is off by
			 * one in SOME cases only..
			 */
			ve_constants->inj_open_time = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case BATT_CORR:
			ve_constants->batt_corr = (gint)((value*60.0)+0.001);
			dload_val = (gint)((value*60.0)+0.001);
			break;
		case PWM_CUR_LIM:
			ve_constants->pwm_curr_lim = tmpi;
			dload_val = tmpi;
			break;
		case PWM_TIME_THRES:
			ve_constants->pwm_time_max = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case FAST_IDLE_THRES:
			ve_constants->fast_idle_thresh = tmpi+40;
			dload_val = tmpi+40;
			break;
		case CRANK_PULSE_NEG_40:
			ve_constants->cr_pulse_neg40 = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case CRANK_PULSE_170:
			ve_constants->cr_pulse_pos170 = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case CRANK_PRIMING_PULSE:
			ve_constants->cr_priming_pulse = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case AFTERSTART_ENRICH:
			ve_constants->as_enrich = tmpi;
			dload_val = tmpi;
			break;
		case AFTERSTART_NUM_CYCLES:
			ve_constants->as_num_cycles = tmpi;
			dload_val = tmpi;
			break;
		case TPS_TRIG_THRESH:
			tmpi = (int)((value*5.0)+.001);
			ve_constants->tps_trig_thresh = tmpi;
			dload_val = tmpi;
			break;
		case ACCEL_ENRICH_DUR:
			ve_constants->accel_duration = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case COLD_ACCEL_ENRICH:
			ve_constants->cold_accel_addon = tmpi_x10;
			dload_val = tmpi_x10;
			break;
		case COLD_ACCEL_MULT:
			ve_constants->cold_accel_mult = tmpi;
			dload_val = tmpi;
			break;
		case DECEL_CUT:
			ve_constants->decel_cut = tmpi;
			dload_val = tmpi;
			break;
		case EGO_TEMP_ACTIVE:
			ve_constants->ego_temp_active = tmpi+40;
			dload_val = tmpi+40;
			break;
		case EGO_RPM_ACTIVE:
			ve_constants->ego_rpm_active = tmpi/100;
			dload_val = tmpi/100;
			break;
		case EGO_SW_VOLTAGE:	
			tmpi = (int)((value*51.0)+.001);
			ve_constants->ego_sw_voltage = tmpi;
			dload_val = tmpi;
			break;
		case EGO_STEP:	
			ve_constants->ego_step = tmpi;
			dload_val = tmpi;
			break;
		case EGO_EVENTS:	
			ve_constants->ego_events = tmpi;
			dload_val = tmpi;
			break;
		case EGO_LIMIT:	
			ve_constants->ego_limit = tmpi;
			dload_val = tmpi;
			break;
		case NUM_SQUIRTS:
			/* This actuall effects another variable */
			num_squirts = tmpi;
			ve_constants->divider = 
					(gint)(((float)num_cylinders/
					(float)num_squirts)+0.001);
			dload_val = ve_constants->divider;
			if (num_cylinders % num_squirts)
			{
				err_flag = 1;
				gtk_widget_modify_fg(labels.squirts_lab,
						GTK_STATE_NORMAL,&red);
				gtk_widget_modify_fg(labels.cylinders_lab,
						GTK_STATE_NORMAL,&red);
			}
			else
			{
				gtk_widget_modify_fg(labels.squirts_lab,
						GTK_STATE_NORMAL,&black);
				gtk_widget_modify_fg(labels.cylinders_lab,
						GTK_STATE_NORMAL,&black);
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
			if (num_cylinders % num_squirts)
			{
				err_flag = 1;
				gtk_widget_modify_fg(labels.squirts_lab,
						GTK_STATE_NORMAL,&red);
				gtk_widget_modify_fg(labels.cylinders_lab,
						GTK_STATE_NORMAL,&red);
			}
			else
			{
				gtk_widget_modify_fg(labels.squirts_lab,
						GTK_STATE_NORMAL,&black);
				gtk_widget_modify_fg(labels.cylinders_lab,
						GTK_STATE_NORMAL,&black);
				check_req_fuel_limits();
			}
			break;
		case NUM_INJECTORS:
			/* Updates a shared bitfield */
			num_injectors = tmpi;
			tmp = ve_constants->config12.value;
			tmp = tmp & ~0xf0;	/*clears top 4 bits */
			tmp = tmp | ((tmpi-1) << 4);
			ve_constants->config12.value = tmp;
			dload_val = tmp;
			check_req_fuel_limits();
			break;
		default:
			/* Prevents MS corruption for a SW bug */
			printf("ERROR spinbutton not handled\b\n");
			dl_type = 0;  
			break;
	}
	if (dl_type == IMMEDIATE)
	{
		printf("Immediate download\n");
		write_ve_const(dload_val, offset);
	}
	else if (dl_type == DEFERRED)
	{
		printf("deferred....\n");
//		printf("Test if in gList, if not add it\n");
//		printf("Deferred download (inter-related variable)\n");
//		printf("tests should be performed and downlaod button\nchanegd to RED to inform user to force a download\n");
	}
	return TRUE;

}

void update_const_ve()
{
	gint i;
	gfloat tmp;

	/* req-fuel  */
	/*				/     num_injectors     \
	 *          req_fuel_from_MS * |-------------------------|
	 * Result =                     \ divider*(alternate+1) /
	 *	    -----------------------------------------------
	 *				10
	 *
	 * where divider = num_cylinders/num_squirts;
	 *
	 */
	tmp =	(float)(ve_constants->config12.bit.injectors+1) /
		(float)(ve_constants->divider*(ve_constants->alternate+1));
	tmp *= (float)ve_constants->req_fuel;
	tmp /= 10.0;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_spin),
			tmp);

//	/* req-fuel info box  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_base_spin),
			ve_constants->req_fuel/10.0);

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

	/* inj_open_time */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.inj_open_time_spin),
			ve_constants->inj_open_time/10.0);

	/* batt_corr */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.batt_corr_spin),
			ve_constants->batt_corr/60.0);

	/* pwm_curr_lim */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.pwm_curr_lim_spin),
			ve_constants->pwm_curr_lim);

	/* pwm_time_thresh */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.pwm_time_max_spin),
			ve_constants->pwm_time_max/10.0);

	/* fast_idle_thresh */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.fast_idle_thresh_spin),
			ve_constants->fast_idle_thresh-40.0);

	/* crank pulse -40 */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cr_pulse_neg40_spin),
			ve_constants->cr_pulse_neg40/10.0);

	/* crank pulse 170 */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cr_pulse_pos170_spin),
			ve_constants->cr_pulse_pos170/10.0);

	/* Priming pulse */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cr_priming_pulse_spin),
			ve_constants->cr_priming_pulse/10.0);

	/* Afterstart Enrich % */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.as_enrich_spin),
			ve_constants->as_enrich);

	/* Afterstart Enrich # of cycles */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.as_num_cycles_spin),
			ve_constants->as_num_cycles);

	/* Warmup Enrichment bins */
	for (i=0;i<10;i++)
	{
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.warmup_bins_spin[i]),
			ve_constants->warmup_bins[i]);
	}
	/* accel enrichments */
	for (i=0;i<4;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.accel_bins_spin[i]),
				ve_constants->accel_bins[i]/10.0);
	}

	/* TPS Trigger Threshold */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.tps_trig_thresh_spin),
			ve_constants->tps_trig_thresh/5.0);

	/* Accel Enrich Duration */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.accel_duration_spin),
			ve_constants->accel_duration/10.0);

	/* Cold Accel Enrich Add On */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.cold_accel_addon_spin),
			ve_constants->cold_accel_addon/10.0);

	/* Cold Accel Enrich Multiplier */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.cold_accel_mult_spin),
			ve_constants->cold_accel_mult);

	/* Decel Fuel Cut*/
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.decel_cut_spin),
			ve_constants->decel_cut);

	/* EGO coolant activation temp */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_temp_active_spin),
			ve_constants->ego_temp_active-40);

	/* EGO activation RPM */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_rpm_active_spin),
			ve_constants->ego_rpm_active*100);

	/* EGO switching voltage */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_sw_voltage_spin),
			ve_constants->ego_sw_voltage/51.0);

	/* EGO step percent */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_step_spin),
			ve_constants->ego_step);

	/* EGO events between steps */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_events_spin),
			ve_constants->ego_events);

	/* EGO limit % between steps */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_limit_spin),
			ve_constants->ego_limit);

	/* VE table entries */
	for (i=0;i<64;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.ve_bins_spin[i]),
				ve_constants->ve_bins[i]);
	}

	/* KPA axis VE table entries */
	for (i=0;i<8;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.kpa_bins_spin[i]),
				ve_constants->kpa_bins[i]);
	}

	/* RPM axis VE table entries */
	for (i=0;i<8;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.rpm_bins_spin[i]),
				ve_constants->rpm_bins[i]*100);
	}
	/* CONFIG11-13 related buttons */
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
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.wbo2_but),
                                TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.nbo2_but),
                                TRUE);
	/* Baro Correction, enabled or disabled */
	if (ve_constants->config13.bit.baro_corr)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.baro_ena_but),
                                TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.baro_disa_but),
                                TRUE);
	/* CONFIG11 related buttons */
	/* Map sensor 115kPA or 250 kPA */
	if (ve_constants->config11.bit.map_type)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.map_250_but),
                                TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.map_115_but),
                                TRUE);
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
				GTK_TOGGLE_BUTTON(constants.tbi_but),
                                TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.multi_port_but),
                                TRUE);

	if (ve_constants->alternate > 0)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.alternate_but),
                                TRUE);
	else
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(constants.simul_but),
                                TRUE);
	
}
void check_req_fuel_limits()
{
	gfloat tmp = 0.0;
	gfloat req_fuel_dl = 0.0;
	gint lim_flag = 0;

//	printf("divider = %i\n",ve_constants->divider);
//	printf("alternate = %i\n",ve_constants->alternate);

	tmp =	(float)(ve_constants->divider*(ve_constants->alternate+1))/
		(float)(ve_constants->config12.bit.injectors+1);

	req_fuel_dl = tmp * ve_constants->req_fuel;

	printf("req_fuel_dl %.2f, req_fuel %.2f\n",req_fuel_dl,ve_constants->req_fuel/10.0);
	if ((int)req_fuel_dl != ve_constants->req_fuel)
	{
		if (req_fuel_dl > 255.0)
			lim_flag = 1;
		if (req_fuel_dl < 0.0)
			lim_flag = 1;
	}
		/* req-fuel info box  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_base_spin),
			req_fuel_dl/10.0);
	if (lim_flag)
	{
		gtk_widget_modify_fg(labels.squirts_lab,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_fg(labels.cylinders_lab,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.req_fuel_spin,
				GTK_STATE_NORMAL,&red);
		gtk_widget_modify_text(constants.req_fuel_base_spin,
				GTK_STATE_INSENSITIVE,&red);

	}
	else
	{
		gtk_widget_modify_fg(labels.squirts_lab,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_fg(labels.cylinders_lab,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.req_fuel_spin,
				GTK_STATE_NORMAL,&black);
		gtk_widget_modify_text(constants.req_fuel_base_spin,
				GTK_STATE_INSENSITIVE,&black);
	}
	


	return;
}
