/*!
  \author David Andruczyk

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

#include <configfile.h>
#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <termios.h>
#include <unistd.h>

/* Type definitions */
typedef struct _Serial_Params Serial_Params;
typedef struct _Firmware_Details Firmware_Details;
typedef struct _LookupTable LookupTable;
typedef struct _Reqd_Fuel Reqd_Fuel;
typedef struct _Io_File Io_File;
typedef struct _Viewable_Value Viewable_Value;
typedef struct _Dash_Gauge Dash_Gauge;
typedef struct _Rt_Slider Rt_Slider;
typedef struct _Rt_Text Rt_Text;
typedef struct _Log_Info Log_Info;
typedef struct _Page_Params Page_Params;
typedef struct _Table_Params Table_Params;
typedef struct _Detection_Test Detection_Test;
typedef struct _Detection_Test_Result Detection_Test_Result;
typedef struct _Req_Fuel_Params Req_Fuel_Params;
typedef struct _Command Command;
typedef struct _Io_Message Io_Message;
typedef struct _Text_Message Text_Message;
typedef struct _QFunction QFunction;
typedef struct _Widget_Update Widget_Update;
typedef struct _Io_Cmds Io_Cmds;
typedef struct _Output_Data Output_Data;
typedef struct _Rtv_Map Rtv_Map;
typedef struct _DebugLevel DebugLevel;
typedef struct _Group Group;
typedef struct _BindGroup BindGroup;
typedef struct _Ve_View_3D Ve_View_3D;
typedef struct _Vex_Import Vex_Import;
typedef struct _Logview_Data Logview_Data;
typedef struct _TTMon_Data TTMon_Data;
typedef struct _MultiExpr MultiExpr;
typedef struct _MultiSource MultiSource;
typedef struct _CmdLineArgs CmdLineArgs;

/*! 
 \brief _Serial_Params holds all variables related to the state of the serial
 port being used by MegaTunix
 */
struct _Serial_Params
{
        gint fd;		/*! File descriptor */
        gchar *port_name;	/*! textual name of comm port */
        gboolean open;		/*! flag, TRUE for open FALSE for closed */
        gint read_wait;		/*! time delay between each read */
        gint errcount;		/*! Serial I/O errors read error count */
        struct termios oldtio;	/*! serial port settings before we touch it */
        struct termios newtio;	/*! serial port settings we use when running */
};


/*!
 \brief _LookupTable is a mini-container holding hte filename and table
 info for each lookuptable stored in the LookupTables hashtable
 */
struct _LookupTable
{
	gint *array;		/*! the table itself */
	gchar *filename;	/*! The relative filename where 
				    this table came from */
};

/*! 
 \brief _Firmware_Details stores all attributes about the firmware being used
 after detection (\see interrogate_ecu ) including lists of tabs to be loaded
 by the glade loader (\see load_gui_tabs ), the configuration for the realtime
 variables map (\see rtv_map_loader) and the sliderss_map_file (\see 
 load_runtime_sliders )
 */
struct _Firmware_Details
{
	gchar *name;		/*! textual name */
	gchar *profile_filename;/*! Interrogation profile filename */
	gchar **tab_list;	/*! vector string of tabs to load */
	gchar **tab_confs;	/*! Tab configuration files */
	gchar *rtv_map_file;	/*! realtime vars map filename */
	gchar *sliders_map_file;/*! runtime sliders map filename */
	gchar *rtt_map_file;	/*! runtime text map filename */
	gchar *status_map_file;	/*! runtime status map filename */
	gchar *rt_cmd_key;	/*! string key to hashtable for RT command */
	gchar *ve_cmd_key;	/*! string key to hashtable for VE command */
	gchar *ign_cmd_key;	/*! string key to hashtable for Ign command */
	gchar *raw_mem_cmd_key;	/*! string key to hashtable for RAW command */
	gchar *SignatureVia;	/*! Key to retrieve signature string */
	gchar *TextVerVia;	/*! Key to retrieve text version string */
	gchar *NumVerVia;	/*! Key to retrieve numerical version string */
        gint rtvars_size;       /*! Size of Realtime vars datablock */
        gint ignvars_size;      /*! Size of Realtime vars datablock */
        gint memblock_size;     /*! Size of Raw_Memory datablock */
	gint capabilities;	/*! Enum list of capabilities*/
	gboolean multi_page;	/*! Multi-page firmware */
	gboolean chunk_support;	/*! Supports Chunk Write */
	gint total_pages;	/*! How many pages do we handle? */
	gint total_tables;	/*! How many tables do we handle? */
	gint ro_above;		/*! Read Only debug pages above this one */
	gint trigmon_page;	/*! Trigger Monitor RO Page */
	gint toothmon_page;	/*! Tooth Monitor RO Page */
	gchar *chunk_write_cmd;	/*! Command to send to chunk write data... */
	gchar *write_cmd;	/*! Command to send to write data... */
	gchar *burn_cmd;	/*! Command to send to burn data... */
	gchar *page_cmd;	/*! Command to send to change pages ... */
	Page_Params **page_params;/*! special vars per page */
	Table_Params **table_params;/*! details each table */
	Req_Fuel_Params **rf_params;/*! req_fuel params */
};


