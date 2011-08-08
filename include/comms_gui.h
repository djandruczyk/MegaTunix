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

#ifndef __COMMS_GUI_H__
#define __COMMS_GUI_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean update_errcounts(void);
void update_comms_status(void);
gboolean reset_errcounts(GtkWidget *);
gboolean check_potential_ports(const gchar *);
gboolean enumerate_dev(GtkWidget *, gpointer);	/* Help find usb/serial adapter */
/* Prototypes */

#endif
