/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/firmware.h
  \ingroup Headers
  \brief Global firmware structure. This should be refactored to be fully
  ECU agnostic
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#include <enums.h>
#include <gtk/gtk.h>



typedef enum
{	/* up to 32 Capability flags.... */
	/* No capabilities =        = Standard B&G code with no modifications */
	MS1                = 1<<0,
	MS1_STD            = 1<<1,
	MS1_E              = 1<<2,
	MS1_DT             = 1<<3,
	MS2                = 1<<4,
	MS2_STD            = 1<<5,
	MS2_E              = 1<<6,
	MS2_E_COMPMON      = 1<<7,
	MS3			       = 1<<8,
	MS3_NEWSERIAL      = 1<<9,
	PIS                = 1<<10,
	LIBREEMS           = 1<<11,
	JIMSTIM            = 1<<12
}Capability;


typedef struct _Io_File Io_File;
typedef struct _Firmware_Details Firmware_Details;
typedef struct _Table_Params Table_Params;
typedef struct _TE_Params TE_Params;
typedef struct _Page_Params Page_Params;
typedef struct _Deferred_Data Deferred_Data;
typedef struct _Req_Fuel_Params Req_Fuel_Params;


/*!
 \brief The _Req_Fuel_Params structure is used to store the current and last
 values of the interdependant required fuel parameters for the function
 that verifies req_fuel status and downloads it.  There is one structure
 allocated PER Table (even if the table's aren't fuel..)
 */
struct _Req_Fuel_Params
{
	gint num_squirts;		/*!< Number of squirts */
	gint num_cyls;			/*!< Number of Cylidners */
	gint num_inj;			/*!< Number of Injectors */
	gint divider;			/*!< Divisor factor */
	gint alternate;			/*!< Alt/Simultaneous */
	gint last_num_squirts;		/*!< last num_squirts */
	gint last_num_cyls;		/*!< last num_cyls */
	gint last_num_inj;		/*!< last num_inj */
	gint last_divider;		/*!< last divider */
	gint last_alternate;		/*!< last alt/simul */
	gfloat req_fuel_total;		/*!< Req Fuel Total */
	gfloat last_req_fuel_total;	/*!< Last reqfuel_total */
};



/*!
 \brief _Io_File is the datastructure used for file I/O in the inport/export
 routines.
 \see vetable_import
 \see vetable_export
 */
