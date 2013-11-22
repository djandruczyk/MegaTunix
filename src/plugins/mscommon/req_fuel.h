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
  \file src/plugins/mscommon/req_fuel.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon Required Fuel handling
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __REQ_FUEL_H__
#define __REQ_FUEL_H__

#include <gtk/gtk.h>


typedef enum
{
	REQ_FUEL_DISP=0xf80,
	REQ_FUEL_CYLS,
	REQ_FUEL_RATED_INJ_FLOW,
	REQ_FUEL_RATED_PRESSURE,
	REQ_FUEL_ACTUAL_PRESSURE,
	REQ_FUEL_AFR
}RfHandler;

typedef struct _Reqd_Fuel Reqd_Fuel;

/*! 
 Controls for the Required Fuel Calculator... 
 \brief The _Req_Fuel struct contains jsut about all widgets for the rewuired
 fuel popup.  most of the values are loaded/saved from the main config file
 when used.
 */
struct _Reqd_Fuel
{
	GtkWidget *popup;		/*!< the popup window */
	GtkWidget *calcd_val_spin;	/*!< Preliminary value */
	GtkWidget *reqd_fuel_spin;	/*!< Used value */
	gfloat calcd_reqd_fuel;		/*!< calculated value... */
	gint disp;			/*!< Engine size  1-1000 Cu-in */
	gint cyls;			/*!< # of Cylinders  1-12 */
	gfloat rated_inj_flow;		/*!< Rated injector flow */
	gfloat rated_pressure;		/*!< Rated fuel pressure */
	gfloat actual_pressure;		/*!< Actual fuel pressure */
	gfloat actual_inj_flow;		/*!< injector flow rate (lbs/hr) */
	gfloat target_afr;		/*!< Air fuel ratio 10-25.5 */
	gint table_num;			/*!< Which table this refers to */
	gboolean visible;		/*!< Is it visible? */
};

/* Prototypes */
void check_req_fuel_limits(gint);
gboolean close_popup(GtkWidget *);
gboolean drain_hashtable(gpointer, gpointer, gpointer);
Reqd_Fuel *initialize_reqd_fuel(gint );
void reqd_fuel_change(GtkWidget *);
gint reqd_fuel_popup(GtkWidget *);
void reqfuel_rescale_table(GtkWidget *);
gboolean rf_spin_button_handler(GtkWidget *, gpointer);
gboolean save_reqd_fuel(GtkWidget *, gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
