/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/* Constants/Enrichments Gui Adjustment Structures */

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

/* this is required so we keep track of the gui controls so we
 * can update them easily with a generic function instead of
 * one function per control.... 
 */

struct v1_2_Constants
{
	GtkWidget *inj_open_time_spin;		/* Spinner */
	GtkWidget *batt_corr_spin;		/* Spinner */
	GtkWidget *pwm_curr_lim_spin;		/* Spinner */
	GtkWidget *pwm_time_max_spin;		/* Spinner */
	GtkWidget *fast_idle_thresh_spin;	/* Spinner */
	GtkWidget *req_fuel_spin;		/* Spinner */
	GtkWidget *req_fuel_base_spin;		/* Spinner */
	GtkWidget *cr_pulse_neg40_spin;		/* Spinner */
	GtkWidget *cr_pulse_pos170_spin;	/* Spinner */
	GtkWidget *cr_priming_pulse_spin;	/* Spinner */
	GtkWidget *as_enrich_spin;		/* Spinner */
	GtkWidget *as_num_cycles_spin;		/* Spinner */
	GtkWidget *kpa_bins_spin[8];		/* Spinners */
	GtkWidget *rpm_bins_spin[8];		/* Spinners */
	GtkWidget *ve_bins_spin[64];		/* Spinners */
	GtkWidget *warmup_bins_spin[10];	/* Spinners */
	GtkWidget *tps_trig_thresh_spin;	/* Spinner */
	GtkWidget *accel_duration_spin;		/* Spinner */
	GtkWidget *cold_accel_addon_spin;	/* Spinner */
	GtkWidget *cold_accel_mult_spin;	/* Spinner */
	GtkWidget *accel_bins_spin[4];		/* Spinners */
	GtkWidget *decel_cut_spin;		/* Spinner */
	GtkWidget *ego_temp_active_spin;	/* Spinner */
	GtkWidget *ego_rpm_active_spin;		/* Spinner */
	GtkWidget *ego_sw_voltage_spin;		/* Spinner */
	GtkWidget *ego_step_spin;		/* Spinner */
	GtkWidget *ego_events_spin;		/* Spinner */
	GtkWidget *ego_limit_spin;		/* Spinner */
	GtkWidget *inj_per_cycle_spin;		/* Spinner */
	GtkWidget *injectors_spin;		/* Spinner */
	GtkWidget *cylinders_spin;		/* Spinner */
	GtkAdjustment *cylinders_adj;		/* Adjustment */
	GtkWidget *speed_den_but;		/* Toggle button */
	GtkWidget *alpha_n_but;			/* Toggle button */
	GtkWidget *two_stroke_but;		/* Toggle button */
	GtkWidget *four_stroke_but;		/* Toggle button */
	GtkWidget *multi_port_but;		/* Toggle button */
	GtkWidget *tbi_but;			/* Toggle button */
	GtkWidget *map_115_but;			/* Toggle button */
	GtkWidget *map_250_but;			/* Toggle button */
	GtkWidget *even_fire_but;		/* Toggle button */
	GtkWidget *odd_fire_but;		/* Toggle button */
	GtkWidget *nbo2_but;			/* Toggle button */
	GtkWidget *wbo2_but;			/* Toggle button */
	GtkWidget *simul_but;			/* Toggle button */
	GtkWidget *alternate_but;		/* Toggle button */
	GtkWidget *baro_ena_but;		/* Toggle button */
	GtkWidget *baro_disa_but;		/* Toggle button */
};

struct Reqd_Fuel
{
        GtkWidget *disp_spin;		/* Spinbutton */
        GtkWidget *cyls_spin;		/* Spinbutton */
        GtkWidget *inj_rate_spin;	/* Spinbutton */
        GtkWidget *afr_spin;		/* Spinbutton */
        gint disp;			/* Engine size  1-1000 Cu-in */
        gint cyls;			/* # of Cylinders  1-12 */
        gint inj_rate;			/* injector flow rate (lbs/hr) */
        gfloat afr;			/* Air fuel ratio 10-25.5 */
};

struct Labels
{
	GtkWidget *req_fuel_lab;
	GtkWidget *squirts_lab;
	GtkWidget *injectors_lab;
	GtkWidget *cylinders_lab;
};

struct Buttons
{
	GtkWidget *const_store_but;
	GtkWidget *enrich_store_but;
	GtkWidget *vetable_store_but;
};

	
#endif
