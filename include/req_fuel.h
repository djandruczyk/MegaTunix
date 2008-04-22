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

#ifndef __REQ_FUEL_H__
#define __REQ_FUEL_H__

#include <gtk/gtk.h>

typedef struct _Req_Fuel_Params Req_Fuel_Params;
typedef struct _Reqd_Fuel Reqd_Fuel;

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

/* Prototypes */
gint reqd_fuel_popup(GtkWidget *);
gboolean save_reqd_fuel(GtkWidget *, gpointer);
gboolean close_popup(GtkWidget *);
void req_fuel_change(GtkWidget *);
void check_req_fuel_limits(gint);
Reqd_Fuel * initialize_reqd_fuel(gint );
/* Prototypes */

#endif
