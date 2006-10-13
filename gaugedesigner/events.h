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
	MAJ_TICKS=0x55,
	MIN_TICKS,
	MAJ_TICK_LEN,
	MIN_TICK_LEN,
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
}func;

/* Prototypes */
EXPORT gboolean create_new_gauge(GtkWidget *, gpointer );
EXPORT gboolean create_color_span(GtkWidget *, gpointer );
EXPORT gboolean entry_changed_handler(GtkWidget *, gpointer );
EXPORT gboolean set_antialiased_mode(GtkWidget *, gpointer );
void update_attributes(GladeXML * );
/* Prototypes */

#endif
