#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <fcntl.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <glib/gstring.h>
#include <loader_common.h>
#include <ms1_loader.h>
#include <ms2_loader.h>
#include <mtxloader.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


gconstpointer *global_data = NULL;

int main (int argc, char *argv[])
{
	GtkWidget *dialog = NULL;
	gchar * filename = NULL;
	gchar * fname = NULL;
	GError *error = NULL;
	GtkBuilder *builder = NULL;
	GtkWidget *main_window = NULL;

	gtk_init (&argc, &argv);
        global_data = g_new0(gconstpointer, 1);

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
	else
	{
		builder = gtk_builder_new();
		if(!gtk_builder_add_from_file(builder,filename,&error))
		{
			g_warning ("Couldn't load builder file: %s", error->message);
			g_error_free(error);
			exit(-1);
		}
	}
	main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
	DATA_SET(global_data,"main_window",main_window);
	DATA_SET(global_data,"builder",builder);
	g_free(fname);
	g_free(filename);
	gtk_builder_connect_signals (builder,NULL);
	load_defaults();
	gtk_widget_show_all (main_window);
	gtk_main ();

	return 0;
}

/* Parse a line of output, which may actually be more than one line. To handle
 * multiple lines, the string is split with g_strsplit(). */
void output (gchar *line, gboolean free_it)
{
	static GtkBuilder *builder = NULL;
	static GtkWidget *textview = NULL;
	GtkTextBuffer * textbuffer = NULL;
	GtkTextIter iter;
	GtkWidget *parent = NULL;
	GtkAdjustment * adj = NULL;
	
	if (!builder)
		builder = DATA_GET(global_data,"builder");
	g_return_if_fail(builder);
	if (!textview)
		textview = GTK_WIDGET(gtk_builder_get_object(builder, "textview"));
	g_return_if_fail(textview);

	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	gtk_text_buffer_get_end_iter (textbuffer, &iter);
	gtk_text_buffer_insert(textbuffer,&iter,line,-1);
	parent = gtk_widget_get_parent(textview);
	if (parent != NULL)
	{
		adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
		adj->value = adj->upper;
	}
	if (free_it)
		g_free(line);
	while (gtk_events_pending())
		gtk_main_iteration();
}


/* Perform a ping operation when the Ping button is clicked. */
G_MODULE_EXPORT gboolean get_signature (GtkButton *button)
{
	GtkWidget *widget =  NULL;
	gchar * port = NULL;
	gint port_fd = 0;

	widget = GTK_WIDGET(gtk_builder_get_object(DATA_GET(global_data,"builder"), "port_entry"));
	port = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	if (g_utf8_strlen(port, -1) == 0)
		return FALSE;
	/* If we got this far, all is good argument wise */
	
	port_fd = open_port(port);
	if (!(port_fd  > 0))
	{
		output("Could NOT open Port, You should check perms.\n",FALSE);
		return FALSE;
	}
	setup_port(port_fd,9600);
	if (!get_ecu_signature(port_fd))
	{
		close_port(port_fd);
		port_fd = open_port(port);
		setup_port(port_fd,115200);
		get_ecu_signature(port_fd);
	}
	close_port(port_fd);

	return TRUE;
}


G_MODULE_EXPORT gboolean persona_choice (GtkWidget *widget, gpointer data)
{
	gint persona = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		DATA_SET(global_data,"persona",OBJ_GET(widget,"persona"));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(DATA_GET(global_data,"builder"),"load_button")),TRUE);
	}
	return TRUE;
}


