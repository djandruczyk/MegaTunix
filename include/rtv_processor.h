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
  \file include/rtv_processor.h
  \ingroup Headers
  \brief Header for the runtime var map processor functionality
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __RTV_PROCESSOR_H__
#define __RTV_PROCESSOR_H__

#include <gtk/gtk.h>
#include <configfile.h>
#include <enums.h>

/* Prototypes */
void flush_rt_arrays(void);
gfloat handle_complex_expr(gconstpointer *, void *,ConvType);
gfloat handle_complex_expr_obj(GObject *, void *,ConvType);
gfloat handle_multi_expression(gconstpointer *, guchar *, GHashTable *);
gfloat handle_special(gconstpointer *,gchar *);
gboolean lookup_current_index(const gchar *, gint *);
gboolean lookup_current_value(const gchar *, gfloat *);
gboolean lookup_precision(const gchar *, gint *);
gboolean lookup_previous_n_values(const gchar *, gint, gfloat *);
gboolean lookup_previous_n_skip_x_values(const gchar *, gint, gint, gfloat *);
gboolean lookup_previous_nth_value(const gchar *, gint, gfloat *);
gboolean lookup_previous_value(const gchar *, gfloat *);
void process_rt_vars(void *,gint);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
