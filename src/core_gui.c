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
#include <core_gui.h>
#include <defines.h>
#include <enums.h>
#include <general_gui.h>
#include <gui_handlers.h>
#include <tuning_gui.h>



/* Default window size and MINIMUM size as well... */
static gint def_width=620;
static gint def_height=525;
gint width = 0;
gint height = 0;
gint main_x_origin = 0;
gint main_y_origin = 0;
extern gboolean tips_in_use;
GtkWidget *main_window = NULL;
GtkTooltips *tip = NULL;
GtkWidget *notebook = NULL;
static struct 
{
	gchar *frame_name;	/* Textual name at the top of the frame */
	void (*Function) (GtkWidget *);	/* builder function */
	gchar *tab_name;	/* The Tab textual name for the main gui */
	PageIdent page_ident;	/* Page Identifier... */
} notebook_tabs[] = { 
{ "About MegaTunix", build_about, "_About",ABOUT_PAGE},
{ "General MegaTunix Settings", build_general, "_General",GENERAL_PAGE},
{ "MegaSquirt Communications Parameters", build_comms, "_Communications",COMMS_PAGE},
//{ "MegaSquirt Tuning", build_tuning, "_Tuning",STANDARD,TUNING_PAGE},
};

static int num_tabs = sizeof(notebook_tabs) / sizeof(notebook_tabs[0]);

int setup_gui()
{
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *hbox;
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
		gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

		notebook_tabs[i].Function(frame);

		label = gtk_label_new_with_mnemonic (notebook_tabs[i].tab_name);

		g_object_set_data(G_OBJECT(frame),"page_ident",GINT_TO_POINTER(notebook_tabs[i].page_ident));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	}
	g_signal_connect(G_OBJECT(notebook),"switch-page",
			G_CALLBACK(page_changed),NULL);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	button = gtk_button_new_with_label("Exit");
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,20);
	g_signal_connect(G_OBJECT(button),"pressed",
			G_CALLBACK(leave),NULL);

	if(tips_in_use)
		gtk_tooltips_enable(tip);
	else
		gtk_tooltips_disable(tip);

	gtk_widget_show_all(main_window);

	return TRUE;
}
