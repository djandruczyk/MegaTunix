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

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <defines.h>
#include <glade/glade.h>
#include <gtk/gtk.h>

typedef enum
{
	ADJ_LOW_UNIT_PARTNER,
	ADJ_HIGH_UNIT_PARTNER,
	ADJ_START_ANGLE_PARTNER,
	ADJ_SWEEP_ANGLE_PARTNER
}func;

typedef enum
{
	IMPORT_XML = 0x1AA,
	EXPORT_XML
}StdButton;


/* Prototypes */
EXPORT gboolean create_new_gauge(GtkWidget *, gpointer );
EXPORT gboolean close_current_gauge(GtkWidget *, gpointer );
EXPORT gboolean create_polygon_event(GtkWidget *, gpointer );
EXPORT gboolean create_text_block_event(GtkWidget *, gpointer );
EXPORT gboolean create_color_span_event(GtkWidget *, gpointer );
EXPORT gboolean set_antialiased_mode(GtkWidget *, gpointer );
EXPORT gboolean change_font(GtkWidget *, gpointer );
EXPORT gboolean xml_button_handler(GtkWidget *, gpointer );
EXPORT gboolean animate_gauge(GtkWidget *, gpointer );
EXPORT gboolean toggle_skip_params(GtkWidget *, gpointer );
EXPORT gboolean link_range_spinners(GtkWidget *, gpointer );
void update_attributes(void);
void reset_onscreen_controls(void);
void reset_onscreen_c_ranges(void);
void reset_onscreen_a_ranges(void);
void reset_onscreen_tblocks(void);
void reset_onscreen_tgroups(void);
void reset_onscreen_polygons(void);
void update_onscreen_c_ranges(void);
void update_onscreen_a_ranges(void);
void update_onscreen_tblocks(void);
void update_onscreen_tgroups(void);
void update_onscreen_polygons(void);
gboolean tg_spin_button_handler(GtkWidget *, gpointer );
gboolean generic_spin_button_handler(GtkWidget *, gpointer );
gboolean alter_tblock_data(GtkWidget *, gpointer );
gboolean alter_c_range_data(GtkWidget *, gpointer );
gboolean alter_a_range_data(GtkWidget *, gpointer );
gboolean alter_tgroup_data(GtkWidget *, gpointer );
gboolean alter_polygon_data(GtkWidget *, gpointer );
gboolean remove_c_range(GtkWidget *, gpointer );
gboolean remove_a_range(GtkWidget *, gpointer );
gboolean remove_tblock(GtkWidget *, gpointer );
gboolean remove_tgroup(GtkWidget *, gpointer );
gboolean remove_polygon(GtkWidget *, gpointer );
gboolean sweep_gauge(gpointer data);
/* Prototypes */

#endif
