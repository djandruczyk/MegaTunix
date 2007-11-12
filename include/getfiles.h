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


typedef enum
{
	PERSONAL=0x99a,
	SYSTEM,
}FileClass;

struct _MtxFileIO
{
	GtkFileChooserAction action;	/* Action, save,open, etc.. */
	GtkWidget *parent;		/* Parent widget for transient windows */
	gboolean on_top;		/* Set it transient? */
	gchar *filter;			/* CSV pairs of File filter string */
	gchar *absolute_path;		/* absolute path to set filesel to */
	gchar *default_path;		/* default path to set filesel to */
	gchar *external_path;		/* external path rel to homedir */
	gchar *shortcut_folders;	/* CSV list of ADDITIONAL shortcut 
					   folders */
	gchar *default_extension;	/* Default file extension */
	gchar *default_filename;	/* If no name passed,suggest this name*/
	gchar *filename;		/* Filename to save (save ONLY) */
	gchar *title;			/* Choser title */
};

/* Static private functions */
#if GTK_MINOR_VERSION >= 8
GtkFileChooserConfirmation confirm_overwrite_callback (GtkFileChooser *, gpointer );
#endif

/* Prototypes */
gchar ** get_files(gchar *, gchar *, GArray **);
gchar * get_file(gchar *, gchar *);
gchar * choose_file(MtxFileIO *);
void free_mtxfileio(MtxFileIO *);
void getfiles_errmsg(const gchar * );
/* Prototypes */

#endif
