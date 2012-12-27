/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/ms1/ms1_helpers.c
  \ingroup MS1Plugin,Plugins
  \brief MS1 comm.xml related post function helpers
  \author David Andruczyk
  */

#include <ms1_plugin.h>


/*!
  \brief Handler to re-enable the reboot button on the error status tab
  */
G_MODULE_EXPORT void enable_reboot_button_pf(void)
{
	ENTER();
	gtk_widget_set_sensitive(lookup_widget_f("error_status_reboot_button"),TRUE);
	EXIT();
	return;
}

