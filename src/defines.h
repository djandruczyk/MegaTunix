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

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <glib.h>


#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */


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