struct _Io_File
{
	GIOChannel *iochannel;		/*!< IO Channel for File Stream */
	gchar *filename;		/*!< File Name */
	FileIoType iotype;		/*!< Inpout or Output enumeration */
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
	gchar *name;		/*!< textual name */
	gchar *profile_filename;/*!< Interrogation profile filename */
	gchar *actual_signature;/*!< the raw signature from the ECU */
	gint signature_len;	/*!< Length of signature in bytes */
	gchar *text_revision;	/*!< Textual revision string */
	gint txt_rev_len;	/*!< Length of txt_rev in bytes */
	gchar **tab_list;	/*!< vector string of tabs to load */
	gchar **tab_confs;	/*!< Tab configuration files */
	gchar *rtv_map_file;	/*!< realtime vars map filename */
	gchar *sliders_map_file;/*!< runtime sliders map filename */
	gchar *rtt_map_file;	/*!< runtime text map filename */
	gchar *status_map_file;	/*!< runtime status map filename */
	gchar *rt_command;	/*!< New RT Command string */
	gchar *get_all_command;	/*!< New Get All Command string */
	gchar *read_command;	/*!< New Read Command string */
	gchar *write_command;	/*!< New Write Command string */
	gchar *table_write_command;	/*!< Table Write Command string */
	gchar *chunk_write_command;	/*!< New Chunk Write Command string */
	gchar *burn_all_command;/*!< New burn all command string */
	gchar *burn_command;	/*!< New burn command string */
	gchar *raw_mem_command;	/*!< New raw_mem command string */
	gchar *page_command;	/*!< New page change command */
	gchar *SignatureVia;	/*!< Key to retrieve signature string */
	gchar *TextVerVia;	/*!< Key to retrieve text version string */
	gchar *NumVerVia;	/*!< Key to retrieve numerical version string */
	TempUnits ecu_temp_units;	/*!< Scale of ecu temp scaled units */
	gint interchardelay;	/*!< Inter char delay (MS-II mostly) */
	gint ro_above;		/*!< read only above this page */
	gint canID;		/*!< CanID for this firmware.. */
	gint rtvars_size;       /*!< Size of Realtime vars datablock */
	gint ms2_rt_page;	/*!< Page where the MS2 RT vars reside */
	gint memblock_size;     /*!< Size of Raw_Memory datablock */
	gint capabilities;	/*!< Enum list of capabilities*/
	gint ecu_revision;	/*!< Numeric ECU revision */
	gboolean bigendian;	/*!< ECU use Big Endian byte ordering */
	gboolean multi_page;	/*!< Multi-page firmware */
	gboolean chunk_support;	/*!< Supports Chunk Write */
	gboolean can_capable;	/*!< Supports CAnbus and sub modules */
	gint total_pages;	/*!< How many pages do we handle? */
	gint total_te_tables;	/*!< How many te tables do we handle? */
	gint total_tables;	/*!< How many tables do we handle? */
	gint trigmon_page;	/*!< Trigger Monitor RO Page */
	gint clt_table_page;	/*!< Where CLT table is stored */
	gint mat_table_page;	/*!< Where MAT table is stored */
	gint ego_table_page;	/*!< Where EGO table is stored */
	gint maf_table_page;	/*!< Where MAF table is stored */
	gint toothmon_page;	/*!< Tooth Monitor RO Page */
	gint compositemon_page;	/*!< Composite Monitor RO Page (ms2-extra only)*/
	Page_Params **page_params;/*!< special vars per page */
	Table_Params **table_params;/*!< details each table */
	Req_Fuel_Params **rf_params;/*!< req_fuel params */
	TE_Params **te_params;	/*!< Table Editor Tables */
	guint8 *rt_data;	/*!< RT data */
	guint8 *rt_data_last;	/*!< RT data */
	guint8 **ecu_data;	/*!< ECU data arrays, 2 levels */
	guint8 **ecu_data_last;	/*!< ECU data arrays, 2 levels */
	guint8 **ecu_data_backup;	/*!< ECU data arrays, 2 levels */
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
	gboolean is_spark;	/*!< Ignition map generator can write */
	gboolean is_fuel;	/*!< If true next 7 params must exist */
	gint dtmode_page;	/*!< DT mode page (msns-e ONLY) */
	gint dtmode_offset;	/*!< DT mode offset (msns-e ONLY) */
	gint dtmode_mask;	/*!< DT mode mask (msns-e ONLY) */
	gint num_cyl_page;	/*!< page where num_cylinders value is located */
	gint num_cyl_offset;	/*!< Where num_cylinders value is located */
	gint num_cyl_mask;	/*!< MASK for num_cyliners */
	gint num_inj_page;	/*!< page where num_injectors value is located */
	gint num_inj_offset;	/*!< Where num_injectors value is located */
	gint num_inj_mask;	/*!< MASK for num_injectors */
	gint stroke_page;	/*!< page where stroke value is located */
	gint stroke_offset;	/*!< Where stroke value is located */
	gint stroke_mask;	/*!< MASK for stroke */
	gint alternate_page;	/*!< page Where alternate value is located */
	gint alternate_offset;	/*!< Where alternate value is located */
	gint divider_page;	/*!< page Where divider value is located */
	gint divider_offset;	/*!< Where divider value is located */
	gint rpmk_page;		/*!< page Where rpmk value is located */
	gint rpmk_offset;	/*!< Where rpmk value is located */
	gint reqfuel_page;	/*!< page Where reqfuel value is located */
	gint reqfuel_offset;	/*!< Where reqfuel value is located */
	DataSize reqfuel_size;	/*!< Size of variable, (ms2 uses larger) */
	gint x_page;		/*!< what page the rpm (X axis) resides in */
	gint x_base;		/*!< where rpm table starts (X axis) */
	gint x_raw_lower;	/*!< Raw lower value Limit */
	gint x_raw_upper;	/*!< Raw upper value Limit */
	DataSize x_size;	/*!< enumeration size for the var */
	gint x_bincount;	/*!< how many RPM bins (X axis) */
	gchar *table_name;	/*!< Name for the 3D Table editor title */
	gchar *bind_to_list;	/*!< Bind to list for sensitivity */
	MatchType match_type;	/*!< Match type for sensitivity */
	gboolean x_multi_source;/*!< uses multiple keyed sources? */
	gchar *x_source_key;	/*!< this is the key to sources_hash to 
				 *  get the search key for x_multi_hash
				 */
	gchar *x_multi_expr_keys;/*!< keys to x_multi_hash */
	gchar *x_sources;	/*!< comma sep list of sources */
	gchar *x_suffixes;	/*!< comma sep list of suffixes */
	gchar *x_fromecu_mults;	/*!< comma sep list of x conv. multiplier */
	gchar *x_fromecu_adds;	/*!< comma sep list of x conv. adders */
	gfloat *x_fromecu_mult;	/*!< multiplier */
	gfloat *x_fromecu_add;	/*!< multiplier */
	gchar *x_precisions;	/*!< comma sep list of precisions */
	gchar *x_lookuptables;	/*!< comma sep list of lookuptables */
	GHashTable *x_multi_hash;/*!< Hash table to store the above */
	gchar *x_source;	/*!< X datasource for 3d displays */
	gchar *x_suffix;	/*!< text suffix used on 3D view */
	gboolean x_ignore_algorithm;	/*!< If don't heed algorithms */
	gboolean x_complex;	/*!< If using complex math */
	gchar *x_fromecu_conv_expr;	/*!< x conversion expression */
	gchar *x_toecu_conv_expr;	/*!< x conversion expression */
	void *x_ul_eval;	/*!< evaluator for x variable */
	void *x_dl_eval;	/*!< evaluator for x variable */
	gint x_precision;	/*!< how many decimal places */
	GObject *x_object;	/*!< Container for lookuptable deps */

