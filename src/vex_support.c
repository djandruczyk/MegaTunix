/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <enums.h>
#include <globals.h>
#include <stdio.h>
#include <structures.h>
#include <vex_support.h>

//extern struct  


void vetable_export()
{
	printf("export VEtable\n");
	present_filesavebox(VE_EXPORT);
}

void present_filesavebox(FileIoType data)
{
	GtkWidget *file_selector;

	file_selector = gtk_file_selection_new("Please select a filename to save the VEtable to");
	g_object_set_data(G_OBJECT(file_selector),"iotype",
                                GINT_TO_POINTER(data));

	gtk_file_selection_set_select_multiple(
			GTK_FILE_SELECTION(file_selector),
			FALSE);

	g_signal_connect (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (check_filename),
			(gpointer) file_selector);

	g_signal_connect_swapped (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->ok_button),
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			(gpointer) file_selector);

	g_signal_connect_swapped (GTK_OBJECT
			(GTK_FILE_SELECTION (file_selector)->cancel_button),
			"clicked",
			G_CALLBACK (gtk_widget_destroy),
			(gpointer) file_selector);

	/* Display that dialog */

	gtk_widget_show (file_selector);

}


void vetable_import()
{
	printf("import VEtable\n");
	present_filesavebox(VE_IMPORT);
}