G_MODULE_EXPORT gboolean load_firmware (GtkButton *button)
{
	GtkWidget *widget = NULL;
	gchar *port;
	gchar *filename = NULL;
	gint port_fd = 0;
	gint file_fd = 0;
	FirmwareType type = MS1;

	widget = GTK_WIDGET(gtk_builder_get_object(DATA_GET(global_data,"builder"), "port_entry"));
	port = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	widget = GTK_WIDGET(gtk_builder_get_object (DATA_GET(global_data,"builder"), "filechooser_button"));
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

	if (g_utf8_strlen(port, -1) == 0)
		return FALSE;
	if (filename == NULL)
		return FALSE;

	/* If we got this far, all is good argument wise */
	if (!lock_port(port))
	{
		output("Could NOT LOCK Serial Port,\nYou should check for other programs using the serial port\n",FALSE);
		return FALSE;
	}
	port_fd = open_port(port);
	if (port_fd > 0)
		output("Port successfully opened\n",FALSE);
	else
	{
		output("Could NOT open Port, You should check perms\n",FALSE);
		unlock_port();
		return FALSE;
	}
#ifdef __WIN32__
	file_fd = open(filename, O_RDWR | O_BINARY );
#else
	file_fd = g_open(filename,O_RDONLY,S_IRUSR);
#endif
	if (file_fd > 0 )
		output("Firmware file successfully opened\n",FALSE);
	else
	{
		output("Could NOT open firmware file, check permissions/paths\n",FALSE);
		close_port(port_fd);
		unlock_port();
		return FALSE;
	}
	if ((GINT)DATA_GET(global_data,"persona") == MS1)
	{
		setup_port(port_fd, 9600);
		do_ms1_load(port_fd,file_fd);
	}
	else if ((GINT)DATA_GET(global_data,"persona") == MS2)
	{
		setup_port(port_fd, 115200);
		do_ms2_load(port_fd,file_fd);
	}
	/*
	type = detect_firmware(filename);
	if (type == MS1)
	{
		setup_port(port_fd, 9600);
		do_ms1_load(port_fd,file_fd);
	}
	else if (type == MS2)
	{
		setup_port(port_fd,115200);
		do_ms2_load(port_fd,file_fd);
	}
	*/

	close_port(port_fd);
	unlock_port();
	return TRUE;
}


G_MODULE_EXPORT gboolean leave(GtkWidget * widget, gpointer data)
{
	save_defaults();
	gtk_main_quit();
	return FALSE;
}


/*!
 \brief about_popup makes the about tab and presents the MegaTunix logo
 */
G_MODULE_EXPORT gboolean about_popup(GtkWidget *widget, gpointer data)
{
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		gchar *authors[] = {"David J. Andruczyk",NULL};
		gchar *artists[] = {"Alan Barrow",NULL};
		gtk_show_about_dialog(NULL,
				"name","MegaTunix Firmware Loading Software",
				"version",VERSION,
				"copyright","David J. Andruczyk(2011)",
				"comments","MTXloader is a Graphical Firmware Loader designed to make it easy and (hopefully) intuitive to upgrade the firmware on your MS-I or MS-II MegaSquirt powered vehicle. This tool is capable of loading any firmware version for both series of ECUs.  This is based upon code provided by James Murray, Ken Culver from the MS2-Extra project.  Please send suggestions to the author for ways to improve mtxloader.",
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
	GtkBuilder *builder = NULL;
	ConfigFile *cfgfile;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	GtkFileFilter *filter = NULL;
	GObject *object = NULL;

	builder = DATA_GET(global_data,"builder");
	g_return_if_fail(builder);

	filename = g_strconcat(HOME(), PSEP,".MegaTunix",PSEP,"config", NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(cfg_read_string(cfgfile, "Serial", "override_port", &tmpbuf))
		{
			gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object (builder, "port_entry")),tmpbuf);
			g_free(tmpbuf);
		}
		if(cfg_read_string(cfgfile, "MTXLoader", "last_file", &tmpbuf))
		{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(gtk_builder_get_object (builder, "filechooser_button")),tmpbuf);
			filter = gtk_file_filter_new();
			gtk_file_filter_add_pattern(filter,"*.s19");
			gtk_file_filter_add_pattern(filter,"*.S19");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(gtk_builder_get_object (builder, "filechooser_button")),filter);

			g_free(tmpbuf);
		}
		object = gtk_builder_get_object(builder,"ms1_rbutton");
		if (G_IS_OBJECT(object))
			OBJ_SET(object,"persona",GINT_TO_POINTER(MS1));
		object = gtk_builder_get_object(builder,"ms2_rbutton");
		if (G_IS_OBJECT(object))
			OBJ_SET(object,"persona",GINT_TO_POINTER(MS2));
		object = gtk_builder_get_object(builder,"freeems_rbutton");
		if (G_IS_OBJECT(object))
			OBJ_SET(object,"persona",GINT_TO_POINTER(FREEEMS));
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
		tmpbuf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gtk_builder_get_object (DATA_GET(global_data,"builder"), "filechooser_button")));
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
}


void boot_jumper_prompt()
{
	GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(DATA_GET(global_data,"main_window")),GTK_DIALOG_MODAL,GTK_MESSAGE_WARNING,GTK_BUTTONS_OK,"\nPlease jumper the boot jumper on\nthe ECU and power cycle it\nand click OK when done");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


