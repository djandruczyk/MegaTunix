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

#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <glib.h>


#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */


/* Megasquirt constants defined in C provided by Perry Harrington */
 
/* If you are using GCC, the bitfield order is MSB first.  
 * Other compilers may be LSB first 
 */

#ifdef MSB_BITFIELD /* PDA's possibly? */
union squirt 
{
	unsigned char	value;
	struct 
	{
		unsigned char reserved	:2;
		unsigned char firing2	:1;	/* 0 = not squirting 1 = squirting */
		unsigned char sched2	:1;	/* 0 = nothing scheduled 1 = scheduled to squirt */
		unsigned char firing1	:1;	/* 0 = not squirting 1 = squirting */
		unsigned char sched1	:1;	/* 0 = nothing scheduled 1 = scheduled to squirt */
		unsigned char inj2	:1;	/* 0 = no squirt 1 = squirt */
		unsigned char inj1	:1;	/* 0 = no squirt 1 = squirt */
	} bit;
};
 
union engine 
{
	unsigned char      value;
	struct 
	{
		unsigned char reserved	:1;
		unsigned char mapaen	:1;	/* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */
		unsigned char tpsden	:1;	/* 0 = not in deacceleration mode 1 = in deacceleration mode */
		unsigned char tpsaen	:1;	/* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
		unsigned char warmup	:1;	/* 0 = not in warmup 1 = in warmup */
		unsigned char startw	:1;	/* 0 = not in startup warmup 1 = in warmup enrichment */
		unsigned char crank	:1;	/* 0 = engine not cranking 1 = engine cranking */
		unsigned char running	:1;	/* 0 = engine not running 1 = running */
	} bit;
};
#else		/* LSB first architectures, x86 */
union squirt 
{
	unsigned char      value;
	struct 
	{
		unsigned char inj1	:1;	/* 0 = no squirt 1 = squirt */
		unsigned char inj2	:1;	/* 0 = no squirt 1 = squirt */
		unsigned char sched1	:1;	/* 0 = nothing scheduled 1 = scheduled to squirt */
		unsigned char firing1	:1;	/* 0 = not squirting 1 = squirting */
		unsigned char sched2	:1;	/* 0 = nothing scheduled 1 = scheduled to squirt */
		unsigned char firing2	:1;	/* 0 = not squirting 1 = squirting */
		unsigned char reserved	:2;
	} bit;
};
 
union engine 
{
	unsigned char      value;
	struct 
	{
		unsigned char running	:1;	/* 0 = engine not running 1 = running */
		unsigned char crank	:1;	/* 0 = engine not cranking 1 = engine cranking */
		unsigned char startw	:1;	/* 0 = not in startup warmup 1 = in warmup enrichment */
		unsigned char warmup	:1;	/* 0 = not in warmup 1 = in warmup */
		unsigned char tpsaen	:1;	/* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
		unsigned char tpsden	:1;	/* 0 = not in deacceleration mode 1 = in deacceleration mode */
		unsigned char mapaen	:1;	/* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */
		unsigned char reserved	:1;
	} bit;
};
#endif
 
struct ms_raw_data_v1_and_v2 
{	/* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into ms_data_v1_and_v2 (struct)
	 */
        unsigned char	secl;
        union squirt	squirt;
        union engine	engine;
        unsigned char	baro;
        unsigned char	map;
        unsigned char	mat;
        unsigned char	clt;
        unsigned char	tps;
        unsigned char	batt;
        unsigned char	ego;
        unsigned char	egocorr;
        unsigned char	aircorr;
        unsigned char	warmcorr;
        unsigned char	rpm;
        unsigned char	pw;
        unsigned char	tpsaccel;
        unsigned char	barocorr;
        unsigned char	gammae;
        unsigned char	vecurr;
        unsigned char	bspot1;
        unsigned char	bspot2;
        unsigned char	bspot3;
};

