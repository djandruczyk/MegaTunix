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

#ifndef __RUNTIME_SLIDERS_H__
#define __RUNTIME_SLIDERS_H__

#include <enums.h>
#include <gtk/gtk.h>


typedef struct _Rt_Slider Rt_Slider;
/*! 
 \brief The _Rt_Slider struct contains info on the runtime display tab sliders
 as they are now stored in the config file and adjustable in position
 and placement and such..
 */
struct _Rt_Slider
{
	gchar *ctrl_name;	/*! Ctrl name in config file (key in hash) */
	GtkWidget *parent;	/*! Parent of the table below  */
	GtkWidget *label;	/*! Label in runtime display */
	GtkWidget *textval;	/*! Label in runtime display */
	GtkWidget *pbar;	/*! progress bar for the data */
	gint table_num;		/*! Refers to the table number in the profile*/
	gint tbl;		/*! Table number (0-3) */
	gint row;		/*! Starting row */
	gchar *friendly_name;	/*! text for Label above */
	gint lower;		/*! Lower limit */
	gint upper;		/*! Upper limit */
	GArray *history;	/*! where the data is from */
	gfloat last_percentage;	/*! last percentage of on screen slider */
	GObject *object;	/*! object of obsession.... */
	gboolean enabled;	/*! Pretty obvious */
	gint count;		/*! used to making sure things update */
	gint rate;		/*! used to making sure things update */
	gint last_upd;		/*! used to making sure things update */
	WidgetType class;	/*! Slider type... */
};

/* Prototypes */
void load_sliders_pf(void );
void load_ve3d_sliders(gint );
void register_rt_range(GtkWidget *);
Rt_Slider * add_slider(gchar *, gint, gint, gint, gchar *,TabIdent );
gboolean free_ve3d_sliders(gint);
/* Prototypes */

#endif
