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

#include <defines.h>
#include <protos.h>
#include <constants.h>
#include <globals.h>
#include <gtk/gtk.h>


extern GdkColor red;
extern GdkColor black;
extern struct Buttons buttons;


void set_store_red()
{
	/* Let user know to burn vars byt turnign button text red */
	gtk_widget_modify_fg(GTK_BIN(buttons.const_store_but)->child,
			GTK_STATE_NORMAL,&red);
	gtk_widget_modify_fg(GTK_BIN(buttons.const_store_but)->child,
			GTK_STATE_PRELIGHT,&red);
	gtk_widget_modify_fg(GTK_BIN(buttons.enrich_store_but)->child,
			GTK_STATE_NORMAL,&red);
	gtk_widget_modify_fg(GTK_BIN(buttons.enrich_store_but)->child,
			GTK_STATE_PRELIGHT,&red);
	gtk_widget_modify_fg(GTK_BIN(buttons.vetable_store_but)->child,
			GTK_STATE_NORMAL,&red);
	gtk_widget_modify_fg(GTK_BIN(buttons.vetable_store_but)->child,
			GTK_STATE_PRELIGHT,&red);
}

void set_store_black()
{
	gtk_widget_modify_fg(GTK_BIN(buttons.const_store_but)->child,
			GTK_STATE_NORMAL,&black);
	gtk_widget_modify_fg(GTK_BIN(buttons.const_store_but)->child,
			GTK_STATE_PRELIGHT,&black);
	gtk_widget_modify_fg(GTK_BIN(buttons.enrich_store_but)->child,
			GTK_STATE_NORMAL,&black);
	gtk_widget_modify_fg(GTK_BIN(buttons.enrich_store_but)->child,
			GTK_STATE_PRELIGHT,&black);
	gtk_widget_modify_fg(GTK_BIN(buttons.vetable_store_but)->child,
			GTK_STATE_NORMAL,&black);
	gtk_widget_modify_fg(GTK_BIN(buttons.vetable_store_but)->child,
			GTK_STATE_PRELIGHT,&black);
}

