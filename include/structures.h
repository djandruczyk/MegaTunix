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

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <termios.h>
#include <unistd.h>


/*! 
 \brief Serial_Params holds all variables related to the state of the serial
 port being used by MegaTunix
 */
struct Serial_Params
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
 \brief Firmware_Details stores all attributes about the firmware being used
 after detection (\see interrogate_ecu ) including lists of tabs to be loaded
 by the glade loader (\see load_gui_tabs ), the configuration for the realtime
 variables map (\see rtv_map_loader) and the sliderss_map_file (\see 
 load_runtime_sliders )
 */
struct Firmware_Details
{
	gchar *name;		/*! textual name */
	gchar **tab_list;	/*! vector string of tabs to load */
	gchar **tab_confs;	/*! Tab configuration files */
	gchar *rtv_map_file;	/*! realtime vars map filename */
	gchar *sliders_map_file;/*! runtime sliders map filename */
        gint rtvars_size;       /*! Size of Realtime vars datablock */
        gint ignvars_size;      /*! Size of Realtime vars datablock */
        gint memblock_size;     /*! Size of Raw_Memory datablock */
	gboolean multi_page;	/*! Multi-page firmware? */
	gint total_pages;	/*! How many pages do we handle? */
	gint total_tables;	/*! How many tables do we handle? */
	gchar *write_cmd;	/*! Command to send to write data... */
	gchar *burn_cmd;	/*! Command to send to burn data... */
	struct Page_Params **page_params;/*! special vars per page */
	struct Table_Params **table_params;/*! details each table */
};

/*! 
 Controls for the Required Fuel Calculator... 
 \brief The Req_Fuel struct contains jsut about all widgets for the rewuired
 fuel popup.  most of the values are loaded/saved from the main config file
 when used.
 */
struct Reqd_Fuel
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
	gboolean visible;		/*! Is it visible? */
};


/*!
 \brief Io_File is the datastructure used for file I/O in the inport/export
 routines.
 \see vetable_import
 \see vetable_export
 */
struct Io_File
{
	GIOChannel *iochannel;
	gchar *filename;
	FileIoType iotype;
};

/*!
 \brief Viewable_Value is the datastructure bound 
 to every trace viewed in the logviewer. 
 */
struct Viewable_Value
{
	GdkGC *font_gc;			/*! GC used for the fonts */
	GdkGC *trace_gc;		/*! GC used for the trace */
	GtkWidget *d_area;		/*! Drawing Area */
	GObject *object;		/*! object */
	gchar *vname;			/*! Name of widget being logged */
	gboolean is_float;		/*! TRUE or FALSE */
	gint last_y;			/*! Last point on screen of trace */
	gint last_index;		/*! latest entryu into data array */
	gfloat min;			/*! for auto-scaling */
	gfloat max;			/*! for auto-scaling */
	gfloat lower;			/*! hard limits to use for scaling */
	gfloat upper;			/*! hard limits to use for scaling */
	gfloat cur_low;			/*! User limits to use for scaling */
	gfloat cur_high;		/*! User limits to use for scaling */
	GArray *data_array;		/*! History of all values recorded */
	struct Log_Info *log_info;	/*! important */
};
	
/*! 
 \brief The Rt_Slider struct contains info on the runtime display tab sliders
 as they are now stored in the config file and adjustable in position
 and placement and such..
 */
struct Rt_Slider
{
	gchar *ctrl_name; /*! Control name in config file (key in hashtable) */
	GtkWidget *parent;/*! Parent of the table below  */
	GtkWidget *table; /*! Table to contain the next 3 widgets */
	GtkWidget *label; /*! Label in runtime display */
	GtkWidget *textval; /*! Label in runtime display */
	GtkWidget *pbar;  /*! progress bar for the data */
	gint tbl;	  /*! Table number (0-3) */
	gint row;	  /*! Starting row */
	gchar *friendly_name; /*! text for Label above */
	gint lower;	 /*! Offset into limits[] structure array */
	gint upper;	 /*! offset into Runtime_Common struct (using []) */
	gfloat * history;/*! where the data is from the runtime section */
	GObject *object;  /*! object of obsession.... */
	gboolean enabled; /*! Pretty obvious */
	gint count;	  /*! used to making sure things update */
	gint rate;	  /*! used to making sure things update */
	gint last_upd;	  /*! used to making sure things update */
};


/*! 
 \brief the Default_Limits struct maps field names to limits. used for 
 importing datalogs from megatune mainly as that peice of work 
 uses different field names per log type/version.
 */
struct Default_Limits
{
	gchar *field;		/*! Field name */
	gfloat lower;		/*! Lower Limit */
	gfloat upper;		/*! Upper Limit */
};

/*! 
 \brief The Log_Info datastructure is populated when a datalog file is opened
 for viewing in the Datalog viewer.
 */
struct Log_Info
{
	gint field_count;	/*! How many fields in the logfile */
	gchar *delimiter;	/*! delimiter between fields for this logfile */
	GArray *log_list;	/*! List of objects */
	gint active_viewables;	/*! Number of active traces.. */
	gfloat global_position;	/*! Where we are in the logviewer (playback) */
};

/*! 
 \brief The Page_Params structure contains fields defining the size of the 
 page returned from the ECU, the VEtable, RPm and Load table base offsets and
 sizes, along with a flag signifying if it's a spark table
 */
struct Page_Params
{
	gint is_spark;		/*! What page this is */
	gint size;		/*! total size of this page as returned... */
	gint cfg11_offset;	/*! Where config11 value is located */
	gint cfg12_offset;	/*! Where config12 value is located */
	gint cfg13_offset;	/*! Where config13 value is located */
	gint alternate_offset;	/*! Where alternate value is located */
	gint divider_offset;	/*! Where divider value is located */
	gint rpmk_offset;	/*! Where rpmk value is located */
	gint reqfuel_offset;	/*! Where reqfuel value is located */
};


