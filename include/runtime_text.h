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

#ifndef __RUNTIME_TEXT_H__
#define __RUNTIME_TEXT_H__

#include <enums.h>
#include <gtk/gtk.h>

typedef struct _Rt_Text Rt_Text;
/*! 
 \brief The _Rt_Text struct contains info on the floating runtime var text 
 display
 */
struct _Rt_Text
{
	gchar *ctrl_name;	/*! Ctrl name in config file (key in hash) */
	GtkWidget *parent;	/*! Parent of the table below  */
	GtkWidget *name_label;	/*! Label in runtime display */
	GtkWidget *textval;	/*! Label in runtime display */
	gchar *friendly_name;	/*! text for Label above */
	GArray *history;	/*! where the data is from */
	GObject *object;	/*! object of obsession.... */
	gint count;		/*! used to making sure things update */
	gint rate;		/*! used to making sure things update */
	gint last_upd;		/*! used to making sure things update */
};

/* Prototypes */
void load_rt_text_pf(void );
Rt_Text * add_rtt(GtkWidget *, gchar *, gchar *);
/* Prototypes */

#endif
