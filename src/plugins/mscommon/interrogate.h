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
  \file src/plugins/mscommon/interrogate.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon interrogation
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __INTERROGATE_H__
#define __INTERROGATE_H__

#include <configfile.h>
#include <gtk/gtk.h>
#include <firmware.h>


typedef enum
{
	RESULT_DATA=0x440,
	RESULT_TEXT
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
	gchar *actual_test;	/*!< machine parsable test string */
	guint8 *send_buf;	/*!< Actual test stream to send to ECU */
	guint8 *recv_buf;	/*!< Actual test stream read back from ECU */
	guint32 result_type;/*!< DATA or TEXT */
	gchar *result_str;	/*!< Result of test stored for matching */
	gint send_len;		/*!< number of bytes to send for this test */
	gint recv_len;		/*!< number of bytes we received as a response */
	gboolean ms3_crc32;	/*!< Flag if we are using MS3 CRC32 wrappers */

};
/* Prototypes */
gboolean check_for_match(GHashTable *,gchar *);
gboolean determine_ecu(GArray *,GHashTable *);
void free_results_array(GArray *);
void free_tests_array(GArray *);
Page_Params * initialize_page_params(void);
Table_Params * initialize_table_params(void);
TE_Params * initialize_te_params(void);
gboolean interrogate_ecu(void);
void interrogate_error(const gchar *, gint);
gboolean load_firmware_details(Firmware_Details *, const gchar * );
gint translate_capabilities(const gchar *);
void update_interrogation_gui_pf(void);
GArray *validate_and_load_tests(GHashTable **);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
