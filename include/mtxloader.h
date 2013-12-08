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
  \file include/mtxloader.h
  \ingroup Headers
  \brief Header for the graphical firmware loader
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MTXLOADER_H__
#define __MTXLOADER_H__

#include <gtk/gtk.h>

/* Prototypes */
gboolean about_popup (GtkWidget *, gpointer);
gboolean get_signature (GtkButton*);
void gui_progress_update(gfloat);
void init_controls(void);
gboolean leave (GtkWidget *, gpointer);
void load_defaults(void);
gboolean load_firmware (GtkButton*);
void lock_buttons(void);
void output(gchar *, gboolean);
gboolean persona_choice (GtkWidget *, gpointer);
void save_defaults(void);
void unlock_buttons(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
