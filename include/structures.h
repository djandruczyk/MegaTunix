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

/* Constants/Enrichments Gui Adjustment Structures */

#ifndef __STRUCTURES_H__
#define __STRUCTURES_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <termios.h>
#include <unistd.h>


/* Serial parameters, */
struct Serial_Params
{
        gint fd;		/* File descriptor */
        gchar *port_name;	/* textual name of comm port */
        gboolean open;		/* flag, TRUE for open FALSE for closed */
        gint read_wait;		/* time delay between each read */
        gint errcount;		/* Serial I/O errors read error count */
        struct termios oldtio;	/* serial port settings before we touch it */
        struct termios newtio;	/* serial port settings we use when running */
};

struct Firmware_Details
{
	gchar *name;		/* textual name */
	gchar **tab_list;	/* vector string of tabs to load */
	gchar *rtv_map_file;	/* realtime vars map filename */
        gint rtvars_size;       /* Size of Realtime vars datablock */
        gint ignvars_size;      /* Size of Realtime vars datablock */
        gint memblock_size;     /* Size of Raw_Memory datablock */
	gboolean multi_page;	/* Multi-page firmware? */
	gint total_pages;	/* How many pages do we handle? */
	struct Page_Params *page_params[8];	/* details on data per page.. */
};

/* Progress bars that are updated from various functions... */
struct DynamicProgress
{
	GtkWidget *ww_clt_pbar;
	GtkWidget *ww_warmcorr_pbar;
	GtkWidget *ww_ego_pbar;
	GtkWidget *ww_map_pbar;
};

/* Misc widgets that need to be modified/updated during runtime */
struct DynamicMisc
{
	GtkWidget *warmwizard_table;
        GtkWidget *status[7];           /* Status boxes */
        GtkWidget *ww_status[7];           /* Status boxes */
	GtkWidget *tbl1_reqd_fuel_frame;
};

/* this is required so we keep track of the gui controls so we
 * can update them as needed (color changes, sensitivity, etc...
 */
struct DynamicSpinners
{
	GtkWidget *warmwizard[10];		/* Spinner */
};

/* Controls for the Required Fuel Calculator... */
struct Reqd_Fuel
{
	GtkWidget *popup;		/* the popup window */
	GtkWidget *calcd_val_spin;	/* Preliminary value */
	GtkWidget *reqd_fuel_spin;	/* Used value */
	gfloat calcd_reqd_fuel;		/* calculated value... */
        gint disp;			/* Engine size  1-1000 Cu-in */
        gint cyls;			/* # of Cylinders  1-12 */
        gfloat rated_inj_flow;		/* Rated injector flow */
        gfloat rated_pressure;		/* Rated fuel pressure */
        gfloat actual_pressure;		/* Actual fuel pressure */
        gfloat actual_inj_flow;		/* injector flow rate (lbs/hr) */
        gfloat target_afr;		/* Air fuel ratio 10-25.5 */
        gint page;			/* Which page is this for */
	gboolean visible;		/* Is it visible? */
};


/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct DynamicLabels
{
	GtkWidget *ww_cr_pulse_hightemp_lab;
	GtkWidget *warmwizard_lab[10];
	GtkWidget *dlog_file_lab;
	GtkWidget *ww_clt_lab;
	GtkWidget *ww_warmcorr_lab;
	GtkWidget *ww_ego_lab;
	GtkWidget *ww_map_lab;

};

/* These are defined here instead of the individual .c files as
 * we manipulate their attributes to give feedback to the user
 */
struct DynamicButtons
{
	GtkWidget *tools_revert_but;		/* revert to old data */
	GtkWidget *stop_dlog_but;		/* Stop DataLogging */
	GtkWidget *start_dlog_but;		/* Start DataLogging */
	GtkWidget *close_dlog_but;		/* Close DataLog file */
	GtkWidget *logplay_sel_log_but;		/* Select playback log */
	GtkWidget *logplay_sel_parm_but;	/* Select rt parms for play */
	GtkWidget *logplay_start_rt_but;	/* Logplay star realtime */
	GtkWidget *logplay_stop_rt_but;		/* Logplay stop realtime */
};

/* These are defined here instead of the individual .c files as
 * we manipulate their attributes to give feedback to the user
 */
