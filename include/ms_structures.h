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
	 * and fed into ms_data_v1_and_v2 (struct)
	 */
	unsigned char	secl;		/* Offset 0 */
	union squirt	squirt;		/* Offset 1 */
	union engine	engine;		/* Offset 2 */
	unsigned char	baro;		/* Offset 3 */
	unsigned char	map;		/* Offset 4 */
	unsigned char	mat;		/* Offset 5 */
	unsigned char	clt;		/* Offset 6 */
	unsigned char	tps;		/* Offset 7 */
	unsigned char	batt;		/* Offset 8 */
	unsigned char	ego;		/* Offset 9 */
	unsigned char	egocorr;	/* Offset 10 */
	unsigned char	aircorr;	/* Offset 11 */
	unsigned char	warmcorr;	/* Offset 12 */
	unsigned char	rpm;		/* Offset 13 */
	unsigned char	pw1;		/* Offset 14 */
	unsigned char	tpsaccel;	/* Offset 15 */
	unsigned char	barocorr;	/* Offset 16 */
	unsigned char	gammae;		/* Offset 17 */
	unsigned char	vecurr1;	/* Offset 18 */
	unsigned char	bspot1;		/* Offset 19 */
	unsigned char	bspot2;		/* Offset 20 */
	unsigned char	bspot3;		/* Offset 21 */
	/* NOTE: the last 3 are used diffeently in the other MS variants
	 * like the dualtable, squirtnspark and such...
	 */
};

struct Raw_Runtime_Dualtable 
{       /* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into ms_data_v1_and_v2 (struct)
	 */
	unsigned char	secl;		/* Offset 0 */
	union squirt	squirt;		/* Offset 1 */
	union engine	engine;		/* Offset 2 */
	unsigned char	baro;		/* Offset 3 */
	unsigned char	map;		/* Offset 4 */
	unsigned char	mat;		/* Offset 5 */
	unsigned char	clt;		/* Offset 6 */
	unsigned char	tps;		/* Offset 7 */
	unsigned char	batt;		/* Offset 8 */
	unsigned char	ego;		/* Offset 9 */
	unsigned char	egocorr;	/* Offset 10 */
	unsigned char	aircorr;	/* Offset 11 */
	unsigned char	warmcorr;	/* Offset 12 */
	unsigned char	rpm;		/* Offset 13 */
	unsigned char	pw1;		/* Offset 14 */
	unsigned char	tpsaccel;	/* Offset 15 */
	unsigned char	barocorr;	/* Offset 16 */
	unsigned char	gammae;		/* Offset 17 */
	unsigned char	vecurr1;	/* Offset 18 */
	unsigned char	pw2;		/* Offset 19 */
	unsigned char	vecurr2;	/* Offset 20 */
	unsigned char	idleDC;		/* Offset 21 */
};


struct Runtime_Common 
{	/* This is the OUTPUT after the raw runtime structure has been 
	 * parsed and fed here.  We keep around both the RAW and converted
	 * values so that either can be datalogged. We also hav variables
	 * here for dualtable variables as dataloging gets all info from 
	 * this structure....
	 * This structure is ordered such that all of the "bigger" variables
	 * are first, as this structure gets array referenced in the 
	 * dataloging code, and having a float on the wrong boundary gives 
	 * BAD data...
	 */
				/*     Offset ------  Description */
	float		baro_volts;	/* 0 Baro in volts (0-5) */
	float		batt_volts;	/* 4 BATT in Volts  (0-5) */
	float		clt_volts;	/* 8 CLT in volts  (0-5) */
	float		ego_volts;	/* 12 EGO in Volts  (0-5) */
	float		map_volts;	/* 16 MAP in volts  (0-5) */
	float		mat_volts;	/* 20 MAT in volts  (0-5) */
	float		tps_volts;	/* 24 TPS in volts  (0-5) */
	float		dcycle1;	/* 28 Injector 1 duty cycle  */
	float		dcycle2;	/* 32 Injector 2 duty cycle (DT) */
        float		pw1;		/* 36 Injector squirt time in ms */
        float		pw2;		/* 40 injector squirt time in ms (DT) */
	float		tps;		/* 44 TPS in % fullscale (converted) */
	short		clt;		/* 48 CLT in degrees (converted) */
	short		mat;		/* 50 MAT in degrees (converted) */
        unsigned short	rpm;		/* 52 Computed engine RPM - rpm */
        unsigned char	secl;		/* 54 low seconds - from 0 to 255, then rollover */
        union squirt	squirt;		/* 55 Event variable bit field for Injector Firing */
        union engine	engine;		/* 56 Variable bit-field to hold engine current status */
        unsigned char	vecurr1;	/* 57 Current VE value Table 1 */
        unsigned char	vecurr2;	/* 58 Current VE table Table 2 */
	unsigned char	baro;		/* 59 Barometer in KPA (converted) */
	unsigned char	map;		/* 60 MAP in KPA (converted) */
        unsigned char	gammae;		/* 61 Total Gamma Enrichments % */
        unsigned char	baro_raw;	/* 62 Barometer ADC Raw Counts */
        unsigned char	batt_raw;	/* 63 BATT Voltage ADC Raw Reading */
        unsigned char	clt_raw;	/* 64 CLT Sensor ADC Raw Counts */
	unsigned char	ego_raw;	/* 65 EGO Sensor ADC Raw Reading */
        unsigned char	map_raw;	/* 66 MAP Sensor ADC Raw Counts */
        unsigned char	mat_raw;	/* 67 MAT Sensor ADC Raw Counts */
        unsigned char	tps_raw;	/* 68 TPS Sensor ADC Raw Counts */
        unsigned char	aircorr;	/* 69 Air Density Correction % */
        unsigned char	barocorr;	/* 70 Baro Correction % */
	unsigned char	egocorr;	/* 71 Oxygen Sensor Correction  %*/
        unsigned char	tpsaccel;	/* 72 Acceleration enrichment % */
        unsigned char	warmcorr;	/* 73 Total Warmup Correction % */
        unsigned char	idleDC;		/* 74 IdlePWM dutycycle */
	unsigned char	bspot1;		/* 75 blank spot 1 (for Std Runtime) */
	unsigned char	bspot2;		/* 76 blank spot 2 (for Std Runtime) */
	unsigned char	bspot3;		/* 77 blank spot 3 (for Std Runtime) */
};

