/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
 \file src/combo_loader.c
  \ingroup CoreMtx
  \brief Handles the special case ComboBoxEntry widgets
  This takes care of setting up the underlying TreeModel used for the combo
  box and sets up the regex parser so users can type in subtrings and on
  a match the selection is made.  The model has additional fields on it for
  the ECU specific bit value associated with the varius choices. The remaining
  ecu specific data (location information) is loaded via the tabloader
  \author David Andruczyk
  */

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#endif

#include <assert.h>
#include <combo_loader.h>
#include <combo_mask.h>
#include <debugging.h>
#include <defines.h>
#include <glade/glade.h>
#include <gui_handlers.h>
#include <keyparser.h>

/*!
  \brief Sets up a megatunix comboboxentry widget. Reads the data from the 
  passed ConfigFile *, and sets up the choices as well as the combo model
  and an entry completion/regex alloing textual based entry.
  \param object is the pointer to combo widget object
  \param cfgfile is the pointer to datamap file assocated with this widget
  \param section is the section within the cfgfile
  */
G_MODULE_EXPORT void combo_setup(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar ** choices = NULL;
	gchar ** vector = NULL;
	gint num_choices = 0;
	gint num_bitvals = 0;
	gint i = 0;
	const gchar *name = NULL;
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkEntryCompletion *completion = NULL;
	GtkWidget *entry = NULL;
	extern gconstpointer *global_data;

	ENTER();

	cfg_read_string(cfgfile,section,"choices",&tmpbuf);

	choices = parse_keys(tmpbuf,&num_choices,",");
	g_free(tmpbuf);

	if (!cfg_read_string(cfgfile,section,"bitvals",&tmpbuf))
	{
		MTXDBG(CRITICAL,_("\"bitvals\" key is MISSING, critical fault, not setting up control \n"));
		return;
	}
	vector = parse_keys(tmpbuf,&num_bitvals,",");
	g_free(tmpbuf);
	if (num_bitvals != num_choices)
	{
		name = glade_get_widget_name(GTK_WIDGET(object));
		MTXDBG(CRITICAL,_("\"bitvals\" BIG PROBLEM, combobox %s choices %i and bits %i don't match up\n"),(name == NULL ? "undefined" : name),num_choices,num_bitvals);
		return;
	}

	store = gtk_list_store_new(COMBO_COLS,G_TYPE_STRING,G_TYPE_UCHAR);

	for (i=0;i<num_choices;i++)
	{
		gtk_list_store_append(store,&iter);
		assert(choices[i]);
		assert(vector[i]);
		gtk_list_store_set(store,&iter,CHOICE_COL,choices[i],BITVAL_COL,(guchar)g_ascii_strtoull(vector[i],NULL,10),-1);

	}
//	OBJ_SET_FULL(object,"model",store,gtk_list_store_clear);
	g_strfreev(vector);
	g_strfreev(choices);

	gtk_combo_box_set_model(GTK_COMBO_BOX(object),GTK_TREE_MODEL(store));
	g_object_unref(store);
/*#if GTK_MINOR_VERSION < 24 */
	if (GTK_IS_COMBO_BOX_ENTRY(object))
	{
		gint width = 0;
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(object),CHOICE_COL);
/*
#else
	if (GTK_IS_COMBO_BOX(object))
	{
		gint width = 0;
		g_object_set(object,"entry-text-column",CHOICE_COL,NULL);
		gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(object),CHOICE_COL);
#endif
*/
		entry = gtk_bin_get_child(GTK_BIN (object));
		/* Nasty hack, but otherwise the entry is an obnoxious size.. */
		if ((width = (GINT)OBJ_GET((GtkWidget *)object,"max_chars")) > 0)
			gtk_entry_set_width_chars(GTK_ENTRY(entry),width);
		else
			gtk_entry_set_width_chars(GTK_ENTRY(entry),12);

		gtk_widget_set_size_request(GTK_WIDGET(object),-1,(3*(GINT)DATA_GET(global_data,"font_size")));

		completion = gtk_entry_completion_new();
		gtk_entry_set_completion(GTK_ENTRY(entry),completion);
		g_object_unref(completion);
		gtk_entry_completion_set_model(completion,GTK_TREE_MODEL(store));
		gtk_entry_completion_set_text_column(completion,CHOICE_COL);
		gtk_entry_completion_set_inline_completion(completion,TRUE);
		gtk_entry_completion_set_inline_selection(completion,TRUE);
		gtk_entry_completion_set_popup_single_match(completion,FALSE);
		OBJ_SET(object,"arrow-size",GINT_TO_POINTER(1));
	}
	EXIT();
	return;
}
