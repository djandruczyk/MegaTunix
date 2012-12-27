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

/*!
  \file src/plugins/ms3/ms3_plugin.h
  \ingroup MS3Plugin,Headers
  \brief MS3 Plugin specific plugin init/shutdown code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif
	
#ifndef __MS3_PLUGIN_H__
#define __MS3_PLUGIN_H__

#include <plugindefines.h>
#include <debugging.h>
#include <defines.h>
#include <configfile.h>

#ifdef __MS3_PLUGIN_C__
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
void plugin_init(gconstpointer *);
void plugin_shutdown(void);
void register_ecu_enums(void);
void deregister_ecu_enums(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
