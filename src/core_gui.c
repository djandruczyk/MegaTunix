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

#include <about_gui.h>
#include <comms_gui.h>
#include <config.h>
#include <constants_gui.h>
#include <core_gui.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <enrichments_gui.h>
#include <enums.h>
#include <general_gui.h>
#include <globals.h>
#include <gui_handlers.h>
#include <ignition_gui.h>
#include <lowlevel_gui.h>
#include <runtime_gui.h>
#include <tuning_gui.h>
#include <tools_gui.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>



/* Default window size and MINIMUM size as well... */
static int def_width=717;
static int def_height=575;
int width;
int height;
int main_x_origin;
int main_y_origin;
extern gint tips_in_use;
GtkWidget *main_window;
GtkTooltips *tip;
static struct 
{
	gchar *frame_name;	/* Textual name at the top of the frame */
	GuiFramePage page;	/* identifier used when building each frame */
	gchar *tab_name;	/* The Tab textual name for the main gui */
	gboolean enabled;	/* Is the tab enabled (sensitive) or not? */
} notebook_tabs[] = { 
{ "About MegaTunix", ABOUT_PAGE, "About",TRUE},
{ "General MegaTunix Settings", GENERAL_PAGE, "General",TRUE},
{ "MegaSquirt Communications Parameters", COMMS_PAGE, "Communications",TRUE},
{ "MegaSquirt Constants", CONSTANTS_PAGE, "Constants",TRUE},
{ "MegaSquirt Enrichments", ENRICHMENTS_PAGE, "Enrichments",TRUE},
{ "MegaSquirt VE Table(s)", VETABLES_PAGE, "VE Table(s)",TRUE},
{ "MegaSquirt Ignition Settings", IGNITION_PAGE, "Ignition Settings",FALSE},
{ "MegaSquirt Runtime Display", RUNTIME_PAGE, "Runtime Disp.",TRUE},
{ "MegaSquirt Tuning", TUNING_PAGE, "Tuning",TRUE},
{ "MegaSquirt Tools", TOOLS_PAGE, "Tools",TRUE},
{ "MegaSquirt Advanced Diagnostics", LOWLEVEL_PAGE, "Low-Level",TRUE},
{ "MegaSquirt DataLogging", DATALOGGING_PAGE, "DataLogging",TRUE},
{ "MegaSquirt Warmup Wizard", WARMWIZARD_PAGE, "Warmup Wizard",TRUE}
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
	gtk_window_set_title(GTK_WINDOW(main_window),"MegaTunix "VERSION);
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

		framebuild_dispatch(frame,notebook_tabs[i].page, notebook_tabs[i].enabled);

		label = gtk_label_new (notebook_tabs[i].tab_name);
		if (notebook_tabs[i].enabled == FALSE)
			gtk_widget_set_sensitive(GTK_WIDGET(label),FALSE);
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

int framebuild_dispatch(GtkWidget *frame, GuiFramePage data, gboolean frame_enabled)
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
		case IGNITION_PAGE:
			build_ignition(frame);
			break;
		case WARMWIZARD_PAGE:
			build_warmwizard(frame);
			break;
	}
	if (frame_enabled == FALSE)
		gtk_widget_set_sensitive(frame,FALSE);
	return TRUE;
}
