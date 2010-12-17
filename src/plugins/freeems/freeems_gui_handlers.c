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
#include <defines.h>
#include <firmware.h>
#include <freeems_plugin.h>
#include <freeems_gui_handlers.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT void common_gui_init(void)
{
	void (*ecu_gui_init)(GladeXML *) = NULL;
	GladeXML *xml = NULL;
	GtkWidget *item = NULL;

	xml  = DATA_GET(global_data,"main_xml");
	/* This function is for doing any gui finalization on the CORE gui
	   for stuff specific to this firmware family.
	   */
	/* If ECU lib has a call run it */
	if (get_symbol_f("ecu_gui_init",(void *)&ecu_gui_init))
		ecu_gui_init(xml);
}
