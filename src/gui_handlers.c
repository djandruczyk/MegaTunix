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
#include "protos.h"
#include "defines.h"
#include "globals.h"
#include "constants.h"


static int req_fuel_popup = FALSE;
static GtkWidget *popup;
static int paused_handlers = FALSE;
static int rpmk_offset = 99;
extern int raw_reader_running;
extern int raw_reader_stopped;
extern int read_wait_time;
extern struct ms_ve_constants *ve_constants;
extern struct v1_2_Constants constants;
struct 
{
	GtkWidget *disp_spin;		/* Engine size  1-1000 Cu-in */
	GtkWidget *cyls_spin;		/* # of Cylinders  1-12 */
	GtkWidget *inj_rate_spin;	/* injector flow rate (lbs/hr) */
	GtkWidget *afr_spin;		/* Air fuel ratio 10-25.5 */
	gint disp;
	gint cyls;
	gint inj_rate;
	gfloat afr;
}reqd_fuel ;


void leave(GtkWidget *widget, gpointer *data)
{
	save_config();
	stop_serial_thread();
	/* Free all buffers */
	close_serial();
	mem_dealloc();
	gtk_main_quit();
}

int text_entry_handler(GtkWidget * widget, gpointer *data)
{
	gchar *entry_text;
	gint offset;
	if (paused_handlers)
		return TRUE ;
	entry_text = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	offset = (gint)gtk_object_get_data(G_OBJECT(widget),"offset");
	switch ((gint)data)
	{
/*		case WARMUP_NEG_40:
			ve_constants->warmup_bins[0] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[0], offset);
			break;
		case WARMUP_NEG_20:
			ve_constants->warmup_bins[1] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[1], offset);
			break;
		case WARMUP_0:
			ve_constants->warmup_bins[2] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[2], offset);
			break;
		case WARMUP_20:
			ve_constants->warmup_bins[3] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[3], offset);
			break;
		case WARMUP_40:
			ve_constants->warmup_bins[4] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[4], offset);
			break;
		case WARMUP_60:
			ve_constants->warmup_bins[5] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[5], offset);
			break;
		case WARMUP_80:
			ve_constants->warmup_bins[6] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[6], offset);
			break;
		case WARMUP_100:
			ve_constants->warmup_bins[7] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[7], offset);
			break;
		case WARMUP_130:
			ve_constants->warmup_bins[8] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[8], offset);
			break;
		case WARMUP_160:
			ve_constants->warmup_bins[9] = atoi(entry_text);
			write_ve_const(ve_constants->warmup_bins[9], offset);
			break;
	*/
		default:
			break;
	}
	/* update the widget in case data was out of bounds */
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
	 * Fairly generic, should work for multiple statusbars
	 *
	 */

	gtk_statusbar_pop(GTK_STATUSBAR(status_bar),
			context_id);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar),
			context_id,
			message);
}
	
int reqd_fuel_popup()
{
	GtkWidget *button;
	GtkWidget *spinner;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *table;
	GtkAdjustment *adj;

	req_fuel_popup=TRUE;
	//	reqd_fuel.disp = 350;
	reqd_fuel.cyls = 4;
	//	reqd_fuel.inj_rate = 19;
	reqd_fuel.afr = 14.7;
	popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(popup),"Required Fuel Calc");
	gtk_container_set_border_width(GTK_CONTAINER(popup),10);
	gtk_widget_realize(popup);
	g_signal_connect(G_OBJECT(popup),"delete_event",
			G_CALLBACK (close_popup),
			NULL);
	g_signal_connect(G_OBJECT(popup),"destroy_event",
			G_CALLBACK (close_popup),
			NULL);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(popup),vbox);
	frame = gtk_frame_new("Constants for your vehicle");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table =gtk_table_new(4,3,FALSE);	
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);

	label = gtk_label_new("Engine Displacement (CID)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Number of Cylinders");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Injector Flow (lbs/hr)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Air-Fuel Ratio");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Engine Displacement */
	adj = (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.disp,1.0,1000,
			1.0,10.0,0);
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_DISP));
	reqd_fuel.disp_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Number of Cylinders */
	adj = (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.cyls,1.0,16,
			1.0,1.0,0);
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_CYLS));
	reqd_fuel.cyls_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Fuel injector flow rate in lbs/hr */
	adj = (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.inj_rate,1.0,100.0,
			1.0,1.0,0);
	spinner = gtk_spin_button_new(adj,0,0);
	gtk_widget_set_size_request(spinner,65,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_INJ_RATE));
	reqd_fuel.inj_rate_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	/* Target Air Fuel Ratio */
	adj =  (GtkAdjustment *) gtk_adjustment_new(reqd_fuel.afr,10.0,25.5,
			0.1,0.1,0);
	spinner = gtk_spin_button_new(adj,0,1);
	gtk_widget_set_size_request(spinner,65,-1);
	g_signal_connect (G_OBJECT(spinner), "value_changed",
			G_CALLBACK (spinner_changed),
			GINT_TO_POINTER(REQ_FUEL_AFR));
	reqd_fuel.afr_spin = spinner;
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_table_attach (GTK_TABLE (table), spinner, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox),5);

	button = gtk_button_new_with_label("Calculate\nand Close");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(update_reqd_fuel),
			NULL);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK (close_popup),
			NULL);

	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,15);
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK (close_popup),
			NULL);

	gtk_widget_show_all(popup);

	return TRUE;
}

