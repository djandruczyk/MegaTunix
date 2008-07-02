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

#ifndef __COMBO_LOADER_H__
#define __COMBO_LOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>

enum
{
	CHOICE_COL,
	BITMASK_COL,
	BITSHIFT_COL,
	COMBO_COLS
}ComboCols;

/* Prototypes */
void combo_setup(GObject *, ConfigFile *, gchar * );
/* Prototypes */

#endif