	gint y_page;		/*!< what page the load (Y axis) resides in */
	gint y_base;		/*!< where load table starts  (Y Axis) */
	gint y_raw_lower;	/*!< Raw lower value Limit */
	gint y_raw_upper;	/*!< Raw upper value Limit */
	DataSize y_size;	/*!< enumeration size for the var */
	gint y_bincount;	/*!< how many load bins (Y axis) */
	gboolean y_multi_source;/*!< uses multiple keyed sources? */
	gchar *y_source_key;	/*!< text name of variable we find to determine
				 *  the correct key for y_multi_hash
				 */
	gchar *y_multi_expr_keys;/*!< keys to y_multi_hash */
	gchar *y_sources;	/*!< comma sep list of sources */
	gchar *y_suffixes;	/*!< comma sep list of suffixes */
	gchar *y_fromecu_mults;	/*!< comma sep list of x conv. multipliers */
	gchar *y_fromecu_adds;	/*!< comma sep list of x conv. adders */
	gfloat *y_fromecu_mult;	/*!< multiplier */
	gfloat *y_fromecu_add;	/*!< multiplier */
	gchar *y_precisions;	/*!< comma sep list of precisions */
	gchar *y_lookuptables;	/*!< comma sep list of lookuptables */
	GHashTable *y_multi_hash;/*!< Hash table to store the above */
	gchar *y_source;	/*!< Y datasource for 3d displays */
	gchar *y_suffix;	/*!< text suffix used on 3D view */
	gboolean y_ignore_algorithm;	/*!< If don't heed algorithms */
	gboolean y_complex;	/*!< If using complex math */
	gchar *y_fromecu_conv_expr;	/*!< y conversion expression */
	gchar *y_toecu_conv_expr;	/*!< y conversion expression */
	void *y_ul_eval;	/*!< evaluator for y variable */
	void *y_dl_eval;	/*!< evaluator for y variable */
	gint y_precision;	/*!< how many decimal places */
	GObject *y_object;	/*!< Container for lookuptable deps */

