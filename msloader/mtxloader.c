#include <config.h>
#include <defines.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

EXPORT void load_firmware (GtkButton*);
EXPORT void abort_load (GtkButton*);
EXPORT gboolean leave (GtkWidget *, gpointer);

GIOChannel *channel = NULL;
GladeXML *xml = NULL;
GtkWidget *main_window = NULL;
GPid pid;

int main (int argc, char *argv[])
{
	GtkWidget *textview = NULL;
	gchar * filename = NULL;
	gtk_init (&argc, &argv);
	filename = get_file(g_build_filename(GUI_DATA_DIR,"mtxloader.glade",NULL),NULL);
	xml = glade_xml_new (filename, NULL, NULL);
	g_free(filename);
	main_window = glade_xml_get_widget (xml, "main_window");
	textview = glade_xml_get_widget (xml, "textview");
	glade_xml_signal_autoconnect (xml);
	gtk_widget_show_all (main_window);
	gtk_main ();

	return 0;
}

/* Parse a line of output, which may actually be more than one line. To handle
 * multiple lines, the string is split with g_strsplit(). */
static void
parse_output (gchar *line)
{
	gchar **lines, **ptr;
	gchar * tmpbuf = NULL;
	GtkWidget *textview;
	GtkTextIter iter;
	GtkTextBuffer * textbuffer = NULL;
	GtkWidget *parent = NULL;
	GtkAdjustment * adj = NULL;


	/* Load the list store, split the string into lines and create a pointer. */
	textview = glade_xml_get_widget (xml, "textview");
	lines = g_strsplit (line, "\n", -1);
	ptr = lines;

	/* Loop through each of the lines, parsing its content. */
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	while (*ptr != NULL)
	{
		/* If this is an empty string, move to the next string. */
		if (g_ascii_strcasecmp (*ptr, "") == 0)
		{
			++ptr;
			continue;
		}
		gtk_text_buffer_get_end_iter (textbuffer, &iter);
		tmpbuf = g_strdup_printf("%s\n",*ptr);
		gtk_text_buffer_insert(textbuffer,&iter,tmpbuf,-1);
		g_free(tmpbuf);
		parent = gtk_widget_get_parent(textview);
		if (parent != NULL)
		{
			adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
			adj->value = adj->upper;
		}



		++ptr;
	}

	g_strfreev (lines);
}

/* This is a watch function that is called when the IO channel has data to read. */
static gboolean
read_output (GIOChannel *channel,
		         GIOCondition condition,
		         gpointer data)
{
	GError *error = NULL;
	GIOStatus status;
	gchar *line;
	gsize len, term;

  /* Read the current line of data from the IO channel. */
	status = g_io_channel_read_line (channel, &line, &len, &term, &error);
	
	/* If some type of error has occurred, handle it. */
	if (status != G_IO_STATUS_NORMAL || line == NULL || error != NULL) 
	{
		if (error) 
		{
			g_warning ("Error reading IO channel: %s", error->message);
			g_error_free (error);
		}

    /* Disable the stop button and enable the ping button for future action. */
    gtk_widget_set_sensitive (glade_xml_get_widget (xml, "abort"), FALSE);
    gtk_widget_set_sensitive (glade_xml_get_widget (xml, "load"), TRUE);
    
    if (channel != NULL)
      g_io_channel_shutdown (channel, TRUE, NULL);
    channel = NULL;
    
		return FALSE;
	}

  /* Parse the line if an error has not occurred. */
  parse_output (line);
	g_free (line);

	return TRUE;
}

/* Perform a ping operation when the Ping button is clicked. */
EXPORT void load_firmware (GtkButton *button)
{
	GtkWidget *port_entry = NULL;
	gint argc, fout, ret;
	const gchar *port;
	gchar *command = NULL;
	gchar *filename = NULL;
	gchar **argv = NULL;
	MtxFileIO *fileio = NULL;

	port_entry = glade_xml_get_widget (xml, "port_entry");
	port = gtk_entry_get_text(GTK_ENTRY(port_entry));
	fileio = g_new0(MtxFileIO, 1);
	//fileio->external_path = g_strdup("MTX_Firmware");
	fileio->title = g_strdup("Pick your new s19 file");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->default_extension = g_strdup("s19");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);

	if (g_utf8_strlen(port, -1) == 0)
		return;
	if (filename == NULL)
		return;

	command = g_strdup_printf("msloader %s %s",port,filename);

	g_shell_parse_argv (command, &argc, &argv, NULL);
	ret = g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,&pid, NULL, &fout, NULL, NULL);

	g_strfreev (argv);
	g_free (command);

	if (!ret)
		g_warning ("The 'msloader' instruction has failed!");
	else
	{
		/* Disable the msloader button and enable the Abort button. */
		gtk_widget_set_sensitive (glade_xml_get_widget (xml, "abort"), TRUE);
		gtk_widget_set_sensitive (glade_xml_get_widget (xml, "load"), FALSE);

		/* Create a new IO channel and monitor it for data to read. */
		channel = g_io_channel_unix_new (fout);
		g_io_add_watch (channel, G_IO_IN | G_IO_ERR | G_IO_HUP, read_output, NULL);
		g_io_channel_unref (channel);
	}
}

/* Kill the current msloader process when the Stop button is pressed. */
EXPORT void abort_load (GtkButton *button)
{
  gtk_widget_set_sensitive (glade_xml_get_widget (xml, "abort"), FALSE);
  gtk_widget_set_sensitive (glade_xml_get_widget (xml, "load"), TRUE);
  
  kill (pid, SIGINT);
}


EXPORT gboolean leave(GtkWidget * widget, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}
