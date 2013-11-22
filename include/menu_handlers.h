/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file include/menu_handlers.h
  \ingroup Headers
  \brief Header for the global menu handlers
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MENU_HANDLERS_H__
#define __MENU_HANDLERS_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

/* Enumerations */
typedef enum
{
	ALL_TABLE_IMPORT=0x230,
	ALL_TABLE_EXPORT,
	ECU_BACKUP,
	ECU_RESTORE
}FioAction;

/* Prototypes */
gboolean check_tab_existance(TabIdent );
gboolean jump_to_tab(GtkWidget *, gpointer );
gboolean settings_transfer(GtkWidget *, gpointer );
void setup_menu_handlers_pf(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
