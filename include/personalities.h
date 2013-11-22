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
  \file include/personalities.h
  \ingroup Headers
  \brief Header for the personality selection functions
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __PERSONALITIES_H__
#define __PERSONALITIES_H__

#include <enums.h>
#include <gtk/gtk.h>

typedef struct _PersonaElement PersonaElement;
/*!
  \struct _PersonaElement
  \brief Datastructure to hold details of the currently loaded ECU Personality 
  plugin
  */
struct _PersonaElement 
{
	gchar *filename;	/*!< Filename */
	gchar *sequence;	/*!< Sequence number */
	gchar *persona;		/*!< To match against CLI options */
	gchar *dirname;		/*!< Dirname */
	gchar *name;		/*!< Shortname in choice box */
	gchar *ecu_lib;		/*!< ECU specific library */
	gchar *common_lib;	/*!< Common Library */
	gchar *baud_str;	/*!< Baud string, i.e. 9600,8,n,1 */
	gboolean def;		/*!< Default choice */
};
/* Prototypes */
void free_persona_element(gpointer, gpointer);
void persona_dialog_response(GtkDialog *, gint , gpointer );
gboolean persona_selection(GtkWidget *, gpointer);
gint persona_seq_sort(gconstpointer a, gconstpointer b);
gboolean personality_choice(void);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
