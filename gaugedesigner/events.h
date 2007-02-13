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
	NAME_XPOS,
	NAME_YPOS,
	UNITS_XPOS,
	UNITS_YPOS,
	VALUE_XPOS,
	VALUE_YPOS,
	TICK_INSET,
	START_ANGLE,
	STOP_ANGLE,
	LBOUND,
	UBOUND,
	NAME_STR,
	UNITS_STR,
	NAME_SCALE,
	UNITS_SCALE,
	VALUE_SCALE,
	NAME_FONT,
	UNITS_FONT,
	VALUE_FONT,
	NEEDLE_TAIL,
	NEEDLE_WIDTH,
	PRECISION,
	AA,
	ADJ_UNIT_PARTNER,
	ADJ_ANGLE_PARTNER,
}func;

typedef enum
{
	IMPORT_XML = 0x1AA,
	EXPORT_XML
}StdButton;

typedef enum
{
	ANTIALIAS = 0x1BB,
	SHOW_VALUE,
}CheckButton;


/* Prototypes */
EXPORT gboolean create_new_gauge(GtkWidget *, gpointer );
EXPORT gboolean close_current_gauge(GtkWidget *, gpointer );
EXPORT gboolean create_text_block(GtkWidget *, gpointer );
EXPORT gboolean create_color_span(GtkWidget *, gpointer );
EXPORT gboolean set_antialiased_mode(GtkWidget *, gpointer );
EXPORT gboolean change_font(GtkWidget *, gpointer );
EXPORT gboolean xml_button_handler(GtkWidget *, gpointer );
EXPORT gboolean animate_gauge(GtkWidget *, gpointer );
EXPORT gboolean toggle_skip_params(GtkWidget *, gpointer );
void update_attributes(void);
void reset_onscreen_controls(void);
void reset_onscreen_ranges(void);
void reset_onscreen_tblocks(void);
void reset_onscreen_tgroups(void);
void update_onscreen_ranges(void);
void update_onscreen_tblocks(void);
void update_onscreen_tgroups(void);
gboolean alter_tblock_data(GtkWidget *, gpointer );
gboolean alter_crange_data(GtkWidget *, gpointer );
gboolean alter_tgroup_data(GtkWidget *, gpointer );
gboolean remove_crange(GtkWidget *, gpointer );
gboolean remove_tblock(GtkWidget *, gpointer );
gboolean remove_tgroup(GtkWidget *, gpointer );
gboolean sweep_gauge(gpointer data);
/* Prototypes */

#endif
