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
#include <firmware.h>
#include <threads.h>

/* Prototypes */
void init(void);
gboolean read_config(void);
void save_config(void);
void make_megasquirt_dirs(void);
void mem_alloc(void);
void mem_dealloc(void);
void dealloc_textmessage(Text_Message * );
void dealloc_message(Io_Message * );
void dealloc_w_update(Widget_Update * );
void dealloc_table_params(Table_Params * );
void dealloc_qfunction(QFunction * );
void dealloc_array(GArray *, ArrayType );
Io_Message * initialize_io_message(void);
OutputData * initialize_outputdata(void);
Text_Message * initialize_text_message(void);
Page_Params * initialize_page_params(void);
Table_Params * initialize_table_params(void);
/* Prototypes */

#endif
