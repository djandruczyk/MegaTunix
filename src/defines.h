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

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */


/* Definitions */

/* Memroy offsets */
#define VE_TABLE_OFFSET		0
#define WARMUP_BINS_OFFSET	68
#define ACCEL_BINS_OFFSET	78
#define RPM_BINS_OFFSET		100
#define KPA_BINS_OFFSET		108

/* Gui frames */
#define ABOUT_PAGE		0x01
#define GENERAL_PAGE		0x02
#define COMMS_PAGE		0x03
#define CONSTANTS_PAGE		0x04
#define RUNTIME_PAGE		0x05
#define ENRICHMENTS_PAGE	0x06
#define VETABLES_PAGE		0x07
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
#define SELECT_LOGFILE		0x35
#define TRUNCATE_LOGFILE	0x36
#define CLOSE_LOGFILE		0x37
#define START_DATALOGGING	0x38
#define STOP_DATALOGGING	0x39
#define CLASSIC_LOG		0x3a
#define CUSTOM_LOG		0x3b
#define COMMA			0x3c
#define TAB			0x3d
#define SPACE			0x3e

/* Spinbuttons */
#define REQ_FUEL_DISP		0x40
#define REQ_FUEL_CYLS		0x41
#define REQ_FUEL_INJ_RATE	0x42
#define REQ_FUEL_AFR		0x43
#define REQ_FUEL		0x44
//#define REQ_FUEL_2		0x45
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
#define TPS_TRIG_THRESH		0x53
#define ACCEL_ENRICH_DUR	0x54
#define COLD_ACCEL_ENRICH	0x55
#define COLD_ACCEL_MULT		0x56
#define DECEL_CUT		0x57
#define EGO_TEMP_ACTIVE		0x58
#define EGO_RPM_ACTIVE		0x59
#define EGO_SW_VOLTAGE		0x5a
#define EGO_STEP		0x5b
#define EGO_EVENTS		0x5c
#define EGO_LIMIT		0x5d
#define NUM_SQUIRTS		0x5e
#define NUM_CYLINDERS		0x5f
#define NUM_INJECTORS		0x60

/* text entries */
#define MS_RESET_COUNT		0x7a
#define MS_SER_ERRCOUNT		0x7b

/* Group classes */
/* Classes are used for widget groups, i.e. the VEtable, RPM bins, etc... */
#define	ACCEL			0x80
#define RPM			0x81
#define KPA			0x82
#define VE			0x83
#define WARMUP			0x84

/* Download modes */
#define IMMEDIATE		0x90
#define DEFERRED		0x91


#endif
