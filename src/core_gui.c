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
#include <enums.h>
#include <general_gui.h>
#include <gui_handlers.h>
#include <ignition_gui.h>
#include <logviewer_gui.h>
#include <memory_gui.h>
#include <runtime_gui.h>
#include <tuning_gui.h>
#include <tools_gui.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>



/* Default window size and MINIMUM size as well... */
static int def_width=700;
static int def_height=532;
int width;
int height;
int main_x_origin;
int main_y_origin;
extern gint tips_in_use;
GtkWidget *main_window;
GtkTooltips *tip;
GtkWidget *notebook;
static struct 
{
	gchar *frame_name;	/* Textual name at the top of the frame */
	void (*Function) (GtkWidget *);	/* builder function */
	gchar *tab_name;	/* The Tab textual name for the main gui */
	Capabilities capabilities;	/* What does it do */
	PageIdent page_ident;	/* Page Identifier... */
} notebook_tabs[] = { 
{ "About MegaTunix", build_about, "_About",STANDARD,ABOUT_PAGE},
{ "General MegaTunix Settings", build_general, "_General",STANDARD,GENERAL_PAGE},
{ "MegaSquirt Communications Parameters", build_comms, "Co_mmunications",STANDARD,COMMS_PAGE},
{ "MegaSquirt Vital Settings", build_eng_vitals, "E_ngine Vitals",STANDARD,ENG_VITALS_PAGE},
{ "MegaSquirt Constants", build_constants_1, "ECU _Constants",STANDARD,CONSTANTS_PAGE},
//{ "MegaSquirt DualTable Parameters", build_dt_params, "_DT Options",DUALTABLE,DT_PARAMS_PAGE},
{ "MegaSquirt Ignition Parameters", build_ignition, "_Ignition Settings",S_N_SPARK | S_N_EDIS,IGNITON_PAGE},
{ "MegaSquirt Runtime Display", build_runtime, "_Runtime Disp.",STANDARD,RUNTIME_PAGE},
{ "MegaSquirt Tuning", build_tuning, "_Tuning",STANDARD,TUNING_PAGE},
{ "MegaSquirt Tools", build_tools, "T_ools",STANDARD,TOOLS_PAGE},
{ "MegaSquirt Raw Memory Viewer", build_memory, "_Memory Viewer",RAW_MEMORY,RAW_MEM_PAGE},
{ "MegaSquirt Warmup Wizard", build_warmwizard, "_Warmup Wizard",STANDARD,WARMUP_WIZ_PAGE},
{ "MegaSquirt DataLogging", build_datalogging, "_DataLogging",STANDARD,DATALOGGING_PAGE},
{ "MegaSquirt Visual Log Viewer", build_logviewer, "Log View/_Playback",STANDARD,LOGVIEWER_PAGE},
};

static int num_tabs = sizeof(notebook_tabs) / sizeof(notebook_tabs[0]);

int setup_gui()
{
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *button;
	extern GList *dt_controls;
	extern GList *raw_mem_controls;
	extern GList *ign_controls;
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

		if (notebook_tabs[i].capabilities & RAW_MEMORY)
		{
			raw_mem_controls = g_list_append(raw_mem_controls, 
					(gpointer)frame);
			raw_mem_controls = g_list_append(raw_mem_controls, 
					(gpointer)label);
		}
		if (notebook_tabs[i].capabilities & DUALTABLE)
		{
			dt_controls = g_list_append(dt_controls, 
					(gpointer)frame);
			dt_controls = g_list_append(dt_controls, 
					(gpointer)label);
		}
		if (notebook_tabs[i].capabilities & (S_N_SPARK|S_N_EDIS))
		{
			ign_controls = g_list_append(ign_controls, 
					(gpointer)frame);
			ign_controls = g_list_append(ign_controls, 
					(gpointer)label);
		}
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
