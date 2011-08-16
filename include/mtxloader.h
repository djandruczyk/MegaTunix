/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file include/mtxloader.h
  \ingroup Headers
  \brief Header for the graphical firmware loader
  \author David Andruczyk
  */

#ifndef __MTXLOADER_H__
#define __MTXLOADER_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean load_firmware (GtkButton*);
gboolean get_signature (GtkButton*);
gboolean leave (GtkWidget *, gpointer);
gboolean about_popup (GtkWidget *, gpointer);
gboolean persona_choice (GtkWidget *, gpointer);
void load_defaults(void);
void save_defaults(void);
void init_controls(void);
void output(gchar *, gboolean);
void boot_jumper_prompt(void);
void lock_buttons(void);
void unlock_buttons(void);
void gui_progress_update(gfloat);


#endif
