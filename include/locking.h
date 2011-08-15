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
  \file include/locking.h
  \ingroup Headers
  \brief
  \author David Andruczyk
  */

#ifndef __LOCKING_H__
#define __LOCKING_H__

/* Prototypes */
void create_mtx_lock(void);
void remove_mtx_lock(void);
void unix_create_mtx_lock(void);
void unix_remove_mtx_lock(void);
void win32_create_mtx_lock(void);
void win32_remove_mtx_lock(void);
gboolean lock_serial(gchar *);
void unlock_serial(void);
/* Prototypes */

#endif
