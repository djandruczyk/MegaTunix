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
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <raweditor.h>
#include <structures.h>

GArray *raw_editor_widgets = NULL;
gboolean swap_labels(GtkWidget *widget, GdkEvent *event, gpointer data);


/*!
 \brief finish_raweditor() completes the raweditor view by populating the
 gui with a notebook populated by several tables to access any variable in the
 firmware via a raw textentry (in hex). The notebook will have a page for each
 page defined in the ECU firmware's interrogation profile
 */
EXPORT void finish_raweditor(void)
{
	extern gboolean thread_protect;
	GtkWidget *sw = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *ebox = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *lbl_table = NULL;
	GtkWidget *table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *top = NULL;
	GtkWidget *entry = NULL;
	gint row = 0;
	gint col = 0;
	gint i = 0;
	gint j = 0;
	gint cols = 8;
	GdkColor purple = { 0, 61000, 57000, 65535};
	extern GdkColor white;
	extern struct Firmware_Details *firmware;
	extern GHashTable *dynamic_widgets;
	extern GList *** ve_widgets;

	top = g_hash_table_lookup(dynamic_widgets,"raweditor_top_vbox1");
	if (!GTK_IS_WIDGET(top))
		return;

	notebook = gtk_notebook_new ();
	        gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
		        gtk_box_pack_start(GTK_BOX(top),notebook,TRUE,TRUE,0);


	for (i=0;i<firmware->total_pages;i++)
	{
		label = gtk_label_new(g_strdup_printf("Page %i",i));
		frame = gtk_frame_new(g_strdup_printf("Page %i RAW ECU data (in HEX)",i));
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,label);

		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
		gtk_container_add(GTK_CONTAINER(frame),vbox);

		sw = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				GTK_POLICY_AUTOMATIC,
				GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start(GTK_BOX(vbox),sw,TRUE,TRUE,5);

		hbox = gtk_hbox_new(FALSE,0);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),hbox);
		ebox = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(hbox),ebox,FALSE,TRUE,5);
		lbl_table = gtk_table_new((firmware->page_params[i]->length)/cols,1,TRUE);
		g_object_set_data(G_OBJECT(lbl_table),"format",GINT_TO_POINTER(MTX_HEX));
		gtk_container_add(GTK_CONTAINER(ebox),lbl_table);
		g_signal_connect(G_OBJECT(ebox),"button_press_event",
				G_CALLBACK(swap_labels),NULL);

		table = gtk_table_new((firmware->page_params[i]->length)/cols,cols,TRUE);
		gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,5);
		row = 0;
		col = 0;
		for (j=0;j<firmware->page_params[i]->length;j++)
		{
			entry = gtk_entry_new();
			g_object_set_data(G_OBJECT(entry),"page",GINT_TO_POINTER(i));
			g_object_set_data(G_OBJECT(entry),"offset",GINT_TO_POINTER(j));
			g_object_set_data(G_OBJECT(entry),"base",GINT_TO_POINTER(16));
			g_object_set_data(G_OBJECT(entry),"raw_lower",GINT_TO_POINTER(0));
			g_object_set_data(G_OBJECT(entry),"raw_upper",GINT_TO_POINTER(255));
			if (firmware->page_params[i]->is_spark)
				g_object_set_data(G_OBJECT(entry),"ign_parm",GINT_TO_POINTER(TRUE));

			ve_widgets[i][j] = g_list_append(ve_widgets[i][j],(gpointer)entry);
			g_object_set_data(G_OBJECT(entry),"handler",
					GINT_TO_POINTER(GENERIC));
			g_signal_connect (G_OBJECT(entry), "changed",
					G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect (G_OBJECT(entry), "activate",
					G_CALLBACK(std_entry_handler),NULL);
			g_signal_connect (G_OBJECT(entry), "key_press_event",
					G_CALLBACK(key_event),NULL);

			gtk_entry_set_width_chars(GTK_ENTRY(entry),4);
			gtk_entry_set_has_frame(GTK_ENTRY(entry),TRUE);
			gtk_table_attach(GTK_TABLE(table),entry,col,col+1,row,row+1,
					(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
					(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL), 0, 0);

			if (row%2)
				gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&purple);
			else
				gtk_widget_modify_base(entry,GTK_STATE_NORMAL,&white);
			if (col == 0)
			{
				label = gtk_label_new(g_strdup_printf("0x%.4X",(row*cols)));
				gtk_table_attach(GTK_TABLE(lbl_table),label,0,1,row,row+1,
						(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
						(GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL), 0, 0);
			}
			col++;
			if (col >= cols)
			{
				col = 0;
				row++;
			}
		}
		if (thread_protect)
			gdk_threads_enter();
		while (gtk_events_pending ())
			gtk_main_iteration ();
		if (thread_protect)
			gdk_threads_leave();
	}
	gtk_widget_show_all(top);
	
}


/*!
 \brief swap_labels() swaps the address labels between hex or decimal
 base on a mouse click in the table.
 \param widget (GtkWidget *) the eventbox we click in
 \param event (GdkEvent *) the event type
 \param data (gpointer) unused
 */
gboolean swap_labels(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gint format = 0;
	gint i = 0;
	GtkTable *table = NULL;
	gchar * tmpbuf = NULL;
	GtkTableChild *child = NULL;

	table = (GtkTable *)gtk_bin_get_child(GTK_BIN(widget));
	if (GTK_IS_TABLE(table))
		format = (gint)g_object_get_data(G_OBJECT(table),"format");
	else
		return FALSE;

	if (format == MTX_HEX)
	{
		for (i=0;i<table->nrows;i++)
		{
			child = g_list_nth_data(table->children,i);
			if (GTK_IS_LABEL(child->widget))
			{
				tmpbuf = g_strdup_printf("%i",child->top_attach*8);
				gtk_label_set_text(GTK_LABEL(child->widget),tmpbuf);
				g_free(tmpbuf);
			}
		}
		g_object_set_data(G_OBJECT(table),"format",GINT_TO_POINTER(MTX_DECIMAL));
	}
	else
	{
		for (i=0;i<table->nrows;i++)
		{
			child = g_list_nth_data(table->children,i);
			if (GTK_IS_LABEL(child->widget))
			{
				tmpbuf = g_strdup_printf("0x%.4X",child->top_attach*8);
				gtk_label_set_text(GTK_LABEL(child->widget),tmpbuf);
				g_free(tmpbuf);
			}
		}
		g_object_set_data(G_OBJECT(table),"format",GINT_TO_POINTER(MTX_HEX));
	}

	return FALSE;
}
