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
        gint fd;                 /* File descriptor */
        gchar *port_name;          /* textual name of comm port */
        gboolean open;		/* flag, TRUE for open FALSE for closed */
        gint poll_timeout;       /* Pollng interval in MILLISECONDS */
        gint read_wait;          /* time delay between each read */
        gint table0_size;	/* Size of VEtable/page_0 datablock */
        gint table1_size;	/* Size of VEtable/page_1 datablock */
        gint rtvars_size;	/* Size of Realtime vars datablock */
        gint ignvars_size;	/* Size of Realtime vars datablock */
        gint memblock_size;	/* Size of Raw_Memory datablock */
        struct termios oldtio;  /* serial port settings before we touch it */
        struct termios newtio;  /* serial port settings we use when running */
        gint errcount;           /* Serial I/O errors read error count */
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
	GtkWidget *p0_map_tps_frame;
	GtkWidget *p1_map_tps_frame;
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
	GtkWidget *fast_idle_temp_spin;		/* Spinner */
	GtkWidget *slow_idle_temp_spin;		/* Spinner */
	GtkWidget *ego_temp_active_spin;	/* Spinner */
	GtkWidget *req_fuel_total_1_spin;	/* Spinner */
	GtkWidget *req_fuel_per_squirt_1_spin;	/* Spinner */
	GtkWidget *inj_per_cycle_1_spin;	/* Spinner */
	GtkWidget *injectors_1_spin;		/* Spinner */
	GtkWidget *cylinders_1_spin;		/* Spinner */
	GtkWidget *req_fuel_total_2_spin;	/* Spinner */
	GtkWidget *req_fuel_per_squirt_2_spin;	/* Spinner */
	GtkWidget *inj_per_cycle_2_spin;	/* Spinner */
	GtkWidget *injectors_2_spin;		/* Spinner */
	GtkWidget *cylinders_2_spin;		/* Spinner */
	GtkWidget *warmwizard[10];		/* Spinner */
	GtkWidget *cooling_fan_temp_spin;
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
        gint table;			/* Which table is this for */
	gboolean visible;		/* Is it visible? */
};


/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct DynamicLabels
{
	GtkWidget *req_fuel_lab;
	GtkWidget *fast_idle_temp_lab;
	GtkWidget *cr_pulse_hightemp_lab;
	GtkWidget *ww_cr_pulse_hightemp_lab;
	GtkWidget *warmup_bins_lab[10];
	GtkWidget *warmwizard_lab[10];
	GtkWidget *warmup_lab;
	GtkWidget *p0_map_tps_lab;
	GtkWidget *p1_map_tps_lab;
	GtkWidget *dlog_file_lab;
	GtkWidget *ww_clt_lab;
	GtkWidget *ww_warmcorr_lab;
	GtkWidget *ww_ego_lab;
	GtkWidget *ww_map_lab;
        GtkWidget *cooling_fan_temp_lab;
	GtkWidget *timing_multi_lab;
	GtkWidget *output_boost_lab;

};

/* These are defined here instead of the individual .c files as
 * we manipulate their attributes to give feedback to the user
 */
struct DynamicButtons
{
	GtkWidget *speed_den_but;		/* Toggle button */
	GtkWidget *alpha_n_but;			/* Toggle button */
	GtkWidget *two_stroke_but;		/* Toggle button */
	GtkWidget *four_stroke_but;		/* Toggle button */
	GtkWidget *multi_port_but;		/* Toggle button */
	GtkWidget *tbi_but;			/* Toggle button */
	GtkWidget *map_115_but;			/* Toggle button */
	GtkWidget *map_250_but;			/* Toggle button */
	GtkWidget *even_fire_but;		/* Toggle button */
	GtkWidget *odd_fire_but;		/* Toggle button */
	GtkWidget *simul_but;			/* Toggle button */
	GtkWidget *alternate_but;		/* Toggle button */
	GtkWidget *baro_ena_but;		/* Toggle button */
	GtkWidget *baro_disa_but;		/* Toggle button */
	GtkWidget *nbo2_but;			/* Toggle button */
	GtkWidget *wbo2_but;			/* Toggle button */
	GtkWidget *tools_revert_but;		/* revert to old data */
	GtkWidget *stop_dlog_but;		/* Stop DataLogging */
	GtkWidget *start_dlog_but;		/* Start DataLogging */
	GtkWidget *close_dlog_but;		/* Close DataLog file */
	GtkWidget *logplay_sel_log_but;		/* Select playback log */
	GtkWidget *logplay_sel_parm_but;	/* Select rt parms for play */
	GtkWidget *logplay_start_rt_but;	/* Logplay star realtime */
	GtkWidget *logplay_stop_rt_but;		/* Logplay stop realtime */
	GtkWidget *onoff_idle_but;		/* B&G idle method */
	GtkWidget *pwm_idle_but;		/* Fielding PWM idle method */
	GtkWidget *dt_mode;			/* Dualtable mode select */
	GtkWidget *inj1_not_driven;		/* Inj1 not driven */
	GtkWidget *inj1_table1;			/* Inj1 driven from table 1 */
	GtkWidget *inj1_table2;			/* Inj1 driven from table 2 */
	GtkWidget *inj2_not_driven;		/* Inj2 not driven */
	GtkWidget *inj2_table1;			/* Inj2 driven from table 1 */
	GtkWidget *inj2_table2;			/* Inj2 driven from table 2 */
	GtkWidget *inj1_gammae;			/* Inj1 gammae */
	GtkWidget *inj2_gammae;			/* Inj2 gammae */
	GtkWidget *boost_39hz;
	GtkWidget *boost_19hz;
	GtkWidget *boost_10hz;
	GtkWidget *time_based_but;
	GtkWidget *trig_return_but;
	GtkWidget *normal_out_but;
	GtkWidget *invert_out_but;
	GtkWidget *multi_spark_but;
	GtkWidget *norm_spark_but;
	GtkWidget *boost_retard_but;
	GtkWidget *noboost_retard_but;
};

/* Simple struct to store the pointers to the entry and button
 * for the tools_gui and vex_support files.  
 */
struct Tools
{
	GtkWidget *export_but;
	GtkWidget *export_comment_entry;
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
	Capabilities flags;/* DT, Temp_dep, IGN or whatever ... */
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
	Capabilities flags;
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

	/* Interrogator structures.... */
struct Canidate
{
	GHashTable *bytecounts;	/* byte count for each of the 10 test cmds */
	gchar *sig_str;		/* Signature string to search for */
	gchar *quest_str;	/* Ext Version string to search for */
	gint ver_num;		/* Version number to search for */
	gchar *firmware_name;	/* Name of this firmware */
	Capabilities capabilities;	/* Bitmask of capabilities.... */
};

struct Command
{
	gint page;		/* ms page in memory where it resides */
	gchar *string;		/* command to get the data */
	gchar *desc;		/* command description */
	gchar *handle;		/* command description */
	gint len;		/* Command length in chars to send */
	gboolean store_data;	/* Store returned data */
	StoreType store_type;	/* Store data where */
};

#endif
