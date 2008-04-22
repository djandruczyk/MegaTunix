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

#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
void enable_interrogation_button_cb(void);
void start_statuscounts_cb(void);
void spawn_read_ve_const_cb(void);
void enable_get_data_buttons_cb(void);
void enable_ttm_buttons_cb(void);
void enable_reboot_button_cb(void);
void conditional_start_rtv_tickler_cb(void);
void set_store_black_cb(void);
void enable_3d_buttons_cb(void);
void disable_burner_buttons_cb(void);
void reset_temps_cb(void);
void simple_read_cb(void *, XmlCmdType);
gboolean read_ve_const(void *, XmlCmdType);
gboolean ms2_burn_all_helper(void *, XmlCmdType);
/* Prototypes */

#endif
