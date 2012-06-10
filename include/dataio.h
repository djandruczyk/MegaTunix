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
  \file include/dataio.h
  \ingroup Headers
  \brief Handler for dealing with data input/output to the ECU
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DATAIO_H__
#define __DATAIO_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Prototypes */
void dump_output(gint, guchar *);
gint read_data(gint , guint8 **, gboolean);
gboolean read_wrapper(gint, guint8 *, size_t, gint *);
gboolean write_data(Io_Message *);
gboolean write_wrapper(gint, const guint8 *, size_t, gint *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
