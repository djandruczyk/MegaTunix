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

#include <config.h>
#include <configfile.h>
#include <combo_mask.h>
#include <gtk/gtk.h>


G_DEFINE_TYPE_WITH_CODE (MaskEntry, mask_entry, GTK_TYPE_ENTRY,G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE,mask_entry_editable_init));

void mask_entry_set_background (MaskEntry *entry)
{
	static const GdkColor error_color = { 0, 65535, 60000, 60000 };

	if (entry->mask)
	{
		if (!g_regex_match_simple (entry->mask, gtk_entry_get_text (GTK_ENTRY (entry)), 0, 0))
		{
			gtk_widget_modify_base (GTK_WIDGET (entry), GTK_STATE_NORMAL, &error_color);
			return;
		}
	}

	gtk_widget_modify_base (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
}



void mask_entry_changed (GtkEditable *editable)
{
	mask_entry_set_background (MASK_ENTRY (editable));
}



void mask_entry_init (MaskEntry *entry)
{
	entry->mask = NULL;
}



void mask_entry_class_init (MaskEntryClass *klass)
{ }



void mask_entry_editable_init (GtkEditableClass *iface)
{
	iface->changed = mask_entry_changed;
}

