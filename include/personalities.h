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

#ifndef __PERSONALITIES_H__
#define __PERSONALITIES_H__

#include <enums.h>
#include <gtk/gtk.h>

typedef struct _PersonaElement PersonaElement;
struct _PersonaElement 
{
	gchar *filename;	/* Filename */
	gchar *dirname;		/* Dirname */
	gchar *name;		/* Shortname in choice box */
	gchar *ecu_lib;		/* ECU specific library */
	gchar *common_lib;	/* Common Library */
	gint baud;		/* Random data */
	gboolean def;		/* Default choice */
};
/* Prototypes */
gboolean personality_choice(void);
gboolean persona_selection(GtkWidget *, gpointer);
/* Prototypes */

#endif
