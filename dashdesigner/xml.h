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

#ifndef __DASH_XML_H__
#define __DASH_XML_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


/* Prototypes */

EXPORT gboolean import_dash_xml(GtkWidget *, gpointer );
EXPORT gboolean export_dash_xml_default(GtkWidget *, gpointer );
EXPORT gboolean export_dash_xml_as(GtkWidget *, gpointer );
			 
/* Prototypes */

#endif
