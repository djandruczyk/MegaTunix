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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <config.h>
#include <sys/types.h>
#include <termios.h>
#include <gtk/gtk.h>

struct Serial_Params
{
	int fd;			/* File descriptor */
	int comm_port;		/* DOS/Windows COM port number, 1-8 typically */
	int open;		/* flag, 1 for open 0 for closed */
	int poll_timeout;	/* Pollng interval in MILLISECONDS */
	int read_wait;		/* time delay between each read */
	int raw_bytes;		/* number of bytes to read for realtime vars */
	int veconst_size;	/* Size of VEtable/constants datablock */
	struct termios oldtio;	/* serial port settings before we touch it */
	struct termios newtio;	/* serial port settings we use when running */
	int errcount;		/* Serial I/O errors read error count */
} serial_params;
/* Megasquirt constants defined in C provided by Perry Harrington */
 
/* If you are using GCC, the bitfield order is MSB first.  
 * Other compilers may be LSB first 
 */

#ifdef MSB_BITFIELD /* PDA's possibly? */
union squirt 
{
	unsigned char   value;
	struct 
	{
		unsigned char reserved  :2;
		unsigned char firing2   :1;     /* 0 = not squirting 1 = squirti
						   ng */
		unsigned char sched2    :1;     /* 0 = nothing scheduled 1 = sch
						   eduled to squirt */
		unsigned char firing1   :1;     /* 0 = not squirting 1 = squirti
						   ng */
		unsigned char sched1    :1;     /* 0 = nothing scheduled 1 = sch
						   eduled to squirt */
		unsigned char inj2      :1;     /* 0 = no squirt 1 = squirt */
		unsigned char inj1      :1;     /* 0 = no squirt 1 = squirt */
	} bit;
};
union engine 
{
	unsigned char      value;
	struct 
	{
		unsigned char reserved  :1;
		unsigned char mapaen    :1;     /* 0 = not in MAP acceleration m
						   ode 1 = MAP deaceeleration mode */
		unsigned char tpsden    :1;     /* 0 = not in deacceleration mod
						   e 1 = in deacceleration mode */
		unsigned char tpsaen    :1;     /* 0 = not in TPS acceleration m
						   ode 1 = TPS acceleration mode */
		unsigned char warmup    :1;     /* 0 = not in warmup 1 = in warm
						   up */
		unsigned char startw    :1;     /* 0 = not in startup warmup 1 =
						   in warmup enrichment */
		unsigned char crank     :1;     /* 0 = engine not cranking 1 = e
						   ngine cranking */
		unsigned char running   :1;     /* 0 = engine not running 1 = ru
						   nning */
	} bit;
};
#else           /* LSB first architectures, x86 */
union squirt 
{
	unsigned char      value;
	struct 
	{
		unsigned char inj1      :1;     /* 0 = no squirt 1 = squirt */
		unsigned char inj2      :1;     /* 0 = no squirt 1 = squirt */
		unsigned char sched1    :1;     /* 0 = nothing scheduled 1 = sch
						   eduled to squirt */
		unsigned char firing1   :1;     /* 0 = not squirting 1 = squirti
						   ng */
		unsigned char sched2    :1;     /* 0 = nothing scheduled 1 = sch
						   eduled to squirt */
		unsigned char firing2   :1;     /* 0 = not squirting 1 = squirti
						   ng */
		unsigned char reserved  :2;
	} bit;
};
 
union engine 
{
	unsigned char      value;
	struct 
	{
		unsigned char running   :1;     /* 0 = engine not running 1 = running */
		unsigned char crank     :1;     /* 0 = engine not cranking 1 = engine cranking */
		unsigned char startw    :1;     /* 0 = not in startup warmup 1 = in warmup enrichment */
		unsigned char warmup    :1;     /* 0 = not in warmup 1 = in warmup */
		unsigned char tpsaen    :1;     /* 0 = not in TPS acceleration mode 1 = TPS acceleration mode */
		unsigned char tpsden    :1;     /* 0 = not in deacceleration mode 1 = in deacceleration mode */
		unsigned char mapaen    :1;     /* 0 = not in MAP acceleration mode 1 = MAP deaceeleration mode */
						   
		unsigned char reserved  :1;
	} bit;
};
#endif
 
