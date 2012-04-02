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
  \file include/wrappers.h
  \ingroup Headers
  \brief Header for the wrapper functions to deal with symbol issues on windows
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

#include <gtk/gtk.h>

/* Prototypes */
void *evaluator_create_w(char *);
void evaluator_destroy_w( void *);
double evaluator_evaluate_x_w(void *, gdouble);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