struct DynamicEntries
{
	GtkWidget *comms_reset_entry;
	GtkWidget *runtime_reset_entry;
	GtkWidget *comms_sioerr_entry;
	GtkWidget *runtime_sioerr_entry;
	GtkWidget *comms_readcount_entry;
	GtkWidget *runtime_readcount_entry;
	GtkWidget *comms_ve_readcount_entry;
	GtkWidget *runtime_ve_readcount_entry;
	GtkWidget *vex_comment_entry;
	GtkWidget *ecu_revision_entry;
	GtkWidget *extended_revision_entry;
	GtkWidget *ecu_signature_entry;
};

/* Logable data sorted by offset into runtime_data memory block */
struct Logables
{
	GtkWidget *widgets[64];
	gboolean index[64];
};

struct Io_File
{
	GIOChannel *iochannel;
	gchar *filename;
	FileIoType iotype;
};

/* Viewable_Value is the datastructure bound 
 * to every trace viewed in the logviewer. 
 */
struct Viewable_Value
{
	GdkGC *font_gc;			/* GC used for the fonts */
	GdkGC *trace_gc;		/* GC used for the trace */
	GtkWidget *d_area;		/* Drawing Area */
	gchar *vname;			/* Name of widget being logged */
	gint runtime_offset;		/* Offset into runtime struct */
	gint size;			/* 1=byte, 2=short, 4=float */
	gint last_y;			/* Last point on screen of trace */
	gfloat min;			/* for auto-scaling */
	gfloat max;			/* for auto-scaling */
	gfloat lower;			/* hard limits to use for scaling */
	gfloat upper;			/* hard limits to use for scaling */
	gfloat cur_low;			/* User limits to use for scaling */
	gfloat cur_high;		/* User limits to use for scaling */
	GArray *data_array;		/* History of all values recorded */
};
	
/* The Rt_Control struct contains info on the runtime display tab controls
 * as they are now stored in the config file and adjustable in position
 * and placement and such..
 */
struct Rt_Control
{
	gchar *ctrl_name; /* Control name in config file (key in hashtable) */
	GtkWidget *parent;/* Parent of the table below  */
	GtkWidget *table; /* Table to contain the next 3 widgets */
	GtkWidget *label; /* Label in runtime display */
	GtkWidget *data;  /* textual representation of the data */
	GtkWidget *pbar;  /* progress bar for the data */
	gint tbl;	  /* Table number (0-3) */
	gint row;	  /* Starting row */
	gchar *friendly_name; /* text for Label above */
	gint limits_index; /* Offset into limits[] structure array */
	gint runtime_offset; /* offset into Runtime_Common struct (using []) */
	gint size;	  /* UCHAR,SHORT or FLOAT (1,2, or 4) */
	gboolean enabled; /* Pretty obvious */
	gint count;	  /* used to making sure things update */
	gint rate;	  /* used to making sure things update */
	gint last_upd;	  /* used to making sure things update */
	Capability flags;/* DT, Temp_dep, IGN or whatever ... */
};

/* The Def_Control struct contains info on the default controls that are 
 * written to the config file on startup.
 */
struct Default_Control
{
	gchar *ctrl_name;
	gint tbl;
	gint row;
	gchar *friendly_name;
	gint limits_index;
	gint runtime_offset;
	gint size;
	gboolean enabled;
	Capability flags;
};

/* the Default_Limits struct maps field names to limits. used for importing
 * datalogs from megatune mainly as that friggin peice of work uses different
 * friggin field names per log type/version.
 */

struct Default_Limits
{
	gchar *field;		/* Field name */
	gfloat lower;		/* Lower Limit */
	gfloat upper;		/* Upper Limit */
	LogType logtype;	/* Datalog type it's found in... */
};

/* The Log_Info datastructure is populated when a datalog file is opened
 * for viewing in the Datalog viewer.
 */
struct Log_Info
{
	gint field_count;	/* How many fields in the logfile */
	gchar *delimiter;	/* delimiter between fields for this logfile */
	gchar **fields;		/* NULL term'd vector of string field names */
	GArray *fields_data;	/* Array of arrays for stored data */
	GArray *lowers;		/* Array of lower limits for each field */
	GArray *uppers;		/* Array of upper limits for each field */
};

