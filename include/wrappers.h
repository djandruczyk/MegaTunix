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

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

#include <gtk/gtk.h>

/* Prototypes */
void *eval_create(char *);
void eval_destroy( void *);
double eval_x(void *, gdouble);
/* Prototypes */

#endif
