/*
  Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 
  Linux Megasquirt tuning software
  
  
  This software comes under the GPL (GNU Public License)
  You may freely copy,distribute, etc. this as long as all the source code
  is made available for FREE.
  
  No warranty is made or implied. You use this program at your own risk.
 */

/*! Global Variables */

#ifndef __MS_STRUCTURES_H__
#define __MS_STRUCTURES_H__

#include <config.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <unions.h>

/*! 
 \brief The Raw_Runtime_Std structure is mapped to the buffer returned from the
 ECU on a realtime read for access to some of the members of the structure.
 \warning This structure will be deprecated soon, as realtiem data is handled
 in a differrent manner since 0.6.0
 */
struct Raw_Runtime_Std
{
	guchar	secl;		/*! Offset 0, ms_clock */
	union	squirt squirt;	/*! Offset 1, squirt status */
	union	engine engine;	/*! Offset 2, engine status */
	guchar	baro;		/*! Offset 3, baro reading */
	guchar	map;		/*! Offset 4, map reading */
	guchar	mat;		/*! Offset 5, mat reading */
	guchar	clt;		/*! Offset 6, clt reading */
	guchar	tps;		/*! Offset 7, tps reading */
	guchar	batt;		/*! Offset 8, bat reading */
	guchar	ego;		/*! Offset 9, o2 reading */
	guchar	egocorr;	/*! Offset 10 o2 correction factor */
	guchar	aircorr;	/*! Offset 11, mat correction factor */
	guchar	warmcorr;	/*! Offset 12, clt correction factor */
	guchar	rpm;		/*! Offset 13, rpm */
	guchar	pw1;		/*! Offset 14, pulsewidth */
	guchar	tpsaccel;	/*! Offset 15, accel enrich amount */
	guchar	barocorr;	/*! Offset 16, baro correction */
	guchar	gammae;		/*! Offset 17, sum of all enrichments */
	guchar	vecurr1;	/*! Offset 18, ve currently */
	guchar	bspot1;		/*! Offset 19, blank spot 1 */
	guchar	bspot2;		/*! Offset 20, blank spot 2 */
	guchar	bspot3;		/*! Offset 21, blank spot 3 */
	/*! NOTE: the last 3 are used diffeently in the other MS variants
	 * like the dualtable, squirtnspark and such...
	 */
};

struct Ve_Const_Std
{
        /*! TYPE          Variable              Offset,  Comment */
        guchar	ve_bins[64];		/*! 0, VE table, 64 bytes */
        guchar	cr_pulse_neg40;		/*! 64, cr pulse at -40 deg F */
        guchar	cr_pulse_pos170;	/*! 65, cr pulse at 170 deg F */
        guchar	as_enrich;		/*! 66, Enrich over base (%) */
        guchar	as_num_cycles;		/*! 67, enrich for X cycles */
        guchar	warmup_bins[10];	/*! 68, Warmup bins  */
        guchar	accel_bins[4];		/*! 78, TPS Accel bins */
        guchar	cold_accel_addon;	/*! 82, Accel addon at -40deg */
        guchar	tps_trig_thresh;	/*! 83, TPS trig thr in V/sec */
        guchar	accel_duration;		/*! 84, Accel duration(secs) */
        guchar	decel_cut;		/*! 85, decel fuel cut % */
        guchar	ego_temp_active;	/*! 86, EGO activation pt */
        guchar	ego_events;		/*! 87, ign events betw steps */
        guchar	ego_step;		/*! 88, correction % */
        guchar	ego_limit;		/*! 89, +/- limit */
        guchar	req_fuel;		/*! 90, require fuel */
        guchar	divider;		/*! 91, IRQ / factor for pulse*/
        guchar	alternate;		/*! 92, alternate inj drivers */
        guchar	inj_open_time;		/*! 93, inj open time */
        guchar	inj_ocfuel;		/*! 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        guchar	pwm_curr_lim;		/*! 95, curr limit PWM duty cycle */
        guchar	pwm_time_max;		/*! 96, Peak hold time */
        guchar	batt_corr;		/*! 97, Batt Voltage Corr */
        gshort	rpmk;			/*! 98, 12K/ncyl */
        guchar	rpm_bins[8];		/*! 100, VEtable RPM bins */
        guchar	load_bins[8];		/*! 108, VEtable KPA bins */

        union	config11 config11;	/*! 116, Config for PC Config */
        union	config12 config12;	/*! 117, Config for PC Config */
        union	config13 config13;	/*! 118, Config for PC Config */
        guchar	cr_priming_pulse;	/*! 119, priming pulse b4 start*/
        guchar	ego_rpm_active;		/*! 120, EGO RPM trigger volt  */
        guchar	fast_idle_thresh;	/*! 121, fast idle temp thresh */
        guchar	ego_sw_voltage;		/*! 122, EGO flip pt voltage */
        guchar	cold_accel_mult;	/*! 123, Cold Accel * factor */
        guchar	pad1;			/*! 124, Padding to 128 bytes */
        guchar	pad2;			/*! 125, Padding to 128 bytes */
        guchar	pad3;			/*! 126, Padding to 128 bytes */
        guchar	pad4;			/*! 127, Padding to 128 bytes */
}; 

	/*! VE table and Constants for Page 0 */
