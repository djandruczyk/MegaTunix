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
#include <defines.h>
#include <protos.h>
#include <globals.h>

/* Default window size and MINIMUM size as well... */
static int def_width=690;
static int def_height=530;
int width;
int height;
int main_x_origin;
int main_y_origin;
extern gint tips_in_use;
GtkWidget *main_window;
GtkTooltips *tip;
static struct 
{
	gchar *frame_name;
	gint identifier;
	gchar *tab_name;
} notebook_tabs[] = { 
{ "About MegaTunix", ABOUT_PAGE, "About"},
{ "General MegaTunix Settings", GENERAL_PAGE, "General"},
{ "MegaSquirt Communications Parameters", COMMS_PAGE, "Communications"},
{ "MegaSquirt Constants", CONSTANTS_PAGE, "Constants"},
{ "MegaSquirt Enrichments", ENRICHMENTS_PAGE, "Enrichments"},
{ "MegaSquirt VE Table(s)", VETABLES_PAGE, "VE Tables"},
{ "MegaSquirt Runtime Display", RUNTIME_PAGE, "Runtime Disp."},
{ "MegaSquirt Tuning", TUNING_PAGE, "Tuning"},
{ "MegaSquirt Tools", TOOLS_PAGE, "Tools"},
{ "MegaSquirt Advanced Diagnostics", LOWLEVEL_PAGE, "Low-Level"},
{ "MegaSquirt DataLogging", DATALOGGING_PAGE, "DataLogging"}
};

static int num_tabs = sizeof(notebook_tabs) / sizeof(notebook_tabs[0]);

int setup_gui()
{
	GtkWidget *frame;
	GtkWidget *notebook;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *button;
	gint i=0;

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

	tip = gtk_tooltips_new();

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(main_window),vbox);

	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(vbox),notebook,TRUE,TRUE,0);

	for (i=0;i<num_tabs;i++)
	{
		frame = gtk_frame_new (notebook_tabs[i].frame_name);
	        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

		framebuild_dispatch(frame,notebook_tabs[i].identifier);

	        label = gtk_label_new (notebook_tabs[i].tab_name);
	        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	}

	button = gtk_button_new_with_label("Exit");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
        g_signal_connect(G_OBJECT(button),"pressed",
                        G_CALLBACK(leave),NULL);

	if(tips_in_use)
		gtk_tooltips_enable(tip);
	else
		gtk_tooltips_disable(tip);

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
		case VETABLES_PAGE:
			build_vetable(frame);
			break;
		case TUNING_PAGE:
			build_tuning(frame);
			break;
		case TOOLS_PAGE:
			build_tools(frame);
			break;
		case LOWLEVEL_PAGE:
			build_lowlevel(frame);
			break;
		case DATALOGGING_PAGE:
			build_datalogging(frame);
			break;
	}
	return TRUE;
}