struct ms_data_v1_and_v2 {      
	unsigned char	secl;		/* low seconds - from 0 to 255, then rollover */
	union squirt	squirt;		/* Event variable bit field for Injector Firing */
	union engine	engine;		/* Variable bit-field to hold engine current status */
	unsigned char	baro;		/* Barometer ADC Raw Reading - KPa (0 - 255) */
	unsigned char	map;		/* Manifold Absolute Pressure ADC Raw Reading - KPa (0 - 255) */
	unsigned char	mat;		/* Manifold Air Temp converted via lookuptable */
	unsigned char	clt;		/* Coolant Temperature converted via lookuptable */
	unsigned char	tps;		/* Throttle Position Sensor open percentage (0-100) */
	float		batt;		/* Battery Voltage ADC Raw Reading - converted to volts */
	float		ego;		/* Exhaust Gas Oxygen ADC Raw Reading - converted to volts */
	unsigned char	egocorr;	/* Oxygen Sensor Correction */
	unsigned char	aircorr;	/* Air Density Correction lookup - percent */
	unsigned char	warmcorr;	/* Total Warmup Correction - percent */
	unsigned short	rpm;		/* Computed engine RPM - rpm */
	float		pw;		/* injector squirt time in millesec (0 to 25.5 millisec) - applied */
	unsigned char	tpsaccel;	/* Acceleration enrichment - percent */
	unsigned char	barocorr;	/* Barometer Lookup Correction - percent */
	unsigned char	gammae;		/* Total Gamma Enrichments - percent */
	unsigned char	vecurr;		/* Current VE value from lookup table - percent */
	unsigned char	bspot1;		/* Blank Spot 1 */
	unsigned char	bspot2;		/* Blank Spot 2 */
	unsigned char	bspot3;		/* Blank Spot 3 */
};

union config11 
{
	unsigned char      value;
	struct 
	{
		unsigned char map_type	:2;	/* 00 115KPA, 01-250kpa, 10,11
						 * user-defined */
		unsigned char eng_type	:1;	/* 0 = 4-stroke, 1 = 2-stroke */
		unsigned char inj_type	:1;	/* 0 = multi-port, 1 = TBI */
		unsigned char cylinders	:4;	/* 0000 = 1 cyl 
						 * 0001 = 2 cyl
						 * 0010 = 3 cyl
						 * 0011 = 4 cyl
						 * 0100 = 5 cyl
						 * 0101 = 6 cyl
						 * 0110 = 7 cyl
						 * 0111 = 8 cyl
						 * 1000 = 9 cyl
						 * 1001 = 10 cyl
						 * 1010 = 11 cyl
						 * 1011 = 12 cyl
						 */
	} bit;
};

union config12
{
	unsigned char      value;
	struct 
	{
		unsigned char clt_type	:2;	/* 00 = GM, 
						 * 01,10,11 = user defined */
		unsigned char mat_type	:2;	/* 00 = GM,
						 * 01,10,11 = user defined */
		unsigned char injectors	:4;	/* 0000 = 1 injector 
						 * 0001 = 2 injectors
						 * 0010 = 3 injectors
						 * 0011 = 4 injectors
						 * 0100 = 5 injectors
						 * 0101 = 6 injectors
						 * 0110 = 7 injectors
						 * 0111 = 8 injectors
						 * 1000 = 9 injectors
						 * 1001 = 10 injectors
						 * 1010 = 11 injectors
						 * 1011 = 12 injectors
						 */
	} bit;
};

union config13
{
	unsigned char      value;
	struct 
	{
		unsigned char firing	:1;	/* 0 = normal, 1=odd fire */
		unsigned char ego_type	:1;	/* 0 = narrow, 1=wide */
		unsigned char inj_strat	:1;	/* 0 = SD, 1 = Alpha-N */
		unsigned char baro_corr	:1;	/* 0 = Enrichment off (100%)
						 * 1 = Enrichment on 
						 */
	} bit;
};