	gint z_page;		/*!< what page the vetable resides in */
	gint z_base;		/*!< where the vetable starts */
	gint z_raw_lower;	/*!< Raw lower value Limit */
	gint z_raw_upper;	/*!< Raw upper value Limit */
	DataSize z_size;	/*!< enumeration size for the var */
	gboolean z_multi_source;/*!< uses multiple keyed sources? */
	gchar *z_source_key;	/*!< text name of variable we find to determine
				 * the correct key for z_multi_hash
				 */
	gchar *z_multi_expr_keys;/*!< keys to z_multi_hash */
	gchar *z_sources;	/*!< comma sep list of sources */
	gchar *z_suffixes;	/*!< comma sep list of suffixes */
	gchar *z_fromecu_mults;	/*!< comma sep list of x conv. multiplies */
	gchar *z_fromecu_adds;	/*!< comma sep list of x conv. adders */
	gfloat *z_fromecu_mult;	/*!< multiplier */
	gfloat *z_fromecu_add;	/*!< multiplier */
	gchar *z_precisions;	/*!< comma sep list of precisions */
	gchar *z_lookuptables;	/*!< comma sep list of lookuptables */
	GHashTable *z_multi_hash;/*!< Hash table to store the above */
	gchar *z_source;	/*!< Z datasource for 3d displays */
	gchar *z_suffix;	/*!< text suffix used on 3D view */
	gboolean z_ignore_algorithm;	/*!< If don't heed algorithms */
	gboolean z_complex;	/*!< If using complex math */
	gchar *z_fromecu_conv_expr;	/*!< z conversion expression */
	gchar *z_toecu_conv_expr;	/*!< z conversion expression */
	void *z_ul_eval;	/*!< evaluator for z variable */
	void *z_dl_eval;	/*!< evaluator for z variable */
	gint z_precision;	/*!< how many decimal places */
	gchar * z_depend_on;	/*!< Z axis dependancy string name */
	GObject *z_object;	/*!< Container for lookuptable deps */
	gint last_z_minval;	/*!< Last Minimum value for color scaling */
	gint last_z_maxval;	/*!< Last Maximum value for color scaling */
	gboolean color_update;	/*!< Flag to issue color reset.. */
	gint z_minval;		/*!< Minimum value for color scaling */
	gint z_maxval;		/*!< Maximum value for color scaling */
	gboolean x_use_color;	/*!< Does Xaxis use color */
	gboolean y_use_color;	/*!< Does Yaxis use color */
	gboolean z_use_color;	/*!< Does Zaxis use color */
};


/*!
 \brief The _Page_Params structure contains fields defining the size of the 
 page returned from the ECU, the VEtable, RPm and Load table base offsets and
 sizes, along with a flag signifying if it's a spark table
 */
struct _Page_Params
{
	gint length;		/*!< How big this page is... */
	gint phys_ecu_page;	/*!< True pagenumber to send */
	gboolean dl_by_default;	/*!< Download this page normally */
	gboolean read_only;	/*!< This page is NOT writable */
	gint is_spark;		/*!< does this require alt write cmd? (hacky) */
	gint spconfig_offset;	/*!< Where spconfig value is located (hacky) */
	gboolean needs_burn;	/*!< Flag to indicate burn needed */
	gint checksum;		/*!< Page 32 bit checksum */
};


