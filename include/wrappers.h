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
  \file
  \ingroup Headers
  \brief
  \author David Andruczyk
  */

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

#include <gtk/gtk.h>

/* Prototypes */
void *evaluator_create_w(char *);
void evaluator_destroy_w( void *);
double evaluator_evaluate_x_w(void *, gdouble);
/* Prototypes */

#endif
