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

/* Global Variables */

#ifndef __MS_STRUCTURES_H__
#define __MS_STRUCTURES_H__

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <unions.h>

struct Raw_Runtime_Std
{       /* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into Runtime_Common (struct)
	 */
	guchar	secl;		/* Offset 0 */
	union	squirt squirt;	/* Offset 1 */
	union	engine engine;	/* Offset 2 */
	guchar	baro;		/* Offset 3 */
	guchar	map;		/* Offset 4 */
	guchar	mat;		/* Offset 5 */
	guchar	clt;		/* Offset 6 */
	guchar	tps;		/* Offset 7 */
	guchar	batt;		/* Offset 8 */
	guchar	ego;		/* Offset 9 */
	guchar	egocorr;	/* Offset 10 */
	guchar	aircorr;	/* Offset 11 */
	guchar	warmcorr;	/* Offset 12 */
	guchar	rpm;		/* Offset 13 */
	guchar	pw1;		/* Offset 14 */
	guchar	tpsaccel;	/* Offset 15 */
	guchar	barocorr;	/* Offset 16 */
	guchar	gammae;		/* Offset 17 */
	guchar	vecurr1;	/* Offset 18 */
	guchar	bspot1;		/* Offset 19 */
	guchar	bspot2;		/* Offset 20 */
	guchar	bspot3;		/* Offset 21 */
	/* NOTE: the last 3 are used diffeently in the other MS variants
	 * like the dualtable, squirtnspark and such...
	 */
};

struct Raw_Runtime_Dualtable 
{       /* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into Runtime_Common (struct)
	 */
	guchar	secl;		/* Offset 0 */
	union	squirt squirt;	/* Offset 1 */
	union	engine engine;	/* Offset 2 */
	guchar	baro;		/* Offset 3 */
	guchar	map;		/* Offset 4 */
	guchar	mat;		/* Offset 5 */
	guchar	clt;		/* Offset 6 */
	guchar	tps;		/* Offset 7 */
	guchar	batt;		/* Offset 8 */
	guchar	ego;		/* Offset 9 */
	guchar	egocorr;	/* Offset 10 */
	guchar	aircorr;	/* Offset 11 */
	guchar	warmcorr;	/* Offset 12 */
	guchar	rpm;		/* Offset 13 */
	guchar	pw1;		/* Offset 14 */
	guchar	tpsaccel;	/* Offset 15 */
	guchar	barocorr;	/* Offset 16 */
	guchar	gammae;		/* Offset 17 */
	guchar	vecurr1;	/* Offset 18 */
	guchar	pw2;		/* Offset 19 */
	guchar	vecurr2;	/* Offset 20 */
	guchar	idledc;		/* Offset 21 */
};

struct Raw_Runtime_Ignition 
{       /* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into Runtime_Common (struct)
	 */
	guchar	secl;		/* Offset 0 */
	union	squirt squirt;	/* Offset 1 */
	union	engine engine;	/* Offset 2 */
	guchar	baro;		/* Offset 3 */
	guchar	map;		/* Offset 4 */
	guchar	mat;		/* Offset 5 */
	guchar	clt;		/* Offset 6 */
	guchar	tps;		/* Offset 7 */
	guchar	batt;		/* Offset 8 */
	guchar	ego;		/* Offset 9 */
	guchar	egocorr;	/* Offset 10 */
	guchar	aircorr;	/* Offset 11 */
	guchar	warmcorr;	/* Offset 12 */
	guchar	rpm;		/* Offset 13 */
	guchar	pw1;		/* Offset 14 */
	guchar	tpsaccel;	/* Offset 15 */
	guchar	barocorr;	/* Offset 16 */
	guchar	gammae;		/* Offset 17 */
	guchar	vecurr1;	/* Offset 18 */
	guchar	ctimeH;		/* Offset 19 */
	guchar	ctimeL;		/* Offset 20 */
	guchar	sparkangle;	/* Offset 21 */
};

