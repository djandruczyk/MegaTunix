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

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>

struct v1_2_Constants
{
	GtkAdjustment *inj_open_time;
	GtkAdjustment *batt_corr;
	GtkAdjustment *pwm_curr_lim;
	GtkAdjustment *pwm_time_thresh;
	GtkAdjustment *fast_idle_thresh;
	GtkAdjustment *crank_pulse_40;	
	GtkAdjustment *crank_pulse_170;	
	GtkAdjustment *crank_priming_pulse;	
	GtkAdjustment *afterstart_enrich;	
	GtkAdjustment *afterstart_num_cycles;	
	GtkAdjustment *warmup_neg_40;	
	GtkAdjustment *warmup_neg_20;	
	GtkAdjustment *warmup_0;	
	GtkAdjustment *warmup_20;	
	GtkAdjustment *warmup_40;	
	GtkAdjustment *warmup_60;	
	GtkAdjustment *warmup_80;	
	GtkAdjustment *warmup_100;	
	GtkAdjustment *warmup_130;	
	GtkAdjustment *warmup_160;	
};


