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

#ifndef __INTERROGATE_H__
#define __INTERROGATE_H__

#include <configfile.h>
#include <gtk/gtk.h>
#include <firmware.h>


typedef struct _Detection_Test Detection_Test;
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
/* Prototypes */
gboolean interrogate_ecu(void);
gboolean determine_ecu(GArray *,GHashTable *);
GArray * validate_and_load_tests(GHashTable **);
gboolean check_for_match(GHashTable *,gchar *);
void free_results_array(GArray *);
void free_tests_array(GArray *);
void interrogate_error(gchar *, gint);
gint translate_capabilities(gchar *);
gboolean load_firmware_details(Firmware_Details *, gchar * );
void update_interrogation_gui(Firmware_Details *,GHashTable *);
/* Prototypes */

#endif
