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

#ifndef __GUI_HANDLERS_H__
#define __GUI_HANDLERS_H__

#include <gtk/gtk.h>

/* Prototypes */
void leave(GtkWidget *, gpointer);
gint comm_port_change(GtkEditable *);
gint std_button_handler(GtkWidget *, gpointer);
gint toggle_button_handler(GtkWidget *, gpointer);
gint bitmask_button_handler(GtkWidget *, gpointer);
gint spinner_changed(GtkWidget *, gpointer);
gint classed_spinner_changed(GtkWidget *, gpointer);
gint set_logging_mode(GtkWidget * widget, gpointer *data);
void set_widget_state(gpointer, gpointer);
void set_enhanced_idle_state(gboolean);
void check_req_fuel_limits(void);
void check_config11(unsigned char);
void check_config13(unsigned char);
void check_tblcnf(unsigned char);
void set_dualtable_mode(gboolean);
void start_runtime_display(void);
void stop_runtime_display(void);
gboolean populate_gui(void);
/* Prototypes */

#endif
