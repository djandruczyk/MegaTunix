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

#ifndef __TBLOCKS_H__
#define __TBLOCKS_H__

#include <defines.h>
#include <gauge.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean create_text_block_event(GtkWidget *, gpointer );
void reset_onscreen_tblocks(void);
void update_onscreen_tblocks(void);
gboolean alter_tblock_data(GtkWidget *, gpointer );
gboolean remove_tblock(GtkWidget *, gpointer );
GtkWidget * build_tblock(MtxTextBlock *, gint );
/* Prototypes */

#endif
