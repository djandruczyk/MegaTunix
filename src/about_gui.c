/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt Tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <about_gui.h>
#include <config.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <logo.h>



extern GObject *global_data;
/*!
 \brief build_about makes the about tab and presents the MegaTunix logo
 */
void install_logo(GtkWidget *alignment)
{
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add (GTK_CONTAINER (alignment), image);
	return;

}


void build_about(GtkWidget *frame)
{
	gchar *tmpbuf = NULL;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *alignment;
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	tmpbuf = g_strdup_printf("MegaTunix %s Tuning Software for Unix-class OS's",VERSION);
	label = gtk_label_new(tmpbuf);
	g_free(tmpbuf);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	label = gtk_label_new("designed by David J. Andruczyk");
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,TRUE,0);

	alignment = gtk_alignment_new(0.5,0.5,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),alignment,TRUE,FALSE,0);

	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);

	image = gtk_image_new_from_pixbuf(pixbuf);

	gtk_container_add (GTK_CONTAINER (alignment), image);

	return;
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
		gchar *artists[] = {"Dale Anderson\nChristopher Mire\nTrent Stromkins\nWayne (TurboCamaro)\n71jeep",NULL};
		gtk_show_about_dialog(NULL,
				"name","MegaTunix Tuning Software",
				"version",VERSION,
				"copyright","David J. Andruczyk(2007)",
				"comments","MegaTunix is a Graphical Tuning software designed to make it easy and (hopefully) intuitive to tune your MegaSquirt powered vehicle.  Please send suggestions to the author for ways to improve MegaTunix.",
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

