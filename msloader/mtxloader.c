#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <enums.h>
#include <fcntl.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <glib/gstring.h>
#include <glade/glade.h>
#include <loader.h>
#include <mtxloader.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


GladeXML *xml = NULL;
GtkWidget *main_window = NULL;

int main (int argc, char *argv[])
{
	GtkWidget *dialog = NULL;
	gchar * filename = NULL;
	gchar * fname = NULL;
	gtk_init (&argc, &argv);
	fname = g_build_filename(GUI_DATA_DIR,"mtxloader.glade",NULL);
	filename = get_file(g_strdup(fname),NULL);
	if (!filename)
	{
		printf("ERROR! Could NOT locate %s\n",fname);
		g_free(fname);
		dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"\n<b>MegaTunix/mtxloader</b> doesn't appear to be installed correctly!\n\nDid you forget to run <i>\"sudo make install\"</i> ??\n\n");

		g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"delete_event", G_CALLBACK(gtk_main_quit), dialog);
		g_signal_connect(G_OBJECT(dialog),"destroy_event", G_CALLBACK(gtk_main_quit), dialog);
		gtk_widget_show_all(dialog);
		gtk_main();
		exit(-1);
	}

	g_free(fname);
	xml = glade_xml_new (filename, "main_window", NULL);
	g_free(filename);
	main_window = glade_xml_get_widget (xml, "main_window");
	init_controls();
	glade_xml_signal_autoconnect (xml);
	load_defaults();
	gtk_widget_show_all (main_window);
	gtk_main ();

	return 0;
}

/* Parse a line of output, which may actually be more than one line. To handle
 * multiple lines, the string is split with g_strsplit(). */
void output (gchar *line)
{
	GtkWidget *textview;
	GtkTextBuffer * textbuffer = NULL;
	GtkTextIter iter;
	GtkWidget *parent = NULL;
	GtkAdjustment * adj = NULL;

	textview = glade_xml_get_widget (xml, "textview");
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	gtk_text_buffer_get_end_iter (textbuffer, &iter);
	gtk_text_buffer_insert(textbuffer,&iter,line,-1);
	parent = gtk_widget_get_parent(textview);
	if (parent != NULL)
	{
		adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
		adj->value = adj->upper;
	}
	while (gtk_events_pending())
		gtk_main_iteration();
}


/* Perform a ping operation when the Ping button is clicked. */
EXPORT gboolean get_signature (GtkButton *button)
{
	GtkWidget *widget =  NULL;
	gchar * port = NULL;
	gint port_fd = 0;

	widget = glade_xml_get_widget (xml, "port_entry");
	port = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	if (g_utf8_strlen(port, -1) == 0)
		return FALSE;
	/* If we got this far, all is good argument wise */
	port_fd = setup_port(port);
	if (!(port_fd  > 0))
	{
		output("Could NOT open Port, You should check perms.\n");
		return FALSE;
	}
	get_ecu_signature(port_fd);
	close_port(port_fd);
	return TRUE;
}


