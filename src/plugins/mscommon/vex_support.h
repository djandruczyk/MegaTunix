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
  \file src/plugins/mscommon/vex_support.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon VEX import/export code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VEX_SUPPORT_H__
#define __VEX_SUPPORT_H__

#include <enums.h>
#include <gtk/gtk.h>


typedef enum
{
	VEX_HEADER=0xd0,
	VEX_PAGE,
	VEX_RANGE,
	VEX_TABLE
}ImportParserFunc;

typedef enum
{
	VEX_EVEME=0xe0,
	VEX_USER_REV,
	VEX_USER_COMMENT,
	VEX_DATE,
	VEX_TIME,
	VEX_RPM_RANGE,
	VEX_LOAD_RANGE,
	VEX_NONE
}ImportParserArg;


typedef struct _Vex_Import Vex_Import;


/*!
 \brief The Vex_Import structure holds all fields (lots) needed to load and
 process a VEX (VEtabalt eXport file) and load it into megatunix.
 \see vetable_import
 \see vetable_export
 */
struct _Vex_Import
{	
	gchar *version;		/*!< Version String */
	gchar *revision;	/*!< Revision String */
	gchar *comment;		/*!< Comment String */
	gchar *date;		/*!< Date String */
	gchar *time;		/*!< Time String */
	gint page;		/*!< ecu page */
	gint table;		/*!< ecu table */
	gint total_x_bins;	/*!< total X bins */
	gint *x_bins;		/*!< x bins Array, dynamic */
	gint total_y_bins;	/*!< total y bins */
	gint *y_bins;		/*!< y bins  Array, dynamic */
	gint total_tbl_bins;	/*!< total table bins */
	gint *tbl_bins;		/*!< table bings Array, dynamic */
	gboolean got_page;	/*!< got_page state Flag */
	gboolean got_rpm;	/*!< got_rpm state Flag */
	gboolean got_load;	/*!< got load state Flag */
	gboolean got_ve;	/*!< got ve state Flag */

};


/* Prototypes */
gboolean all_table_export(GIOChannel *);
gboolean all_table_import(GIOChannel *);
void dealloc_vex_struct(Vex_Import *);
void feed_import_data_to_ecu(Vex_Import *);
GIOStatus handler_dispatch(Vex_Import *, ImportParserFunc , ImportParserArg , gchar *, GIOChannel * );
GIOStatus process_header(Vex_Import *, ImportParserArg , gchar *);
GIOStatus process_page(Vex_Import *, gchar * );
GIOStatus process_table(Vex_Import *);
GIOStatus process_vex_line(Vex_Import *, GIOChannel *);
GIOStatus process_vex_range(Vex_Import *, ImportParserArg, gchar *, GIOChannel * );
GIOStatus process_vex_table(Vex_Import *, gchar *, GIOChannel * );
GIOStatus read_number_from_line(gint *, GIOChannel *);
void revert_to_previous_data(void);
void select_table_for_export(gint);
void select_table_for_import(gint);
gboolean select_vex_for_export(GtkWidget *, gpointer );
gboolean select_vex_for_import(GtkWidget *, gpointer );
void single_table_export(GIOChannel *, gint );
void single_table_import(GIOChannel *, gint );
gint vex_comment_parse(GtkWidget *, gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