struct ms_ve_constants
{
	/* TYPE          Variable              Offset,  Comment */
	unsigned char	ve_bins[64];		/* 0, VE table, 64 bytes */
	unsigned char	cr_pulse_neg40;		/* 65, crank pulse at -40 deg F */
	unsigned char	cr_pulse_pos170;	/* 66, crank pulse at 170 deg F */
	unsigned char	as_enrich;		/* 67, Enrich over base (%) */
	unsigned char	as_num_cycles;		/* 68, enrich for X cycles */
	unsigned char	warmup_bins[10];	/* 69, Warmup bins (10 bytes) */
	unsigned char	accel_bins[4];		/* 79, TPS Accel bins, 4 bytes */
	unsigned char	cold_accel_addon;	/* 83, Accel addon at -40 deg F */
	unsigned char	tps_trig_thresh;	/* 84, TPS trig thresh in V/sec */
	unsigned char	accel_duration;		/* 85, Accel for how many seconds */
	unsigned char	decel_cut;		/* 86, decel fuel cut % */
	unsigned char	ego_temp_active;	/* 87, temp when EGO corr is on */
	unsigned char	ego_events;		/* 88, ign events between steps */
	unsigned char	ego_step;		/* 89, correction % */
	unsigned char	ego_limit;		/* 90, +/- limit */
	unsigned char	req_fuel_1;		/* 91, require fuel */
	unsigned char	divider;		/* 92, IRQ divide factor for pulse*/
	unsigned char	alternate;		/* 93, alternate inj drivers */
	unsigned char	inj_open_time;		/* 94, inj open time */
	unsigned char	inj_ocfuel;		/* 95, PW-correlated amount of fuel
						 * injected during open 
						 */
	unsigned char	pwm_curr_lim;		/* 96, curr limit PWM duty cycle */
	unsigned char	pwm_time_max;		/* 97, Peak hold time */
	unsigned char	batt_corr;		/* 98, Battry Voltage Correction */
	unsigned short	rpmk;			/* 99, Constant for RPM 12K/ncyl */
	unsigned char	rpm_bins[8];		/* 101, VEtable RPM bins (8 bytes) */
	unsigned char	kpa_bins[8];		/* 109, VEtable KPA bins (8 bytes) */
	union	config11 config11;		/* 117, Config for PC Config */
	union	config12 config12;		/* 118, Config for PC Config */
	union	config13 config13;		/* 119, Config for PC Config */
	unsigned char	cr_priming_pulse;	/* 120, priming pulse b4 startup */
	unsigned char	ego_rpm_active;		/* 121, EGO RPM trigger voltage */
	unsigned char	fast_idle_thresh;	/* 122, fast idle temp thresh */
	unsigned char	ego_sw_voltage;		/* 123, EGO flip point voltage */
	unsigned char	cold_accel_mult;	/* 124, Cold Accel mult factor */
	unsigned char	pad1;			/* 125, Padding to 128 bytes */
	unsigned char	pad2;			/* 126, Padding to 128 bytes */
	unsigned char	pad3;			/* 127, Padding to 128 bytes */
	unsigned char	pad4;			/* 128, Padding to 128 bytes */
}; 
		
/* Definitions */
/* Gui frames */
#define ABOUT_PAGE		0x01
#define GENERAL_PAGE		0x02
#define COMMS_PAGE		0x03
#define CONSTANTS_PAGE		0x04
#define RUNTIME_PAGE		0x05
#define ENRICHMENTS_PAGE	0x06
#define VETABLE_PAGE		0x07
#define TUNING_PAGE		0x08
#define TOOLS_PAGE		0x09
#define DATALOGGING_PAGE	0x0a

/* Serial I/O READ case handling */
#define REALTIME_VARS		0x20
#define VE_AND_CONSTANTS	0x21
 
/* Buttons */
#define START_REALTIME		0x30
#define STOP_REALTIME		0x31
#define REQD_FUEL_POPUP		0x32
#define READ_FROM_MS		0x33
#define WRITE_TO_MS		0x34

/* Spinbuttons */
#define REQ_FUEL_DISP		0x40
#define REQ_FUEL_CYLS		0x41
#define REQ_FUEL_INJ_RATE	0x42
#define REQ_FUEL_AFR		0x43
#define REQ_FUEL_1		0x44
#define REQ_FUEL_2		0x45
#define	INJ_OPEN_TIME		0x46
#define	BATT_CORR		0x47
#define	PWM_CUR_LIM		0x48
#define	PWM_TIME_THRES		0x49
#define	FAST_IDLE_THRES		0x4a
#define CRANK_PULSE_NEG_40	0x4b
#define CRANK_PULSE_170		0x4c
#define CRANK_PRIMING_PULSE	0x4d
#define AFTERSTART_ENRICH	0x4e
#define AFTERSTART_NUM_CYCLES	0x4f
#define	SER_POLL_TIMEO		0x50
#define	SER_INTERVAL_DELAY	0x51
#define SET_SER_PORT		0x52

/* text entries */
#define WARMUP_NEG_40		0x70
#define WARMUP_NEG_20		0x71
#define WARMUP_0		0x72
#define WARMUP_20		0x73
#define WARMUP_40		0x74
#define WARMUP_60		0x75
#define WARMUP_80		0x76
#define WARMUP_100		0x77
#define WARMUP_130		0x78
#define WARMUP_160		0x79
#define MS_RESET_COUNT		0x7a
#define MS_SER_ERRCOUNT		0x7b

/* Configfile structs. (derived from an older version of XMMS) */

#ifndef MEGASQUIRT_LIN_CONFIGFILE_H
#define MEGASQUIRT_LIN_CONFIGFILE_H

#include <glib.h>

typedef struct
{
        gchar *key;
        gchar *value;
}
ConfigLine;

typedef struct
{
        gchar *name;
        GList *lines;
}
ConfigSection;
typedef struct
{
        GList *sections;
}
ConfigFile;
#endif 

#endif
