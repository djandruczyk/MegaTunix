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
  \file src/plugins/mscommon/mscommon_gui_handlers.h
  \ingroup MSCommonPlugin,Headers
  \brief MSCommon Gui handlers
  \author David Andruczyk
  */

#ifndef __MSCOMMON_GUI_HANDLERS_H__
#define __MSCOMMON_GUI_HANDLERS_H__

#include <configfile.h>
#include <enums.h>
#include <gtk/gtk.h>

typedef enum
{
	INCREMENT_VALUE = LAST_STD_BUTTON_ENUM + 1,
	DECREMENT_VALUE,
	REQFUEL_RESCALE_TABLE,
	REQ_FUEL_POPUP,
	LAST_COMMON_STD_BUTTON_ENUM
}MSCommonStdButton;

typedef enum
{
	LAST_COMMON_TOGGLE_BUTTON_ENUM = LAST_TOGGLE_BUTTON_ENUM + 1
}MSCommonToggleButton;

typedef enum
{
	GENERIC = LAST_BUTTON_ENUM + 3,
	NUM_SQUIRTS_1,
	NUM_SQUIRTS_2,
	NUM_CYLINDERS_1,
	NUM_CYLINDERS_2,
	NUM_INJECTORS_1,
	NUM_INJECTORS_2,
	LOCKED_REQ_FUEL,
	REQ_FUEL_1,
	REQ_FUEL_2,
	MULTI_EXPRESSION,
	ALT_SIMUL,
	LAST_COMMON_BUTTON_ENUM
}MSCommonMtxButton;

/* Prototypes */
gboolean common_std_button_handler(GtkWidget *, gpointer);
gboolean common_toggle_button_handler(GtkWidget *, gpointer);
gboolean common_bitmask_button_handler(GtkWidget *, gpointer);
gboolean common_entry_handler(GtkWidget *, gpointer);
gboolean common_slider_handler(GtkWidget *, gpointer);
gboolean common_combo_handler(GtkWidget *, gpointer);
gboolean common_spin_button_handler(GtkWidget *, gpointer);

void update_combo(GtkWidget *);
void update_entry(GtkWidget *);
void update_checkbutton(GtkWidget *);
gboolean force_update_table(gpointer);
gboolean trigger_group_update(gpointer);
gboolean update_multi_expression(gpointer);
void combo_handle_group_2_update(GtkWidget *);
void combo_handle_algorithms(GtkWidget *);
void handle_group_2_update(GtkWidget *);
void handle_algorithm(GtkWidget *);
void combo_set_labels(GtkWidget *, GtkTreeModel *);
gboolean search_model(GtkTreeModel *, GtkWidget *, GtkTreeIter *);
void toggle_groups_linked(GtkWidget *, gboolean);
void combo_toggle_groups_linked(GtkWidget *,gint);
void combo_toggle_labels_linked(GtkWidget *,gint);
void set_widget_label_from_array(gpointer, gpointer);
void get_essential_bits(GtkWidget *, gint *, gint *, gint *, gint *, gint *, gint *);
void get_essentials(GtkWidget *, gint *, gint *, gint *, DataSize *, gint *);
void update_widget(gpointer, gpointer);
void recalc_table_limits(gint, gint);
void common_gui_init(void);
/* Prototypes */



#endif