/*! 
 Controls for the Required Fuel Calculator... 
 \brief The _Req_Fuel struct contains jsut about all widgets for the rewuired
 fuel popup.  most of the values are loaded/saved from the main config file
 when used.
 */
struct _Reqd_Fuel
{
	GtkWidget *popup;		/*! the popup window */
	GtkWidget *calcd_val_spin;	/*! Preliminary value */
	GtkWidget *reqd_fuel_spin;	/*! Used value */
	gfloat calcd_reqd_fuel;		/*! calculated value... */
        gint disp;			/*! Engine size  1-1000 Cu-in */
        gint cyls;			/*! # of Cylinders  1-12 */
        gfloat rated_inj_flow;		/*! Rated injector flow */
        gfloat rated_pressure;		/*! Rated fuel pressure */
        gfloat actual_pressure;		/*! Actual fuel pressure */
        gfloat actual_inj_flow;		/*! injector flow rate (lbs/hr) */
        gfloat target_afr;		/*! Air fuel ratio 10-25.5 */
        gint page;			/*! Which page is this for */
	gint table_num;			/*! Which table this refers to */
	gboolean visible;		/*! Is it visible? */
};


/*!
 \brief _Io_File is the datastructure used for file I/O in the inport/export
 routines.
 \see vetable_import
 \see vetable_export
 */
struct _Io_File
{
	GIOChannel *iochannel;
	gchar *filename;
	FileIoType iotype;
};


/*!
 \brief _Viewable_Value is the datastructure bound 
 to every trace viewed in the logviewer. 
 */
struct _Viewable_Value
{
	GdkGC *font_gc;			/*! GC used for the fonts */
	GdkGC *trace_gc;		/*! GC used for the trace */
	PangoRectangle *log_rect;	/*! Logcial rectangle around text */
	PangoRectangle *ink_rect;	/*! Ink rectangle around text */
	GObject *object;		/*! object */
	gchar *vname;			/*! Name of widget being logged */
	gboolean is_float;		/*! TRUE or FALSE */
	gboolean force_update;		/*! flag to force update on addition */
	gboolean highlight;		/*! flag it highlight it.. */
	gint last_y;			/*! Last point on screen of trace */
	gint last_index;		/*! latest entryu into data array */
	gchar *data_source;		/*! Textual name of source */
	gfloat min;			/*! for auto-scaling */
	gfloat max;			/*! for auto-scaling */
	gfloat lower;			/*! hard limits to use for scaling */
	gfloat upper;			/*! hard limits to use for scaling */
	gfloat cur_low;			/*! User limits to use for scaling */
	gfloat cur_high;		/*! User limits to use for scaling */
	GArray *data_array;		/*! History of all values recorded */
	Log_Info *log_info;	/*! important */
};
	

struct _Dash_Gauge
{
	GObject *object;		/* Data stroage object for RT vars */
	gchar * source;			/* Source name (unused) */
	GtkWidget *gauge;		/* pointer to gaugeitself */
	GtkWidget *dash;		/* pointer to gauge parent */
};


/*! 
 \brief The _Rt_Slider struct contains info on the runtime display tab sliders
 as they are now stored in the config file and adjustable in position
 and placement and such..
 */
