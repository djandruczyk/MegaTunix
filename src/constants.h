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
	GtkAdjustment *crank_pulse_40;		/* Adjustment */
	GtkAdjustment *crank_pulse_170;		/* Adjustment */
	GtkAdjustment *crank_priming_pulse;	/* Adjustment */
	GtkAdjustment *afterstart_enrich;	/* Adjustment */
	GtkAdjustment *afterstart_num_cycles;	/* Adjustment */
	GtkWidget *warmup_neg_40;		/* Text Entry */
	GtkWidget *warmup_neg_20;		/* Text Entry */
	GtkWidget *warmup_0;			/* Text Entry */
	GtkWidget *warmup_20;			/* Text Entry */
	GtkWidget *warmup_40;			/* Text Entry */
	GtkWidget *warmup_60;			/* Text Entry */
	GtkWidget *warmup_80;			/* Text Entry */
	GtkWidget *warmup_100;			/* Text Entry */
	GtkWidget *warmup_130;			/* Text Entry */
	GtkWidget *warmup_160;			/* Text Entry */	
	GtkWidget *tps_trig_thresh;		/* Text Entry */
	GtkWidget *accel_duration;		/* Text Entry */
	GtkWidget *cold_accel_addon;		/* Text Entry */
	GtkWidget *cold_accel_mult;		/* Text Entry */
	GtkWidget *accel_2v_sec;		/* Text Entry */
	GtkWidget *accel_4v_sec;		/* Text Entry */
	GtkWidget *accel_8v_sec;		/* Text Entry */
	GtkWidget *accel_15v_sec;		/* Text Entry */
	GtkWidget *decel_cut;			/* Text Entry */
	GtkWidget *o2_temp_active;		/* Text Entry */
	GtkWidget *o2_rpm_active;		/* Text Entry */
	GtkWidget *o2_sw_voltage;		/* Text Entry */
	GtkWidget *o2_step;			/* Text Entry */
	GtkWidget *o2_events;			/* Text Entry */
	GtkWidget *o2_limit;			/* Text Entry */
};

#endif
