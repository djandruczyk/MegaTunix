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
#define VEBLOCK_SIZE 128

/* Definitions */

/* Memory offsets */
#define VE1_TABLE_OFFSET	0	/* From page 0 boundary */
#define VE2_TABLE_OFFSET	0	/* From Page 1 boundary */
#define WARMUP_BINS_OFFSET	68	/* From page 0 boundary */
#define ACCEL_BINS_OFFSET	78	/* From page 0 boundary */
#define VE1_RPM_BINS_OFFSET	100	/* From page 0 boundary */
#define VE2_RPM_BINS_OFFSET	100	/* From Page 1 boundary */
#define VE1_KPA_BINS_OFFSET	108	/* From page 0 boundary */
#define VE2_KPA_BINS_OFFSET	108	/* From Page 1 boundary */

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
#define LOWLEVEL_PAGE		0x0b
#define IGNITION_PAGE		0x0c

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
#define TOOLTIPS_STATE		0x3f

/* Spinbuttons */
#define REQ_FUEL_DISP		0x40
#define REQ_FUEL_CYLS		0x41
#define REQ_FUEL_INJ_RATE	0x42
#define REQ_FUEL_AFR		0x43
#define REQ_FUEL		0x44
//#define REQ_FUEL_2		0x45
#define	SER_POLL_TIMEO		0x46
#define	SER_INTERVAL_DELAY	0x47
#define SET_SER_PORT		0x48
#define NUM_SQUIRTS		0x49
#define NUM_CYLINDERS		0x50
#define NUM_INJECTORS		0x51
#define GENERIC			0x52

/* Conversions */
#define ADD			0x60
#define SUB			0x61
#define MULT			0x62
#define DIV			0x63
/* text entries */
#define MS_RESET_COUNT		0x7a
#define MS_SER_ERRCOUNT		0x7b

/* Group classes */
/* Classes are used for widget groups, i.e. the VEtable, RPM bins, etc... */

/* Download modes */
#define IMMEDIATE		0x90
#define DEFERRED		0x91

/* Enumerations */
typedef enum 
{	CONNECTED, 
	CRANKING, 
	RUNNING, 
	WARMUP, 
	AS_ENRICH, 
	ACCEL, 
	DECEL
}RuntimeStatusType;

typedef enum
{
	FAHRENHEIT,
	CELSIUS
}UnitsType;


#endif
