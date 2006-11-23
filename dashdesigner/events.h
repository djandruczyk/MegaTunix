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

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


/* Prototypes */

EXPORT gboolean dashdesigner_about(GtkWidget *, gpointer );
EXPORT gboolean import_dash_xml(GtkWidget *, gpointer );
EXPORT gboolean add_gauge(GtkWidget *, gpointer );
EXPORT gboolean dashdesigner_quit(GtkWidget *, gpointer );
EXPORT gboolean export_dash_xml_default(GtkWidget *, gpointer );
EXPORT gboolean export_dash_xml_as(GtkWidget *, gpointer );
EXPORT gboolean motion_event(GtkWidget *, GdkEventMotion *,gpointer );
EXPORT gboolean button_event(GtkWidget *, GdkEventButton *,gpointer );
			 
/* Prototypes */

#endif
