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
  \file src/plugins/null/null_plugin.h
  \ingroup NullPlugin,Headers
  \brief Null Plugin init/shutdown code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NULL_PLUGIN_H__
#define __NULL_PLUGIN_H__

#include <plugindefines.h>
#include <debugging.h>
#include <defines.h>
#include <configfile.h>

#ifdef __NULL_PLUGIN_C__
#define EXTERN
#else
#define EXTERN extern
#endif

/* Function Pointers */
EXTERN void (*dbg_func_f)(int,const gchar *, const gchar *, gint ,const gchar *, ...);                                                                          
EXTERN void (*error_msg_f)(const gchar *);
EXTERN gboolean (*get_symbol_f)(const gchar *, void **);
/* Function Pointers */

/* Prototypes */
void deregister_ecu_enums(void);
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_ecu_enums(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
