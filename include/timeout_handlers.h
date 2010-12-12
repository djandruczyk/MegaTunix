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

#ifndef __TIMEOUT_HANDLERS_H__
#define __TIMEOUT_HANDLERS_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void start_tickler(TicklerType);
void stop_tickler(TicklerType);
void * signal_read_rtvars_thread(gpointer);
void signal_read_rtvars(void);
gboolean early_interrogation(void);
gboolean personality_choice(void);
/* Prototypes */

#endif
