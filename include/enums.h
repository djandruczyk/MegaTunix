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
	C_TEST,
	GET_ERROR,
	NULL_HANDLER,
}InputHandler;

/* Regular Buttons */
typedef enum
{
	START_REALTIME = 0x20,
	STOP_REALTIME,
	START_PLAYBACK,
	STOP_PLAYBACK,
	READ_VE_CONST,
	READ_RAW_MEMORY,
	BURN_MS_FLASH,
	CHECK_ECU_COMMS,
	INTERROGATE_ECU,
	OFFLINE_MODE,
	SELECT_DLOG_EXP,
	SELECT_DLOG_IMP,
	DLOG_SELECT_DEFAULTS,
	DLOG_SELECT_ALL,
	DLOG_DESELECT_ALL,
	DLOG_DUMP_INTERNAL,
	CLOSE_LOGFILE,
	START_DATALOGGING,
	STOP_DATALOGGING,
	EXPORT_VETABLE,
	IMPORT_VETABLE,
	REBOOT_GETERR,
	REVERT_TO_BACKUP,
	BACKUP_ALL,
	RESTORE_ALL,
	SELECT_PARAMS,
	REQ_FUEL_POPUP,
	RESCALE_TABLE,
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
	OFFLINE_FIRMWARE_CHOICE,
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
	SER_INTERVAL_DELAY,
	SET_SER_PORT,
	NUM_SQUIRTS_1,
	NUM_SQUIRTS_2,
	NUM_CYLINDERS_1,
	NUM_CYLINDERS_2,
	NUM_INJECTORS_1,
	NUM_INJECTORS_2,
	TRIGGER_ANGLE,
	LOGVIEW_ZOOM,
	DEBUG_LEVEL,
	GENERIC,
	ALT_SIMUL,
}SpinButton;

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
	DATALOG_INT_DUMP,
	DATALOG_EXPORT,
	DATALOG_IMPORT,
	FULL_BACKUP,
	FULL_RESTORE
}FileIoType;


typedef enum
{	
	RED=0xb0,
	BLACK
}GuiColor;

typedef enum
{
	HEADER=0xc0,
	PAGE,
	RANGE,
	TABLE
}ImportParserFunc;

typedef enum
{
	VEX_EVEME=0xd0,
	VEX_USER_REV,
	VEX_USER_COMMENT,
	VEX_DATE,
	VEX_TIME,
	VEX_RPM_RANGE,
	VEX_LOAD_RANGE,
	VEX_NONE,
}ImportParserArg;

typedef enum
{
	FONT=0xe0,
	TRACE,
	GRATICULE,
	HIGHLIGHT,
}GcType;

typedef enum
{	/* up to 32 Capability flags.... */
	/* No capabilities == Standard B&G code with no modifications */
	STANDARD	= 1<<0,
	DUALTABLE	= 1<<1,
	MSNS_E		= 1<<2,
}Capability;

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
	CONVERSIONS	= 1<<2,
	SERIAL_RD	= 1<<3,
	SERIAL_WR	= 1<<4,
	IO_PROCESS	= 1<<5,
	THREADS		= 1<<6,
	REQ_FUEL	= 1<<7,
	TABLOADER	= 1<<8,
	RTMLOADER	= 1<<9,
	COMPLEX_EXPR	= 1<<10,
	CRITICAL	= 1<<30,
}Dbg_Class;

typedef enum guint
{
	INTERROGATOR_SHIFT	= 0,
	OPENGL_SHIFT		= 1,
	CONVERSIONS_SHIFT	= 2,
	SERIAL_RD_SHIFT		= 3,
	SERIAL_WR_SHIFT		= 4,
	IO_PROCESS_SHIFT	= 5,
	THREADS_SHIFT		= 6,
	REQ_FUEL_SHIFT		= 7,
	TABLOADER_SHIFT		= 8,
	RTMLOADER_SHIFT		= 9,
	COMPLEX_EXPR_SHIFT	= 10,
	CRITICAL_SHIFT		= 30,
}Dbg_Shift;

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
	NULL_CMD,
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
	IO_UPDATE_VE_CONST,
	IO_LOAD_REALTIME_MAP,
	IO_LOAD_GUI_TABS,
	IO_REBOOT_GET_ERROR,
	IO_GET_BOOT_PROMPT,
	IO_BOOT_READ_ERROR,
	IO_JUST_BOOT,
	IO_CLEAN_REBOOT,
}Io_Command;

typedef enum
{
	UPD_REALTIME = 0x140,
	UPD_LOGBAR,
	UPD_LOGVIEWER,
	UPD_WIDGET,
	UPD_DATALOGGER,
	UPD_VE_CONST,
	UPD_READ_VE_CONST,
	UPD_ENABLE_THREE_D_BUTTONS,
	UPD_RAW_MEMORY,
	UPD_SET_STORE_RED,
	UPD_SET_STORE_BLACK,
	UPD_LOAD_GUI_TABS,
	UPD_LOAD_REALTIME_MAP,
	UPD_LOAD_RT_STATUS,
	UPD_LOAD_RT_SLIDERS,
	UPD_POPULATE_DLOGGER,
	UPD_COMMS_STATUS,
	UPD_WRITE_STATUS,
	UPD_REENABLE_INTERROGATE_BUTTON,
	UPD_REENABLE_GET_DATA_BUTTONS,
	UPD_START_REALTIME,
	UPD_GET_BOOT_PROMPT,
	UPD_REBOOT_GET_ERROR,
	UPD_JUST_BOOT,
	UPD_FORCE_UPDATE,
}UpdateFunction;

typedef enum
{
	MTX_INT = 0x160,
	MTX_ENUM,
	MTX_BOOL,
	MTX_STRING,
}DataType;

typedef enum
{
	ABOUT_PAGE=0x170,
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
	VE3D_VIEWER_PAGE,
}PageIdent;

typedef enum
{
	VE_EMB_BIT=0x180,
	VE_VAR,
	RAW_VAR,
}ComplexExprType;

typedef enum
{
	UPLOAD=0x190,
	DOWNLOAD,
	RTV,
}ConvType;

typedef enum
{
	MTX_HEX=0x1a0,
	MTX_DECIMAL,
}Base;

typedef enum
{
	MTX_ENTRY=0x1b0,
//	MTX_SPINBUTTON,
	MTX_LABEL,
}WidgetType;
#endif
