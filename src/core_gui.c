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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/poll.h>
/* DO NOT include defines.h, as protos.h already does... */
#include "protos.h"
#include "globals.h"

int main_x_origin;
int main_y_origin;
static int def_width=640;
static int def_height=480;
int width;
int height;
GtkWidget *main_window;

int setup_gui()
{
	GtkWidget *frame;
	GtkWidget *notebook;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *button;

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* set name so MegaTunixrc can alter the settings */
	gtk_widget_set_name(main_window, "main window");
        gtk_window_move((GtkWindow *)main_window, main_x_origin, main_y_origin);
        gtk_widget_set_size_request(main_window,def_width,def_height);
        gtk_window_set_default_size(GTK_WINDOW(main_window),width,height);
        gtk_window_set_title(GTK_WINDOW(main_window),"MegaTunix " VERSION);
        gtk_container_set_border_width(GTK_CONTAINER(main_window),0);
        g_signal_connect(G_OBJECT(main_window),"destroy_event",
                        G_CALLBACK(leave),NULL);
        g_signal_connect(G_OBJECT(main_window),"delete_event",
                        G_CALLBACK(leave),NULL);

        gtk_widget_realize(main_window);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(main_window),vbox);

	notebook = gtk_notebook_new ();
	gtk_widget_set_name(notebook, "main notebook");
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(vbox),notebook,TRUE,TRUE,0);

	/* About Page */
	frame = gtk_frame_new ("About MegaTunix");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,ABOUT_PAGE);

        label = gtk_label_new ("About");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* About Page */

	/* General Page */
	frame = gtk_frame_new ("General MegaTunix Settings");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,GENERAL_PAGE);

        label = gtk_label_new ("General");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* General Page */

	/* Comms Page */
	frame = gtk_frame_new ("MegaSquirt Communications Parameters");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,COMMS_PAGE);

        label = gtk_label_new ("Communications");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* Comms Page */

	/* Constants Page */
	frame = gtk_frame_new ("MegaSquirt Constants");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,CONSTANTS_PAGE);

        label = gtk_label_new ("Constants");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* Constants Page */

	/* Enrichments Page */
	frame = gtk_frame_new ("MegaSquirt Enrichments");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,ENRICHMENTS_PAGE);

        label = gtk_label_new ("Enrichments");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* Enrichments Page */

	/* VE Table Page */
	frame = gtk_frame_new ("MegaSquirt VE Table");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,VETABLE_PAGE);

        label = gtk_label_new ("VE Table");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* VE Table Page */

	/* Runtime Page */
	frame = gtk_frame_new ("MegaSquirt Runtime Display");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,RUNTIME_PAGE);

        label = gtk_label_new ("Runtime Disp.");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* Runtime Page */

	/* Tuning Page */
	frame = gtk_frame_new ("MegaSquirt Tuning");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,TUNING_PAGE);

        label = gtk_label_new ("Tuning");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* Tuning Page */

	/* Tools Page */
	frame = gtk_frame_new ("MegaSquirt Tools");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,TOOLS_PAGE);

        label = gtk_label_new ("Tools");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* Tools Page */

	/* Datalogging Page */
	frame = gtk_frame_new ("MegaSquirt DataLogging");
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	framebuild_dispatch(frame,DATALOGGING_PAGE);

        label = gtk_label_new ("DataLogging");
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	/* DataLogging Page */

	button = gtk_button_new_with_label("Exit");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
        g_signal_connect(G_OBJECT(button),"pressed",
                        G_CALLBACK(leave),NULL);


	gtk_widget_show_all(main_window);

	return TRUE;
}

int framebuild_dispatch(GtkWidget *frame, gint data)
{
	switch (data)
	{
		case ABOUT_PAGE:
			build_about(frame);
			break;
		case GENERAL_PAGE:
			build_general(frame);
			break;
		case COMMS_PAGE:
			build_comms(frame);
			break;
		case CONSTANTS_PAGE:
			build_constants(frame);
			break;
		case ENRICHMENTS_PAGE:
			build_enrichments(frame);
			break;
		case RUNTIME_PAGE:
			build_runtime(frame);
			break;
		case VETABLE_PAGE:
			build_vetable(frame);
			break;
		case TUNING_PAGE:
			build_tuning(frame);
			break;
		case TOOLS_PAGE:
			build_tools(frame);
			break;
		case DATALOGGING_PAGE:
			build_datalogging(frame);
			break;
	}
	return TRUE;
}
