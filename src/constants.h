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
 * one function per control.... (ugly..)
 */

struct v1_2_Constants
{
	GtkWidget *inj_open_time_spin;		/* Spinner */
	GtkWidget *batt_corr_spin;		/* Spinner */
	GtkWidget *pwm_curr_lim_spin;		/* Spinner */
	GtkWidget *pwm_time_max_spin;		/* Spinner */
	GtkWidget *fast_idle_thresh_spin;	/* Spinner */
	GtkWidget *req_fuel_1_spin;		/* Spinner */
	GtkWidget *req_fuel_2_spin;		/* Spinner */
	GtkWidget *cr_pulse_neg40_spin;		/* Spinner */
	GtkWidget *cr_pulse_pos170_spin;	/* Spinner */
	GtkWidget *cr_priming_pulse_spin;	/* Spinner */
	GtkWidget *as_enrich_spin;		/* Spinner */
	GtkWidget *as_num_cycles_spin;		/* Spinner */
	GtkWidget *kpa_bins_spin[8];		/* Spinners */
	GtkWidget *rpm_bins_spin[8];		/* Spinners */
	GtkWidget *ve_bins_spin[64];		/* Spinners */
	GtkWidget *warmup_bins_spin[10];	/* Spinners */
	GtkWidget *tps_trig_thresh_ent;		/* Text Entry */
	GtkWidget *accel_duration_ent;		/* Text Entry */
	GtkWidget *cold_accel_addon_ent;	/* Text Entry */
	GtkWidget *cold_accel_mult_ent;		/* Text Entry */
	GtkWidget *accel_bins_spin[4];		/* Spinners */
	GtkWidget *decel_cut_ent;		/* Text Entry */
	GtkWidget *ego_temp_active_ent;		/* Text Entry */
	GtkWidget *ego_rpm_active_ent;		/* Text Entry */
	GtkWidget *ego_sw_voltage_ent;		/* Text Entry */
	GtkWidget *ego_step_ent;		/* Text Entry */
	GtkWidget *ego_events_ent;		/* Text Entry */
	GtkWidget *ego_limit_ent;		/* Text Entry */
};

#endif
