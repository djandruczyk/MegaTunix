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

#ifndef __INIT_H__
#define __INIT_H__

#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void init(void);
gboolean read_config(void);
void save_config(void);
void make_megasquirt_dirs(void);
void mem_alloc(void);
void mem_dealloc(void);
void dealloc_textmessage(struct Text_Message * );
void dealloc_message(struct Io_Message * );
void dealloc_w_update(struct Widget_Update * );
void dealloc_table_params(struct Table_Params * );
struct Io_Message * initialize_io_message(void);
struct Text_Message * initialize_text_message(void);
struct Canidate * initialize_canidate(void);
struct Page_Params * initialize_page_params(void);
struct Table_Params * initialize_table_params(void);
/* Prototypes */

#endif