struct Raw_Runtime_Enhanced 
{       /* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into Runtime_Common (struct)
	 */
	guchar	secl;		/* Offset 0 */
	union	squirt squirt;	/* Offset 1 */
	union	engine engine;	/* Offset 2 */
	guchar	baro;		/* Offset 3 */
	guchar	map;		/* Offset 4 */
	guchar	mat;		/* Offset 5 */
	guchar	clt;		/* Offset 6 */
	guchar	tps;		/* Offset 7 */
	guchar	batt;		/* Offset 8 */
	guchar	ego;		/* Offset 9 */
	guchar	egocorr;	/* Offset 10 */
	guchar	aircorr;	/* Offset 11 */
	guchar	warmcorr;	/* Offset 12 */
	guchar	rpm;		/* Offset 13 */
	guchar	pw1;		/* Offset 14 */
	guchar	tpsaccel;	/* Offset 15 */
	guchar	barocorr;	/* Offset 16 */
	guchar	gammae;		/* Offset 17 */
	guchar	vecurr1;	/* Offset 18 */
	guchar	ctimeH;		/* Offset 19 */
	guchar	ctimeL;		/* Offset 20 */
	guchar	sparkangle;	/* Offset 21 */
	guchar	afrtarget;	/* Offset 22 */
	guchar	fpadc;		/* Offset 23 */
	guchar	egtadc;		/* Offset 24 */
	guchar	cltiatangle;	/* Offset 25 */
};


struct Runtime_Common 
{	/* This is the OUTPUT after the raw runtime structure has been 
	 * parsed and fed here.  We keep around both the RAW and converted
	 * values so that either can be datalogged. We also hav variables
	 * here for dualtable variables as dataloging gets all info from 
	 * this structure....
	 * This structure is ordered such that all of the "bigger" variables
	 * are first, as this structure gets array referenced in the 
	 * dataloging code, and having a gfloat on the wrong boundary gives 
	 * BAD data...
	 */
				/*     Offset ------  Description */
	gfloat	baro_volts;	/* 0 Baro in volts (0-5) */
	gfloat	batt_volts;	/* 4 BATT in Volts  (0-5) */
	gfloat	clt_volts;	/* 8 CLT in volts  (0-5) */
	gfloat	ego_volts;	/* 12 EGO in Volts  (0-5) */
	gfloat	map_volts;	/* 16 MAP in volts  (0-5) */
	gfloat	mat_volts;	/* 20 MAT in volts  (0-5) */
	gfloat	tps_volts;	/* 24 TPS in volts  (0-5) */
	gfloat	dcycle1;	/* 28 Injector 1 duty cycle  */
	gfloat	dcycle2;	/* 32 Injector 2 duty cycle (DT) */
        gfloat	pw1;		/* 36 Injector squirt time in ms */
        gfloat	pw2;		/* 40 injector squirt time in ms (DT) */
	gfloat	tps;		/* 44 TPS in % fullscale (converted) */
	gshort	clt;		/* 48 CLT in degrees (converted) */
	gshort	mat;		/* 50 MAT in degrees (converted) */
        gshort	rpm;		/* 52 Computed engine RPM - rpm */
        guchar	secl;		/* 54 low seconds - from 0 to 255, then rollover */
        union squirt squirt;	/* 55 Event variable bit field for Injector Firing */
        union engine engine;	/* 56 Variable bit-field to hold engine current status */
        guchar	vecurr1;	/* 57 Current VE value for Table 1 */
        guchar	vecurr2;	/* 58 Current VE value for Table 2 */
	guchar	baro;		/* 59 Barometer in KPA (converted) */
	guchar	map;		/* 60 MAP in KPA (converted) */
        guchar	gammae;		/* 61 Total Gamma Enrichments % */
        guchar	baro_raw;	/* 62 Barometer ADC Raw Counts */
        guchar	batt_raw;	/* 63 BATT Voltage ADC Raw Reading */
        guchar	clt_raw;	/* 64 CLT Sensor ADC Raw Counts */
	guchar	ego_raw;	/* 65 EGO Sensor ADC Raw Reading */
        guchar	map_raw;	/* 66 MAP Sensor ADC Raw Counts */
        guchar	mat_raw;	/* 67 MAT Sensor ADC Raw Counts */
        guchar	tps_raw;	/* 68 TPS Sensor ADC Raw Counts */
        guchar	aircorr;	/* 69 Air Density Correction % */
        guchar	barocorr;	/* 70 Baro Correction % */
	guchar	egocorr;	/* 71 Oxygen Sensor Correction  %*/
        guchar	tpsaccel;	/* 72 Acceleration enrichment % */
        guchar	warmcorr;	/* 73 Total Warmup Correction % */
        guchar	idledc;		/* 74 IdlePWM dutycycle */
        guchar	ctimeH;		/* 75 SquirtnSpark Cycletime H */
        guchar	ctimeL;		/* 76 SquirtnSpark Cycletime H */
        guchar	sparkangle;	/* 77 SquirtnSpark sparkangle */
	guchar	bspot1;		/* 78 blank spot 1 (for Std Runtime) */
	guchar	bspot2;		/* 79 blank spot 2 (for Std Runtime) */
	guchar	bspot3;		/* 80 blank spot 3 (for Std Runtime) */
};