struct _Rt_Slider
{
	gchar *ctrl_name;	/*! Ctrl name in config file (key in hash) */
	GtkWidget *parent;	/*! Parent of the table below  */
	GtkWidget *label;	/*! Label in runtime display */
	GtkWidget *textval;	/*! Label in runtime display */
	GtkWidget *pbar;	/*! progress bar for the data */
	gint table_num;		/*! Refers to the table number in the profile*/
	gint tbl;		/*! Table number (0-3) */
	gint row;		/*! Starting row */
	gchar *friendly_name;	/*! text for Label above */
	gint lower;		/*! Lower limit */
	gint upper;		/*! Upper limit */
	GArray *history;	/*! where the data is from */
	gfloat last_percentage;	/*! last percentage of on screen slider */
	GObject *object;	/*! object of obsession.... */
	gboolean enabled;	/*! Pretty obvious */
	gint count;		/*! used to making sure things update */
	gint rate;		/*! used to making sure things update */
	gint last_upd;		/*! used to making sure things update */
	WidgetType class;	/*! Slider type... */
};


/*! 
 \brief The _Rt_Text struct contains info on the floating runtime var text 
 display
 */
struct _Rt_Text
{
	gchar *ctrl_name;	/*! Ctrl name in config file (key in hash) */
	GtkWidget *parent;	/*! Parent of the table below  */
	GtkWidget *name_label;	/*! Label in runtime display */
	GtkWidget *textval;	/*! Label in runtime display */
	gchar *friendly_name;	/*! text for Label above */
	GArray *history;	/*! where the data is from */
	GObject *object;	/*! object of obsession.... */
	gint count;		/*! used to making sure things update */
	gint rate;		/*! used to making sure things update */
	gint last_upd;		/*! used to making sure things update */
};


/*! 
 \brief The _Log_Info datastructure is populated when a datalog file is opened
 for viewing in the Datalog viewer.
 */
struct _Log_Info
{
	gint field_count;	/*! How many fields in the logfile */
	gchar *delimiter;	/*! delimiter between fields for this logfile */
	GArray *log_list;	/*! List of objects */
};


/*! 
 \brief The _Page_Params structure contains fields defining the size of the 
 page returned from the ECU, the VEtable, RPm and Load table base offsets and
 sizes, along with a flag signifying if it's a spark table
 */
struct _Page_Params
{
	gint length;		/*! How big this page is... */
	gint truepgnum;		/*! True pagenumber to send */
	gint is_spark;		/*! does this require alt write cmd? */
	gint spconfig_offset;	/*! Where spconfig value is located */
};


/*! 
 \brief The _Table_Params structure contains fields defining table parameters
 One struct is allocated per table, and multiple tables per page are allowed
 */
struct _Table_Params
{
	gboolean is_fuel;	/*! If true next 7 params must exist */
	gint dtmode_offset;	/*! DT mode offset (msns-e ONLY) */
	gint dtmode_page;	/*! DT mode page (msns-e ONLY) */
	gint cfg11_offset;	/*! Where config11 value is located */
	gint cfg12_offset;	/*! Where config12 value is located */
	gint cfg13_offset;	/*! Where config13 value is located */
	gint alternate_offset;	/*! Where alternate value is located */
	gint divider_offset;	/*! Where divider value is located */
	gint rpmk_offset;	/*! Where rpmk value is located */
	gint reqfuel_offset;	/*! Where reqfuel value is located */
	gint x_page;		/*! what page the rpm (X axis) resides in */
	gint x_base;		/*! where rpm table starts (X axis) */
	gint x_bincount;	/*! how many RPM bins (X axis) */
	gchar *table_name;	/*! Name for the 3D Table editor title */
	gboolean x_multi_source;/*! uses multiple keyed sources? */
	gchar *x_source_key;	/*! this is the key to sources_hash to 
				 *  get the search key for x_multi_hash
				 */
	gchar *x_multi_expr_keys;/*! keys to x_multi_hash */
	gchar *x_sources;	/*! comma sep list of sources */
	gchar *x_suffixes;	/*! comma sep list of suffixes */
	gchar *x_conv_exprs;	/*! comma sep list of x conv. expressions */
	gchar *x_precisions;	/*! comma sep list of precisions */
	GHashTable *x_multi_hash;/*! Hash table to store the above */
	gchar *x_source;	/*! X datasource for 3d displays */
	gchar *x_suffix;	/*! text suffix used on 3D view */
	gchar *x_conv_expr;	/*! x conversion expression */
	void *x_eval;		/*! evaluator for x variable */
	gint x_precision;	/*! how many decimal places */

