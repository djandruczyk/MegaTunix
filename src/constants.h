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
	GtkAdjustment *inj_open_time;		/* Adjustment */
	GtkAdjustment *batt_corr;		/* Adjustment */
	GtkAdjustment *pwm_curr_lim;		/* Adjustment */
	GtkAdjustment *pwm_time_thresh;		/* Adjustment */
	GtkAdjustment *fast_idle_thresh;	/* Adjustment */
	GtkAdjustment *req_fuel_1;		/* Adjustment */
	GtkAdjustment *req_fuel_2;		/* Adjustment */
	GtkAdjustment *crank_pulse_neg40;	/* Adjustment */
	GtkAdjustment *crank_pulse_pos170;	/* Adjustment */
	GtkAdjustment *crank_priming_pulse;	/* Adjustment */
	GtkAdjustment *afterstart_enrich;	/* Adjustment */
	GtkAdjustment *afterstart_num_cycles;	/* Adjustment */
	GtkWidget *warmup_entries[8];		/* Text Entries */
	GtkWidget *tps_trig_thresh;		/* Text Entry */
	GtkWidget *accel_duration;		/* Text Entry */
	GtkWidget *cold_accel_addon;		/* Text Entry */
	GtkWidget *cold_accel_mult;		/* Text Entry */
	GtkWidget *accel_bins[4];		/* Text Entries */
	GtkWidget *decel_cut;			/* Text Entry */
	GtkWidget *ego_temp_active;		/* Text Entry */
	GtkWidget *ego_rpm_active;		/* Text Entry */
	GtkWidget *ego_sw_voltage;		/* Text Entry */
	GtkWidget *ego_step;			/* Text Entry */
	GtkWidget *ego_events;			/* Text Entry */
	GtkWidget *ego_limit;			/* Text Entry */
};

#endif
