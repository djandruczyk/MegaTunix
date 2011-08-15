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

/*!
  \file include/watches.h
  \ingroup Headers
  \brief
  \author David Andruczyk
  */

/* Runtime Gui Structures */

#ifndef __WATCHES_H__
#define __WATCHES_H__

#include <defines.h>
#include <gtk/gtk.h>

/*!
  \brief Enumerations for types of watches 
  */
typedef enum
{
	SINGLE_BIT_STATE,	/*!< Single bit state (level) watch */
	SINGLE_BIT_CHANGE,	/*!< Single bit change (change) watch */
	MULTI_BIT,		/*!< Multi-bit watch (not implemented yet) */
	EXACT,			/*!< Exact val watch (not implemented yet) */
	THRESHOLD,		/*!< Threshold val watch (not implemented yet) */
	RANGE,			/*!< Range val watch (not implemented yet) */
	VALUE_CHANGE,		/*!< Watch that triggers only on valud changes */
	MULTI_VALUE,		/*!< Calls function with multiple values */
	WATCH_COUNT
}WatchStyle;


typedef struct _DataWatch DataWatch;


/*!
  \brief DataWatch structure contains the fields common to all watches
  */
struct _DataWatch
{
	guint32 id;		/*!< Watch ID */
	gint bit;		/*!< Bit to watch */
	gfloat low;		/*!< Low point (range watch) */
	gfloat high;		/*!< Highpoint (range watch) */
	gfloat exact;		/*!< Exact value watch */
	gint threshold;		/*!< threshold watch */
	gfloat f_val;		/*!< Integer value */
	gpointer user_data;	/*!< user data to pass to user function */
	WatchStyle style;	/*!< type of watch */
	gboolean state;		/*!< state for bit watches */
	gboolean one_shot;	/*!< Run only once then evaporate */
	gchar * function;	/*!< function to call when watch strikes */
	gint num_vars;		/*!< number of variables */
	gfloat val;		/*!< single value result location */
	gfloat last_val;	/*!< Last value */
	gfloat *vals;		/*!< multi value result location */
	void (*func) (DataWatch *);/*!< Function pointer */
	gchar * varname;	/*!< Variable name (rtv internal name) to check */
	gchar ** varnames;	/*!< List of Variable names (rtv internal name) to check */
};
/* Prototypes */
 void fire_off_rtv_watches_pf(void);
guint32 create_single_bit_state_watch(const gchar *, gint, gboolean, gboolean, const gchar *, gpointer);
guint32 create_single_bit_change_watch(const gchar *, gint, gboolean, const gchar *, gpointer);
guint32 create_value_change_watch(const gchar *, gboolean, const gchar *, gpointer);
guint32 create_multi_value_watch(gchar **, gboolean, const gchar *, gpointer);
void watch_destroy(gpointer);
void remove_watch(guint32);
void process_watches(gpointer, gpointer, gpointer);
gboolean watch_active(guint32 );
/* Prototypes */

#endif
