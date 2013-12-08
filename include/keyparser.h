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
  \file include/keyparser.h
  \ingroup Headers
  \brief Header for the config key parsing functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KEYPARSER_H__
#define __KEYPARSER_H__

#include <gtk/gtk.h>

/* Prototypes */
gchar ** parse_keys(const gchar *, gint *, const gchar * );
gint * parse_keytypes(const gchar *, gint *, const gchar * );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