	gint y_page;		/*! what page the load (Y axis) resides in */
	gint y_base;		/*! where load table starts  (Y Axis) */
	gint y_bincount;	/*! how many load bins (Y axis) */
	gboolean y_multi_source;/*! uses multiple keyed sources? */
	gchar *y_source_key;	/* text name of variable we find to determine
				 *  the correct key for y_multi_hash
				 */
	gchar *y_multi_expr_keys;/*! keys to x_multi_hash */
	gchar *y_sources;	/*! comma sep list of sources */
	gchar *y_suffixes;	/*! comma sep list of suffixes */
	gchar *y_conv_exprs;	/*! comma sep list of x conv. expressions */
	gchar *y_precisions;	/*! comma sep list of precisions */
	GHashTable *y_multi_hash;/*! Hash table to store the above */
	gchar *y_source;	/*! Y datasource for 3d displays */
	gchar *y_suffix;	/*! text suffix used on 3D view */
	gchar *y_conv_expr;	/*! y conversion expression */
	void *y_eval;		/*! evaluator for y variable */
	gint y_precision;	/*! how many decimal places */

	gint z_page;		/*! what page the vetable resides in */
	gint z_base;		/*! where the vetable starts */
	gboolean z_multi_source;/*! uses multiple keyed sources? */
	gchar *z_source_key;	/* text name of variable we find to determine
				 * the correct key for z_multi_hash
				 */
	gchar *z_multi_expr_keys;/*! keys to x_multi_hash */
	gchar *z_sources;	/*! comma sep list of sources */
	gchar *z_suffixes;	/*! comma sep list of suffixes */
	gchar *z_conv_exprs;	/*! comma sep list of x conv. expressions */
	gchar *z_precisions;	/*! comma sep list of precisions */
	GHashTable *z_multi_hash;/*! Hash table to store the above */
	gchar *z_source;	/*! Z datasource for 3d displays */
	gchar *z_suffix;	/*! text suffix used on 3D view */
	gchar *z_conv_expr;	/*! z conversion expression */
	void *z_eval;		/*! evaluator for z variable */
	gint z_precision;	/*! how many decimal places */
};


/*!
 \brief The _Detection_Test struct holds the basics for each ECU test.
 a friendly human readable test name (this matches up eith test names in the 
 actual profile), the actual_test string (a machine parsable form), and a 
 test_vector,  which is the result of splitting up the actual_test string into
 it's component parts. 
 */
struct _Detection_Test
{
	gchar *test_name;	/* Friendly test name, like "MS-II_RTvars" */
	gchar *test_desc;	/* Gui displayed test description */
	gchar *actual_test;	/* machine parsable test string */
	gchar **test_vector;	/* Vector split of test (csv split) */
	GArray *test_arg_types;	/* Array of enums describing test arguments */
	gint test_arg_count;	/* number of args in the test */
	gchar *result_str;	/* Result of test stored for matching */
	gint num_bytes;		/* Number of bytes returned for this test */

};



/*!
 \brief The _Req_Fuel_Params structure is used to store the current and last
 values of the interdependant required fuel parameters for the function
 that verifies req_fuel status and downloads it.  There is one structure
 allocated PER Table (even if the table's aren't fuel..)
 */
struct _Req_Fuel_Params
{
	gint num_squirts;
	gint num_cyls;
	gint num_inj;
	gint divider;
	gint alternate;
	gint last_num_squirts;
	gint last_num_cyls;
	gint last_num_inj;
	gint last_divider;
	gint last_alternate;
	gfloat req_fuel_total;
	gfloat last_req_fuel_total;
};


/*!
 \brief the _Command struct is used to store details on the commands that
 are valid for the ECU, they are loaded from a config file "tests" normally
 installed in /usr/local/share/MegaTunix/Interrogator/tests. There will be
 one Command struct created per command, and they are used to interrogate the
 target ECU.
 */
struct _Command
{
	gchar *string;		/*! command to get the data */
	gchar *desc;		/*! command description */
	gchar *key;		/*! key into cmd_details hashtable */
	gint len;		/*! Command length in chars to send */
	gboolean multipart;	/*! Multipart command? (raw_memory) */
	gint cmd_int_arg;	/*! multipart arg, integer */
	gboolean store_data;	/*! Store returned data ? */
	StoreType store_type;	/*! Store data where */
};


/*!
 \brief _Io_Message structure is used for passing data around in threads.c for
 kicking off commands to send data to/from the ECU or run specified handlers.
 messages and postfunctiosn can be bound into this strucutre to do some complex
 things with a simple subcommand.
 \see _Io_Cmds
 */
