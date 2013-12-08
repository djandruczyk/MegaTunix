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
  \file include/init.h
  \ingroup Headers
  \brief Header for thememory init/dealloc and config load/save routines
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __INIT_H__
#define __INIT_H__

#include <gtk/gtk.h>
#include <firmware.h>
#include <threads.h>

/* Prototypes */
void cleanup(void *);
void init(void);
Gui_Message * initialize_gui_message(void);
Io_Message * initialize_io_message(void);
OutputData * initialize_outputdata(void);
Text_Message * initialize_text_message(void);
void dealloc_array(GArray *, ArrayType );
void dealloc_gauge(gpointer, gpointer);
void dealloc_gui_message(Gui_Message * );
void dealloc_io_message(Io_Message * );
void dealloc_list(gpointer, gpointer, gpointer);
void dealloc_lists_hash(gpointer);
void dealloc_lookuptable(gpointer data);
void dealloc_qfunction(QFunction * );
void dealloc_rtt(gpointer);
gboolean dealloc_rtt_model(GtkTreeModel *, GtkTreePath *, GtkTreeIter *,gpointer);
void dealloc_rtv_map(gpointer);
void dealloc_rtv_object(gconstpointer *);
void dealloc_slider(gpointer);
void dealloc_tabinfo(gpointer, gpointer);
void dealloc_tabinfos(gpointer );
void dealloc_table_params(Table_Params * );
void dealloc_te_params(TE_Params * );
void dealloc_textmessage(Text_Message * );
void dealloc_w_update(Widget_Update * );
void dealloc_widget(gpointer, gpointer);
void make_mtx_dirs(void);
void mem_alloc(void);
void mem_dealloc(void);
gboolean read_config(void);
void save_config(void);
void xml_cmd_free(gpointer);
void xml_arg_free(gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