EXPORT gboolean load_firmware (GtkButton *button)
{
	GtkWidget *widget = NULL;
	gchar *port;
	gchar *filename = NULL;
	gint port_fd = 0;
	gint file_fd = 0;
	EcuState ecu_state = NOT_LISTENING;
	gint result = FALSE;

	widget = glade_xml_get_widget (xml, "port_entry");
	port = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	widget = glade_xml_get_widget (xml, "filechooser_button");
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

	if (g_utf8_strlen(port, -1) == 0)
		return FALSE;
	if (filename == NULL)
		return FALSE;

	/* If we got this far, all is good argument wise */
	port_fd = setup_port(port);
	if (port_fd > 0)
		output("Port successfully opened\n");
	else
	{
		output("Could NOT open Port check permissions\n");
		return FALSE;
	}
#ifdef __WIN32__
	file_fd = open(filename, O_RDWR | O_BINARY );
#else
	file_fd = g_open(filename,O_RDONLY,S_IRUSR);
#endif
	if (file_fd > 0 )
		output("Firmware file successfully opened\n");
	else
	{
		output("Could NOT open firmware file, check permissions/paths\n");
		return FALSE;
	}
	ecu_state = detect_ecu(port_fd);
	switch (ecu_state)
	{
		case NOT_LISTENING:
			output("NO response to signature request\n");
			break;
		case IN_BOOTLOADER:
			output("ECU is in bootloader mode\n");
			break;
		case LIVE_MODE:
			output("ECU detected in LIVE! mode, attempting to access bootloader\n");
			result = jump_to_bootloader(port_fd);
			if (result)
			{
				ecu_state = detect_ecu(port_fd);
				if (ecu_state == IN_BOOTLOADER)
				{
					output("ECU is in bootloader mode, good!\n");
					break;
				}
				else
					output("Could NOT attain bootloader mode\n");
			}
			else
				output("Could NOT attain bootloader mode\n");
			break;
	}
	if (ecu_state != IN_BOOTLOADER)
	{
		boot_jumper_prompt();
		ecu_state = detect_ecu(port_fd);
		if (ecu_state != IN_BOOTLOADER)
		{
			output("Unable to get to the bootloader, update FAILED!\n");
			return FALSE;
		}
		else
			output("Got into the bootloader, good!\n");

	}
	result = prepare_for_upload(port_fd);
	if (!result)
	{
		output("Failure getting ECU into a state to accept the new firmware\n");
		return FALSE;
	}
	upload_firmware(port_fd,file_fd);
	output("Firmware upload completed...\n");
	reboot_ecu(port_fd);
	output("ECU reboot complete\n");
	close_port(port_fd);
	return TRUE;
}


EXPORT gboolean leave(GtkWidget * widget, gpointer data)
{
	save_defaults();
	gtk_main_quit();
	return FALSE;
}


/*!
 \brief about_popup makes the about tab and presents the MegaTunix logo
 */
EXPORT gboolean about_popup(GtkWidget *widget, gpointer data)
{
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		gchar *authors[] = {"David J. Andruczyk",NULL};
		gchar *artists[] = {"Alan Barrow",NULL};
		gtk_show_about_dialog(NULL,
				"name","MegaTunix Firmware Loading Software",
				"version",VERSION,
				"copyright","David J. Andruczyk(2008)",
				"comments","MTXloader is a Graphical Firmware Loader designed to make it easy and (hopefully) intuitive to upgrade the firmware on your MS-I MegaSquirt powered vehicle.  Please send suggestions to the author for ways to improve mtxloader.",
				"license","GPL v2",
				"website","http://megatunix.sourceforge.net",
				"authors",authors,
				"artists",artists,
				"documenters",authors,
				NULL);
	}
#endif
	return TRUE;
}


void load_defaults()
{
	ConfigFile *cfgfile;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	filename = g_strconcat(HOME(), PSEP,".MegaTunix",PSEP,"config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(cfg_read_string(cfgfile, "Serial", "override_port", &tmpbuf))
		{
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (xml, "port_entry")),tmpbuf);
			g_free(tmpbuf);
		}
		if(cfg_read_string(cfgfile, "MTXLoader", "last_file", &tmpbuf))
		{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(glade_xml_get_widget (xml, "filechooser_button")),tmpbuf);
			g_free(tmpbuf);
		}
		cfg_free(cfgfile);
		g_free(filename);
	}
}


void save_defaults()
{
	ConfigFile *cfgfile;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	filename = g_strconcat(HOME(), PSEP,".MegaTunix",PSEP,"config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		tmpbuf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(glade_xml_get_widget (xml, "filechooser_button")));
		if (tmpbuf)
			cfg_write_string(cfgfile, "MTXLoader", "last_file", tmpbuf);
		g_free(tmpbuf);
		cfg_write_file(cfgfile,filename);
		cfg_free(cfgfile);
		g_free(filename);
	}
}


void textbuffer_changed(GtkTextBuffer *buffer, gpointer data)
{
	GtkTextMark *insert, *end;
	GtkTextIter end_iter;
	GtkTextIter insert_iter;
	gchar * text = NULL;
	insert = gtk_text_buffer_get_insert (buffer);
	end = gtk_text_buffer_get_mark (buffer,"end");
	gtk_text_buffer_get_iter_at_mark(buffer,&insert_iter,insert);
	gtk_text_buffer_get_iter_at_mark(buffer,&end_iter,end);

	text = gtk_text_buffer_get_text(buffer,&insert_iter,&end_iter,TRUE);
	printf("insert text \"%s\"\n",text);
}


