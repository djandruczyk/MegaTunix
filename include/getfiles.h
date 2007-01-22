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

#ifndef __GETFILES_H__
#define __GETFILES_H__

#include <gtk/gtk.h>

typedef struct _MtxFileIO MtxFileIO;

struct _MtxFileIO
{
	GtkFileChooserAction action;	/* Action, save,open, etc.. */
	gchar *filter;			/* File filter string */
	gchar *stub_path;		/* starting path to set filesel to */
	gchar **shortcut_folders;	/* CSV list of ADDITIONAL shortcut 
					   folders */
	gchar *filename;		/* Filename to save (save ONLY) */
	gchar *title;			/* Choser title */
};

/* Prototypes */
gchar ** get_files(gchar *, gchar *);
gchar * get_file(gchar *, gchar *);
gchar * choose_file(MtxFileIO *);
void free_mtxfileio(MtxFileIO *);
/* Prototypes */

#endif
