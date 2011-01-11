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

#ifndef __FREEMS_GUI_HANDLERS_H__
#define __FREEMS_GUI_HANDLERS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>

/* Enumerations */
typedef enum
{
	SOFT_BOOT_ECU = LAST_STD_BUTTON_ENUM + 1,
	HARD_BOOT_ECU
}FreeEMSStdButton;
/* Enumerations */

/* Prototypes */
gboolean common_button_handler(GtkWidget *, gpointer);
gboolean common_bitmask_button_handler(GtkWidget *, gpointer);
void common_gui_init(void);
/* Prototypes */

#endif