int close_popup(GtkWidget *widget, gpointer *data)
{
	gtk_widget_destroy(popup);
	req_fuel_popup=FALSE;
	return TRUE;
}

int update_reqd_fuel(GtkWidget *widget, gpointer *data)
{
	gfloat tmp1,tmp2;

	tmp1 = 36.0*((double)reqd_fuel.disp)*4.27793;
	tmp2 = ((double) reqd_fuel.cyls) \
		* ((double)(reqd_fuel.afr)) \
		* ((double)(reqd_fuel.inj_rate));

	ve_constants->req_fuel_1 = 10.0*(tmp1/tmp2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_1_spin),
			ve_constants->req_fuel_1/10.0);

	ve_constants->rpmk = (int)(12000.0/((double)reqd_fuel.cyls));
	write_ve_const(ve_constants->rpmk, rpmk_offset);

	return TRUE;
}

int generic_spinner_changed(GtkWidget *widget, gpointer *data)
{
        gfloat value = 0.0;
        gint offset = 0;
	gint class = 0;
        if (paused_handlers)
                return TRUE;
        value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
        offset = GPOINTER_TO_INT(data);
	/* Class is set to determine the course of action */
	class = (gint) gtk_object_get_data(G_OBJECT(widget),"class");
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
				= (gint)((value*10.0)+.01);
			write_ve_const((gint)((value*10.0)+.01), offset);
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
	gint tmp = 0;
	if (paused_handlers)
		return TRUE;
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	offset = (gint) gtk_object_get_data(G_OBJECT(widget),"offset");
	printf("spinner value: %.1f ,offset %i\n",value,offset);

	switch ((gint)data)
	{
		case SET_SER_PORT:
			if(serial_params.open)
			{
				if (raw_reader_running)
					stop_serial_thread();
				close_serial();
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
		case INJ_OPEN_TIME:
			/* This funny conversion is needed cause of 
			 * some weird quirk when multiplying the float
			 * by ten and then converting to int (is off by
			 * one in SOME cases only..
			 */
			tmp = (int)((value*10.0)+.01);
			ve_constants->inj_open_time = tmp;
			write_ve_const(ve_constants->inj_open_time, offset);
			break;
		case BATT_CORR:
			tmp = (int)((value*10.0)+.01);
			ve_constants->batt_corr = tmp;
			write_ve_const(ve_constants->batt_corr, offset);
			break;

		default:
			break;
	}
	return TRUE;

}

void update_const_ve()
{
	char buff[10];
	gint i;

	/* req-fuel  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_1_spin),
			ve_constants->req_fuel_1/5.0);

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
	g_snprintf(buff,10,"%.2f",ve_constants->tps_trig_thresh/5.0);
	gtk_entry_set_text(GTK_ENTRY(constants.tps_trig_thresh_ent),
			buff);

	/* Accel Enrich Duration */
	g_snprintf(buff,10,"%.1f",ve_constants->accel_duration/10.0);
	gtk_entry_set_text(GTK_ENTRY(constants.accel_duration_ent),
			buff);

	/* Cold Accel Enrich Add On */
	g_snprintf(buff,10,"%.1f",ve_constants->cold_accel_addon/10.0);
	gtk_entry_set_text(GTK_ENTRY(constants.cold_accel_addon_ent),
			buff);

	/* Cold Accel Enrich Multiplier */
	g_snprintf(buff,10,"%i",ve_constants->cold_accel_mult);
	gtk_entry_set_text(GTK_ENTRY(constants.cold_accel_mult_ent),
			buff);

	/* Decel Fuel Cut*/
	g_snprintf(buff,10,"%i",ve_constants->decel_cut);
	gtk_entry_set_text(GTK_ENTRY(constants.decel_cut_ent),
			buff);

	/* EGO coolant activation temp */
	g_snprintf(buff,10,"%i",ve_constants->ego_temp_active-40);
	gtk_entry_set_text(GTK_ENTRY(constants.ego_temp_active_ent),
			buff);

	/* EGO activation RPM */
	g_snprintf(buff,10,"%i",ve_constants->ego_rpm_active*100);
	gtk_entry_set_text(GTK_ENTRY(constants.ego_rpm_active_ent),
			buff);

	/* EGO switching voltage */
	g_snprintf(buff,10,"%.2f",(ve_constants->ego_sw_voltage/255.0)*5);
	gtk_entry_set_text(GTK_ENTRY(constants.ego_sw_voltage_ent),
			buff);

	/* EGO step percent */
	g_snprintf(buff,10,"%i",ve_constants->ego_step);
	gtk_entry_set_text(GTK_ENTRY(constants.ego_step_ent),
			buff);

	/* EGO events between steps */
	g_snprintf(buff,10,"%i",ve_constants->ego_events);
	gtk_entry_set_text(GTK_ENTRY(constants.ego_events_ent),
			buff);

	/* EGO limit % between steps */
	g_snprintf(buff,10,"%i",ve_constants->ego_limit);
	gtk_entry_set_text(GTK_ENTRY(constants.ego_limit_ent),
			buff);

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

	// Stub function, does nothing yet... 
}
