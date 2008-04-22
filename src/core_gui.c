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
#include <args.h>
#include <comms_gui.h>
#include <config.h>
#include <configfile.h>
#include <core_gui.h>
#include <dashboard.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <getfiles.h>
#include <logo.h>
#include <menu_handlers.h>
#include <serialio.h>
#include <stdlib.h>
#include <string.h>
#include <tabloader.h>
#include <tuning_gui.h>
#include <widgetmgmt.h>



/* Default window size and MINIMUM size as well... */
static gint def_width=640;
static gint def_height=480;
gint width = 0;
gint height = 0;
GtkWidget *main_window = NULL;
GtkTooltips *tip = NULL;
extern GObject *global_data;


/*!
 \brief setup_gui() creates the main window, main notebook, and the static
 tabs and populates them with data
 */
int setup_gui()
{
	gchar *fname = NULL;
	gchar *filename = NULL;
	GtkWidget *window = NULL;
	GtkWidget *top_box = NULL;
	GtkWidget *child = NULL;
	GtkWidget *label = NULL;
	GtkWidget *notebook = NULL;
	gint i = 0;
	GladeXML *xml = NULL;
	gint tabcount = 0;
	gboolean *hidden_list;
	gboolean tips_in_use;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	CmdLineArgs *args = OBJ_GET(global_data,"args");

	fname = g_build_filename(GUI_DATA_DIR,"main.glade",NULL);
	filename = get_file(g_strdup(fname),NULL);
	if (!filename)
	{
		printf("ERROR!  Could locate %s\n",fname);
		g_free(fname);
		printf("MegaTunix does NOT seem to be installed correctly, make sure\n\"make install\" has been run by root from the top level source directory...\n");
		exit(-1);
	}
	else
		xml = glade_xml_new(filename, "mtx_top_vbox",NULL);
	g_free(fname);
	g_free(filename);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(leave),NULL);
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(leave),NULL);
	main_window = window;
	gtk_window_set_focus_on_map((GtkWindow *)window,FALSE);
	top_box = glade_xml_get_widget(xml,"mtx_top_vbox");
	gtk_container_add(GTK_CONTAINER(window),top_box);

	glade_xml_signal_autoconnect(xml);
	OBJ_SET(global_data,"main_xml",xml);

	tip = gtk_tooltips_new();
	x = (gint)OBJ_GET(global_data,"main_x_origin");
	y = (gint)OBJ_GET(global_data,"main_y_origin");
	w = (gint)OBJ_GET(global_data,"width");
	h = (gint)OBJ_GET(global_data,"height");
	gtk_window_move((GtkWindow *)main_window, x, y);
	gtk_widget_set_size_request(main_window,def_width,def_height);
	gtk_window_resize(GTK_WINDOW(main_window),w,h);
	gtk_window_set_title(GTK_WINDOW(main_window),"MegaTunix "VERSION);
	finalize_core_gui(xml);

	tips_in_use = (gboolean)OBJ_GET(global_data,"tips_in_use");
	if(tips_in_use)
		gtk_tooltips_enable(tip);
	else
		gtk_tooltips_disable(tip);

	if (!args->hide_maingui)
		gtk_widget_show_all(main_window);

	/* Tabs that should be hidden.... */
	notebook = glade_xml_get_widget(xml,"toplevel_notebook");
	tabcount = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	hidden_list = (gboolean *)OBJ_GET(global_data,"hidden_list");
	for (i=0;i<tabcount;i++)
	{
		if(hidden_list[i] == TRUE)
		{
			/* Get tab and child label and hide it.. */
			child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
			label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook),child);
			gtk_widget_hide(child);
			gtk_widget_hide(label);
		}
	}

	return TRUE;
}