struct Ve_Const_Std
{
        /* TYPE          Variable              Offset,  Comment */
        guchar	ve_bins[64];		/* 0, VE table, 64 bytes */
        guchar	cr_pulse_neg40;		/* 64, cr pulse at -40 deg F */
        guchar	cr_pulse_pos170;	/* 65, cr pulse at 170 deg F */
        guchar	as_enrich;		/* 66, Enrich over base (%) */
        guchar	as_num_cycles;		/* 67, enrich for X cycles */
        guchar	warmup_bins[10];	/* 68, Warmup bins  */
        guchar	accel_bins[4];		/* 78, TPS Accel bins */
        guchar	cold_accel_addon;	/* 82, Accel addon at -40deg */
        guchar	tps_trig_thresh;	/* 83, TPS trig thr in V/sec */
        guchar	accel_duration;		/* 84, Accel duration(secs) */
        guchar	decel_cut;		/* 85, decel fuel cut % */
        guchar	ego_temp_active;	/* 86, EGO activation pt */
        guchar	ego_events;		/* 87, ign events betw steps */
        guchar	ego_step;		/* 88, correction % */
        guchar	ego_limit;		/* 89, +/- limit */
        guchar	req_fuel;		/* 90, require fuel */
        guchar	divider;		/* 91, IRQ / factor for pulse*/
        guchar	alternate;		/* 92, alternate inj drivers */
        guchar	inj_open_time;		/* 93, inj open time */
        guchar	inj_ocfuel;		/* 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        guchar	pwm_curr_lim;		/* 95, curr limit PWM duty cycle */
        guchar	pwm_time_max;		/* 96, Peak hold time */
        guchar	batt_corr;		/* 97, Batt Voltage Corr */
        gshort	rpmk;			/* 98, 12K/ncyl */
        guchar	rpm_bins[8];		/* 100, VEtable RPM bins */
        guchar	load_bins[8];		/* 108, VEtable KPA bins */

        union	config11 config11;	/* 116, Config for PC Config */
        union	config12 config12;	/* 117, Config for PC Config */
        union	config13 config13;	/* 118, Config for PC Config */
        guchar	cr_priming_pulse;	/* 119, priming pulse b4 start*/
        guchar	ego_rpm_active;		/* 120, EGO RPM trigger volt  */
        guchar	fast_idle_thresh;	/* 121, fast idle temp thresh */
        guchar	ego_sw_voltage;		/* 122, EGO flip pt voltage */
        guchar	cold_accel_mult;	/* 123, Cold Accel * factor */
        guchar	pad1;			/* 124, Padding to 128 bytes */
        guchar	pad2;			/* 125, Padding to 128 bytes */
        guchar	pad3;			/* 126, Padding to 128 bytes */
        guchar	pad4;			/* 127, Padding to 128 bytes */
}; 

	/* VE table and Constants for Page 0 */
struct Ve_Const_DT_1
{
        /* TYPE          Variable              Offset,  Comment */
        guchar	ve_bins[64];		/* 0, VE table, 64 bytes */
        guchar	cr_pulse_neg40;		/* 64, cr pulse at -40 deg F */
        guchar	cr_pulse_pos170;	/* 65, cr pulse at 170 deg F */
        guchar	as_enrich;		/* 66, Enrich over base (%) */
        guchar   as_num_cycles;		/* 67, enrich for X cycles */
        guchar	warmup_bins[10];	/* 68, Warmup bins  */
        guchar	accel_bins[4];		/* 78, TPS Accel bins */
        guchar	cold_accel_addon;	/* 82, Accel addon at -40deg */
        guchar	tps_trig_thresh;	/* 83, TPS trig thr in V/sec */
        guchar	accel_duration;		/* 84, Accel duration(secs) */
        guchar	decel_cut;		/* 85, decel fuel cut % */
        guchar	ego_temp_active;	/* 86, EGO activation pt */
        guchar	ego_events;		/* 87, ign events betw steps */
        guchar	ego_step;		/* 88, correction % */
        guchar	ego_limit;		/* 89, +/- limit */
        guchar	req_fuel;		/* 90, require fuel */
        guchar	divider;		/* 91, IRQ / factor for pulse*/
        union	tblcnf tblcnf;		/* 92, inj config per table */
        guchar	inj_open_time;		/* 93, inj open time */
        guchar	inj_ocfuel;		/* 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        guchar	pwm_curr_lim;		/* 95, curr limit PWM duty cycle */
        guchar	pwm_time_max;		/* 96, Peak hold time */
        guchar	batt_corr;		/* 97, Batt Voltage Corr */
        gshort	rpmk;			/* 98, 12K/ncyl */
        guchar	rpm_bins[8];		/* 100, VEtable RPM bins */
        guchar	load_bins[8];		/* 108, VEtable KPA bins */

