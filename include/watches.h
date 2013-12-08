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
  \file include/watches.h
  \ingroup Headers
  \brief Header for the datawatch code.
  \author David Andruczyk
  */

/* Runtime Gui Structures */

#ifdef __cplusplus
extern "C" {
#endif

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
	MULTI_VALUE_HISTORY,	/*!< Calls function with multiple historical values */
	WATCH_COUNT
}WatchStyle;


typedef struct _RtvWatch RtvWatch;


/*!
  \brief RtvWatch structure contains the fields common to all watches
  */
struct _RtvWatch
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
	gint last_index;	/*!< Last index for historical watches */
	gint count;		/*!< How many values in the historical buffer */
	gfloat val;		/*!< single value result location */
	gfloat last_val;	/*!< Last value */
	gfloat *vals;		/*!< multi value result location */
	gfloat **hist_vals;	/*!< multi value historical result location */
	void (*func) (RtvWatch *);/*!< Function pointer */
	gchar * varname;	/*!< Variable name (rtv internal name) to check */
	gchar ** varnames;	/*!< List of Variable names (rtv internal name) to check */
};
/* Prototypes */
guint32 create_rtv_multi_value_historical_watch(gchar **, gboolean, const gchar *, gpointer);
guint32 create_rtv_multi_value_watch(gchar **, gboolean, const gchar *, gpointer);
guint32 create_rtv_single_bit_change_watch(const gchar *, gint, gboolean, const gchar *, gpointer);
guint32 create_rtv_single_bit_state_watch(const gchar *, gint, gboolean, gboolean, const gchar *, gpointer);
guint32 create_rtv_value_change_watch(const gchar *, gboolean, const gchar *, gpointer);
gboolean fire_off_rtv_watches(void);
void process_rtv_watches(gpointer, gpointer, gpointer);
void remove_rtv_watch(guint32);
gboolean rtv_watch_active(guint32 );
void rtv_watch_destroy(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
