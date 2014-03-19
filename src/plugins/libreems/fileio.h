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
  \file src/plugins/libreems/fileio.h
  \ingroup LibreEMSPlugin,Headers
  \brief LibreEMS File IO functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void backup_all_ecu_settings(gchar  *);
void restore_all_ecu_settings(gchar  *);
gboolean select_file_for_ecu_backup(GtkWidget *, gpointer );
gboolean select_file_for_ecu_restore(GtkWidget *, gpointer );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
