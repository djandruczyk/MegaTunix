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
  \file include/logviewer_events.h
  \ingroup Headers
  \brief Header for logviewer gui event handling
  \author David Andruczyk
  */

#ifndef __LOGVIEWER_EVENTS_H__
#define __LOGVIEWER_EVENTS_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean lv_configure_event(GtkWidget *, GdkEventConfigure *, gpointer);
gboolean lv_expose_event(GtkWidget *, GdkEventExpose *, gpointer);
gboolean lv_mouse_motion_event(GtkWidget *, GdkEventMotion *, gpointer);
gboolean lv_mouse_button_event(GtkWidget *, GdkEventButton *, gpointer);
gboolean logviewer_button_event(GtkWidget *, gpointer );
void highlight_tinfo(gint, gboolean );

/* Prototypes */

#endif
