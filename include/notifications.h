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
void set_store_buttons_state(GuiState);
void update_statusbar(GtkWidget *, int, gchar *);
void no_ms_connection(void);
void warn_user(gchar *);
void squirt_cyl_inj_set_state(GuiState);
void interdep_state(GuiState, gint );
void warn_file_not_empty(void);
gint close_dialog(GtkWidget *, gpointer);
/* Prototypes */

#endif
