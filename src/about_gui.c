/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*! 
  \file src/about_gui.c
  \ingroup CoreMtx
  \brief Creates the about MegaTunix tab with the Logo
  \author David Andryczk
  \author Dale Anderson (logo)
  */

#include <about_gui.h>
#include <config.h>
#include <debugging.h>
#include <logo.h>
#include <widgetmgmt.h>

/*!
  \brief build_about makes the about tab and presents the MegaTunix logo
  \param parent is the container to place the logo in
  */
G_MODULE_EXPORT void install_logo(GtkWidget *parent)
{
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	ENTER();

	pixbuf = gdk_pixbuf_new_from_inline(sizeof(Logo),Logo,TRUE,NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add (GTK_CONTAINER (parent), image);
	EXIT();
	return;
}


/*!
  \brief Builds the about MegaTunix Tab and loads the main Logo
  \param frame is the parent frame for about window
  */
G_MODULE_EXPORT void build_about(GtkWidget *frame)
{
	gchar *tmpbuf = NULL;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *alignment;
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	ENTER();

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	tmpbuf = g_strdup_printf(_("MegaTunix Tuning Software for Unix-class OS's"),GIT_COMMIT);
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
	EXIT();
	return;
}


/*!
  \brief about_popup makes the about tab and presents the MegaTunix logo
  \param widget is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean about_popup(GtkWidget *widget, gpointer data)
{
	ENTER();

#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		const gchar *authors[] = {"David J. Andruczyk",NULL};
		const gchar *artists[] = {"Dale Anderson\nChristopher Mire\nTrent Stromkins\nWayne (TurboCamaro)\n71jeep",NULL};
		gchar *comments = g_strdup_printf("MegaTunix is a Graphical Tuning software designed to make it easy and (hopefully) intuitive to tune your EFI powered vehicle.  Please send suggestions to the author for ways to improve MegaTunix.\nGit Hash: %s",GIT_HASH);
		gtk_show_about_dialog(GTK_WINDOW(lookup_widget("main_window")),
				"name","MegaTunix Tuning Software",
				"version",GIT_COMMIT,
				"copyright","David J. Andruczyk(2012)",
				"comments",comments,
				"license","GPL v2",
				"website","http://megatunix.sourceforge.net",
				"authors",authors,
				"artists",artists,
				"documenters",authors,
				NULL);
		g_free(comments);
	}
#endif
	EXIT();
	return TRUE;
}