void boot_jumper_prompt()
{
	GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new_with_markup(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_WARNING,GTK_BUTTONS_OK,"\nPlease Jumper the boot jumper on\nthe ECU and power cycle it\nand click OK when done");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


void init_controls()
{
	GtkWidget *widget = NULL;
	GtkWidget *widget2 = NULL;
	widget = glade_xml_get_widget (xml, "enter_clt_button");
	OBJ_SET(widget, "sensor", GINT_TO_POINTER(CLT));
	widget2 = glade_xml_get_widget (xml, "use_clt_toggle");
	OBJ_SET(widget2, "button", widget);

	widget = glade_xml_get_widget (xml, "enter_iat_button");
	OBJ_SET(widget, "sensor", GINT_TO_POINTER(IAT));
	widget2 = glade_xml_get_widget (xml, "use_iat_toggle");
	OBJ_SET(widget2, "button", widget);

}

EXPORT gboolean use_sensor(GtkWidget *widget, gpointer data)
{
	GtkWidget *button = NULL;
	button = (GtkWidget *)OBJ_GET(widget, "button");
	gtk_widget_set_sensitive(button,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	return TRUE;
}


EXPORT gboolean get_sensor_info(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *temp1,*temp2,*temp3,*ohms1,*ohms2,*ohms3,*units,*bias;
	samples sample;
	gint response = 0;
	gchar *fname = NULL;
	gchar *filename = NULL;
	GladeXML *d_xml = NULL;
	gchar unit;
	gint bias_val = 0;
	SensorType type;

	type = (SensorType)GPOINTER_TO_INT(OBJ_GET(widget,"sensor"));
	fname = g_build_filename(GUI_DATA_DIR,"mtxloader.glade",NULL);
	filename = get_file(g_strdup(fname),NULL);
	g_free(fname);
	d_xml = glade_xml_new (filename, "get_sensor_dialog", NULL);
	g_free(filename);
	dialog = glade_xml_get_widget (d_xml, "get_sensor_dialog");
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK)
	{	
		temp1 = glade_xml_get_widget(d_xml,"temp1");
		temp2 = glade_xml_get_widget(d_xml,"temp1");
		temp3 = glade_xml_get_widget(d_xml,"temp1");
		ohms1 = glade_xml_get_widget(d_xml,"ohms1");
		ohms2 = glade_xml_get_widget(d_xml,"ohms2");
		ohms3 = glade_xml_get_widget(d_xml,"ohms3");
		units = glade_xml_get_widget(d_xml,"F_radio");
		bias = glade_xml_get_widget(d_xml,"bias");
		sample.t1 = strtol(gtk_editable_get_chars(GTK_EDITABLE(temp1),0,-1),NULL,10);
		sample.t2 = strtol(gtk_editable_get_chars(GTK_EDITABLE(temp2),0,-1),NULL,10);
		sample.t3 = strtol(gtk_editable_get_chars(GTK_EDITABLE(temp3),0,-1),NULL,10);
		sample.r1 = strtol(gtk_editable_get_chars(GTK_EDITABLE(ohms1),0,-1),NULL,10);
		sample.r2 = strtol(gtk_editable_get_chars(GTK_EDITABLE(ohms2),0,-1),NULL,10);
		sample.r3 = strtol(gtk_editable_get_chars(GTK_EDITABLE(ohms3),0,-1),NULL,10);
		sample.r3 = strtol(gtk_editable_get_chars(GTK_EDITABLE(ohms3),0,-1),NULL,10);
		sample.r3 = strtol(gtk_editable_get_chars(GTK_EDITABLE(ohms3),0,-1),NULL,10);
		bias_val = strtol(gtk_editable_get_chars(GTK_EDITABLE(bias),0,-1),NULL,10);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(units)))
			unit = 'F';
		else
			unit = 'C';
		//thermistor(unit,&samples,bias_val,type,NULL);
		//write_inc_file();
	
	}
	gtk_widget_destroy(dialog);
	return TRUE;
}
