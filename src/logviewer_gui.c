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
#include <datalogging_const.h>
#include <defines.h>
#include <enums.h>
#include <globals.h>
#include <gui_handlers.h>
#include <logviewer_gui.h>

void build_logviewer(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *frame;
	GSList *group;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	/* Traces frame */
	frame = gtk_frame_new("Log Viewer");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	vbox2 = gtk_vbox_new(TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	/* Settings/Parameters frame... */
	frame = gtk_frame_new("Viewer Parameters");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,15);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);

	button = gtk_radio_button_new_with_label(NULL,"Realtime Mode");
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	button = gtk_radio_button_new_with_label(group,"Playback Mode");
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox2,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Select Parameters to view");
	g_signal_connect(G_OBJECT(button),"clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(SEL_PARAMS));
	gtk_box_pack_start(GTK_BOX(vbox2),button,TRUE,FALSE,0);
	
	/* Not written yet */
	return;
}

void present_lviewer_choices()
{
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *sep;
	extern GtkTooltips *tip;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint table_rows;
	gint table_cols = 5;
	/* basty hack to prevent a compiler warning... */
	gint max = sizeof(mt_compat_names)/sizeof(gchar *);
	max = sizeof(logable_names)/sizeof(gchar *);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window),575,300);
	gtk_window_set_title(GTK_WINDOW(window),"Logviewer Realtime choices");
	g_signal_connect_swapped(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);
	g_signal_connect_swapped(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_container_set_border_width(GTK_CONTAINER(window),5);
	frame = gtk_frame_new("Select Variables to view from the list below");
	gtk_container_add(GTK_CONTAINER(window),frame);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	max = sizeof(logable_names)/sizeof(gchar *);
	table_rows = ceil((float)max/(float)table_cols);
	table = gtk_table_new(table_rows,table_cols,TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

	j = 0;
	k = 0;
	for (i=0;i<max;i++)
	{
		button = gtk_check_button_new_with_label(logable_names[i]);
		gtk_tooltips_set_tip(tip,button,logable_names_tips[i],NULL);
		//logables.widgets[i] = button;
		//              if ((dualtable) && (i >= STD_LOGABLES))
		//                      gtk_widget_set_sensitive(button,FALSE);
		g_object_set_data(G_OBJECT(button),"index",
				GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled",
				G_CALLBACK(view_value_set),
				NULL);
		gtk_table_attach (GTK_TABLE (table), button, j, j+1, k, k+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (0), 0, 0);
		j++;

		if (j == 5)
		{
			k++;
			j = 0;
		}
	}

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,TRUE,0);
	button = gtk_button_new_with_label("Close");
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer)window);

	gtk_widget_show_all(window);
	printf("present choices...\n");
	return;
}

gboolean view_value_set(GtkWidget *widget, gpointer data)
{
	return TRUE;
}
