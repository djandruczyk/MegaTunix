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

#ifndef __ENUMS_H__
#define __ENUMS_H__

/* Serial data comes in and handled by the following possibilities.
 * dataio.c uses these to determine which course of action to take...
 */
typedef enum
{
	REALTIME_VARS,
	VE_AND_CONSTANTS_1,
	VE_AND_CONSTANTS_2,
	IGNITION_VARS,
	RAW_MEMORY
}InputData;

/* Regular Buttons */
typedef enum
{
	START_REALTIME,
	STOP_REALTIME,
	READ_VE_CONST,
	BURN_MS_FLASH,
	SELECT_DLOG_EXP,
	SELECT_DLOG_IMP,
	CLOSE_LOGFILE,
	START_DATALOGGING,
	STOP_DATALOGGING,
	EXPORT_VETABLE,
	IMPORT_VETABLE,
	REVERT_TO_BACKUP,
	BACKUP_ALL,
	RESTORE_ALL,
	SELECT_PARAMS,
	REQD_FUEL_POPUP,
}StdButton;

/* Toggle/Radio buttons */
typedef enum
{
	TOOLTIPS_STATE,
        FAHRENHEIT,
        CELSIUS,
	COMMA,
	TAB,
	SPACE,
	REALTIME_VIEW,
	PLAYBACK_VIEW
}ToggleButton;

typedef enum
{
	MT_CLASSIC_LOG,
	MT_FULL_LOG,
	CUSTOM_LOG
}LoggingMode;
	
/* spinbuttons... */
typedef enum
{
	REQ_FUEL_DISP,
	REQ_FUEL_CYLS,
	REQ_FUEL_RATED_INJ_FLOW,
	REQ_FUEL_RATED_PRESSURE,
	REQ_FUEL_ACTUAL_PRESSURE,
	REQ_FUEL_AFR,
	REQ_FUEL_1,
	REQ_FUEL_2,
	SER_POLL_TIMEO,
	SER_INTERVAL_DELAY,
	SET_SER_PORT,
	NUM_SQUIRTS_1,
	NUM_SQUIRTS_2,
	NUM_CYLINDERS_1,
	NUM_CYLINDERS_2,
	NUM_INJECTORS_1,
	NUM_INJECTORS_2,
	TRIGGER_ANGLE,
	LOGVIEW_SCROLL,
	DEBUG_LEVEL,
	GENERIC
}SpinButton;

/* Conversions for download, converse on upload.. */
typedef enum
{
	ADD,
	SUB,
	MULT,
	DIV,
	NOTHING
}Conversions;

/* Runtime Status flags */
typedef enum 
{       
	CONNECTED, 
        CRANKING, 
        RUNNING, 
        WARMUP, 
        AS_ENRICH, 
        ACCEL, 
        DECEL
}RuntimeStatus;

typedef enum
{
	VE_EXPORT = 10,
	VE_IMPORT,
	DATALOG_EXPORT,
	DATALOG_IMPORT,
	FULL_BACKUP,
	FULL_RESTORE
}FileIoType;


typedef enum
{	
	RED,
	BLACK
}GuiState;

typedef enum
{
	HEADER,
	PAGE,
	RANGE,
	TABLE
}ImportParserFunc;

typedef enum
{
	EVEME,
	USER_REV,
	USER_COMMENT,
	DATE,
	TIME,
	RPM_RANGE,
	LOAD_RANGE,
	NONE
}ImportParserArg;

typedef enum
{
	FONT,
	TRACE,
	GRATICULE
}GcType;

typedef enum
{
	KPA,
	RPM
}TableType;

typedef enum
{	/* up to 32 Capability flags.... */
	/* No capabilities == Standard B&G code with no modifications */
	STD		= 0,
	DUALTABLE	= 1<<0,
	S_N_SPARK	= 1<<1,
	S_N_EDIS	= 1<<2,
	IAC_PWM		= 1<<3,
	IAC_STEPPER	= 1<<4,
	BOOST_CTRL	= 1<<5,
	OVERBOOST_SFTY	= 1<<6,
	LAUNCH_CTRL	= 1<<7,
	TEMP_DEP	= 1<<8,	/* Temp units dependancy, used by Rt_Controls */
	O2_DEP		= 1<<9	/* O2 scale dependancy, used by Rt_Controls */
}Capabilities;

typedef enum
{
	MTX,
	MT_CLASSIC,
	MT_FULL,
	MT_RAW
}LogType;

typedef enum
{
	ABOUT = 0,
	GENERAL,
	COMMS,
	ENG_VITALS,
	ECU_CONSTANTS,
	DT_OPTIONS,
	ENRICHMENTS,
	VETABLE_1,
	VETABLE_2,
	SPARK_TABLE,
	SPARK_SETTINGS,
	RUNTIME_DISP,
	TUNING,
	TOOLS,
	LOW_LEVEL,
	WARMUP_WIZARD,
	DATALOGGING,
	LOGVIEWER
}PageName;

typedef enum
{
	NO_DEBUG = 0,
	UNHANDELED_CTRL,
	OPENGL,
	IO_ERROR,
	DL_CONV,
	UL_CONV,
	DL_TRANS,
	MAX
}DebugLevel;

#endif
