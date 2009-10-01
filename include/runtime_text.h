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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <watches.h>


typedef enum
{
	RTT_INT_NAME,
	RTT_DATA,
	RTT_NUM_COLS
}RttTVCols;

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
	gboolean show_prefix;	/*! show prefix (friendly name) or not */
	gboolean markup;	/*! Uses markup or not? */
};

/* Prototypes */
void load_rt_text_pf(void );
Rt_Text * add_rtt(GtkWidget *, gchar *, gchar *, gboolean);
Rt_Text * add_custom_rtt(GtkWidget *, gchar *, gchar *, gboolean);
void rtt_update_values(gpointer,gpointer,gpointer);
void rtt_update_start_watches(gpointer,gpointer,gpointer);
void add_additional_rtt(GtkWidget *);
gboolean load_rtt_xml_elements(xmlNode *, GHashTable *, GtkWidget *);
void load_rtt(xmlNode *, GHashTable *, GtkWidget *);
void setup_rtt_treeview(GtkWidget *);
EXPORT void update_rtt_wrapper(DataWatch *, gfloat);
/* Prototypes */

#endif
