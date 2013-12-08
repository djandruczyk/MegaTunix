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
  \file include/conversions.h
  \ingroup Headers
  \brief Header for the ecu<->world and temp scale conversions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>


/* Prototypes */
gdouble c_to_f(gdouble);
gdouble c_to_k(gdouble);
gfloat calc_value(gfloat, gfloat *, gfloat *, ConvDir);
gfloat convert_after_upload(GtkWidget *);
gint convert_before_download(GtkWidget *, gfloat);
void convert_temps(gpointer,gpointer);
gdouble f_to_k(gdouble);
gdouble f_to_c(gdouble);
gdouble k_to_c(gdouble);
gdouble k_to_f(gdouble);
void reset_temps(gpointer);
gdouble temp_to_ecu(gdouble);
gdouble temp_to_host(gdouble);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