struct _Io_Message
{
	Io_Command cmd;		/*! Source command (initiator)*/
	CmdType command;	/*! Command type */
	gchar *out_str;		/*! Data sent to the ECU  for READ_CMD's */
	gint page;		/*! Virtual Page number */
	gint truepgnum;		/*! Physical page number */
	gint out_len;		/*! number of bytes in out_str */
	gint offset;		/*! used for RAW_MEMORY and more */
	GArray *funcs;		/*! List of functiosn to be dispatched... */
	InputHandler handler;	/*! Command handler for inbound data */
	void *payload;		/*! data passed along, arbritrary size.. */
	gboolean need_page_change; /*! flag to set if we need to change page */
};


/*
 \brief _Text_Message strcture is used for a thread to pass messages up
 a GAsyncQueue to the main gui thread for updating a textview in a thread
 safe manner. A dispatch queue runs 5 times per second checking for messages
 to dispatch...
 */
struct _Text_Message
{
	gchar *view_name;	/*! Textview name */
	gchar *tagname;		/*! Texttag to use */
	gchar *msg;		/*! message to display */
	gboolean count;		/*! display a counter */
	gboolean clear;		/*! Clear the window? */
};


/*
 \brief _QFunction strcture is used for a thread to pass messages up
 a GAsyncQueue to the main gui thread for running any arbritrary function
 by name.
 */
struct _QFunction
{
	gchar *func_name;	/*! Function Name */
	gint  dummy;		/*! filler for more shit later.. */
};


/*
 \brief _Widget_Update strcture is used for a thread to pass a widget update
 call up a GAsyncQueue to the main gui thread for updating a widget in 
 a thread safe manner. A dispatch queue runs 5 times per second checking 
 for messages to dispatch...
 */
struct _Widget_Update
{
	gchar *widget_name;	/*! Widget name */
	WidgetType type;	/*! what type of widget are we updating */
	gchar *msg;		/*! message to display */
};


/*!
 \brief _Io_Cmds stores the basic data for the critical megasquirt command
 codes. (realtime, VE, ign and spark) including the length of each of those
 commands.  
 \warning This really should be done a better way...
 */
struct _Io_Cmds
{
	gchar *realtime_cmd;	/*! Command sent to get RT vars.... */
	gint rt_cmd_len;	/*! length in bytes of rt_cmd_len */
	gchar *veconst_cmd;	/*! Command sent to get VE/Const vars.... */
	gint ve_cmd_len;	/*! length in bytes of veconst_cmd */
	gchar *ignition_cmd;	/*! Command sent to get Ignition vars.... */
	gint ign_cmd_len;	/*! length in bytes of ignition_cmd */
	gchar *raw_mem_cmd;	/*! Command sent to get raw_mem vars.... */
	gint raw_mem_cmd_len;	/*! length in bytes of raw_mem_cmd */
};


/*! 
 \brief _Output_Data A simple wrapper struct to pass data to the output 
 function which makes the function a lot simpler.
 */
struct _Output_Data
{
	gint canID;		/*! CAN Module ID (MS-II ONLY) */
	gint page;		/*! Page in ECU */
	gint offset;		/*! Offset in block */
	gint value;		/*! Value to send */
	gint len;		/*! Length of chunk write block */
	guchar *data;		/*! Block of data for chunk write */
	gboolean ign_parm;	/*! Ignition parameter, True or False */
	gboolean queue_update;	/*! If true queues a member widget update */
	WriteMode mode;		/*! Write mode enum */
};


/*! 
 \brief _RtvMap is the RealTime Variables Map structure, containing fields to
 access the realtime derived data via a hashtable, and via raw index. Stores
 timestamps of each incoming data byte for advanced future use.
 */
struct _Rtv_Map
{
	gint raw_total;		/*! Number of raw variables */
	gint derived_total;	/*! Number of derived variables */
	gchar **raw_list;	/*! Char List of raw variables by name */
	GArray *rtv_array;	/*! Realtime Values array of lists.. */
	GArray *ts_array;	/*! Timestamp array */
	GArray *rtv_list;	/*! List of derived vars IN ORDER */
	GHashTable *rtv_hash;	/*! Hashtable of rtv derived values indexed by
				 * it's internal name */
};


