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

#include <3d_vetable.h>
#include <comms_gui.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <mode_select.h>
#include <rtv_processor.h>
#include <runtime_gui.h>
#include <stdlib.h>
#include <structures.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>

extern gboolean connected;
extern gint active_page;
extern GdkColor white;
extern GdkColor black;
extern GdkColor red;

gboolean forced_update = TRUE;
gboolean no_update = FALSE;

gboolean update_runtime_vars()
{
	gint i = 0;
	struct Ve_View_3D * ve_view = NULL;
	extern GHashTable *rt_sliders;
	extern GHashTable *ww_sliders;
	extern GHashTable **ve3d_sliders;
	GtkWidget * tmpwidget=NULL;
	extern struct Firmware_Details *firmware;
	extern GHashTable * dynamic_widgets;
	gfloat coolant = 0.0;
	static gfloat last_coolant = 0.0;

	if(no_update)
		return FALSE;
	/* If OpenGL window is open, redraw it... */
	for (i=0;i<firmware->total_tables;i++)
	{
		tmpwidget = g_hash_table_lookup(dynamic_widgets,g_strdup_printf("ve_view_%i",i));
		if (GTK_IS_WIDGET(tmpwidget))
		{
			ve_view = (struct Ve_View_3D *)g_object_get_data(
					G_OBJECT(tmpwidget),"ve_view");
			if ((ve_view != NULL) && (ve_view->drawing_area->window != NULL)) 
				gdk_window_invalidate_rect (ve_view->drawing_area->window, &ve_view->drawing_area->allocation, FALSE);
			g_hash_table_foreach(ve3d_sliders[i],rt_update_values,NULL);
		}
	}

	/* Update all the dynamic RT Sliders */
	if (active_page == RUNTIME_PAGE)	/* Runtime display is visible */
	{
		g_hash_table_foreach(rt_sliders,rt_update_values,NULL);
		g_list_foreach(get_list("runtime_status"),rt_update_status,NULL);
		// "Connected" Status box
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_connected_label"),connected);
	}

	if (active_page == WARMUP_WIZ_PAGE)	/* Warmup wizard is visible */
	{
		g_hash_table_foreach(ww_sliders,rt_update_values,NULL);
		g_list_foreach(get_list("ww_status"),rt_update_status,NULL);

		if (!lookup_current_value("cltdeg",&coolant))
			dbg_func(__FILE__": update_runtime_vars()\n\t Error getting current value of \"cltdeg\" from datasource\n",CRITICAL);
		if ((coolant != last_coolant) || (forced_update))
			warmwizard_update_status(coolant);
		last_coolant = coolant;

		// "Connected" Status box
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_connected_label"),connected);

	}

	forced_update = FALSE;

	return TRUE;
}

void reset_runtime_status()
{
	// Runtime screen
	g_list_foreach(get_list("runtime_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	// Warmup Wizard screen
	g_list_foreach(get_list("ww_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));

}

void rt_update_status(gpointer key, gpointer data)
{
	GtkWidget *widget = (GtkWidget *) key;
	gint bitval = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint value = 0;
	gint last_entry = 0;
	static GObject *object = NULL;
	static gfloat * history = NULL;
	static gchar * source = NULL;
	static gchar * last_source = "";
	extern struct Rtv_Map *rtv_map;

	g_return_if_fail(GTK_IS_WIDGET(widget));

	source = (gchar *)g_object_get_data(G_OBJECT(widget),"source");
	if ((g_strcasecmp(source,last_source) != 0))
	{
		object = NULL;
		object = (GObject *)g_hash_table_lookup(rtv_map->rtv_hash,source);
		if (!object)
			return;
		history = (gfloat *)g_object_get_data(object,"history");
	}
	last_entry = (gint)g_object_get_data(object,"last_entry");

	bitval = (gint)g_object_get_data(G_OBJECT(widget),"bitval");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");
	bitshift = (gint)g_object_get_data(G_OBJECT(widget),"bitshift");
	
	value = (gint)history[last_entry];
	if (((value &bitmask) >> bitshift) == bitval) // enable it
		gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
	else	// disable it..
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
		
	last_source = source;

}


void rt_update_values(gpointer key, gpointer value, gpointer data)
{
	struct Rt_Slider *slider = (struct Rt_Slider *)value;
	gint count = slider->count;
	gint rate = slider->rate;
	gint last_upd = slider->last_upd;
	gfloat tmpf = 0.0;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gchar * tmpbuf = NULL;
	gint last_entry = 0;
	gint hist_size = 0;
	gfloat now = 0.0;
	gfloat last = 0.0;
	gfloat percentage = 0.0;
	gboolean is_float = FALSE;

	last_entry = (gint)g_object_get_data(slider->object,"last_entry");
	hist_size = (gint)g_object_get_data(slider->object,"hist_max");
	is_float = (gboolean)g_object_get_data(slider->object,"is_float");
	now = slider->history[last_entry];
	if (last_entry == 0)
		last = slider->history[hist_size-1];
	else
		last = slider->history[(last_entry-1)];

	upper = (gfloat)slider->upper;
	lower = (gfloat)slider->lower;
	
	if ((now != last) || (forced_update))
	{
		percentage = (now-lower)/(upper-lower);
		tmpf = percentage <= 1.0 ? percentage : 1.0;
		tmpf = tmpf >= 0.0 ? tmpf : 0.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(slider->pbar),
				tmpf);

		if ((abs(count-last_upd) > 5) || (forced_update))
		{
			if (is_float)
				tmpbuf = g_strdup_printf("%.2f",now);
			else
				tmpbuf = g_strdup_printf("%i",(gint)now);
			
			gtk_label_set_text(GTK_LABEL(slider->textval),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}
	}
	else if ((count % 30 == 0))
	{
		if (is_float)
			tmpbuf = g_strdup_printf("%.2f",now);
		else
			tmpbuf = g_strdup_printf("%i",(gint)now);
		gtk_label_set_text(GTK_LABEL(slider->textval),tmpbuf);
		g_free(tmpbuf);
		last_upd = count;
	}

	rate++;
	if (rate >25)
		rate = 25;
	if (last_upd > 5000)
		last_upd = 0;
	count++;
	if (count > 5000)
		count = 0;
	slider->rate = rate;
	slider->count = count;
	slider->last_upd = last_upd;
	return;
}
