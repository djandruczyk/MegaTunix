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
  \file include/notifications.h
  \ingroup Headers
  \brief Header for notification message functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NOTIFICATIONS_H__
#define __NOTIFICATIONS_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
gint close_dialog(GtkWidget *, gpointer);
void conn_warning(void);
gint dialog_response(GtkWidget *, gpointer );
void get_response(GtkWidget *, gint, gpointer );
void kill_conn_warning(void);
void no_ms_connection(void);
void error_msg(const gchar *);
gboolean reset_infolabel(gpointer);
gboolean reset_infolabel_wrapper(gpointer);
void set_group_color(GuiColor, const gchar * );
void set_reqfuel_color(GuiColor, gint );
void set_title(gchar *);
gboolean set_warning_flag(gpointer);
void set_widget_color(gpointer, gpointer );
void update_logbar(const gchar *, const gchar *, gchar *, gboolean, gboolean,gboolean);
void warn_user(const gchar *);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