struct Ve_Const_DT_1
{
        /*! TYPE          Variable              Offset,  Comment */
        guchar	ve_bins[64];		/*! 0, VE table, 64 bytes */
        guchar	cr_pulse_neg40;		/*! 64, cr pulse at -40 deg F */
        guchar	cr_pulse_pos170;	/*! 65, cr pulse at 170 deg F */
        guchar	as_enrich;		/*! 66, Enrich over base (%) */
        guchar   as_num_cycles;		/*! 67, enrich for X cycles */
        guchar	warmup_bins[10];	/*! 68, Warmup bins  */
        guchar	accel_bins[4];		/*! 78, TPS Accel bins */
        guchar	cold_accel_addon;	/*! 82, Accel addon at -40deg */
        guchar	tps_trig_thresh;	/*! 83, TPS trig thr in V/sec */
        guchar	accel_duration;		/*! 84, Accel duration(secs) */
        guchar	decel_cut;		/*! 85, decel fuel cut % */
        guchar	ego_temp_active;	/*! 86, EGO activation pt */
        guchar	ego_events;		/*! 87, ign events betw steps */
        guchar	ego_step;		/*! 88, correction % */
        guchar	ego_limit;		/*! 89, +/- limit */
        guchar	req_fuel;		/*! 90, require fuel */
        guchar	divider;		/*! 91, IRQ / factor for pulse*/
        union	tblcnf tblcnf;		/*! 92, inj config per table */
        guchar	inj_open_time;		/*! 93, inj open time */
        guchar	inj_ocfuel;		/*! 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        guchar	pwm_curr_lim;		/*! 95, curr limit PWM duty cycle */
        guchar	pwm_time_max;		/*! 96, Peak hold time */
        guchar	batt_corr;		/*! 97, Batt Voltage Corr */
        gshort	rpmk;			/*! 98, 12K/ncyl */
        guchar	rpm_bins[8];		/*! 100, VEtable RPM bins */
        guchar	load_bins[8];		/*! 108, VEtable KPA bins */

        union	config11 config11;	/*! 116, Config for PC Config */
        union	config12 config12;	/*! 117, Config for PC Config */
        union	config13 config13;	/*! 118, Config for PC Config */
        guchar	cr_priming_pulse;	/*! 119, priming pulse b4 start*/
        guchar	ego_rpm_active;		/*! 120, EGO RPM trigger volt  */
        guchar	fastidle_temp;		/*! 121, fast idle temp */
        guchar	ego_sw_voltage;		/*! 122, EGO flip pt voltage */
        guchar	cold_accel_mult;	/*! 123, Cold Accel * factor */
        guchar	slowidle_temp;		/*! 124, Dualtable */
        guchar	fastidlespd;		/*! 125, Fast idle speed RPM/10 */
        guchar	slowidlespd;		/*! 126, Slow idle speed RPM/10 */
        guchar	idlethresh;		/*! 127, TPS A/D count threshold BELOW which idle PWM is used */
}; 

	/*! VE table and Constants for Page 1 */
struct Ve_Const_DT_2
{
        /*! TYPE          Variable              Offset,  Comment */
        guchar	ve_bins[64];		/*! 128, VE table, 64 bytes */
	guchar	unused[26];		/*! 192, unused... */
        guchar	req_fuel;		/*! 218, require fuel */
        guchar	divider;		/*! 219, IRQ / factor for pulse*/
        guchar	unused_220;		/*! 220, inj config per table */
        guchar	inj_open_time;		/*! 221, inj open time */
        guchar	inj_ocfuel;		/*! 222, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        guchar	pwm_curr_lim;		/*! 223, curr limit PWM duty 
						 * cycle
 						 */
        guchar	pwm_time_max;		/*! 224, Peak hold time */
        guchar	batt_corr;		/*! 225, Batt Voltage Corr */
        gshort	rpmk;			/*! 226, 12K/ncyl */
        guchar	rpm_bins[8];		/*! 228, VEtable RPM bins */
        guchar	load_bins[8];		/*! 236, VEtable KPA bins */
        union	config11 config11;	/*! 244, Config for PC Config */
        union	config12 config12;	/*! 245, Config for PC Config */
	guchar	unused246;		/*! 246, Unused  */
	union	bcfreq bcfreq;		/*! 247, pwm rate for boost */ 
						/*! 0-ERROR, 1=39hz, 2=19 Hz */
						/*! 3 = 10 Hz */
	guchar	bcfreqdiv;		/*! 248, ??????? */
	guchar	bcpgain;		/*! 249, proportional Gain % */
	guchar	bcdgain;		/*! 250, derivative Gain % */
	guchar	revlimit;		/*! 251, maxrev limit rpm/100 */ 
	guchar	launchlimit;		/*! 252, launch control revlim */
	guchar	shiftlo;		/*! 253, lo shift light thresh */
	guchar	shifthi;		/*! 254, hi shift light thresh */
	guchar	crank_rpm;		/*! 255, rpm/100 */
};

#endif
