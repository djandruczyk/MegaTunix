/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <assert.h>
#include <config.h>
#include <configfile.h>
#include <combo_loader.h>
#include <combo_mask.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <math.h>
#include <keyparser.h>
#include <stdlib.h>
#include <stringmatch.h>


extern GObject *global_data;

void combo_setup(GObject *object, ConfigFile *cfgfile, gchar * section)
{
	gchar *tmpbuf = NULL;
	gchar ** choices = NULL;
	gchar ** vector = NULL;
	gint num_choices = 0;
	gint num_bitvals = 0;
	gint i = 0;
	gint bitmask = 0;
	gint width = 0;
	gchar *tmpstr = NULL;
	gchar *regexp = NULL;
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GtkEntryCompletion *completion = NULL;
        GtkWidget *entry = NULL;

	cfg_read_string(cfgfile,section,"choices",&tmpbuf);

	choices = parse_keys(tmpbuf,&num_choices,",");
	//tmpstr = g_strdelimit(tmpbuf,",",'|');
	//regexp = g_strdup_printf("%s",tmpstr);
	tmpstr = g_utf8_normalize(tmpbuf,-1,G_NORMALIZE_DEFAULT);
	regexp = g_utf8_casefold(tmpstr,-1);
	g_free(tmpbuf);
	g_free(tmpstr);

	if (!cfg_read_string(cfgfile,section,"bitvals",&tmpbuf))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": combo_loader()\n\t\"bitvals\" key is MISSING, critical fault, not setting up control \n"));
		return;
	}
	vector = parse_keys(tmpbuf,&num_bitvals,",");
	g_free(tmpbuf);
	if (!cfg_read_int(cfgfile,section,"bitmask",&bitmask))
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": combo_loader()\n\t\"bitvals\" key is MISSING, critical fault, not setting up control \n"));
		return;
	}

	if (num_bitvals != num_choices)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": combo_loader()\n\t\"bitvals\" BIG PROBLEM, combobox  choices %i and bits %i don't match up\n",num_choices,num_bitvals));
		return;
	}


	store = gtk_list_store_new(COMBO_COLS,G_TYPE_STRING,G_TYPE_UCHAR);

	for (i=0;i<num_choices;i++)
	{
		gtk_list_store_append(store,&iter);
		assert(choices[i]);
		assert(vector[i]);
		gtk_list_store_set(store,&iter,CHOICE_COL,g_strdup(choices[i]),BITVAL_COL,(guchar)g_ascii_strtoull(vector[i],NULL,10),-1);

	}
	g_strfreev(vector);
	g_strfreev(choices);

	gtk_combo_box_set_model(GTK_COMBO_BOX(object),GTK_TREE_MODEL(store));
	if (GTK_IS_COMBO_BOX_ENTRY(object))
	{
		gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(object),CHOICE_COL);
		entry = mask_entry_new_with_mask(regexp);
		/* Nasty hack, but otherwise the entry is an obnoxious size.. */
		if ((width = (gint)OBJ_GET((GtkWidget *)object,"max_chars")) > 0)
			gtk_entry_set_width_chars(GTK_ENTRY(entry),width);
		else
			gtk_entry_set_width_chars(GTK_ENTRY(entry),12);

		gtk_widget_set_size_request(GTK_WIDGET(object),-1,(3*(gint)OBJ_GET(global_data,"font_size")));

		gtk_container_remove (GTK_CONTAINER (object), GTK_BIN (object)->child);
		gtk_container_add (GTK_CONTAINER (object), entry);

		completion = gtk_entry_completion_new();
		gtk_entry_set_completion(GTK_ENTRY(entry),completion);
		gtk_entry_completion_set_model(completion,GTK_TREE_MODEL(store));
		gtk_entry_completion_set_text_column(completion,CHOICE_COL);
		gtk_entry_completion_set_inline_completion(completion,TRUE);
		gtk_entry_completion_set_inline_selection(completion,TRUE);
		gtk_entry_completion_set_popup_single_match(completion,FALSE);
		OBJ_SET(object,"arrow-size",GINT_TO_POINTER(1));
	}
	g_free(regexp);
			
}
