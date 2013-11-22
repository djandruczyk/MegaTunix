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
  \file include/apicheck.h
  \ingroup Headers
  \brief API checker headers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __APICHECK_H__
#define __APICHECK_H__

#include <config.h>
#include <gtk/gtk.h>
#include <configfile.h>

/* Prototypes */
gboolean get_file_api(ConfigFile *, gint *, gint *);
gboolean set_file_api(ConfigFile *, gint, gint);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
