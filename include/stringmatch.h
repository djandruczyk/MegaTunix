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
  \file include/stringmatch.h
  \ingroup Headers
  \brief Header for the string to enum matching code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __STRINGMATCH_H__
#define __STRINGMATCH_H__

#include <gtk/gtk.h>
#include <enums.h>

#define PRINT_TOKEN(token) printf(#token" is %i\n", token);

/* Prototypes */
void build_string_2_enum_table(void);
void dump_hash(gpointer,gpointer,gpointer);
gint translate_string(const gchar *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