/*!
 \brief The _TE_Params structure contains fields defining table editor params
 One struct is allocated per table, and multiple tables per page are allowed
 This differs a bit in lingo to how the MS-II developers named things in 
 their code.  On those, table is analagous to "page", why they used the 
 language they did is beyond me, but it makes little sense to say "fuel table
 2 inside of table5,  makes more sense to say "fuel table 2 in page5".
 This structure defines a specific 2d  table arranged in a column
 format.  Current design restriction is that a table can't span more
 than on CAN IDs which only makes sense.
 */
struct _TE_Params
{
	gchar *title;		/*!< Title used on TE window */
	gboolean gauge_temp_dep;/*!< Temperature dependant? */
	gboolean reversed;	/*!< reverse order of bins */
	gchar *gauge;		/*!< Generic Gauge to stick in lower left */
	gchar *c_gauge;		/*!< C Gauge to stick in lower left */
	gchar *f_gauge;		/*!< F Gauge to stick in lower left */
	gchar *gauge_datasource;/*!< Gauge datasource */
	gchar *bg_color;	/*!< BG Color (string) */
	gchar *grat_color;	/*!< Graticule Color (string) */
	gchar *trace_color;	/*!< Trace Color (string) */
	gchar *cross_color;	/*!< Cross Color (string) */
	gchar *marker_color;	/*!< Marker Color (string) */
	gchar *bind_to_list;	/*!< Bind to list for sensitivity */
	MatchType match_type;	/*!< Match type for sensitivity */
	gchar *x_axis_label;	/*!< X Axis label string */
	gchar *y_axis_label;	/*!< Y Axis label string */
	GList *entries;		/*!< Entry widget pointers */
	gint bincount;		/*!< Number of bins for x and 1 */

	gboolean x_temp_dep;	/*!< Temperature dependant? */
	gboolean x_lock;	/*!< Flag for 2d table editor */
	gboolean x_use_color;	/*!< USe color? */
	gint x_page;		/*!< what page this column resides in */
	gint x_base;		/*!< offset of column in page  */
	gint x_raw_lower;	/*!< X raw lower in ECU units */
	gint x_raw_upper;	/*!< X raw upper in ECU units */
	gint x_2d_lower_limit;	/*!< X lower limit */
	gint x_2d_upper_limit;	/*!< X upper limit */
	DataSize x_size;	/*!< enumeration size for the var */
	gfloat *x_fromecu_mult;	/*!< ul conv multiplier */
	gfloat *x_fromecu_add;	/*!< ul conv adder */
	gchar *x_source;	/*!< column datsource */
	gint x_precision;	/*!< column precisions */
	gchar *x_name;		/*!< column name */
	gchar *x_units;		/*!< column units */
	gboolean y_temp_dep;	/*!< Temperature dependant? */
	gboolean y_lock;	/*!< Flag for 2d table editor */
	gboolean y_use_color;	/*!< USe color? */
	gint y_page;		/*!< what page this column resides in */
	gint y_base;		/*!< offset of column in page  */
	gint y_raw_lower;	/*!< Y raw lower in ECU units */
	gint y_raw_upper;	/*!< Y raw upper in ECU units */
	gint y_2d_lower_limit;	/*!< Y lower limit */
	gint y_2d_upper_limit;	/*!< Y upper limit */
	DataSize y_size;	/*!< enumeration size for the var */
	gfloat *y_fromecu_mult;	/*!< ul conv multiplier */
	gfloat *y_fromecu_add;	/*!< ul conv adder */
	gchar *y_source;	/*!< column datsource */
	gint y_precision;	/*!< column precisions */
	gchar *y_name;		/*!< column name */
	gchar *y_units;		/*!< column units */
};


/*!
  \brief _Deferred_Data is a container for a block of data that is dependant
  on something else, and thus this holds onto it it for a deferred send to the
  ECU at some future time after further verification of its dependencies
  */
struct _Deferred_Data
{
	gint canID;		/*!< canBus ID */
	gint page;		/*!< ECU Page */
	gint offset;		/*!< OFfset in page */
	gint value;		/*!< Value at this offset in this page */
	DataSize size;		/*!< Size of this data */
};


/* Prototypes */
void load_firmware_file(Io_File *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