/*! 
 \brief _DebugLevel stores the debugging name, handler, class (bitmask) and 
 shift (forgot why this is here) and a enable/disable flag. Used to make the
 debugging core a little more configurable
 */
struct _DebugLevel
{
	gchar * name;		/*! Debugging name */
	gint	handler;	/*! Signal handler name */
	Dbg_Class dclass;	/*! Bit mask for this level (0-31) */
	Dbg_Shift dshift;	/*! Bit shift amount */
	gboolean enabled;	/*! Enabled or not? */
};


/*!
 \brief _Group holds common settings from groups of control as defined in a 
 datamap file.  This should reduce redundancy and significantly shrink the 
 datamap files.
 */
struct _Group
{
	gchar **keys;		/*! String array for key names */
	gint *keytypes;		/*! Int array of key types... */
	GObject *object;	/*! To hold the data cleanly */
	gint num_keys;		/* How many keys we hold */
	gint num_keytypes;	/* How many keytypes we hold */
	gint page;		/* page of this group of data */
};


/*!
 \brief _BindGroup is a small container used to pass multiple params into
 a function that is limited to a certain number of arguments...
 */
struct _BindGroup
{
	ConfigFile *cfgfile;	/*! where the configfile ptr goes... */
	GHashTable *groups;	/*! where the groups table goes */
};


/*!
 \brief the _Ve_View_3D structure contains all the field to create and 
 manipulate a 3D view of a MegaSquirt VE/Spark table, and should work in
 theory for any sized table
 */
struct _Ve_View_3D
{
	gint beginX;
	gint beginY;
	gint active_y;
	gint active_x;
	gfloat dt;
	gfloat sphi;
	gfloat stheta;
	gfloat sdepth;
	gfloat zNear;
	gfloat zFar;
	gfloat aspect;
	gfloat h_strafe;
	gfloat v_strafe;
	gfloat z_offset;
	gfloat x_trans;
	gfloat y_trans;
	gfloat z_trans;
	gfloat x_scale;
	gfloat y_scale;
	gfloat z_scale;
	gfloat x_max;
	gfloat y_max;
	gfloat z_max;
	gint x_precision;
	gint y_precision;
	gint z_precision;
	/* Simple sources*/
	gchar *x_source;
	gchar *x_suffix;
	gchar *x_conv_expr;
	void *x_eval;
	gchar *y_source;
	gchar *y_suffix;
	gchar *y_conv_expr;
	void *y_eval;
	gchar *z_source;
	gchar *z_suffix;
	gchar *z_conv_expr;
	void *z_eval;
	/* Multi-sources */
	gchar * x_source_key;
	gboolean x_multi_source;
	GHashTable *x_multi_hash;
	gchar * y_source_key;
	gboolean y_multi_source;
	GHashTable *y_multi_hash;
	gchar * z_source_key;
	gboolean z_multi_source;
	GHashTable *z_multi_hash;

	GtkWidget *drawing_area;
	GtkWidget *window;
	GtkWidget *burn_but;
	GObject *dep_obj;
	gint y_base;
	gint y_page;
	gint y_bincount;
	gint x_base;
	gint x_page;
	gint x_bincount;
	gint z_base;
	gint z_page;
	gchar *table_name;
	gint table_num;
	gboolean tracking_focus;
	gboolean fixed_scale;
	GtkWidget *tracking_button;
};


/*!
 \brief The Vex_Import structure holds all fields (lots) needed to load and
 process a VEX (VEtabalt eXport file) and load it into megatunix.
 \see vetable_import
 \see vetable_export
 */
struct _Vex_Import
{	
	gchar *version;		/* String */
	gchar *revision;	/* String */
	gchar *comment;		/* String */
	gchar *date;		/* String */
	gchar *time;		/* String */
	gint page;		/* Int */
	gint table;		/* Int */
	gint total_x_bins;	/* Int */
	gint *x_bins;		/* Int Array, dynamic */
	gint total_y_bins;	/* Int */
	gint *y_bins;		/* Int Array, dynamic */
	gint total_tbl_bins;	/* Int */
	gint *tbl_bins;		/* Int Array, dynamic */
	gboolean got_page;	/* Flag */
	gboolean got_rpm;	/* Flag */
	gboolean got_load;	/* Flag */
	gboolean got_ve;	/* Flag */
	
};


