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
  \file src/plugins/freeems/freeems_benchtest.h
  \ingroup FreeEMSPlugin,Headers
  \brief FreeEMS Bench Test functions
  \author David Andruczyk
  */

#ifndef __FREEEMS_BENCHTEST_H__
#define __FREEEMS_BENCHTEST_H__

#include <gtk/gtk.h>

typedef struct _Bt_Data Bt_Data;

struct _Bt_Data
{
	guint8 events_per_cycle;
	guint16 cycles;
	guint16 ticks_per_event;
	guint8 events[6];
	guint16 pw_sources[6];
};

/* Prototypes */
void benchtest_validate_and_run(void);
void benchtest_stop(void);
gboolean pull_data_from_gui(Bt_Data *);
/* Prototypes */

#endif
