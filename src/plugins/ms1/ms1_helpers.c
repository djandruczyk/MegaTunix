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

#include <args.h>
#include <config.h>
#include <defines.h>
#include <widgetmgmt.h>


G_MODULE_EXPORT void enable_reboot_button_pf(void)
{
	gdk_threads_enter();
	gtk_widget_set_sensitive(lookup_widget_f("error_status_reboot_button"),TRUE);
	gdk_threads_leave();
}