void finalize_core_gui(GladeXML * xml)
{
	/* Initializes base gui and installs things like the logo and
	 * other dynamic bits that can't be set via glade statically 
	 */
	GtkTextBuffer * textbuffer = NULL;
	GtkWidget *alignment = NULL;
	GtkWidget *button = NULL;
	GtkWidget *cbutton = NULL;
	GtkWidget *ebox = NULL;
	GtkWidget *label = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *image = NULL;
	GdkPixbuf *pixbuf = NULL;
	gchar * tmpbuf = NULL;
	gint temp_units;
	extern Serial_Params *serial_params;

	temp_units = (gint)OBJ_GET(global_data,"temp_units");
	widget = glade_xml_get_widget(xml,"toplevel_notebook");
	register_widget("toplevel_notebook",widget);
	/* Set about tab title */
	label = glade_xml_get_widget(xml,"about_title_label");
	tmpbuf = g_strdup_printf("MegaTunix %s Tuning Software for Unix-class OS's",VERSION);
	gtk_label_set_text(GTK_LABEL(label),tmpbuf);
	g_free(tmpbuf);

	/* Info status label at base of UI */
	widget = glade_xml_get_widget(xml,"info_label");
	register_widget("info_label",widget);

	/* Load Main MegaTunix logo */
	alignment = glade_xml_get_widget(xml,"logo_alignment");
	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add (GTK_CONTAINER (alignment), image);

	/* Set about tab identifier */
	OBJ_SET(glade_xml_get_widget(xml,"about_frame"),"tab_ident",GINT_TO_POINTER(ABOUT_TAB));

	/* Tab visibility menuitem */
	widget = glade_xml_get_widget(xml,"show_tab_visibility_menuitem");
	register_widget("show_tab_visibility_menuitem",widget);

	/* General Tab, Tooltips */
	button = glade_xml_get_widget(xml,"tooltips_cbutton");
	OBJ_SET(button,"handler",GINT_TO_POINTER(TOOLTIPS_STATE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),(gboolean)OBJ_GET(global_data,"tips_in_use"));

	/* General Tab, Temp Scales */
	button = glade_xml_get_widget(xml,"fahrenheit_rbutton");
	OBJ_SET(button,"handler",GINT_TO_POINTER(FAHRENHEIT));
	if (temp_units == FAHRENHEIT)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	button = glade_xml_get_widget(xml,"celsius_rbutton");
	OBJ_SET(button,"handler",GINT_TO_POINTER(CELSIUS));
	if (temp_units == CELSIUS)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);

	/* General Tab, Dashboard Ebox */
	ebox = glade_xml_get_widget(xml,"dash_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This box provides your choice for the active dashboard to be used",NULL);

	/* General Tab, Dashboard 1 */
	button = glade_xml_get_widget(xml,"dash1_choice_button");
	cbutton = glade_xml_get_widget(xml,"dash1_cbutton");
	g_signal_connect(G_OBJECT(cbutton),"toggled",G_CALLBACK(remove_dashboard),GINT_TO_POINTER(1));
	tmpbuf = (gchar *)OBJ_GET(global_data,"dash_1_name");
	if ((tmpbuf) && (strlen(tmpbuf) != 0))
		gtk_button_set_label(GTK_BUTTON(button),tmpbuf);
	else
		gtk_button_set_label(GTK_BUTTON(button),"Choose a Dashboard File");

	OBJ_SET(button,"label",gtk_bin_get_child(GTK_BIN(button)));
	OBJ_SET(cbutton,"label",gtk_bin_get_child(GTK_BIN(button)));
	label  = gtk_bin_get_child(GTK_BIN(button));
#if GTK_MINOR_VERSION >= 6
	if (gtk_minor_version >= 6)
		gtk_label_set_ellipsize(GTK_LABEL(label),PANGO_ELLIPSIZE_MIDDLE);
