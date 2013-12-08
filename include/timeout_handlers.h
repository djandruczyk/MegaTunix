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
  \file include/timeout_handlers.h
  \ingroup Headers
  \brief Header for the global generic timeout handlers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TIMEOUT_HANDLERS_H__
#define __TIMEOUT_HANDLERS_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean check_for_first_time(void);
gboolean early_interrogation(void);
gboolean personality_choice(void);
gboolean run_function(gboolean(*)(gpointer));
void *signal_read_rtvars_thread(gpointer);
void start_tickler(TicklerType);
void stop_tickler(TicklerType);
void timeout_done(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
