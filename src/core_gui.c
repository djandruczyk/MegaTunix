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
#include <dt_params_gui.h>
#include <eng_vitals_gui.h>
#include <enrichments_gui.h>
#include <enums.h>
#include <general_gui.h>
#include <gui_handlers.h>
#include <ignition_gui.h>
#include <logviewer_gui.h>
#include <lowlevel_gui.h>
#include <runtime_gui.h>
#include <tuning_gui.h>
#include <tools_gui.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>



/* Default window size and MINIMUM size as well... */
static int def_width=700;
static int def_height=535;
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
	void (*Function) (GtkWidget *);	/* builder function */
	gchar *tab_name;	/* The Tab textual name for the main gui */
	gboolean enabled;	/* Is the tab enabled (sensitive) or not? */
	gboolean dt_specific;	/* Dualtable ONLY page */
} notebook_tabs[] = { 
{ "About MegaTunix", build_about, "_About",TRUE, FALSE},
{ "General MegaTunix Settings", build_general, "_General",TRUE, FALSE},
{ "MegaSquirt Communications Parameters", build_comms, "_Communications",TRUE, FALSE},
{ "MegaSquirt Vital Settings", build_eng_vitals, "E_ngine Vitals",TRUE, FALSE},
{ "MegaSquirt Constants", build_constants_1, "Table_1 Constants",TRUE, FALSE},
{ "MegaSquirt Constants", build_constants_2, "Table_2 Constants",TRUE, TRUE},
{ "MegaSquirt DualTable Parameters", build_dt_params, "_DT Options",TRUE, TRUE},
{ "MegaSquirt Enrichments", build_enrichments, "_Enrichments",TRUE, FALSE},
{ "MegaSquirt VE Table(1)", build_vetable_1, "_VE Table(1)",TRUE, FALSE},
{ "MegaSquirt VE Table(2)", build_vetable_2, "_VE Table(2)",TRUE, TRUE},
{ "MegaSquirt Ignition Settings", build_ignition, "_Ignition Settings",FALSE, FALSE},
{ "MegaSquirt Runtime Display", build_runtime, "_Runtime Disp.",TRUE, FALSE},
{ "MegaSquirt Tuning", build_tuning, "_Tuning",TRUE, FALSE},
{ "MegaSquirt Tools", build_tools, "T_ools",TRUE, FALSE},
{ "MegaSquirt Advanced Diagnostics", build_lowlevel, "_Low-Level",FALSE, FALSE},
{ "MegaSquirt Warmup Wizard", build_warmwizard, "_Warmup Wizard",TRUE, FALSE},
{ "MegaSquirt DataLogging", build_datalogging, "_DataLogging",TRUE, FALSE},
{ "MegaSquirt DataLog Viewer", build_logviewer, "Log _Playback",TRUE, FALSE},
};

static int num_tabs = sizeof(notebook_tabs) / sizeof(notebook_tabs[0]);

int setup_gui()
{
	GtkWidget *frame;
	GtkWidget *notebook;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *button;
	extern GList *dt_widgets;
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

		notebook_tabs[i].Function(frame);

		label = gtk_label_new_with_mnemonic (notebook_tabs[i].tab_name);

		if (notebook_tabs[i].dt_specific == TRUE)
		{
			dt_widgets = g_list_append(dt_widgets, 
					(gpointer)frame);
			dt_widgets = g_list_append(dt_widgets, 
					(gpointer)label);
		}

		if (notebook_tabs[i].enabled == FALSE)
		{
			gtk_widget_set_sensitive(frame,FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(label),FALSE);
		}
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
