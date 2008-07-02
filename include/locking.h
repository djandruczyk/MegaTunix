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

#ifndef __LOCKING_H__
#define __LOCKING_H__

/* Prototypes */
void create_mtx_lock(void);
void win32_create_mtx_lock(void);
void unix_create_mtx_lock(void);
/* Prototypes */

#endif