/*! 
 \brief The Table_Params structure contains fields defining table parameters
 One struct is allocated per table, and multiple tables per page are allowed
 */
struct Table_Params
{
	gint page;		/*! What page this table lies in */
	gint tbl_base;		/*! where the vetable starts */
	gint rpm_base;		/*! where rpm table starts */
	gint load_base;		/*! where load table starts */
	gint rpm_bincount;	/*! how many RPM bins */
	gint load_bincount;	/*! how many load bins... */
	gboolean is_spark;	/*! is this a spark table?? */
};


/*! 
 \brief The Canidate structure is used for interrogation, each potential
 firmware is interrogated and as it is the data is fed into this structure
 for comparison against know values (interrogation profiles), upon a match
 the needed data is copied into a Firmware_Details structure
 \see Firmware_Details
 */
struct Canidate
{
	gchar *name;		/*! Name of this firmware */
	gchar *filename;	/*! absolute path to this canidate profile */
	GHashTable *bytecounts;	/*! byte count for each of the 10 test cmds */
	gchar *sig_str;		/*! Signature string to search for */
	gchar *quest_str;	/*! Ext Version string to search for */
	gint ver_num;		/*! Version number to search for */
	gchar *load_tabs;	/*! list of tabs to load into the gui */
	gchar *tab_confs;	/*! Tab configuration files */
	gchar *rtv_map_file;	/*! name of realtime vars map file */
	gchar *sliders_map_file;/*! runtime sliders map filename */
	Capability capabilities;/*! Bitmask of capabilities.... */
	gchar *rt_cmd_key;	/*! string key to hashtable for RT command */
	gchar *ve_cmd_key;	/*! string key to hashtable for VE command */
	gchar *ign_cmd_key;	/*! string key to hashtable for Ign command */
	gchar *raw_mem_cmd_key;	/*! string key to hashtable for RAW command */
	gchar *write_cmd;	/*! Command to send to write data... */
	gchar *burn_cmd;	/*! Command to send to burn data... */
	gboolean multi_page;	/*! Multi-page firmware ??? */
	gint total_pages;	/*! how many pages do we handle? */
	gint total_tables;	/*! how many tables do we handle? */
	GHashTable *lookuptables;/*! Lookuptables hashtable... */
	struct Page_Params **page_params;/*! special vars per page */
	struct Table_Params **table_params;/*! details on ve/rpm/load tables*/
};

/*!
 \brief the Command struct is used to store details on the commands that
 are valid for the ECU, they are loaded from a config file "tests" normally
 installed in /usr/local/share/MegaTunix/Interrogator/tests. There will be
 one Command struct created per command, and they are used to interrogate the
 target ECU.
 */
struct Command
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
 \brief Io_Message structure is used for passing data around in threads.c for
 kicking off commands to send data to/from the ECU or run specified handlers.
 messages and postfunctiosn can be bound into this strucutre to do some complex
 things with a simple subcommand.
 \see Io_Cmds
 */
struct Io_Message
{
	Io_Command cmd;		/*! Source command (initiator)*/
	CmdType command;	/*! Command type */
	gchar *out_str;		/*! Data sent to the ECU  for READ_CMD's */
	gint page;		/*! multipage firmware specific */
	gint out_len;		/*! number of bytes in out_str */
	gint offset;		/*! used for RAW_MEMORY and more */
	GArray *funcs;		/*! List of functiosn to be dispatched... */
	InputHandler handler;	/*! Command handler for inbound data */
	void *payload;		/*! data passed along, arbritrary size.. */
};

/*!
 \brief Io_Cmds stores the basic data for the critical megasquirt command
 codes. (realtime, VE, ign and spark) including the length of each of those
 commands.  
 \warning This really should be done a better way...
 */
struct Io_Cmds
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
 \brief OutputData A simple wrapper struct to pass data to the output 
 function which makes the function a lot simpler.
 */
struct OutputData
{
	gint page;		/*! Page in ECU */
	gint offset;		/*! Offset in block */
	gint value;		/*! Value to send */
	gboolean ign_parm;	/*! Ignition parameter, True or False */
};


/*! 
 \brief RtvMap is the RealTime Variables Map structure, containing fields to
 access the realtime derived data via a hashtable, and via raw index. Stores
 timestamps of each incoming data byte for advanced future use.
 */
struct Rtv_Map
{
	gint raw_total;		/*! Number of raw variables */
	gint derived_total;	/*! Number of derived variables */
	gchar **raw_list;	/*! Char List of raw variables by name */
	GArray *rtv_array;	/*! Realtime Values array of lists.. */
	GTimeVal *ts_array;	/*! Timestamp array */
	gint ts_position;	/*! Latest timestamp marker position.. */
	gint ts_max;		/*! # elements in the timestamp array */
	GArray *rtv_list;	/*! List of derived vars IN ORDER */
	GHashTable *rtv_hash;	/*! Hashtable of rtv derived values indexed by
				 * it's internal name */
};


/*! 
 \brief DebugLevel stores the debugging name, handler, class (bitmask) and 
 shift (forgot why this is here) and a enable/disable flag. Used to make the
 debugging core a little more configurable
 */
struct DebugLevel
{
	gchar * name;		/*! Debugging name */
	gint	handler;	/*! Signal handler name */
	Dbg_Class dclass;	/*! Bit mask for this level (0-31) */
	Dbg_Shift dshift;	/*! Bit shift amount */
	gboolean enabled;	/*! Enabled or not? */
};

#endif
