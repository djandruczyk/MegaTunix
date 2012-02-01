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

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>


/* Prototypes */
void reset_temps(gpointer);
void convert_temps(gpointer,gpointer);
gint convert_before_download(GtkWidget *, gfloat);
gfloat convert_after_upload(GtkWidget *);
gfloat calc_value(gfloat, gfloat *, gfloat *, ConvDir);
gdouble f_to_k(gdouble);
gdouble f_to_c(gdouble);
gdouble k_to_c(gdouble);
gdouble k_to_f(gdouble);
gdouble c_to_f(gdouble);
gdouble c_to_k(gdouble);
gdouble temp_to_ecu(gdouble);
gdouble temp_to_host(gdouble);
/* Prototypes */

#endif