struct ms_raw_data_v1_and_v2 
{       /* This is RAW data that comes in via serial from the MegaSquirt
	 * these values will be modified by post_process():
	 * and fed into ms_data_v1_and_v2 (struct)
	 */
	unsigned char   secl;
	union squirt    squirt;
	union engine    engine;
	unsigned char   baro;
	unsigned char   map;
	unsigned char   mat;
	unsigned char   clt;
	unsigned char   tps;
	unsigned char   batt;
	unsigned char   ego;
	unsigned char   egocorr;
	unsigned char   aircorr;
	unsigned char   warmcorr;
	unsigned char   rpm;
	unsigned char   pw;
	unsigned char   tpsaccel;
	unsigned char   barocorr;
	unsigned char   gammae;
	unsigned char   vecurr;
	unsigned char   bspot1;
	unsigned char   bspot2;
	unsigned char   bspot3;
};
struct ms_data_v1_and_v2 
{      
        unsigned char   secl;		/* low seconds - from 0 to 255, then rollover */
        union squirt    squirt;		/* Event variable bit field for Injector Firing */
        union engine    engine;		/* Variable bit-field to hold engine current status */
        unsigned char   baro;		/* Barometer ADC Raw Reading - KPa (0 - 255) */
        unsigned char   map;		/* Manifold Absolute Pressure ADC Raw Reading - KPa (0 - 255) */
        unsigned char   mat;		/* Manifold Air Temp converted via lookuptable */
        unsigned char   mat_volt;	/* Manifold Air Temp ADC voltage(0-5) */
        unsigned char   clt;		/* Coolant Temperature converted via lookuptable */
        unsigned char   clt_volt;	/* Coolant Temp ADC voltage (0-5) */
        unsigned char   tps;		/* Throttle Position Sensor open percentage (0-100) */
        unsigned char   tps_volt;	/* Throttle Pos ADC voltage (0-5) */
        float           batt;		/* Battery Voltage ADC Raw Reading - converted to volts */
        float           ego;		/* Exhaust Gas Oxygen ADC Raw Reading - converted to volts */
        unsigned char   egocorr;	/* Oxygen Sensor Correction */
        unsigned char   aircorr;	/* Air Density Correction lookup - percent */
        unsigned char   warmcorr;	/* Total Warmup Correction - percent */
        unsigned short  rpm;		/* Computed engine RPM - rpm */
        float           pw;		/* injector squirt time in millesec (0 to 25.5 millisec) - applied */
	float		dcycle;		/* Injector duty cycle */
        unsigned char   tpsaccel;	/* Acceleration enrichment - percent */
        unsigned char   barocorr;	/* Barometer Lookup Correction - percent */
        unsigned char   gammae;		/* Total Gamma Enrichments - percent */
        unsigned char   vecurr;		/* Current VE value from lookup table - percent */
        unsigned char   bspot1;		/* Blank Spot 1 */
        unsigned char   bspot2;		/* Blank Spot 2 */
        unsigned char   bspot3;		/* Blank Spot 3 */
};
union config11 
{
        unsigned char      value;
        struct 
        {
                unsigned char map_type  :2;     /* 00 115KPA, 01-250kpa, 10,11
                                                 * user-defined */
                unsigned char eng_type  :1;     /* 0 = 4-stroke, 1 = 2-stroke */
                unsigned char inj_type  :1;     /* 0 = multi-port, 1 = TBI */
                unsigned char cylinders :4;     /* 0000 = 1 cyl 
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
                unsigned char clt_type  :2;     /* 00 = GM, 
                                                 * 01,10,11 = user defined */
                unsigned char mat_type  :2;     /* 00 = GM,
                                                 * 01,10,11 = user defined */
                unsigned char injectors :4;     /* 0000 = 1 injector 
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
                unsigned char firing    :1;     /* 0 = normal, 1=odd fire */
                unsigned char ego_type  :1;     /* 0 = narrow, 1=wide */
                unsigned char inj_strat :1;     /* 0 = SD, 1 = Alpha-N */
                unsigned char baro_corr :1;     /* 0 = Enrichment off (100%)
                                                 * 1 = Enrichment on 
                                                 */
        } bit;
};

struct ms_ve_constants
{
        /* TYPE          Variable              Offset,  Comment */
        unsigned char   ve_bins[64];            /* 0, VE table, 64 bytes */
        unsigned char   cr_pulse_neg40;         /* 64, cr pulse at -40 deg F */
        unsigned char   cr_pulse_pos170;        /* 65, cr pulse at 170 deg F */
        unsigned char   as_enrich;              /* 66, Enrich over base (%) */
        unsigned char   as_num_cycles;          /* 67, enrich for X cycles */
        unsigned char   warmup_bins[10];        /* 68, Warmup bins  */
        unsigned char   accel_bins[4];          /* 78, TPS Accel bins */
        unsigned char   cold_accel_addon;       /* 82, Accel addon at -40deg */
        unsigned char   tps_trig_thresh;        /* 83, TPS trig thr in V/sec */
        unsigned char   accel_duration;         /* 84, Accel duration(secs) */
        unsigned char   decel_cut;              /* 85, decel fuel cut % */
        unsigned char   ego_temp_active;        /* 86, EGO activation pt */
        unsigned char   ego_events;             /* 87, ign events betw steps */
        unsigned char   ego_step;               /* 88, correction % */
        unsigned char   ego_limit;              /* 89, +/- limit */
        unsigned char   req_fuel;		/* 90, require fuel */
        unsigned char   divider;                /* 91, IRQ / factor for pulse*/
        unsigned char   alternate;              /* 92, alternate inj drivers */
        unsigned char   inj_open_time;          /* 93, inj open time */
        unsigned char   inj_ocfuel;             /* 94, PW-correlated amount of 
						 * fuel injected during open 
                                                 */
        unsigned char   pwm_curr_lim;           /* 95, curr limit PWM duty 
						 * cycle
 						 */
        unsigned char   pwm_time_max;           /* 96, Peak hold time */
        unsigned char   batt_corr;              /* 97, Batt Voltage Corr */
        unsigned short  rpmk;                   /* 98, 12K/ncyl */
        unsigned char   rpm_bins[8];            /* 100, VEtable RPM bins */
        unsigned char   kpa_bins[8];            /* 108, VEtable KPA bins */

        union   config11 config11;              /* 116, Config for PC Config */
        union   config12 config12;              /* 117, Config for PC Config */
        union   config13 config13;              /* 118, Config for PC Config */
        unsigned char   cr_priming_pulse;       /* 119, priming pulse b4 start*/
        unsigned char   ego_rpm_active;         /* 120, EGO RPM trigger volt  */
        unsigned char   fast_idle_thresh;       /* 121, fast idle temp thresh */
        unsigned char   ego_sw_voltage;         /* 122, EGO flip pt voltage */
        unsigned char   cold_accel_mult;        /* 123, Cold Accel * factor */
        unsigned char   pad1;                   /* 124, Padding to 128 bytes */
        unsigned char   pad2;                   /* 125, Padding to 128 bytes */
        unsigned char   pad3;                   /* 126, Padding to 128 bytes */
        unsigned char   pad4;                   /* 127, Padding to 128 bytes */
}; 

#endif
