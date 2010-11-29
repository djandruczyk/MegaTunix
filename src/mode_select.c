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
#include <firmware.h>
#include <mode_select.h>
#include <threads.h>

gchar *states[] = {"FALSE","TRUE"};


/*!
 \brief set_widget_sensitive() is used to set a widgets state.  This function
 exists because we call it from a g_list_foreach() whereas a straight call to
 gtk_widget_set_sensitive from there would result in typecheck warnings
 \param widget (gpointer) pointer to widget to change sensitivity
 \param state (gpointer) the state to set it to
 */
G_MODULE_EXPORT void set_widget_sensitive(gpointer widget, gpointer state)
{
        gtk_widget_set_sensitive(GTK_WIDGET(widget),(GBOOLEAN)state);
}


/*!
 \brief set_widget_active() is used to set a toggle buttonstate.  This function
 exists because we call it from a g_list_foreach() whereas a straight call to
 gtk_toggle_button_set_active from there would result in typecheck warnings
 \param widget (gpointer) pointer to widget to change sensitivity
 \param state (gpointer) the state to set it to.
 */
G_MODULE_EXPORT void set_widget_active(gpointer widget, gpointer state)
{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(GBOOLEAN)state);
}


/*!
 \brief drain_hashtable() is called to send all the data from a hashtable to
 the ECU
 \param offset (gpointer) offset in ecu_data this value goes to
 \param value (gpointer) pointer to OutputData Struct
 \param page (gpointer) unused.
 */
G_MODULE_EXPORT gboolean drain_hashtable(gpointer offset, gpointer value, gpointer user_data)
{
	Deferred_Data *data = (Deferred_Data *)value;

	send_to_ecu(data->canID,data->page,data->offset,data->size,data->value,FALSE);
	g_free(data);
	/* called per element from the hash table to drain and send to ECU */
	return TRUE;
}
