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
#include <defines.h>
#include <about_gui.h>
#include <args.h>
#include <comms_gui.h>
#include <configfile.h>
#include <core_gui.h>
#include <dashboard.h>
#include <debugging.h>
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
#include <widgetmgmt.h>



/* Default window size and MINIMUM size as well... */
static gint def_width=640;
static gint def_height=400;
gint width = 0;
gint height = 0;
extern gconstpointer *global_data;


/*!
 \brief setup_gui() creates the main window, main notebook, and the static
 tabs and populates them with data
 */
int setup_gui(void)
{
	gchar *fname = NULL;
	gchar *filename = NULL;
	GtkWidget *window = NULL;
	GtkWidget *top_box = NULL;
	GtkWidget *child = NULL;
	GtkWidget *label = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *dialog = NULL;
	gint i = 0;
	GladeXML *xml = NULL;
	gint tabcount = 0;
	gboolean *hidden_list;
	gboolean tips_in_use;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	GtkSettings *settings = gtk_settings_get_default();
	CmdLineArgs *args = DATA_GET(global_data,"args");

#ifdef __WIN32__
	fname = g_build_filename(GUI_DATA_DIR,"main_win32.glade",NULL);
#else
	fname = g_build_filename(GUI_DATA_DIR,"main.glade",NULL);
#endif
	filename = get_file(g_strdup(fname),NULL);
	if (!filename)
	{
		printf(_("ERROR! Could NOT locate file %s.\n - Did you forget to run \"sudo make install\" ?\n"),fname);
		g_free(fname);
		dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,_("\n<b>MegaTunix</b> doesn't appear to be installed correctly!\n\nDid you forget to run <i>\"sudo make install\"</i> ??\n\n"));

		g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"delete_event", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"destroy_event", G_CALLBACK(gtk_main_quit), dialog);
		gtk_widget_show_all(dialog);
		gtk_main();
		if (global_data)
		{
			g_dataset_destroy(global_data);
			g_free(global_data);
		}
		exit(-1);
	}
	else
		xml = glade_xml_new(filename, "mtx_top_vbox",NULL);
	g_free(fname);
	g_free(filename);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	register_widget("main_window",window);
	DATA_SET(global_data,"font_size",GINT_TO_POINTER(PANGO_PIXELS(pango_font_description_get_size(GTK_WIDGET(window)->style->font_desc))));
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(leave),NULL);
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(leave),NULL);
	gtk_window_set_focus_on_map((GtkWindow *)window,FALSE);
	top_box = glade_xml_get_widget(xml,"mtx_top_vbox");
	gtk_container_add(GTK_CONTAINER(window),top_box);

	glade_xml_signal_autoconnect(xml);
	DATA_SET_FULL(global_data,"main_xml",xml,g_object_unref);

	x = (GINT)DATA_GET(global_data,"main_x_origin");
	y = (GINT)DATA_GET(global_data,"main_y_origin");
	w = (GINT)DATA_GET(global_data,"width");
	h = (GINT)DATA_GET(global_data,"height");
	tips_in_use = (GBOOLEAN)DATA_GET(global_data,"tips_in_use");
	if (gtk_minor_version >= 14)
	{
		if (tips_in_use)
			g_object_set(settings,"gtk-enable-tooltips",TRUE,NULL);
		else
			g_object_set(settings,"gtk-enable-tooltips",FALSE,NULL);
	}
	gtk_window_move((GtkWindow *)window, x, y);
	gtk_widget_set_size_request(window,def_width,def_height);
	gtk_window_resize(GTK_WINDOW(window),w,h);
	gtk_window_set_title(GTK_WINDOW(window),"MegaTunix "VERSION);
	finalize_core_gui(xml);

	if (!args->hide_maingui)
		gtk_widget_show_all(window);

	/* Tabs that should be hidden.... */
	notebook = glade_xml_get_widget(xml,"toplevel_notebook");
	tabcount = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	hidden_list = (gboolean *)DATA_GET(global_data,"hidden_list");
	for (i=0;i<tabcount;i++)
	{
		child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i);
		label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook),child);
		gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
		gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook),gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),i),TRUE);
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
	CmdLineArgs *args = DATA_GET(global_data,"args");

	temp_units = (GINT)DATA_GET(global_data,"temp_units");
	widget = glade_xml_get_widget(xml,"toplevel_notebook");
	register_widget("toplevel_notebook",widget);
	/* Set about tab title */
	label = glade_xml_get_widget(xml,"about_title_label");
	tmpbuf = g_strdup_printf(_("MegaTunix %s Tuning Software for Unix-class OS's"),VERSION);
	gtk_label_set_text(GTK_LABEL(label),tmpbuf);
	g_free(tmpbuf);

	/* Info status label at base of UI */
	widget = glade_xml_get_widget(xml,"info_label");
	register_widget("info_label",widget);

	/* Load Main MegaTunix logo */
	alignment = glade_xml_get_widget(xml,"logo_alignment");
	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	gtk_container_add (GTK_CONTAINER (alignment), image);

	/* Set about tab identifier */
	OBJ_SET(glade_xml_get_widget(xml,"about_frame"),"tab_ident",GINT_TO_POINTER(ABOUT_TAB));

	/* Tab visibility menuitem */
	widget = glade_xml_get_widget(xml,"show_tab_visibility_menuitem");
	register_widget("show_tab_visibility_menuitem",widget);

	/* General Tab, Tooltips */
	button = glade_xml_get_widget(xml,"tooltips_cbutton");
	OBJ_SET(button,"handler",GINT_TO_POINTER(TOOLTIPS_STATE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),(GBOOLEAN)DATA_GET(global_data,"tips_in_use"));

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
	gtk_widget_set_tooltip_text(ebox,"This box provides your choice for the active dashboard to be used");

	/* General Tab, Dashboard 1 */
	button = glade_xml_get_widget(xml,"dash1_choice_button");
	cbutton = glade_xml_get_widget(xml,"dash1_cbutton");
	register_widget("dash1_cbutton",cbutton);
	g_signal_connect(G_OBJECT(cbutton),"clicked",G_CALLBACK(remove_dashboard),GINT_TO_POINTER(1));
	tmpbuf = (gchar *)DATA_GET(global_data,"dash_1_name");
	if ((tmpbuf) && (strlen(tmpbuf) != 0))
	{
		gtk_button_set_label(GTK_BUTTON(button),tmpbuf);
		gtk_widget_set_sensitive(GTK_WIDGET(cbutton),TRUE);
	}
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
	register_widget("dash2_cbutton",cbutton);
	g_signal_connect(G_OBJECT(cbutton),"clicked",G_CALLBACK(remove_dashboard),GINT_TO_POINTER(2));
	tmpbuf = (gchar *)DATA_GET(global_data,"dash_2_name");
	if ((tmpbuf) && (strlen(tmpbuf) != 0))
	{
		gtk_button_set_label(GTK_BUTTON(button),tmpbuf);
		gtk_widget_set_sensitive(GTK_WIDGET(cbutton),TRUE);
	}
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
	gtk_widget_set_tooltip_text(ebox,"This box gives you the debugging choices.  Each one is independantly selectable.  Logging output will be written to MTXlog.txt file in your homedir, or in C:\\program files\\megatunix on Win32 platforms...");
	widget = glade_xml_get_widget(xml,"debugging_frame");
	populate_debugging(widget);

	/* General Tab Interrogation frame */
	ebox = glade_xml_get_widget(xml,"ecu_info_ebox");
	gtk_widget_set_tooltip_text(ebox,"This box shows you the MegaSquirt Interrogation report.  Due to the rise of various MegaSquirt variants, several of them unfortunately return the same version number except that their API's aren't compatible.  This window give you some feedback about how the MS responds to various commands and suggests what it thinks is the closest match.");

	/* General Tab Interrogation button */
	ebox = glade_xml_get_widget(xml,"interrogate_button_ebox");
	gtk_widget_set_tooltip_text(ebox,"This button interrogates the connected ECU to attempt to determine what firmware is loaded and to setup the gui to adapt to the capabilities of the loaded version. This method is not 100% foolproof, but it works about 99.5% of the time.  If it MIS-detects your ECU contact the developer with your firmware details.");
	button = glade_xml_get_widget(xml,"interrogate_button");
	register_widget("interrogate_button",button);
	OBJ_SET(button,"handler",GINT_TO_POINTER(INTERROGATE_ECU));

	/* General Tab OFfline mode button */
	ebox = glade_xml_get_widget(xml,"offline_mode_ebox");
	gtk_widget_set_tooltip_text(ebox,"This button Enables \"Offline Mode\" so that you can load tabs specific to an ECU and set settings, modify maps without doing any Serial I/O. This will allow you to modify maps offline when not connected to the vehicle/ECU.");
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
	gtk_widget_set_tooltip_text(ebox,"This window shows the status of the ECU interrogation progress.  The way it works is that we send commands to the ECU and count how much data is returned, which helps us hone in to which firmware for the MS is in use.  This method is not 100% foolproof, as some firmware editions return the same amount of data, AND the same version number making them indistinguishable from the outside interface.  The commands sent are:\n \"R\", which returns the extended runtime variables (only supported by a subset of firmwares, like MSnS-Extra \n \"A\" which returns the runtime variables (22 bytes usually)\n \"C\" which should return the MS clock (1 byte,  but this call fails on the (very old) version 1 MS's)\n \"Q\" Which should return the version number of the firmware multipled by 10\n \"V\" which should return the VEtable and constants, this size varies based on the firmware\n \"S\" which is a \"Signature Echo\" used in some of the variants.  Similar to the \"T\" command (Text version)\n \"I\" which returns the igntion table and related constants (ignition variants ONLY)\n The \"F0/1\" Commands return the raw memory of the MegaSquirt ECU (DT Firmwares only).");

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
	gtk_widget_set_tooltip_text(ebox,"These controls set parameters specific to Serial/Network communication.  The read timeout should be set to 100 ms for serial and low latency network links. Increase this to 300-500 for slower links over long distances.  Since megatunix 0.9.18 serial port setup is dynamic for Linux and Windows,  OS-X users may need to disable auto-scanning and manually type in the device name (/dev/cu...) Type in the device name of your serial connection (Typical values under Windows would be COM1, COM2, etc, Linux would be /dev/ttyS0 or /dev/ttyUSB0, under Mac OS-X with a USB/Serial adapter would be /dev/tty.usbserial0, and under FreeBSD /dev/cuaa0)");

	/* Locate Port button */
