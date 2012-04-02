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
  \file include/mem_mgmt.h
  \ingroup Headers
  \brief Header for generic memory management 
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MEM_MGMT_H__
#define __MEM_MGMT_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
gint _get_sized_data(guint8 *, gint, DataSize, gboolean);
void _set_sized_data(guint8 *, gint, DataSize, gint, gboolean);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
