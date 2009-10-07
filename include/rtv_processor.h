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

#ifndef __RTV_PROCESSOR_H__
#define __RTV_PROCESSOR_H__

#include <gtk/gtk.h>
#include <configfile.h>
#include <enums.h>

/* Prototypes */
void process_rt_vars(void * );
gboolean lookup_precision(gchar *, gint *);
gboolean lookup_current_value(gchar *, gfloat *);
gboolean lookup_previous_value(gchar *, gfloat *);
gboolean lookup_previous_nth_value(gchar *, gint, gfloat *);
gboolean lookup_previous_n_values(gchar *, gint, gfloat *);
gboolean lookup_previous_n_skip_x_values(gchar *, gint, gint, gfloat *);
gfloat handle_complex_expr(GObject *, void *,ConvType);
gfloat handle_special(GObject *,gchar *);
gfloat handle_multi_expression(GObject *, guchar *, GHashTable *);
void flush_rt_arrays(void);
/* Prototypes */

#endif