struct Page_Params
{
	gint size;		/* total size of this page as returned... */
	gint ve_base;		/* where the vetable starts */
	gint rpm_base;		/* where rpm table starts */
	gint load_base;		/* where load table starts */
	gint rpm_bincount;	/* how many RPM bins */
	gint load_bincount;	/* how many load bins... */
	gboolean is_spark;	/* is this a spark table?? */
};
	/* Interrogator structures.... */
struct Canidate
{
	gchar *name;		/* Name of this firmware */
	gchar *filename;	/* absolute path to this canidate profile */
	GHashTable *bytecounts;	/* byte count for each of the 10 test cmds */
	gchar *sig_str;		/* Signature string to search for */
	gchar *quest_str;	/* Ext Version string to search for */
	gint ver_num;		/* Version number to search for */
	gchar *load_tabs;	/* list of tabs to load into the gui */
	gchar *rtv_map_file;	/* name of realtime vars map file */
	Capability capabilities;/* Bitmask of capabilities.... */
	gchar * rt_cmd_key;	/* string key to hashtable for RT command */
	gchar * ve_cmd_key;	/* string key to hashtable for VE command */
	gchar * ign_cmd_key;	/* string key to hashtable for Ign command */
	gchar * raw_mem_cmd_key;/* string key to hashtable for RAW command */
	gboolean multi_page;	/* Multi-page firmware ??? */
	gint total_pages;	/* how many pages do we handle? */
	GHashTable *lookuptables;/* Lookuptables hashtable... */
	struct Page_Params *page_params[8];/* details on ve/rpm/load tables*/
};

struct Command
{
	gint page;		/* ms page in memory where it resides */
	gchar *string;		/* command to get the data */
	gint len;		/* Command length in chars to send */
	gboolean multipart;	/* Multipart command? (raw_memory) */
	gint cmd_int_arg;	/* multipart arg, integer */
	gchar *desc;		/* command description */
	gboolean store_data;	/* Store returned data ? */
	StoreType store_type;	/* Store data where */
};

struct Io_Message
{
	IoCommand cmd;		/* Source command (initiator)*/
	CmdType command;	/* Command type */
	gchar *out_str;		/* Data sent to the ECU  for READ_CMD's */
	gint page;		/* multipage firmware specific */
	gint out_len;		/* number of bytes in out_str */
	gint offset;		/* used for RAW_MEMORY and more */
	GArray *funcs;		/* List of functiosn to be dispatched... */
	InputHandler handler;	/* Command handler for inbound data */
	void *payload;		/* data passed along, arbritrary size.. */
};

struct IoCmds
{
	gchar *realtime_cmd;	/* Command sent to get RT vars.... */
	gint rt_cmd_len;	/* length in bytes of rt_cmd_len */
	gchar *veconst_cmd;	/* Command sent to get VE/Const vars.... */
	gint ve_cmd_len;	/* length in bytes of veconst_cmd */
	gchar *ignition_cmd;	/* Command sent to get Ignition vars.... */
	gint ign_cmd_len;	/* length in bytes of ignition_cmd */
	gchar *raw_mem_cmd;	/* Command sent to get raw_mem vars.... */
	gint raw_mem_cmd_len;	/* length in bytes of raw_mem_cmd */
};

struct OutputData
{
	gint page;		/* Page in ECU */
	gint offset;		/* Offset in block */
	gint value;		/* Value to send */
	gboolean ign_parm;	/* Ignition parameter, True or False */
};

struct RtvMap
{
	gint raw_total;		/* Number of raw variables */
	gint derived_total;	/* Number of derived variables */
	GArray *ts_array;	/* Timestamp array */
	GArray *rtv_array;	/* Realtime Values array of lists.. */
	GHashTable *rtv_hash;	/* Hashtable of rtv derived values indexed by
				 * it's internal name */
};

struct DebugLevel
{
	gchar * name;		/* Debugging name */
	gint	handler;	/* Signal handler name */
	Dbg_Class dclass;	/* Bit mask for this level (0-31) */
	Dbg_Shift dshift;	/* Bit shift amount */
	gboolean enabled;	/* Enabled or not? */
};

#endif
