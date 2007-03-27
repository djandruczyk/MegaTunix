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
#include <mode_select.h>
#include <structures.h>
#include <threads.h>

gchar *states[] = {"FALSE","TRUE"};


/*!
 \brief set_widget_sensitive() is used to set a widgets state.  This function
 exists because we call it from a g_list_foreach() whereas a straight call to
 gtk_widget_set_sensitive from there would result in typecheck warnings
 \param widget (gpointer) pointer to widget to change sensitivity
 \param state (gpointer) the state to set it to
 */
void set_widget_sensitive(gpointer widget, gpointer state)
{
        gtk_widget_set_sensitive(GTK_WIDGET(widget),(gboolean)state);
}


/*!
 \brief set_widget_active() is used to set a toggle buttonstate.  This function
 exists because we call it from a g_list_foreach() whereas a straight call to
 gtk_toggle_button_set_active from there would result in typecheck warnings
 \param widget (gpointer) pointer to widget to change sensitivity
 \param state (gpointer) the state to set it to.
 */
void set_widget_active(gpointer widget, gpointer state)
{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gboolean)state);
}


/*!
 \brief drain_hashtable() is called to send all the dat from a hashtable to
 the ECU
 \param offset (gpointer) offset in ms_data this value goes to
 \param value (gpointer) the value to send
 \param page (gpointer) the page to send
 */
gboolean drain_hashtable(gpointer offset, gpointer value, gpointer page)
{
	extern Firmware_Details *firmware;
	/* called per element from the hash table to drain and send to ECU */
	write_ve_const(NULL, (gint)page, (gint)offset,(gint)value, firmware->page_params[(gint)page]->is_spark, TRUE);
	return TRUE;
}
