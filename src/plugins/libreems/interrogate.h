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
  \file src/plugins/libreems/interrogate.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS Interrogator functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __INTERROGATE_H__
#define __INTERROGATE_H__

#include <gtk/gtk.h>
#include <defines.h>
#include <configfile.h>
#include <enums.h>
#include <firmware.h>
#include <threads.h>


typedef enum
{
	RESULT_DATA=0x440,
	RESULT_TEXT,
	RESULT_LIST
}Test_Result;

typedef enum
{
	COUNT=0x260,
	NUMMATCH,
	SUBMATCH,
	FULLMATCH,
	REGEX
}MatchClass;

typedef struct _Detection_Test Detection_Test;
typedef struct _Location_Details Location_Details;

/*!
 \brief The _Detection_Test struct holds the basics for each ECU test.
 a friendly human readable test name (this matches up eith test names in the 
 actual profile), the actual_test string (a machine parsable form), and a 
 test_vector,  which is the result of splitting up the actual_test string into
 it's component parts. 
 */
struct _Detection_Test
{
	gchar *test_name;	/*!< Friendly test name, like "MS-II_RTvars" */
	gchar *test_desc;	/*!< Gui displayed test description */
	gchar *test_func;	/*!< Function to run */
	void *(*function)(gint *);/*!< Function Pointer */
	guint32 result_type;	/*!< DATA,TEXT or LIST */
	void *result;		/*!< Result of test stored for matching */
	gchar *result_str;	/*!< String version of result */
	gint num_bytes;		/*!< number of bytes in returned string */
};

/*!
 \brief The _Location_Details struct holds the information specific about each
 ECU location ID, including applicable flags, its parent, its ram/flash pages
 and their addresses, and length.
 */
struct _Location_Details
{
	guint16 flags;		/*!< Flags specific to this location ID */
	guint16 parent;		/*!< Parent LocID for nested locations */
	guint8 ram_page;	/*!< Ram Page number */
	guint8 flash_page;	/*!< Flash Page number */
	guint16 ram_address;	/*!< Ram address in Hex */
	guint16 flash_address;	/*!< Flash address in Hex */
	guint16 length;		/*!< How many bytes in this location ID */
};

/* Prototypes */
gboolean check_for_match(GHashTable *, gchar *);
gboolean determine_ecu(GArray *, GHashTable *);
gboolean get_dimensions(gint, gint, gint,gint *, gint *);
Page_Params *initialize_page_params(void);
Table_Params *initialize_table_params(void);
gboolean interrogate_ecu(void);
gboolean load_firmware_details(Firmware_Details *, gchar *);
void request_firmware_build_date(void);
void request_firmware_build_os(void);
void request_firmware_compiler(void);
void request_firmware_version(void);
gchar *request_firmware_version_callback(gint *);
void request_interface_version(void);
GList *request_location_ids(gint *);
Location_Details *request_location_id_details(guint16);
void test_cleanup(gpointer);
gint translate_capabilities(const gchar *);
void update_ecu_info(void);
void update_interrogation_gui_pf(void);
gboolean validate_and_load_tests(GArray **, GHashTable **);


/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
