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

#ifndef __USER_OUTPUTS_H__
#define __USER_OUTPUTS_H__

#include <gtk/gtk.h>

/* Prototypes */
void populate_user_output_choices(void);
gboolean show_user_output_choices(GtkWidget *, gpointer );
GtkTreeModel * create_model(void);
GtkWidget * create_view(void);

/* Prototypes */

#endif
