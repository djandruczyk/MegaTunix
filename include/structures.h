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
#include <gtk/gtk.h>
#include <termios.h>
#include <unistd.h>


struct Serial_Params
{
        int fd;                 /* File descriptor */
        int comm_port;          /* DOS/Windows COM port number, 1-8 typically */
        int open;               /* flag, 1 for open 0 for closed */
        int poll_timeout;       /* Pollng interval in MILLISECONDS */
        int read_wait;          /* time delay between each read */
        int raw_bytes;          /* number of bytes to read for realtime vars */
        int veconst_size;       /* Size of VEtable/constants datablock */
        struct termios oldtio;  /* serial port settings before we touch it */
        struct termios newtio;  /* serial port settings we use when running */
        int errcount;           /* Serial I/O errors read error count */
};


/* This structure contains all the gui pointers to the 
 * objects used on the runtime_gui.  These are needed  
 * so the handlers have something to reference when updating
 * the widget in question when data arrives....
 * A Struct was just more convienent and kept things together.
 */

struct Runtime_Widgets
{
        GtkWidget *secl_lab;            /* Counter label */
        GtkWidget *ego_lab;             /* O2 Voltage */
        GtkWidget *ego_pbar;            /* O2 Voltage bar */
        GtkWidget *baro_lab;            /* baro label from MS */
        GtkWidget *baro_pbar;           /* O2 Voltage bar */
        GtkWidget *map_lab;             /* map label from MS */
        GtkWidget *map_pbar;            /* map value for bar */
        GtkWidget *mat_lab;             /* mat label from MS */
        GtkWidget *mat_pbar;            /* map value for bar */
        GtkWidget *clt_lab;             /* clt label from MS */
        GtkWidget *clt_pbar;            /* map value for bar */
        GtkWidget *tps_lab;             /* tps label from MS */
        GtkWidget *tps_pbar;            /* map value for bar */
        GtkWidget *batt_lab;            /* batt label from MS */
        GtkWidget *batt_pbar;           /* map value for bar */
        GtkWidget *egocorr_lab;         /* egocorr label from MS */
        GtkWidget *egocorr_pbar;        /* egocorr label from MS */
        GtkWidget *aircorr_lab;         /* aircorr label from MS */
        GtkWidget *aircorr_pbar;        /* aircorr label from MS */
        GtkWidget *warmcorr_lab;        /* warmcorr label from MS */
        GtkWidget *warmcorr_pbar;       /* warmcorr label from MS */
        GtkWidget *rpm_lab;             /* rpm label from MS */
        GtkWidget *rpm_pbar;            /* rpm label from MS */
        GtkWidget *pw1_lab;              /* pw label from MS */
        GtkWidget *pw1_pbar;             /* pw label from MS */
        GtkWidget *tpsaccel_lab;        /* tpsaccel label from MS */
        GtkWidget *tpsaccel_pbar;       /* tpsaccel label from MS */
        GtkWidget *barocorr_lab;        /* barocorr label from MS */
        GtkWidget *barocorr_pbar;       /* barocorr label from MS */
        GtkWidget *gammae_lab;          /* gammae label from MS */
        GtkWidget *gammae_pbar;         /* gammae label from MS */
        GtkWidget *vecurr1_lab;          /* vecurr label from MS */
        GtkWidget *vecurr1_pbar;         /* vecurr label from MS */
        GtkWidget *dcycle1_lab;          /* vecurr label from MS */
        GtkWidget *dcycle1_pbar;         /* vecurr label from MS */
        GtkWidget *status[7];           /* Status boxes */
};

struct DynamicMisc
{
	GtkWidget *p0_map_tps_frame;
	GtkWidget *p1_map_tps_frame;
};

/* this is required so we keep track of the gui controls so we
 * can update them as needed (color changes, sensitivity, etc...
 */
struct DynamicSpinners
{
	GtkWidget *fast_idle_thresh_spin;	/* Spinner */
	GtkWidget *req_fuel_total_spin;		/* Spinner */
	GtkWidget *req_fuel_per_squirt_spin;	/* Spinner */
	GtkWidget *ego_temp_active_spin;	/* Spinner */
	GtkWidget *inj_per_cycle_spin;		/* Spinner */
	GtkWidget *injectors_spin;		/* Spinner */
	GtkWidget *cylinders_spin;		/* Spinner */
};

/* Controls for the Required Fuel Calculator... */
struct Reqd_Fuel
{
        GtkWidget *disp_spin;     /* Spinbutton */
        GtkWidget *cyls_spin;     /* Spinbutton */
        GtkWidget *rated_inj_flow_spin;   /* Spinbutton */
        GtkWidget *rated_pressure_spin;   /* Spinbutton */
        GtkWidget *actual_pressure_spin;  /* Spinbutton */
        GtkWidget *afr_spin;      /* Spinbutton */
        gint disp;                /* Engine size  1-1000 Cu-in */
        gint cyls;                /* # of Cylinders  1-12 */
        gfloat rated_inj_flow;    /* Rated injector flow */
        gfloat rated_pressure;    /* Rated fuel pressure */
        gfloat actual_pressure;   /* Actual fuel pressure */
        gfloat actual_inj_flow;   /* injector flow rate (lbs/hr) */
        gfloat afr;               /* Air fuel ratio 10-25.5 */
};

/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct DynamicLabels
{
	GtkWidget *req_fuel_lab;
	GtkWidget *squirts_lab;
	GtkWidget *injectors_lab;
	GtkWidget *cylinders_lab;
	GtkWidget *fastidletemp_lab;
	GtkWidget *cr_pulse_lowtemp_lab;
	GtkWidget *cr_pulse_hightemp_lab;
	GtkWidget *warmup_bins_lab[10];
	GtkWidget *warmup_lab;
	GtkWidget *ego_temp_lab;
	GtkWidget *runtime_clt_lab;
	GtkWidget *runtime_mat_lab;
	GtkWidget *p0_map_tps_lab;
	GtkWidget *p1_map_tps_lab;
	GtkWidget *vex_file_lab;
	GtkWidget *dlog_file_lab;
};

/* These are defined as they are semi-dynamic and are modified
 * during run of MegaTunix for status or units related reasons
 */
struct DynamicAdjustments
{
	GtkAdjustment *fast_idle_temp_adj;
	GtkAdjustment *ego_temp_adj;
	GtkAdjustment *cylinders_adj;		/* Adjustment */
};

/* These are defined here instead of the individual .c files as
 * we manipulate their attributes to give feedback to the user
 */
struct DynamicButtons
{
	GtkWidget *const_store_but;		/* Button */
	GtkWidget *enrich_store_but;		/* Button */
	GtkWidget *vetable_store_but;		/* Button */
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
	GtkWidget *ve_import_but;
	GtkWidget *ve_export_but;
	GtkWidget *ve_clear_vex_but;
	GtkWidget *stop_dlog_but;		/* Stop DataLogging */
	GtkWidget *start_dlog_but;		/* Start DataLogging */
};

/* Simple struct to store hte pointers to the entry and button
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
};

/* Datastructure the holds the expected responses for the data
 * returning commands issued to the MegaSquirt.
 */

struct Command_Limits
{
	gint	A_count;
	gint	C_count;
	gint	Q_count;
	gint	V_count;
	gint	S_count;
	gint	I_count;
};

struct Ve_Widgets
{
	GtkWidget *widget[MS_PAGE_SIZE];
};

	
#endif
