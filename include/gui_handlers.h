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

#ifndef __GUI_HANDLERS_H__
#define __GUI_HANDLERS_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <watches.h>

/* Regular Buttons */
typedef enum
{
	GO_LEFT = 0x990,
	GO_RIGHT,
	GO_UP,
	GO_DOWN
}Direction;


/* Prototypes */
gboolean prevent_close(GtkWidget *, gpointer );
gboolean leave(GtkWidget *, gpointer);
gboolean comm_port_override(GtkEditable *);
gboolean std_button_handler(GtkWidget *, gpointer);
gboolean std_entry_handler(GtkWidget *, gpointer);
gboolean std_combo_handler(GtkWidget *, gpointer);
gboolean entry_changed_handler(GtkWidget *, gpointer );
gboolean toggle_button_handler(GtkWidget *, gpointer);
gboolean bitmask_button_handler(GtkWidget *, gpointer);
gboolean spin_button_handler(GtkWidget *, gpointer);
gboolean widget_grab(GtkWidget *, GdkEventButton *, gpointer );
gboolean key_event(GtkWidget *, GdkEventKey *, gpointer );
gboolean set_algorithm(GtkWidget *, gpointer );
void notebook_page_changed(GtkNotebook *, GtkNotebookPage *, guint, gpointer);
void subtab_changed(GtkNotebook *, GtkNotebookPage *, guint, gpointer);
gboolean focus_out_handler(GtkWidget *, GdkEventFocus *, gpointer );
gboolean slider_value_changed(GtkWidget *, gpointer );
gboolean force_update_table(gpointer);
gboolean trigger_group_update(gpointer );
void prompt_to_save(void);
gboolean prompt_r_u_sure(void);
guint get_bitshift(guint );
void update_misc_gauge(DataWatch *);
glong get_extreme_from_size(DataSize, Extreme);
gboolean clamp_value(GtkWidget *, gpointer);
void refocus_cell(GtkWidget *, Direction);
void set_widget_label_from_array(gpointer, gpointer);
void insert_text_handler(GtkEntry *, const gchar *, gint, gint *, gpointer);
void set_widget_labels(const gchar *);
void swap_labels(GtkWidget *, gboolean);
void switch_labels(gpointer, gpointer);
void combo_toggle_groups_linked(GtkWidget *,gint);
void combo_toggle_labels_linked(GtkWidget *,gint);
void set_widget_label_from_array(gpointer, gpointer);
void combo_set_labels(GtkWidget *, GtkTreeModel *);
gint get_choice_count(GtkTreeModel *);
void recalc_table_limits(gint, gint);
void update_interdependancies_pf(void);




/* Prototypes */

#endif
