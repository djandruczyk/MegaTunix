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
        struct termios oldtio;  /* serial port settings before we touch it */
        struct termios newtio;  /* serial port settings we use when running */
        gint errcount;           /* Serial I/O errors read error count */
};


/* Progress bars that are updated from various functions... */
struct DynamicProgress
{
        GtkWidget *secl_pbar;            /* O2 Voltage bar */
        GtkWidget *ego_pbar;            /* O2 Voltage bar */
        GtkWidget *baro_pbar;           /* O2 Voltage bar */
        GtkWidget *map_pbar;            /* map value for bar */
        GtkWidget *mat_pbar;            /* map value for bar */
        GtkWidget *clt_pbar;            /* map value for bar */
        GtkWidget *tps_pbar;            /* map value for bar */
        GtkWidget *batt_pbar;           /* map value for bar */
        GtkWidget *egocorr_pbar;        /* egocorr label from MS */
        GtkWidget *aircorr_pbar;        /* aircorr label from MS */
        GtkWidget *warmcorr_pbar;       /* warmcorr label from MS */
        GtkWidget *rpm_pbar;            /* rpm label from MS */
        GtkWidget *pw1_pbar;             /* pw label from MS */
        GtkWidget *pw2_pbar;             /* pw label from MS */
        GtkWidget *tpsaccel_pbar;       /* tpsaccel label from MS */
        GtkWidget *barocorr_pbar;       /* barocorr label from MS */
        GtkWidget *gammae_pbar;         /* gammae label from MS */
        GtkWidget *vecurr1_pbar;         /* vecurr1 label from MS */
        GtkWidget *vecurr2_pbar;         /* vecurr2 label from MS */
        GtkWidget *dcycle1_pbar;         /* vecurr1 label from MS */
        GtkWidget *dcycle2_pbar;         /* vecurr2 label from MS */
	GtkWidget *idledc_pbar;
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
        GtkWidget *secl_lab;		/* Counter label */
        GtkWidget *ego_lab;		/* O2 Voltage */
        GtkWidget *baro_lab;		/* baro label from MS */
        GtkWidget *map_lab;		/* map label from MS */
        GtkWidget *mat_lab;		/* mat label from MS */
        GtkWidget *clt_lab;		/* clt label from MS */
        GtkWidget *tps_lab;		/* tps label from MS */
        GtkWidget *batt_lab;		/* batt label from MS */
        GtkWidget *egocorr_lab;		/* egocorr label from MS */
        GtkWidget *aircorr_lab;		/* aircorr label from MS */
        GtkWidget *warmcorr_lab;	/* warmcorr label from MS */
        GtkWidget *rpm_lab;		/* rpm label from MS */
        GtkWidget *pw1_lab;		/* pw label from MS */
        GtkWidget *pw2_lab;		/* pw label from MS */
        GtkWidget *tpsaccel_lab;	/* tpsaccel label from MS */
        GtkWidget *barocorr_lab;	/* barocorr label from MS */
        GtkWidget *gammae_lab;		/* gammae label from MS */
        GtkWidget *vecurr1_lab;		/* vecurr label from MS */
        GtkWidget *dcycle1_lab;		/* vecurr label from MS */
        GtkWidget *dcycle2_lab;		/* vecurr label from MS */
        GtkWidget *idledc_lab;		/* idledc label from MS */
	GtkWidget *req_fuel_lab;
	GtkWidget *fast_idle_temp_lab;
	GtkWidget *slow_idle_temp_lab;
	GtkWidget *cr_pulse_lowtemp_lab;
	GtkWidget *cr_pulse_hightemp_lab;
	GtkWidget *warmup_bins_lab[10];
	GtkWidget *warmwizard_lab[10];
	GtkWidget *warmup_lab;
	GtkWidget *ego_temp_lab;
	GtkWidget *runtime_clt_lab;
	GtkWidget *runtime_mat_lab;
	GtkWidget *p0_map_tps_lab;
	GtkWidget *p1_map_tps_lab;
	GtkWidget *dlog_file_lab;
	GtkWidget *warmwiz_clt_lab;
	GtkWidget *ww_clt_lab;
	GtkWidget *ww_warmcorr_lab;
	GtkWidget *ww_ego_lab;
	GtkWidget *ww_map_lab;
	GtkWidget *cooling_fan_temp_lab;
};

/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct DynamicAdjustments
{
	GtkAdjustment *cooling_fan_temp_adj;
	GtkAdjustment *fast_idle_temp_adj;
	GtkAdjustment *slow_idle_temp_adj;
	GtkAdjustment *ego_temp_adj;
	GtkAdjustment *cylinders_adj;		/* Adjustment */
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
	GtkWidget *parent; 		/* Parent Widget */
	GtkWidget *d_area;		/* Drawing area widget */
	GdkPixmap *trace_pmap;		/* area used for the trace */
	GdkGC *font_gc;			/* GC used for the fonts */
	GdkGC *trace_gc;		/* GC used for the trace */
	GdkGC *grat_gc;			/* GC used for the graticule */
	gint info_width;		/* width in pixels of the info_pmap */
	gchar *vname;			/* Name of widget being logged */
	gint runtime_offset;		/* Offset into runtime struct */
	gint size;			/* 1=byte, 2=short, 4=float */
	gint last_y;			/* Last point on screen of trace */
	gfloat min;			/* for auto-scaling */
	gfloat max;			/* for auto-scaling */
	gint last_grat;			/* last grat line position */
	gint grat_interval;		/* graticule interval in pixels */
	gfloat lower;			/* limits to use for scaling */
	gfloat upper;			/* limits to use for scaling */
	GArray *data_array;		/* History of all values recorded */
};
	
/* Limit are used for the above Viewable_Value
 * strcture to help set sane limits to the traces
 * in the logviewer...
 */
struct Limits
{
	gfloat lower;
	gfloat upper;
};

/* The Rt_Control struct contains info on the runtime display tab controls
 * as they are now stored in the config file and adjustable in position
 * and placement and such..
 */
struct Rt_Control
{
	gchar *ctrl_name; /* Control name in config file (key) */
	GtkWidget *label; /* Label in runtime display */
	GtkWidget *data;  /* textual representation of the data */
	GtkWidget *pbar;  /* progress bar for the data */
	GtkWidget *parent;/* Parent of these widgets (table) */
	gint table;	  /* Table number (0-3) */
	gint row;	  /* Starting row */
	gint col;	  /* Starting Column */
	gchar *friendly_name; /* text for Label above */
	gint limits_index; /* Offset into limits[] structure array */
	gint runtime_offset; /* offset into Runtime_Common struct (using []) */
	gint size;	  /* UCHAR,SHORT or FLOAT (1,2, or 4) */
	gboolean enabled; /* Pretty obvious */
	gint count;	  /* used to making sure things update */
	Capabilities flags;/* DT, Temp_dep, IGN or whatever ... */
};

#endif
