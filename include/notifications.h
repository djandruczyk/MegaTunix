/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __NOTIFICATIONS_H__
#define __NOTIFICATIONS_H__

#include <enums.h>
#include <gtk/gtk.h>

/* Prototypes */
void set_group_color(GuiColor, gchar * );
void set_reqfuel_color(GuiColor, gint );
void set_widget_color(gpointer, gpointer );
void update_logbar(gchar *, gchar *, gchar *, gboolean, gboolean);
void no_ms_connection(void);
void warn_user(gchar *);
void set_title(gchar *);
void conn_warning(void);
void kill_conn_warning(void);
gboolean get_response(GtkWidget *, gpointer );

gint close_dialog(GtkWidget *, gpointer);
gint dialog_response(GtkWidget *, gpointer );
gboolean set_warning_flag(gpointer);
/* Prototypes */

#endif
