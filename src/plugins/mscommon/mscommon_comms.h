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

#ifndef __MSCOMMON_COMMS_H__
#define __MSCOMMON_COMMS_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Externs */
extern void (*dbg_func_f)(gint, gchar *);
extern void (*io_cmd_f)(const gchar *,void *);
extern OutputData *(*initialize_outputdata_f)(void);
extern void (*thread_update_widget_f)(const gchar *, WidgetType, gchar *);
extern void (*thread_update_logbar_f)(const gchar *, const gchar *, gchar *, gboolean, gboolean);
extern gint (*get_multiplier_f)(DataSize);
extern gboolean (*queue_function_f)(const gchar * );
extern void (*recalc_table_limits_f)(gint, gint);
extern void (*thread_refresh_widgets_at_offset_f)(gint, gint);
extern gboolean (*get_symbol_f)(const gchar *, void **);


/* Prototypes */
void queue_burn_ecu_flash(gint);
void queue_ms1_page_change(gint);
gint comms_test(void);
void ms_handle_page_change(gint , gint );
void ms_table_write(gint, gint, guint8 *);
void ms_send_to_ecu(gint, gint, gint, DataSize, gint, gboolean);
void ms_chunk_write(gint, gint, gint, gint, guint8 *);
void send_to_slaves(void *);
void slaves_set_color(GuiColor,const gchar *);
void update_write_status(void *);
void *restore_update(gpointer);
void start_restore_monitor(void);
void *serial_repair_thread(gpointer);


/* Prototypes */

#endif
