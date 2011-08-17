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

/*! 
  \file widgets/combo_mask.c
  \brief Comboboxregex mask init code
  \author David Andruczyk
 */

#include <combo_mask.h>

/* Macro that creates a lot of boilerplate for us */
G_DEFINE_TYPE_WITH_CODE (MaskEntry, mask_entry, GTK_TYPE_ENTRY,G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE,mask_entry_editable_init))

/*!
 \brief gets called when a user wants a new mask entry
 \returns a pointer to a newly created mask entry widget
 */
G_MODULE_EXPORT GtkWidget *mask_entry_new (void)
{
        return GTK_WIDGET (g_object_new (TYPE_MASK_ENTRY, NULL));
}


/*!
 \brief gets called when a user wants a new mask entry
 \returns a pointer to a newly created mask entry widget
 */
G_MODULE_EXPORT GtkWidget *mask_entry_new_with_mask (gchar *mask)
{
	MaskEntry * widget = g_object_new (TYPE_MASK_ENTRY, NULL);
	widget->mask = g_strdup(mask);
	return GTK_WIDGET(widget);
}


/*!
 \brief gets called to set the  entry backgroup color based on a match or not
 \param entry is a pointer to the MaskEntry structure
 */
G_MODULE_EXPORT void mask_entry_set_background (MaskEntry *entry)
{
	gchar *tmpbuf = NULL;
	gchar *tmpstr = NULL;
	gint len = 0;
	static const GdkColor error_color = { 0, 65535, 60000, 60000 };

	if (entry->mask)
	{
		tmpstr = g_utf8_normalize(gtk_entry_get_text (GTK_ENTRY (entry)),-1,G_NORMALIZE_DEFAULT);
		tmpbuf = g_utf8_casefold(tmpstr,-1);
		g_free(tmpstr);
		if (g_regex_match_simple(tmpbuf,entry->mask,0,0))
		{
			gtk_widget_modify_base (GTK_WIDGET (entry), GTK_STATE_NORMAL, &error_color);
			g_free(tmpbuf);
			return;
		}
		g_free(tmpbuf);
	}

	gtk_widget_modify_base (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
	return;
}


/*!
 \brief gets called when the mask entry is changed
 \param editable is a pointer to the GtkEditable structure
 */
G_MODULE_EXPORT void mask_entry_changed (GtkEditable *editable)
{
	mask_entry_set_background (MASK_ENTRY (editable));
}


/*!
 \brief gets called when the mask entry is created
 \param entry is a pointer to the MaskEntry structure
 */
void mask_entry_init (MaskEntry *entry)
{
	entry->mask = NULL;
}


/*!
 \brief gets called when the mask entry class is created
 \param klass is a pointer to the MaskEntryClass structure
 */
void mask_entry_class_init (MaskEntryClass *klass)
{ 
        GObjectClass *obj_class = NULL;
        obj_class = G_OBJECT_CLASS (klass);
        obj_class->finalize = mask_entry_finalize;
}


/*!
 \brief gets called when the mask entry editable is created
 \param iface is a pointer to the GtkEditableClass structure
 */
void mask_entry_editable_init (GtkEditableClass *iface)
{
	iface->changed = mask_entry_changed;
}


/*!
 \brief gets called when the mask entry editable is destroyed
 \param object is a pointer to the GObject structure
 */
void mask_entry_finalize(GObject *object)
{
	MaskEntry *entry = MASK_ENTRY(object);
	if (entry->mask)
		g_free(entry->mask);
	G_OBJECT_CLASS(mask_entry_parent_class)->finalize(object);
}
