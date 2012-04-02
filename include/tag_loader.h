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
  \file include/tag_loader.h
  \ingroup Headers
  \brief Header for the text tag loading functionality
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAG_LOADER_H__
#define __TAG_LOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>

/* Prototypes */
void load_tags(GObject *, ConfigFile *, gchar * );
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
