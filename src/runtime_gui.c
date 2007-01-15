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
#include <dashboard.h>
#include <defines.h>
#include <debugging.h>
#include <dep_processor.h>
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
extern gboolean forced_update;
extern GdkColor white;
extern GdkColor black;
extern GdkColor red;
GHashTable *dash_gauges = NULL;

gboolean forced_update = TRUE;
gboolean no_update = FALSE;
GStaticMutex rtv_mutex = G_STATIC_MUTEX_INIT;


/*!
 \brief update_runtime_vars() updates all of the runtime sliders on all
 visible portions of the gui
 */
gboolean update_runtime_vars()
{
	gint i = 0;
	struct Ve_View_3D * ve_view = NULL;
	extern GHashTable *rt_sliders;
	extern GHashTable *enr_sliders;
	extern GHashTable *ww_sliders;
	extern GHashTable **ve3d_sliders;
	GtkWidget * tmpwidget=NULL;
	extern struct Firmware_Details *firmware;
	extern GHashTable * dynamic_widgets;
	gfloat coolant = 0.0;
	static gfloat last_coolant = 0.0;
	gfloat x,y,z = 0.0;
	gfloat xl,yl,zl = 0.0;
	gchar * string = NULL;
	static gint count = 0;
	static gboolean conn_status = FALSE;
	extern gint * algorithm;

	if(no_update)
		return FALSE;

	count++;
	if (conn_status != connected)
	{
		g_list_foreach(get_list("connected_widgets"),set_widget_sensitive,(gpointer)connected);
		conn_status = connected;
		forced_update = TRUE;
	}
		
	/* If OpenGL window is open, redraw it... */
	for (i=0;i<firmware->total_tables;i++)
	{
		string = g_strdup_printf("ve_view_%i",i);
		tmpwidget = g_hash_table_lookup(dynamic_widgets,string);
		g_free(string);
		if (GTK_IS_WIDGET(tmpwidget))
		{
			ve_view = (struct Ve_View_3D *)g_object_get_data(
					G_OBJECT(tmpwidget),"ve_view");
			if ((ve_view != NULL) && (ve_view->drawing_area->window != NULL))
			{
				if ((algorithm[ve_view->table_num] == ALPHA_N) && (ve_view->an_x_source))
				{
					lookup_current_value(ve_view->an_x_source,&x);
					lookup_previous_value(ve_view->an_x_source,&xl);
				}
				else
				{
					lookup_current_value(ve_view->x_source,&x);
					lookup_previous_value(ve_view->x_source,&xl);
				}
				if ((x != xl) || (forced_update))
					goto redraw;
				if ((algorithm[ve_view->table_num] == ALPHA_N) && (ve_view->an_y_source))
				{
					lookup_current_value(ve_view->an_y_source,&y);
					lookup_previous_value(ve_view->an_y_source,&yl);
				}
				else
				{
					lookup_current_value(ve_view->y_source,&y);
					lookup_previous_value(ve_view->y_source,&yl);
				}
				if ((y != yl) || (forced_update))
					goto redraw;

				if ((algorithm[ve_view->table_num] == ALPHA_N) && (ve_view->an_z_source))
				{
					lookup_current_value(ve_view->an_z_source,&z);
					lookup_previous_value(ve_view->an_z_source,&zl);
				}
				else
				{
					lookup_current_value(ve_view->z_source,&z);
					lookup_previous_value(ve_view->z_source,&zl);
				}
				if ((z != zl) || (forced_update))
					goto redraw;
				goto breakout;
redraw:
				gdk_window_invalidate_rect (ve_view->drawing_area->window, &ve_view->drawing_area->allocation, FALSE);
			}
breakout:
			g_hash_table_foreach(ve3d_sliders[i],rt_update_values,NULL);
		}
	}

	if (dash_gauges)
		g_hash_table_foreach(dash_gauges,update_dash_gauge,NULL);

	if ((active_page == VETABLES_PAGE) ||(active_page == SPARKTABLES_PAGE)||(active_page == AFRTABLES_PAGE)||(active_page == BOOSTTABLES_PAGE)||(active_page == ROTARYTABLES_PAGE))
		draw_ve_marker();
	/* Update all the dynamic RT Sliders */
	if (active_page == RUNTIME_PAGE)	/* Runtime display is visible */
		g_hash_table_foreach(rt_sliders,rt_update_values,NULL);
	if (active_page == ENRICHMENTS_PAGE)	/* Enrichments display is up */
		g_hash_table_foreach(enr_sliders,rt_update_values,NULL);

	if (active_page == WARMUP_WIZ_PAGE)	/* Warmup wizard is visible */
	{
		g_hash_table_foreach(ww_sliders,rt_update_values,NULL);

		if (!lookup_current_value("cltdeg",&coolant))
			dbg_func(g_strdup(__FILE__": update_runtime_vars()\n\t Error getting current value of \"cltdeg\" from datasource\n"),CRITICAL);
		if ((coolant != last_coolant) || (forced_update))
			warmwizard_update_status(coolant);
		last_coolant = coolant;

	}
	if ((count > 60) && (!forced_update))
		forced_update = TRUE;
	g_list_foreach(get_list("runtime_status"),rt_update_status,NULL);
	g_list_foreach(get_list("ww_status"),rt_update_status,NULL);
	forced_update=FALSE;

	if (count > 60 )
		count = 0;

	forced_update = FALSE;
	return TRUE;
}