struct Ve_Const_Std
{
        /* TYPE          Variable              Offset,  Comment */
        unsigned char	ve_bins[64];		/* 0, VE table, 64 bytes */
        unsigned char	cr_pulse_neg40;		/* 64, cr pulse at -40 deg F */
        unsigned char	cr_pulse_pos170;	/* 65, cr pulse at 170 deg F */
        unsigned char	as_enrich;		/* 66, Enrich over base (%) */
        unsigned char	as_num_cycles;		/* 67, enrich for X cycles */
        unsigned char	warmup_bins[10];	/* 68, Warmup bins  */
        unsigned char	accel_bins[4];		/* 78, TPS Accel bins */
        unsigned char	cold_accel_addon;	/* 82, Accel addon at -40deg */
        unsigned char	tps_trig_thresh;	/* 83, TPS trig thr in V/sec */
        unsigned char	accel_duration;		/* 84, Accel duration(secs) */
        unsigned char	decel_cut;		/* 85, decel fuel cut % */
        unsigned char	ego_temp_active;	/* 86, EGO activation pt */
        unsigned char	ego_events;		/* 87, ign events betw steps */
        unsigned char	ego_step;		/* 88, correction % */
        unsigned char	ego_limit;		/* 89, +/- limit */
        unsigned char	req_fuel;		/* 90, require fuel */
        unsigned char	divider;		/* 91, IRQ / factor for pulse*/
        unsigned char	alternate;		/* 92, alternate inj drivers */
        unsigned char	inj_open_time;		/* 93, inj open time */
        unsigned char   inj_ocfuel;		/* 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        unsigned char	pwm_curr_lim;		/* 95, curr limit PWM duty 
						 * cycle
 						 */
        unsigned char	pwm_time_max;		/* 96, Peak hold time */
        unsigned char	batt_corr;		/* 97, Batt Voltage Corr */
        unsigned short	rpmk;			/* 98, 12K/ncyl */
        unsigned char	rpm_bins[8];		/* 100, VEtable RPM bins */
        unsigned char	load_bins[8];		/* 108, VEtable KPA bins */

        union	config11 config11;		/* 116, Config for PC Config */
        union	config12 config12;		/* 117, Config for PC Config */
        union	config13 config13;		/* 118, Config for PC Config */
        unsigned char	cr_priming_pulse;	/* 119, priming pulse b4 start*/
        unsigned char	ego_rpm_active;		/* 120, EGO RPM trigger volt  */
        unsigned char	fast_idle_thresh;	/* 121, fast idle temp thresh */
        unsigned char	ego_sw_voltage;		/* 122, EGO flip pt voltage */
        unsigned char	cold_accel_mult;	/* 123, Cold Accel * factor */
        unsigned char	pad1;			/* 124, Padding to 128 bytes */
        unsigned char	pad2;			/* 125, Padding to 128 bytes */
        unsigned char	pad3;			/* 126, Padding to 128 bytes */
        unsigned char	pad4;			/* 127, Padding to 128 bytes */
}; 

	/* VE table and Constants for Page 0 */
