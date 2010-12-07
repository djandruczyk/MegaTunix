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

#ifndef __MSCOMMON_HELPERS_H__
#define __MSCOMMON_HELPERS_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Externs */
extern void (*io_cmd_f)(const gchar *,void *);
extern GList *(*get_list_f)(gchar *);
extern void (*set_widget_sensitive_f)(gpointer, gpointer);
extern OutputData *(*initialize_outputdata_f)(void);
extern void (*set_title_f)(const gchar *);
extern void (*set_widget_sensitive_f)(gpointer, gpointer);
extern void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
extern void (*process_rt_vars_f)(void * );
extern void (*thread_update_widget_f)(const gchar *, WidgetType, gchar *);

/* Prototypes */
void spawn_read_ve_const_pf(void);
void enable_get_data_buttons_pf(void);
void simple_read_pf(void *, XmlCmdType);
gboolean read_ve_const(void *, XmlCmdType);
gboolean burn_all_helper(void *, XmlCmdType);
void post_single_burn_pf(void *data);
void post_burn_pf(void);
void startup_tcpip_sockets_pf(void);
/* Prototypes */

#endif