/*!
 \brief reset_runtime_statue() sets all of the status indicators to OFF
 to reset the display
 */
void reset_runtime_status()
{
	// Runtime screen
	g_list_foreach(get_list("runtime_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	// Warmup Wizard screen
	g_list_foreach(get_list("ww_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
}


/*!
 \brief rt_update_status() updates the bitfield based status lights on the 
 runtime/warmupwizard displays
 \param key (gpointer) pointer to a widget
 \param data (gpointer) unused
 */
void rt_update_status(gpointer key, gpointer data)
{
	GtkWidget *widget = (GtkWidget *) key;
	gint bitval = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint value = 0;
	gfloat tmpf = 0.0;
	gint previous_value = 0;
	static GObject *object = NULL;
	static GArray * history = NULL;
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
		history = (GArray *)g_object_get_data(object,"history");
	}
	if (!connected)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
		return;
	}

	if (lookup_current_value(source,&tmpf))
		value = (gint) tmpf;
	else
		dbg_func(g_strdup_printf(__FILE__": rt_update_status()\n\t COULD NOT get current value for %s\n",source),CRITICAL);
	if (lookup_previous_value(source,&tmpf))
		previous_value = (gint) tmpf;
	else
		dbg_func(g_strdup_printf(__FILE__": rt_update_status()\n\t COULD NOT get previous value for %s\n",source),CRITICAL);

	bitval = (gint)g_object_get_data(G_OBJECT(widget),"bitval");
	bitmask = (gint)g_object_get_data(G_OBJECT(widget),"bitmask");
	bitshift = (gint)g_object_get_data(G_OBJECT(widget),"bitshift");
	

	/// if the value hasn't changed, don't bother continuing 
	if (((value & bitmask) == (previous_value & bitmask)) && (!forced_update))
		return;	
	
		if (((value & bitmask) >> bitshift) == bitval) // enable it
			gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
		else	// disable it..
			gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
		
	last_source = source;
}


/*!
 \brief rt_update_values() is called for each runtime slider to update
 it's position and label (label is periodic and not every time due to pango
 speed problems)
 \param key (gpointer) unused
 \param value (gpointer) pointer to Rt_Slider
 \param data (gpointer) unused
 */
void rt_update_values(gpointer key, gpointer value, gpointer data)
{
	struct Rt_Slider *slider = (struct Rt_Slider *)value;
	gint count = slider->count;
	gint rate = slider->rate;
	gint last_upd = slider->last_upd;
	gfloat tmpf = 0.0;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gint current_index = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	gfloat percentage = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;
	gboolean is_float = FALSE;

	history = (GArray *)g_object_get_data(slider->object,"history");
	current_index = (gint)g_object_get_data(slider->object,"current_index");
	is_float = (gboolean)g_object_get_data(slider->object,"is_float");
	g_static_mutex_lock(&rtv_mutex);
//	printf("runtime_gui history length is %i, current index %i\n",history->len,current_index);
	current = g_array_index(history, gfloat, current_index);
	if (current_index > 0)
		current_index-=1;
	previous = g_array_index(history, gfloat, current_index);
	g_static_mutex_unlock(&rtv_mutex);

	upper = (gfloat)slider->upper;
	lower = (gfloat)slider->lower;
	
	if ((current != previous) || (forced_update))
	{
		percentage = (current-lower)/(upper-lower);
		tmpf = percentage <= 1.0 ? percentage : 1.0;
		tmpf = tmpf >= 0.0 ? tmpf : 0.0;
		switch (slider->class)
		{
			case MTX_PROGRESS:
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
						(slider->pbar),
						tmpf);
				break;
			case MTX_RANGE:
				gtk_range_set_value(GTK_RANGE(slider->pbar),current);
					break;
		}
		

		/* If changed by more than 5% or has been at least 5 
		 * times withot an update or forced_update is set
		 * */
		
		if ((slider->textval) && ((abs(count-last_upd) > 3) || (forced_update)))
		{
			if (is_float)
				tmpbuf = g_strdup_printf("%.2f",current);
			else
				tmpbuf = g_strdup_printf("%i",(gint)current);

			gtk_label_set_text(GTK_LABEL(slider->textval),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}
		slider->last_percentage = percentage;
	}
	else if (slider->textval && ((abs(count-last_upd)%30) == 0))
	{
		if (is_float)
			tmpbuf = g_strdup_printf("%.2f",current);
		else
			tmpbuf = g_strdup_printf("%i",(gint)current);

		gtk_label_set_text(GTK_LABEL(slider->textval),tmpbuf);
		g_free(tmpbuf);
		last_upd = count;
	}

	rate++;
	if (rate > 25)
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
