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
  \file include/plugin.h
  \ingroup Headers
  \brief Header for plugin load/unload and symbol resolution
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <gtk/gtk.h>

typedef enum
{
	MAIN,
	COMMON,
	ECU,
	NUM_MODULES
}ModIndex;

/* Prototypes */
gboolean get_symbol(const gchar *, void **);
gboolean plugin_function(GtkWidget *, gpointer);
void plugins_init(void);
void plugins_shutdown(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