#ifdef __WIN32__
	widget = glade_xml_get_widget(xml,"locate_port_button");
	gtk_widget_set_sensitive (widget,FALSE);
#endif
	
	/* Read Timeout threshold spinner */
	widget = glade_xml_get_widget(xml,"read_timeout_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(GINT)DATA_GET(global_data,"read_timeout"));

	OBJ_SET(widget,"handler",GINT_TO_POINTER(SER_READ_TIMEOUT));
	
	/* Active COMM Port entry */
	widget = glade_xml_get_widget(xml,"active_port_entry");
	register_widget("active_port_entry",widget);

	/* Autodetect Checkbutton */
	widget = glade_xml_get_widget(xml,"serial_autodetect_cbutton");
	register_widget("serial_autodetect_cbutton",widget);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(COMM_AUTODETECT));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),(GBOOLEAN)DATA_GET(global_data,"autodetect_port"));

	/* Fill in comm port entry if in manual mode */
	if (!(GBOOLEAN)DATA_GET(global_data,"autodetect_port"))
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml,"active_port_entry")),DATA_GET(global_data,"override_port"));

	/* COMMS Tab Read delay subtable */
	ebox = glade_xml_get_widget(xml,"rates_ebox");
	gtk_widget_set_tooltip_text(ebox,"These controls set the polling rate of the serial port (i.e. every 30 ms), as well as the update rates for the runtime text, runtime sliders, and dashboards.  The Datalogging always happens at the raw serial polling rate.  This allows you to reduce the update rate of other things that are less relevant and conserver CPU resources for slower systems.");

	widget = glade_xml_get_widget(xml,"read_wait_spin");
	register_widget("read_wait_spin",widget);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),serial_params->read_wait);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(SER_INTERVAL_DELAY));

	widget = glade_xml_get_widget(xml,"rtslider_fps_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(GINT)DATA_GET(global_data,"rtslider_fps"));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(RTSLIDER_FPS));

	widget = glade_xml_get_widget(xml,"rttext_fps_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(GINT)DATA_GET(global_data,"rttext_fps"));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(RTTEXT_FPS));

	widget = glade_xml_get_widget(xml,"dashboard_fps_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(GINT)DATA_GET(global_data,"dashboard_fps"));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(DASHBOARD_FPS));

	widget = glade_xml_get_widget(xml,"ve3d_fps_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(GINT)DATA_GET(global_data,"ve3d_fps"));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(VE3D_FPS));

	/* COMMS Tab Network ctrls */
	button = glade_xml_get_widget(xml,"reverse_connect_button");
	register_widget("reverse_connect_button",button);
	OBJ_SET(button,"handler",GINT_TO_POINTER(PHONE_HOME));

	widget = glade_xml_get_widget(xml,"reverse_connect_host_entry");
	register_widget("reverse_connect_host_entry",widget);

	button = glade_xml_get_widget(xml,"allow_net_checkbutton");
	register_widget("allow_net_checkbutton",button);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),(GBOOLEAN)DATA_GET(global_data,"network_access"));
	OBJ_SET(button,"handler",GINT_TO_POINTER(TOGGLE_NETMODE));

	widget = glade_xml_get_widget(xml,"netaccess_table");
	if (args->network_mode)
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);

	widget = glade_xml_get_widget(xml,"connected_clients_entry");
	register_widget("connected_clients_entry",widget);

	/* COMMS Tab Stats Frame */
	ebox = glade_xml_get_widget(xml,"ms_stats_ebox");
	gtk_widget_set_tooltip_text(ebox,"This block shows you statistics on the number of good reads of the VE/Constants datablocks, RealTime datablocks and the MegaSquirt hard reset and Serial I/O error counts.  Hard resets are indicative of power problems or excessive electrical noise to the MS (causing cpu resets).  Serial I/O errors are indicative of a poor cable or wireless connection between this host computer and the MS.");

	/* COMMS Tab Stats Entries */
	widget = glade_xml_get_widget(xml,"comms_vecount_entry");
	register_widget("comms_vecount_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_rtcount_entry");
	register_widget("comms_rtcount_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_sioerr_entry");
	register_widget("comms_sioerr_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_reset_entry");
	register_widget("comms_reset_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_pf_queue_entry");
	register_widget("comms_pf_queue_entry",widget);
	widget = glade_xml_get_widget(xml,"comms_gui_queue_entry");
	register_widget("comms_gui_queue_entry",widget);

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
