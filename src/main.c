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
#include <sys/types.h>
#include <sys/stat.h>
#include <defines.h>
#include <defines.h>
#include <protos.h>
#include <globals.h>


gint def_comm_port;

int main(int argc, char ** argv)
{
	gint cfg_result;
	GtkBindingSet *binding_set;


	g_thread_init(NULL);
	gdk_threads_init();

	gtk_init(&argc, &argv);

	gtk_set_locale();
	/* Check to see if we are being run from the correct
	 * directory.
	 */
	if (file_exists("MegaTunixrc"))
		gtk_rc_add_default_file ("MegaTunixrc");

	binding_set = gtk_binding_set_by_class (gtk_type_class (GTK_TYPE_WIDGET));
	gtk_binding_entry_add_signal (binding_set,
			'9', GDK_CONTROL_MASK | GDK_RELEASE_MASK,
			"debug_msg",
			1,
			G_TYPE_STRING, "GtkWidgetClass <ctrl><release>9 test");

	/* We use gtk_rc_parse_string() here so we can make sure it works across theme
	 * changes
	 */

	gtk_rc_parse_string ("style \"testgtk-version-label\" { "
			"   fg[NORMAL] = \"#ff0000\"\n"
			"   font = \"Sans 18\"\n"
			"}\n"
			"widget \"*.testgtk-version-label\" style \"testgtk-version-label\"");

	init();			/* initialize global vars */
	mem_alloc();
	make_megasquirt_dirs();	/*Create config file dirs if missing */

	cfg_result = read_config();
	setup_gui();		

	if (cfg_result < 0)
		open_serial(def_comm_port);
	else	
		open_serial(serial_params.comm_port);

	setup_serial_params();	/* Setup the serial port for I/O */

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return (0) ;
}

gboolean file_exists (const char *filename)
{
  struct stat statbuf;

  return stat (filename, &statbuf) == 0;
}