#endif
	register_widget("dash_1_label",label);
	/* Bind signal to the button to choose a new dash */
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(present_dash_filechooser),
			GINT_TO_POINTER(1));

	/* General Tab, Dashboard 2 */
	button = glade_xml_get_widget(xml,"dash2_choice_button");
	cbutton = glade_xml_get_widget(xml,"dash2_cbutton");
	g_signal_connect(G_OBJECT(cbutton),"toggled",G_CALLBACK(remove_dashboard),GINT_TO_POINTER(2));
	tmpbuf = (gchar *)OBJ_GET(global_data,"dash_2_name");
	if ((tmpbuf) && (strlen(tmpbuf) != 0))
		gtk_button_set_label(GTK_BUTTON(button),tmpbuf);
	else
		gtk_button_set_label(GTK_BUTTON(button),"Choose a Dashboard File");
	OBJ_SET(button,"label",gtk_bin_get_child(GTK_BIN(button)));
	OBJ_SET(cbutton,"label",gtk_bin_get_child(GTK_BIN(button)));
	label  = gtk_bin_get_child(GTK_BIN(button));
#if GTK_MINOR_VERSION >= 6
	if (gtk_minor_version >= 6)
		gtk_label_set_ellipsize(GTK_LABEL(label),PANGO_ELLIPSIZE_MIDDLE);
