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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"
#include "runtime_gui.h"


struct v1_2_Runtime_Gui runtime_data;

int build_runtime(GtkWidget *parent_frame)
{
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *label;
        GtkWidget *entry;

//	memset(runtime_data,0,sizeof(struct v1_2_Runtime_Gui));

        vbox = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(parent_frame),vbox);
        hbox = gtk_hbox_new(FALSE,10);
        gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
        label = gtk_label_new("MegaSquirt Clock");
        gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),4);
	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
        gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
	runtime_data.secl_val = entry;	/*copy pointer to struct for update */
	
	/* Not written yet */
	return TRUE;
}

void update_runtime_vars()
{
	char buff[10];
	/* test to see if data changed 
	 * Why bother wasting CPU to update the GUI when 
	 * you'd just print the same damn thing?
	 * Makes the code a little uglier, but the gui won't
	 * flicker the text widgets at high update rates
	 */
	if (out.secl != out_last.secl)
	{
		g_snprintf(buff,10,"%i",out.secl);
		gtk_entry_set_text(GTK_ENTRY(runtime_data.secl_val),buff);
	}
}
	
