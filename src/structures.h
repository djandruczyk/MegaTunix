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

#ifndef __STRUCTURES_H__
#define __STRUCTURES_H__

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
	GtkWidget *req_fuel_total_spin;		/* Spinner */
	GtkWidget *req_fuel_per_squirt_spin;	/* Spinner */
	GtkWidget *cr_pulse_neg40_spin;		/* Spinner */
	GtkWidget *cr_pulse_pos170_spin;	/* Spinner */
	GtkWidget *cr_priming_pulse_spin;	/* Spinner */
	GtkWidget *as_enrich_spin;		/* Spinner */
	GtkWidget *as_num_cycles_spin;		/* Spinner */
	GtkWidget *ve1_kpa_bins_spin[8];	/* Spinners */
	GtkWidget *ve1_rpm_bins_spin[8];	/* Spinners */
	GtkWidget *ve1_bins_spin[64];		/* Spinners */
	GtkWidget *ve2_kpa_bins_spin[8];	/* Spinners */
	GtkWidget *ve2_rpm_bins_spin[8];	/* Spinners */
	GtkWidget *ve2_bins_spin[64];		/* Spinners */
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

/* Controls for the Required Fuel Calculator... */
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

/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct Labels
{
	GtkWidget *req_fuel_lab;
	GtkWidget *squirts_lab;
	GtkWidget *injectors_lab;
	GtkWidget *cylinders_lab;
	GtkWidget *fastidletemp_lab;
	GtkWidget *cr_pulse_lowtemp_lab;
	GtkWidget *cr_pulse_hightemp_lab;
	GtkWidget *warmup_bins_lab[10];
	GtkWidget *warmup_title;
	GtkWidget *ego_temp_lab;
	GtkWidget *runtime_clt_lab;
	GtkWidget *runtime_mat_lab;
};

/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct Adjustments
{
	GtkAdjustment *fast_idle_temp_adj;
	GtkAdjustment *ego_temp_adj;
};

/* These are defined here instead of the individual .c files as
 * we manipulate their attributes to give feedback to the user
 */
struct Buttons
{
	GtkWidget *const_store_but;
	GtkWidget *enrich_store_but;
	GtkWidget *vetable_store_but;
};

/* These are defined here instead of the individual .c files as
 * we manipulate their attributes to give feedback to the user
 */
struct Counts
{
	GtkWidget *comms_reset_entry;
	GtkWidget *runtime_reset_entry;
	GtkWidget *comms_sioerr_entry;
	GtkWidget *runtime_sioerr_entry;
	GtkWidget *comms_readcount_entry;
	GtkWidget *runtime_readcount_entry;
	GtkWidget *comms_ve_readcount_entry;
	GtkWidget *runtime_ve_readcount_entry;
};

/* Datastructure the holds the expected responses for the data
 * returning commands issued to the MegaSquirt.
 */

struct Command_Limits
{
	gint	A_count;
	gint	C_count;
	gint	Q_count;
	gint	V_count;
	gint	S_count;
	gint	I_count;
};

	
#endif