/*!
 \brief The _Logview_Data struct is a ontainer used within the logviewer_gui.c
 file used to store settigns specific to the logviewer including th pointer to
 the drawing area, and a hashtable and list of pointers to the trace 
 datastructures.
 */
struct _Logview_Data
{
	GdkGC *highlight_gc;	/*! GC used for the highlight */
	GtkWidget *darea;	/*! Trace drawing area... */
	GdkPixmap *pixmap;	/*! pointer to backing pixmap... */
	GdkPixmap *pmap;	/*! pointer to Win32 pixmap hack!!! */
	GHashTable *traces;	/*! Hash table of v_values key'd by name */
	GList *tlist;		/*! Doubly linked lists of v_Values*/
	GList *used_colors;	/*! List of colors in use.... */
	gint active_traces;	/*! how many are active */
	gint spread;		/*! Pixel spread between trace info blocks */
	gint tselect;		/*! Trace that is currently selected */
	PangoFontDescription *font_desc; /*! Font used for text... */
	gint info_width;	/*! Width of left info area */
};

/*!
 * \brief _TTMon_Data struct is a container used to hold private data
 * for the Trigger and Tooth Loggers (MSnS-E only)
 */
struct _TTMon_Data
{
	gint page;		/*! page used to discern them apart */
	GdkPixmap *pixmap;	/*! Pixmap */
	GtkWidget *darea;	/*! Pointer to drawing area */
	gint min_time;		/*! Minimum, trigger/tooth time */
	gint num_maxes;		/*! Hot many long pips per block */
	gint mins_inbetween;	/*! How many normal teeth */
	gint max_time;		/*! Maximum, trigger/tooth time */
	gint midpoint_time;	/*! avg between min and max */
	gint est_teeth;		/*! Estimated number of teeth */
	gint units;		/*! Units multiplier */
	gfloat usable_begin;	/*! Usable begin point for bars */
	gfloat font_height;	/*! Font height needed for some calcs */
	GArray *current;	/*! Current block of times */
	GArray *last;		/*! Last block of times */
	gint wrap_pt;		/*! Wrap point */
	gint vdivisor;		/*! Vertical scaling divisor */
	gfloat peak;		/*! Vertical Peak Value */
	PangoFontDescription *font_desc;	/*! Pango Font Descr */
	PangoLayout *layout;	/*! Pango Layout */
	GdkGC *axis_gc;		/*! axis graphics context */
	GdkGC *trace_gc;	/*! axis graphics context */

};


/*!
 * \brief _MultiExpr is a container struct used for Realtime var processing
 * for vars that depend on ECU state (MAP, need to know current sensor)
 */
struct _MultiExpr
{
	gint lower_limit;	/* Lower limit */	
	gint upper_limit;	/* Upper limit */
	gchar *lookuptable;	/* textual lookuptable name */
	gchar *dl_conv_expr;	/* download (to ecu) conv expression */
	gchar *ul_conv_expr;	/* upload (from ecu) conv expression */
	void *dl_eval;		/* evaluator for download */
	void *ul_eval;		/* evalutator for upload */
};


/*!
 * \brief _MultiSource is a container struct used for Table data handling 
 * for the x/y/z axis's for properly scaling and displaying things on the
 * 3D displays as well as the 2D table scaling.  This allows things to be
 * significantly more adaptable 
 */
struct _MultiSource
{
	gchar *source;		/* name of rtvars datasource */
	gchar *conv_expr;	/* conversion expression ms units to real */
	void * evaluator;	/* evaluator pointer */
	gchar * suffix;		/* textual suffix for this evaluator*/
	gint precision;		/* Precision for floating point */
};


/*!
 * \brief _Args struct is a container to hold the command line argument
 * related variables, used to make mtx quiet, suppress portions of the gui
 * and autolog to files.
 */
struct _CmdLineArgs
{
	gboolean debug;		/* Debug to console */
	gboolean version;	/* Show Version */
	gboolean be_quiet;	/* No error popups */
	gboolean autolog_dump;	/* Automatically dump full logs periodically */
	gboolean hide_rttext;	/* Hide Runtime Variable Window */
	gboolean hide_status;	/* Hide Status Window */
	gboolean hide_maingui;	/* Hide Main Gui (Dash only mode */
	gint autolog_minutes;	/* How many minutes to log per file */
	gchar * autolog_dump_dir;/* What dir to put logs into */
	gchar *autolog_basename;/* Autolog base filename */
};

#endif