        union	config11 config11;	/* 116, Config for PC Config */
        union	config12 config12;	/* 117, Config for PC Config */
        union	config13 config13;	/* 118, Config for PC Config */
        guchar	cr_priming_pulse;	/* 119, priming pulse b4 start*/
        guchar	ego_rpm_active;		/* 120, EGO RPM trigger volt  */
        guchar	fastidle_temp;		/* 121, fast idle temp */
        guchar	ego_sw_voltage;		/* 122, EGO flip pt voltage */
        guchar	cold_accel_mult;	/* 123, Cold Accel * factor */
        guchar	slowidle_temp;		/* 124, Dualtable */
        guchar	fastidlespd;		/* 125, Fast idle speed RPM/10 */
        guchar	slowidlespd;		/* 126, Slow idle speed RPM/10 */
        guchar	idlethresh;		/* 127, TPS A/D count threshold BELOW which idle PWM is used */
}; 

	/* VE table and Constants for Page 1 */
struct Ve_Const_DT_2
{
        /* TYPE          Variable              Offset,  Comment */
        guchar	ve_bins[64];		/* 128, VE table, 64 bytes */
	guchar	unused[26];		/* 192, unused... */
        guchar	req_fuel;		/* 218, require fuel */
        guchar	divider;		/* 219, IRQ / factor for pulse*/
        guchar	unused_220;		/* 220, inj config per table */
        guchar	inj_open_time;		/* 221, inj open time */
        guchar	inj_ocfuel;		/* 222, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        guchar	pwm_curr_lim;		/* 223, curr limit PWM duty 
						 * cycle
 						 */
        guchar	pwm_time_max;		/* 224, Peak hold time */
        guchar	batt_corr;		/* 225, Batt Voltage Corr */
        gshort	rpmk;			/* 226, 12K/ncyl */
        guchar	rpm_bins[8];		/* 228, VEtable RPM bins */
        guchar	load_bins[8];		/* 236, VEtable KPA bins */
        union	config11 config11;	/* 244, Config for PC Config */
        union	config12 config12;	/* 245, Config for PC Config */
	guchar	unused246;		/* 246, Unused  */
	union	bcfreq bcfreq;		/* 247, pwm rate for boost */ 
						/* 0-ERROR, 1=39hz, 2=19 Hz */
						/* 3 = 10 Hz */
	guchar	bcfreqdiv;		/* 248, ??????? */
	guchar	bcpgain;		/* 249, proportional Gain % */
	guchar	bcdgain;		/* 250, derivative Gain % */
	guchar	revlimit;		/* 251, maxrev limit rpm/100 */ 
	guchar	launchlimit;		/* 252, launch control revlim */
	guchar	shiftlo;		/* 253, lo shift light thresh */
	guchar	shifthi;		/* 254, hi shift light thresh */
	guchar	crank_rpm;		/* 255, rpm/100 */
};

	/* MegaSquirtnEDIS Ignition table and Constants */
struct Ignition_Table
{
        /* TYPE          Variable              Offset,  Comment */
        guchar	spark_table[64];	/* 0, Spark table, 64 bytes */
        guchar	rpm_bins[8];		/* 64, RPM Bins, 8 bytes */
        guchar	load_bins[8];		/* 72, RPM Bins, 8 bytes */
	guchar	trig_angle;		/* 80, Trigger angle BTDC */
	guchar	fixed_angle;		/* 81, Fixed angle */
	gchar	trim_angle;		/* 82, Trim Angle +- */
	guchar	crank_angle;		/* 83, Cranking Angle  */
	guchar	spark_hold_cyc;		/* 84, Spark Hold Cycles */
	union	spark_config1 spark_config1;	/* 85, Spark Hold Cycles */
	guchar	soft_rev_rpm;		/* 86, Soft Revlim RPM */
	guchar	soft_rev_angle;		/* 87, Soft Revlim sp. angle */
	guchar	soft_rev_htime;		/* 88, Soft Revlim max time */
	guchar	soft_rev_ctime;		/* 89, Soft Revlim cool time */
	guchar	hard_rev_rpm;		/* 90, Hard Revlim RPM */
	guchar 	out1limit;		/* 91, Output 1 Limit */
	guchar 	out1source;		/* 92, Output 1 source index from secl */
	guchar 	out2limit;		/* 93, Output 2 Limit */
	guchar 	out2source;		/* 94, Output 2 source index from secl */
};

#endif
