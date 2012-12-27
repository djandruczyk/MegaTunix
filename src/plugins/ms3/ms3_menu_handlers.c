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
  \file src/plugins/ms3/ms3_menu_handlers.c
  \ingroup MS3Plugin,Plugins
  \brief MS3 menu handler stubs
  \author David Andruczyk
  */

#include <ms3_plugin.h>
#include <ms3_menu_handlers.h>

extern gconstpointer *global_data;


/*!
  \brief sets up the menus of the core gui with ecu specific handlers
  \param xml is the pointer to the global gui XML
  */
void ecu_plugin_menu_setup(GladeXML *xml)
{
	ENTER();
	EXIT();
	return;
}
