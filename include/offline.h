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
  \file include/offline.h
  \ingroup Headers
  \brief Header for offline mode functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __OFFLINE_H__
#define __OFFLINE_H__

#include <gtk/gtk.h>


/* Prototypes */
void offline_ecu_restore_pf(void);
gchar *present_firmware_choices(void);
gint ptr_sort(gconstpointer , gconstpointer );
gboolean set_offline_mode(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
