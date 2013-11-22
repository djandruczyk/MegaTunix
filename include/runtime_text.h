/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file include/runtime_text.h
  \ingroup Headers
  \brief Header for the runtime text code
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __RUNTIME_TEXT_H__
#define __RUNTIME_TEXT_H__

#include <enums.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <watches.h>


typedef enum
{
	COL_RTT_OBJECT,
	COL_RTT_INT_NAME,
	COL_RTT_DATA,
	COL_RTT_LAST,
	COL_RTT_FGCOLOR,
	COL_RTT_BGCOLOR,
	RTT_NUM_COLS
}RttTVCols;

typedef struct _Rt_Text Rt_Text;
typedef struct _Rtt_Threshold Rtt_Threshold;

/*!
 \brief The _Rt_Text struct contains info on the floating runtime var text 
 display
 */
struct _Rt_Text
{
	gchar *ctrl_name;	/*!< Ctrl name in config file (key in hash) */
	GtkWidget *parent;	/*!< Parent of the table below  */
	GtkWidget *name_label;	/*!< Label in runtime display */
	GtkWidget *textval;	/*!< Label in runtime display */
	gchar *friendly_name;	/*!< text for Label above */
	gchar *label_prefix;	/*!< markup strings */
	gchar *label_suffix;	/*!< markup strings */
	gconstpointer *object;  	/*!< object of obsession.... */
	gint count;		/*!< used to making sure things update */
	gint rate;		/*!< used to making sure things update */
	gint last_upd;		/*!< used to making sure things update */
	gboolean show_prefix;	/*!< show prefix (friendly name) or not */
	gboolean markup;	/*!< Uses markup or not? */
	GPtrArray *thresholds;	/*!< Array of threshold objects */
};

struct _Rtt_Threshold
{
	gfloat low;		/*!< Low threshold */
	gfloat high;		/*!< Low threshold */
	gchar *fg;		/*!< FG Color */
	gchar *bg;		/*!< BG Color */
};

/* Prototypes */
void add_additional_rtt(GtkWidget *);
Rt_Text *add_rtt(GtkWidget *, gchar *);
Rt_Text *create_rtt(gchar *, gchar *, gboolean);
void load_rt_text_pf(void );
void load_rtt(xmlNode *, GtkListStore *, GtkWidget *);
Rtt_Threshold *load_rtt_threshold(xmlNode *);
gboolean load_rtt_xml_elements(xmlNode *, GtkListStore *, GtkWidget *);
gboolean rtt_foreach(GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);
void rtt_thresh_free(gpointer);
void rtt_update_values(gpointer,gpointer,gpointer);
void rtt_update_start_watches(gpointer,gpointer,gpointer);
void setup_rtt_treeview(GtkWidget *);
gboolean update_rttext(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
