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

#ifndef __DEFAULT_LIMITS_H__
#define __DEFAULT_LIMITS_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Structure defined in structures.h */
static struct Default_Limits def_limits[] = {
/* Default visible Controls */
{"Clock",0,0,"Seconds",1,54,UCHAR,TRUE,0},
};
