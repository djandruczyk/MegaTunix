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
	 * and fed into ms_data_v1 (struct)
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

/* text entries */
#define	SER_POLL_TIMEO		0x40
#define	SER_INTERVAL_DELAY	0x41
#define	INJ_OPEN_TIME		0x42
#define	BATT_CORR		0x43
#define	PWM_CUR_LIM		0x44
#define	PWM_TIME_THRES		0x45
#define	FAST_IDLE_THRES		0x46
#define CRANK_PULSE_NEG_40	0x47
#define CRANK_PULSE_170		0x48
#define CRANK_PRIMING_PULSE	0x49
#define AFTERSTART_ENRICH	0x4a
#define AFTERSTART_NUM_CYCLES	0x4b
#define WARMUP_NEG_40		0x4c
#define WARMUP_NEG_20		0x4d
#define WARMUP_0		0x4e
#define WARMUP_20		0x4f
#define WARMUP_40		0x50
#define WARMUP_60		0x51
#define WARMUP_80		0x52
#define WARMUP_100		0x53
#define WARMUP_130		0x54
#define WARMUP_160		0x55

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

