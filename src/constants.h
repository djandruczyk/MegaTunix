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

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

/* this is required so we keep track of the gui controls so we
 * can update them easily with a generic function instead of
 * one function per control.... (ugly..)
 */

struct v1_2_Constants
{
	GtkAdjustment *inj_open_time_adj;	/* Adjustment */
	GtkAdjustment *batt_corr_adj;		/* Adjustment */
	GtkAdjustment *pwm_curr_lim_adj;	/* Adjustment */
	GtkAdjustment *pwm_time_max_adj;	/* Adjustment */
	GtkAdjustment *fast_idle_thresh_adj;	/* Adjustment */
	GtkAdjustment *req_fuel_1_adj;		/* Adjustment */
	GtkAdjustment *req_fuel_2_adj;		/* Adjustment */
	GtkAdjustment *cr_pulse_neg40_adj;	/* Adjustment */
	GtkAdjustment *cr_pulse_pos170_adj;	/* Adjustment */
	GtkAdjustment *cr_priming_pulse_adj;	/* Adjustment */
	GtkAdjustment *as_enrich_adj;		/* Adjustment */
	GtkAdjustment *as_num_cycles_adj;	/* Adjustment */
	GtkWidget *warmup_entries[8];		/* Text Entries */
	GtkWidget *tps_trig_thresh_ent;		/* Text Entry */
	GtkWidget *accel_duration_ent;		/* Text Entry */
	GtkWidget *cold_accel_addon_ent;	/* Text Entry */
	GtkWidget *cold_accel_mult_ent;		/* Text Entry */
	GtkWidget *accel_bins[4];		/* Text Entries */
	GtkWidget *decel_cut_ent;		/* Text Entry */
	GtkWidget *ego_temp_active_ent;		/* Text Entry */
	GtkWidget *ego_rpm_active_ent;		/* Text Entry */
	GtkWidget *ego_sw_voltage_ent;		/* Text Entry */
	GtkWidget *ego_step_ent;		/* Text Entry */
	GtkWidget *ego_events_ent;		/* Text Entry */
	GtkWidget *ego_limit_ent;		/* Text Entry */
};

#endif