#endif
	register_widget("dash_2_label",label);
	/* Bind signal to the button to choose a new dash */
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(present_dash_filechooser),
			GINT_TO_POINTER(2));

	/* General Tab, Debugging frame */
	ebox = glade_xml_get_widget(xml,"debugging_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This box gives you the debugging choices.  Each one is independantly selectable.  Logging output will be written to MTXlog.txt file in your homedir, or in C:\\program files\\megatunix on Win32 platforms...",NULL);
	widget = glade_xml_get_widget(xml,"debugging_frame");
	populate_debugging(widget);

	/* General Tab Interrogation frame */
	ebox = glade_xml_get_widget(xml,"ecu_info_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This box shows you the MegaSquirt Interrogation report.  Due to the rise of various MegaSquirt variants, several of them unfortunately return the same version number except that their API's aren't compatible.  This window give you some feedback about how the MS responds to various commands and suggests what it thinks is the closest match.",NULL);

	/* General Tab Interrogation button */
	ebox = glade_xml_get_widget(xml,"interrogate_button_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This button interrogates the connected ECU to attempt to determine what firmware is loaded and to setup the gui to adapt to the capabilities of the loaded version. This method is not 100\% foolproof, but it works about 99.5\% of the time.  If it MIS-detects your ECU contact the developer with your firmware details.",NULL);
	button = glade_xml_get_widget(xml,"interrogate_button");
	register_widget("interrogate_button",button);
	OBJ_SET(button,"handler",GINT_TO_POINTER(INTERROGATE_ECU));

	/* General Tab OFfline mode button */
	ebox = glade_xml_get_widget(xml,"offline_mode_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This button Enables \"Offline Mode\" so that you can load tabs specific to an ECU and set settings, modify maps without doing any Serial I/O. This will allow you to modify maps offline when not connected to the vehicle/ECU.",NULL);
	button = glade_xml_get_widget(xml,"offline_button");
	register_widget("offline_button",button);
	OBJ_SET(button,"handler",GINT_TO_POINTER(OFFLINE_MODE));

	/* Interrogation results entries */
	widget = glade_xml_get_widget(xml,"ecu_revision_entry");
	register_widget("ecu_revision_entry",widget);
	widget = glade_xml_get_widget(xml,"text_version_entry");
	register_widget("text_version_entry",widget);
	widget = glade_xml_get_widget(xml,"ecu_signature_entry");
	register_widget("ecu_signature_entry",widget);

	/* General Tab Textview */
	ebox = glade_xml_get_widget(xml,"interrogation_status_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This window shows the status of the ECU interrogation progress.  The way it works is that we send commands to the ECU and count how much data is returned, which helps us hone in to which firmware for the MS is in use.  This method is not 100\% foolproof, as some firmware editions return the same amount of data, AND the same version number making them indistinguishable from the outside interface.  The commands sent are:\n \"R\", which returns the extended runtime variables (only supported by a subset of firmwares, like MSnS-Extra \n \"A\" which returns the runtime variables (22 bytes usually)\n \"C\" which should return the MS clock (1 byte,  but this call fails on the (very old) version 1 MS's)\n \"Q\" Which should return the version number of the firmware multipled by 10\n \"V\" which should return the VEtable and constants, this size varies based on the firmware\n \"S\" which is a \"Signature Echo\" used in some of the variants.  Similar to the \"T\" command (Text version)\n \"I\" which returns the igntion table and related constants (ignition variants ONLY)\n The \"F0/1\" Commands return the raw memory of the MegaSquirt ECU (DT Firmwares only).",NULL);

	widget = glade_xml_get_widget(xml,"interr_view");
	register_widget("interr_view",widget);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);
	gtk_text_buffer_create_tag(textbuffer,
			"info",
			"foreground",
			"dark green", NULL);

	/* COMMS Tab Commport frame */
	ebox = glade_xml_get_widget(xml,"commport_ebox");
	gtk_tooltips_set_tip(tip,ebox,"Sets the comm port to use. Type in the device name of your serial connection (Typical values under Windows would be COM1, COM2, etc, Linux would be /dev/ttyS0 or /dev/ttyUSB0, under Mac OS-X with a USB/Serial adapter would be /dev/tty.usbserial0, and under FreeBSD /dev/cuaa0)",NULL);

	/* Active COMM Port entry */
	widget = glade_xml_get_widget(xml,"active_port_entry");
	register_widget("active_port_entry",widget);

	/* Autodetect Checkbutton */
	widget = glade_xml_get_widget(xml,"serial_autodetect_cbutton");
	register_widget("serial_autodetect_cbutton",widget);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(COMM_AUTODETECT));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),(gboolean)OBJ_GET(global_data,"autodetect_port"));

	/* Fill in comm port entry if in manual mode */
	if (!(gboolean)OBJ_GET(global_data,"autodetect_port"))
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml,"active_port_entry")),OBJ_GET(global_data,"override_port"));

	/* COMMS Tab Read delay subtable */
	ebox = glade_xml_get_widget(xml,"read_delay_ebox");
	gtk_tooltips_set_tip(tip,ebox,"Sets the time delay between read attempts for getting the RealTime variables from the ECU, typically should be set around 50 for about 12-18 reads per second from the ECU. Lower values will update things faster but wll use more CPU resources.  This will control the rate at which the Runtime Display page updates.",NULL);
	widget = glade_xml_get_widget(xml,"read_delay_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),serial_params->read_wait);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(SER_INTERVAL_DELAY));

	/* COMMS Tab Start/Stop RT buttons */
	button = glade_xml_get_widget(xml,"start_rt_button");
	register_widget("comms_start_rt_button",button);
	OBJ_SET(button,"handler",GINT_TO_POINTER(START_REALTIME));
	button = glade_xml_get_widget(xml,"stop_rt_button");
	register_widget("comms_stop_rt_button",button);
	OBJ_SET(button,"handler",GINT_TO_POINTER(STOP_REALTIME));

	/* COMMS Tab Stats Frame */
	ebox = glade_xml_get_widget(xml,"ms_stats_ebox");
	gtk_tooltips_set_tip(tip,ebox,"This block shows you statistics on the number of good reads of the VE/Constants datablocks, RealTime datablocks and the MegaSquirt hard reset and Serial I/O error counts.  Hard resets are indicative of power problems or excessive electrical noise to the MS (causing cpu resets).  Serial I/O errors are indicative of a poor cable or wireless connection between this host computer and the MS.",NULL);

	/* COMMS Tab Stats Entries */
	widget = glade_xml_get_widget(xml,"comms_vecount_entry");
	register_widget("comms_vecount_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_rtcount_entry");
	register_widget("comms_rtcount_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_sioerr_entry");
	register_widget("comms_sioerr_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_reset_entry");
	register_widget("comms_reset_entry",widget);

	widget = glade_xml_get_widget(xml,"serial_status_view");
	register_widget("comms_view",widget);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
	gtk_text_buffer_create_tag(textbuffer,
			"warning",
			"foreground",
			"red", NULL);
	gtk_text_buffer_create_tag(textbuffer,
			"info",
			"foreground",
			"dark green", NULL);
}
