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
	REALTIME_VARS=0,
	VE_BLOCK,
	RAW_MEMORY_DUMP,
	C_TEST
}InputHandler;

/* Regular Buttons */
typedef enum
{
	START_REALTIME = 0x20,
	STOP_REALTIME,
	READ_VE_CONST,
	READ_RAW_MEMORY,
	BURN_MS_FLASH,
	CHECK_ECU_COMMS,
	INTERROGATE_ECU,
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
	TOOLTIPS_STATE=0x40,
        FAHRENHEIT,
        CELSIUS,
	COMMA,
	TAB,
	SPACE,
	REALTIME_VIEW,
	PLAYBACK_VIEW,
	HEX_VIEW,
	BINARY_VIEW,
	DECIMAL_VIEW,
}ToggleButton;

typedef enum
{
	MT_CLASSIC_LOG=0x50,
	MT_FULL_LOG,
	CUSTOM_LOG
}LoggingMode;
	
/* spinbuttons... */
typedef enum
{
	REQ_FUEL_DISP=0x60,
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
	CONV_ADD=0x80,
	CONV_SUB,
	CONV_MULT,
	CONV_DIV,
	CONV_NOTHING
}Conversions;

/* Runtime Status flags */
typedef enum 
{       
	STAT_CONNECTED, 
        STAT_CRANKING, 
        STAT_RUNNING, 
        STAT_WARMUP, 
        STAT_AS_ENRICH, 
        STAT_ACCEL, 
        STAT_DECEL
}RuntimeStatus;

typedef enum
{
	VE_EXPORT = 0xa0,
	VE_IMPORT,
	DATALOG_EXPORT,
	DATALOG_IMPORT,
	FULL_BACKUP,
	FULL_RESTORE
}FileIoType;


typedef enum
{	
	RED=0xb0,
	BLACK
}GuiState;

typedef enum
{
	HEADER=0xc0,
	PAGE,
	RANGE,
	TABLE
}ImportParserFunc;

typedef enum
{
	EVEME=0xd0,
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
	FONT=0xe0,
	TRACE,
	GRATICULE
}GcType;

typedef enum
{
	KPA=0xf0,
	RPM
}TableType;

typedef enum
{	/* up to 32 Capability flags.... */
	/* No capabilities == Standard B&G code with no modifications */
	STANDARD	= 1<<0,
	DUALTABLE	= 1<<1,
	S_N_SPARK	= 1<<2,
	S_N_EDIS	= 1<<3,
	ENHANCED	= 1<<4,
	RAW_MEMORY	= 1<<5,
	IAC_PWM		= 1<<6,
	IAC_STEPPER	= 1<<7,
	BOOST_CTRL	= 1<<8,
	OVERBOOST_SFTY	= 1<<9,
	LAUNCH_CTRL	= 1<<10,
}Capabilities;

typedef enum
{
	MTX=0x100,
	MT_CLASSIC,
	MT_FULL,
	MT_RAW
}LogType;

typedef enum
{
	NO_DEBUG 	= 0,
	INTERROGATOR 	= 1<<0,
	OPENGL		= 1<<1,
	DL_CONV		= 1<<2,
	UL_CONV		= 1<<3,
	SERIAL_WR	= 1<<4,
	SERIAL_RD	= 1<<5,
	SERIAL_GEN	= 1<<6,
	IO_PROCESS	= 1<<7,
	THREADS		= 1<<8,
	REQD_FUEL	= 1<<9,
	TABLOADER	= 1<<10,
	CRITICAL	= 1<<11
}Dbg_Class;

typedef enum
{
        VNUM = 0x110,
        EXTVER,
        SIG
}StoreType;

typedef enum
{
	BURN_CMD = 0x120,
	READ_CMD,
	WRITE_CMD,
	COMMS_TEST,
	INTERROGATION
}CmdType;

typedef enum
{
	IO_REALTIME_READ=0x130,
	IO_INTERROGATE_ECU,
	IO_COMMS_TEST,
	IO_READ_VE_CONST,
	IO_READ_RAW_MEMORY,
	IO_BURN_MS_FLASH,
	IO_WRITE_DATA,
}IoCommands;

typedef enum
{
	UPD_REALTIME = 0x140,
	UPD_LOGVIEWER,
	UPD_DATALOGGER,
	UPD_VE_CONST,
	UPD_READ_VE_CONST,
	UPD_RAW_MEMORY,
	UPD_STORE_BLACK,
	UPD_LOAD_GUI_TABS,
}UpdateFunctions;

typedef enum
{
	MTX_INT = 0x150,
	MTX_ENUM,
	MTX_BOOL,
	MTX_STRING,
}DataTypes;

typedef enum
{
	ABOUT_PAGE=0x160,
	GENERAL_PAGE,
	COMMS_PAGE,
	ENG_VITALS_PAGE,
	CONSTANTS_PAGE,
	DT_PARAMS_PAGE,
	IGNITON_PAGE,
	RUNTIME_PAGE,
	TUNING_PAGE,
	TOOLS_PAGE,
	RAW_MEM_PAGE,
	WARMUP_WIZ_PAGE,
	DATALOGGING_PAGE,
	LOGVIEWER_PAGE,
}PageIdent;

typedef enum
{
	STORE_CTRL=1,
	TEMP_DEP,
	INTERDEP_1,
	INTERDEP_2,
}ListIndexes;

#endif
