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

#ifndef __APICHECK_H__
#define __APICHECK_H__

#include <config.h>
#include <gtk/gtk.h>
#include <configfile.h>

/* Prototypes */
gboolean set_file_api(ConfigFile *, gint, gint);
gboolean get_file_api(ConfigFile *, gint *, gint *);
/* Prototypes */

#endif
