/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
	gchar *test_name;	/* Friendly test name, like "MS-II_RTvars" */
	gchar *test_desc;	/* Gui displayed test description */
	gchar *test_func;	/* Function to run */
	void *(*function)(gint *);/* Function Pointer */
	guint32 result_type;	/* DATA,TEXT or LIST */
	void *result;		/* Result of test stored for matching */
	gchar *result_str;	/* String version of result */
	gint num_bytes;		/* number of bytes in returned string */
};

/*!
 \brief The _Location_Details struct holds the information specific about each
 ECU location ID, including applicable flags, its parent, its ram/flash pages
 and their addresses, and length.
 */
struct _Location_Details
{
	guint16 flags;
	guint16 parent;
	guint8 ram_page;
	guint8 flash_page;
	guint16 ram_address;
	guint16 flash_address;
	guint16 length;
};

/* Prototypes */
void test_cleanup(gpointer);
gboolean interrogate_ecu(void);
gchar *request_firmware_version(gint *);
gchar *request_interface_version(gint *);
GList *request_location_ids(gint *);
gchar *request_detailed_interface_version(guint8 *, guint8 *, guint8 *);
Location_Details *request_location_id_details(guint16);
gboolean validate_and_load_tests(GArray **, GHashTable **);
gboolean determine_ecu(GArray *, GHashTable *);
gboolean check_for_match(GHashTable *, gchar *);
void update_interrogation_gui_pf(void);
gboolean load_firmware_details(Firmware_Details *, gchar *);
gint translate_capabilities(const gchar *);
Page_Params * initialize_page_params(void);
Table_Params * initialize_table_params(void);


/* Prototypes */

#endif
