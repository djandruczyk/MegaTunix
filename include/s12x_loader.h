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
  \file include/s12x_loader.h
  \ingroup Headers
  \brief Header for the Motorola S12x loader code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __S12X_LOADER_H__
#define __S12X_LOADER_H__

#include <winserialio.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean check_status(gint);
gboolean do_ms2_load(gint, gint);
gboolean do_libreems_load(gint, gint);
gboolean erase_S12(gint);
void free_s19(guint);
void ms2_enter_boot_mode(gint);
gint read_s19(gint);
gboolean readback_block(gint, gint, guchar *, gint);
void reset_proc(gint);
gboolean send_S12(gint, guint);
void output(gchar *, gboolean);
gboolean wakeup_S12(gint);
void jump_to_loader_if_streaming(gint);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
