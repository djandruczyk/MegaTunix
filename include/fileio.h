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

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void present_filesavebox(FileIoType );
void truncate_file(FileIoType, gchar *);
void close_file(void * );
void check_filename(GtkWidget *, GtkFileSelection *);
void backup_all_ms_settings(void *);
void restore_all_ms_settings(void *);
/* Prototypes */

#endif
