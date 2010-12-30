/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <enums.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT void ecu_gui_init(GladeXML *xml)
{
	/* We don't need anything specific to this ecu initialized */
}

