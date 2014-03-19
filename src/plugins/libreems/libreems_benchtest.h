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
  \file src/plugins/libreems/libreems_benchtest.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS Bench Test functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LIBREEMS_BENCHTEST_H__
#define __LIBREEMS_BENCHTEST_H__

#include <gtk/gtk.h>

typedef struct _Bt_Data Bt_Data;

struct _Bt_Data
{
	guint8	events_per_cycle;
	guint16	cycles;
	guint16	ticks_per_event;
	guint8	events[6];
	guint16	pw_sources[6];
};

/* Prototypes */
void benchtest_bump(void);
gboolean benchtest_clock_update(gpointer);
gboolean benchtest_clock_update_wrapper(gpointer);
void benchtest_stop(void);
void benchtest_validate_and_run(void);
gboolean pull_data_from_gui(Bt_Data *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
