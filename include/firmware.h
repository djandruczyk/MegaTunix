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

#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <req_fuel.h>


typedef struct _Io_File Io_File;
typedef struct _Firmware_Details Firmware_Details;
typedef struct _Table_Params Table_Params;
typedef struct _Page_Params Page_Params;


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
	gchar *actual_signature;/*! the raw signature from the ECU */
	gchar **tab_list;	/*! vector string of tabs to load */
	gchar **tab_confs;	/*! Tab configuration files */
	gchar *rtv_map_file;	/*! realtime vars map filename */
	gchar *sliders_map_file;/*! runtime sliders map filename */
	gchar *rtt_map_file;	/*! runtime text map filename */
	gchar *status_map_file;	/*! runtime status map filename */
	gchar *rt_command;	/*! New RT Command string */
	gchar *get_all_command;	/*! New Get All Command string */
	gchar *ve_command;	/*! New VE Command string */
	gchar *write_command;	/*! New Write Command string */
	gchar *chunk_write_command;	/*! New Chunk Write Command string */
	gchar *burn_all_command;/*! New burn all command string */
	gchar *burn_command;	/*! New burn command string */
	gchar *raw_mem_command;	/*! New raw_mem command string */
	gchar *page_cmd;	/*! Command to send to change pages ... */
	gchar *SignatureVia;	/*! Key to retrieve signature string */
	gchar *TextVerVia;	/*! Key to retrieve text version string */
	gchar *NumVerVia;	/*! Key to retrieve numerical version string */
	gint interchardelay;	/*! Inter char delay (MS-II mostly) */
	gint ro_above;		/*! read only above this page */
	gint canID;		/*! CanID for this firmware.. */
	gint rtvars_size;       /*! Size of Realtime vars datablock */
	gint ms2_rt_page;	/*! Page where the MS2 RT vars reside */
	gint memblock_size;     /*! Size of Raw_Memory datablock */
	gint capabilities;	/*! Enum list of capabilities*/
	gboolean multi_page;	/*! Multi-page firmware */
	gboolean chunk_support;	/*! Supports Chunk Write */
	gboolean can_capable;	/*! Supports CAnbus and sub modules */
	gint total_pages;	/*! How many pages do we handle? */
	gint total_tables;	/*! How many tables do we handle? */
	gint trigmon_page;	/*! Trigger Monitor RO Page */
	gint toothmon_page;	/*! Tooth Monitor RO Page */
	Page_Params **page_params;/*! special vars per page */
	Table_Params **table_params;/*! details each table */
	Req_Fuel_Params **rf_params;/*! req_fuel params */
	guint8 **ecu_data;	/* ECU data arrays, 2 levels */
	guint8 **ecu_data_last;	/* ECU data arrays, 2 levels */
	guint8 **ecu_data_backup;	/* ECU data arrays, 2 levels */
};


/*! 
 \brief The _Table_Params structure contains fields defining table parameters
 One struct is allocated per table, and multiple tables per page are allowed
 This differs a bit in lingo to how the MS-II developers named things in 
 their code.  On those,  table is analagous to "page",  why they used the 
 language they did is beyond me, but it makes little sense to say "fuel table
 2 inside of table5,  makes more sense to say "fuel table 2 in page5".
 This structure defines a specific fuel/spark/afr/other table arranged in a 
 grid format.  Current design restriction is that a table can't span more
 than on CAN IDs which only makes sense.
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
	DataSize x_size;	/*! enumeration size for the var */
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
	DataSize y_size;	/*! enumeration size for the var */
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
	DataSize z_size;	/*! enumeration size for the var */
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
 \brief The _Page_Params structure contains fields defining the size of the 
 page returned from the ECU, the VEtable, RPm and Load table base offsets and
 sizes, along with a flag signifying if it's a spark table
 */
struct _Page_Params
{
	gint length;		/*! How big this page is... */
	gint truepgnum;		/*! True pagenumber to send */
	gboolean dl_by_default;	/*! Download this page or not? */
	gint is_spark;		/*! does this require alt write cmd? */
	gint spconfig_offset;	/*! Where spconfig value is located */
};


/* Prototypes */
void load_firmware_file(Io_File *);

/* Prototypes */

#endif
