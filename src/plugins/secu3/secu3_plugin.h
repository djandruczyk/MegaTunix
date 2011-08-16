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
  \file src/plugins/secu3/secu3_plugin.h
  \ingroup Secu3Plugin,Headers
  \brief Secu3 Plugin init/shutdown code
  \author David Andruczyk
  */

#ifndef __SECU3_PLUGIN_H__
#define __SECU3_PLUGIN_H__

#include <defines.h>
#include <configfile.h>

#ifdef __SECU3_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function Pointers */
/* Function Pointers */

/* Prototypes */
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_common_enums(void);
void deregister_common_enums(void);
/* Prototypes */

#endif