struct Ve_Const_DT_1
{
        /* TYPE          Variable              Offset,  Comment */
        unsigned char	ve_bins[64];		/* 0, VE table, 64 bytes */
        unsigned char	cr_pulse_neg40;		/* 64, cr pulse at -40 deg F */
        unsigned char	cr_pulse_pos170;	/* 65, cr pulse at 170 deg F */
        unsigned char	as_enrich;		/* 66, Enrich over base (%) */
        unsigned char   as_num_cycles;		/* 67, enrich for X cycles */
        unsigned char	warmup_bins[10];	/* 68, Warmup bins  */
        unsigned char	accel_bins[4];		/* 78, TPS Accel bins */
        unsigned char	cold_accel_addon;	/* 82, Accel addon at -40deg */
        unsigned char	tps_trig_thresh;	/* 83, TPS trig thr in V/sec */
        unsigned char	accel_duration;		/* 84, Accel duration(secs) */
        unsigned char	decel_cut;		/* 85, decel fuel cut % */
        unsigned char	ego_temp_active;	/* 86, EGO activation pt */
        unsigned char	ego_events;		/* 87, ign events betw steps */
        unsigned char	ego_step;		/* 88, correction % */
        unsigned char	ego_limit;		/* 89, +/- limit */
        unsigned char	req_fuel;		/* 90, require fuel */
        unsigned char	divider;		/* 91, IRQ / factor for pulse*/
        union tblcnf	tblcnf;			/* 92, inj config per table */
        unsigned char	inj_open_time;		/* 93, inj open time */
        unsigned char	inj_ocfuel;		/* 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        unsigned char	pwm_curr_lim;		/* 95, curr limit PWM duty 
						 * cycle
 						 */
        unsigned char	pwm_time_max;		/* 96, Peak hold time */
        unsigned char	batt_corr;		/* 97, Batt Voltage Corr */
        unsigned short	rpmk;			/* 98, 12K/ncyl */
        unsigned char	rpm_bins[8];		/* 100, VEtable RPM bins */
        unsigned char	load_bins[8];		/* 108, VEtable KPA bins */

        union	config11 config11;		/* 116, Config for PC Config */
        union	config12 config12;		/* 117, Config for PC Config */
        union config13	config13;		/* 118, Config for PC Config */
        unsigned char	cr_priming_pulse;	/* 119, priming pulse b4 start*/
        unsigned char	ego_rpm_active;		/* 120, EGO RPM trigger volt  */
        unsigned char	fastidle_temp;		/* 121, fast idle temp */
        unsigned char	ego_sw_voltage;		/* 122, EGO flip pt voltage */
        unsigned char	cold_accel_mult;	/* 123, Cold Accel * factor */
        unsigned char	slowidle_temp;		/* 124, Dualtable */
        unsigned char	fastidlespd;		/* 125, Fast idle speed RPM/10 */
        unsigned char	slowidlespd;		/* 126, Slow idle speed RPM/10 */
        unsigned char	idlethresh;		/* 127, TPS A/D count threshold BELOW which idle PWM is used */
}; 

	/* VE table and Constants for Page 1 */
struct Ve_Const_DT_2
{
        /* TYPE          Variable              Offset,  Comment */
        unsigned char	ve_bins[64];		/* 128, VE table, 64 bytes */
	unsigned char   unused[26];		/* 192, unused... */
        unsigned char	req_fuel;		/* 218, require fuel */
        unsigned char	divider;		/* 219, IRQ / factor for pulse*/
        union tblcnf	tblcnf;			/* 220, inj config per table */
        unsigned char	inj_open_time;		/* 221, inj open time */
        unsigned char	inj_ocfuel;		/* 222, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        unsigned char	pwm_curr_lim;		/* 223, curr limit PWM duty 
						 * cycle
 						 */
        unsigned char	pwm_time_max;		/* 224, Peak hold time */
        unsigned char	batt_corr;		/* 225, Batt Voltage Corr */
        unsigned short	rpmk;			/* 226, 12K/ncyl */
        unsigned char	rpm_bins[8];		/* 228, VEtable RPM bins */
        unsigned char	load_bins[8];		/* 236, VEtable KPA bins */
        union config11 	config11;		/* 244, Config for PC Config */
        union config12  config12;		/* 245, Config for PC Config */
	unsigned char 	unused246;		/* 246, Unused  */
	unsigned char	bcfreqdiv;		/* 247, PWM rate for boost */
						/* 0-ERROR, 1=39hz, 2=19 Hz */
						/* 3 = 10 Hz */
	unsigned char	bcupdate;		/* 248, milliseconds count for 
						 * controller algorithm */
	unsigned char	bcpgain;		/* 249, proportional Gain % */
	unsigned char	bcdgain;		/* 250, derivative Gain % */
	unsigned char	revlimit;		/* 251, maxrev limit rpm/100 */ 
	unsigned char	launchlimit;		/* 252, launch control revlim */
	unsigned char	shiftlo;		/* 253, lo shift light thresh */
	unsigned char	shifthi;		/* 254, hi shift light thresh */
	unsigned char	crank_rpm;		/* 255, rpm/100 */
};

#endif
