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
#include <firmware.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <math.h>
#include <mode_select.h>
#include <multi_expr_loader.h>
#include <notifications.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <runtime_gui.h>
#include <runtime_sliders.h>
#include <runtime_text.h>
#include <stdlib.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>

extern gboolean connected;
extern gint active_page;
extern gboolean forced_update;
extern GdkColor white;
extern GdkColor black;
extern GdkColor red;
extern gint dbg_lvl;
extern GObject *global_data;
GHashTable *dash_gauges = NULL;

gboolean forced_update = TRUE;
GStaticMutex rtv_mutex = G_STATIC_MUTEX_INIT;


/*!
 \brief update_runtime_vars_pf() updates all of the runtime sliders on all
 visible portions of the gui
 */
EXPORT gboolean update_runtime_vars_pf()
{
	gint i = 0;
	Ve_View_3D * ve_view = NULL;
	extern GHashTable *rtt_hash;
	extern GHashTable *rt_sliders;
	extern GHashTable *enr_sliders;
	extern GHashTable *ww_sliders;
	extern GHashTable **ve3d_sliders;
	GtkWidget * tmpwidget=NULL;
	extern Firmware_Details *firmware;
	extern GHashTable * dynamic_widgets;
	gfloat coolant = 0.0;
	static gfloat last_coolant = 0.0;
	gfloat x,y,z = 0.0;
	gfloat xl,yl,zl = 0.0;
	gchar * string = NULL;
	GHashTable *hash = NULL;
	extern GHashTable *sources_hash;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	MultiSource *multi = NULL;
	static gint count = 0;
	static gboolean conn_status = FALSE;
	extern gint * algorithm;
	extern GStaticMutex dash_mutex;
	extern gboolean interrogated;

	if (!interrogated)
		return FALSE;

	if (!firmware)
		return FALSE;

	count++;
	if (conn_status != connected)
	{
		g_list_foreach(get_list("connected_widgets"),set_widget_sensitive,(gpointer)connected);
		conn_status = connected;
		forced_update = TRUE;
	}
	if ((count > 60) && (!forced_update))
		forced_update = TRUE;

	/* If OpenGL window is open, redraw it... */
	for (i=0;i<firmware->total_tables;i++)
	{
		string = g_strdup_printf("ve_view_%i",i);
		tmpwidget = g_hash_table_lookup(dynamic_widgets,string);
		g_free(string);
		if (GTK_IS_WIDGET(tmpwidget))
		{
			ve_view = (Ve_View_3D *)OBJ_GET(tmpwidget,"ve_view");
			if ((ve_view != NULL) && (ve_view->drawing_area->window != NULL))
			{
				/* Get X values */
				if (ve_view->x_multi_source)
				{
					hash = ve_view->x_multi_hash;
					key = ve_view->x_source_key;
					hash_key = g_hash_table_lookup(sources_hash,key);
					if (algorithm[ve_view->table_num] == SPEED_DENSITY)
					{
						if (hash_key)
							multi = g_hash_table_lookup(hash,hash_key);
						else
							multi = g_hash_table_lookup(hash,"DEFAULT");
					}
					else if (algorithm[ve_view->table_num] == ALPHA_N)
						multi = g_hash_table_lookup(hash,"DEFAULT");
					else if (algorithm[ve_view->table_num] == MAF)
					{
						multi = g_hash_table_lookup(hash,"AFM_VOLTS");
						if(!multi)
							multi = g_hash_table_lookup(hash,"DEFAULT");
					}
					else
						multi = g_hash_table_lookup(hash,"DEFAULT");

					if (!multi)
						printf("multi is null!!\n");

					lookup_current_value(multi->source,&x);
					lookup_previous_value(multi->source,&xl);
				}
				else
				{
					lookup_current_value(ve_view->x_source,&x);
					lookup_previous_value(ve_view->x_source,&xl);
				}
				/* Test X values, redraw if needed */
				if (((fabs(x-xl)/x) > 0.005) || (forced_update))
					goto redraw;

				/* Get Y values */
				if (ve_view->y_multi_source)
				{
					hash = ve_view->y_multi_hash;
					key = ve_view->y_source_key;
					hash_key = g_hash_table_lookup(sources_hash,key);
					if (algorithm[ve_view->table_num] == SPEED_DENSITY)
					{
						if (hash_key)
							multi = g_hash_table_lookup(hash,hash_key);
						else
							multi = g_hash_table_lookup(hash,"DEFAULT");
					}
					else if (algorithm[ve_view->table_num] == ALPHA_N)
						multi = g_hash_table_lookup(hash,"DEFAULT");
					else if (algorithm[ve_view->table_num] == MAF)
					{
						multi = g_hash_table_lookup(hash,"AFM_VOLTS");
						if(!multi)
							multi = g_hash_table_lookup(hash,"DEFAULT");
					}
					else
						multi = g_hash_table_lookup(hash,"DEFAULT");

					if (!multi)
						printf("multi is null!!\n");

					lookup_current_value(multi->source,&y);
					lookup_previous_value(multi->source,&yl);
				}
				else
				{
					lookup_current_value(ve_view->y_source,&y);
					lookup_previous_value(ve_view->y_source,&yl);
				}
				/* Test Y values, redraw if needed */
				if (((fabs(y-yl)/y) > 0.001) || (forced_update))
					goto redraw;

				/* Get Z values */
				if (ve_view->z_multi_source)
				{
					hash = ve_view->z_multi_hash;
					key = ve_view->z_source_key;
					hash_key = g_hash_table_lookup(sources_hash,key);
					if (algorithm[ve_view->table_num] == SPEED_DENSITY)
					{
						if (hash_key)
							multi = g_hash_table_lookup(hash,hash_key);
						else
							multi = g_hash_table_lookup(hash,"DEFAULT");
					}
					else if (algorithm[ve_view->table_num] == ALPHA_N)
						multi = g_hash_table_lookup(hash,"DEFAULT");
					else if (algorithm[ve_view->table_num] == MAF)
					{
						multi = g_hash_table_lookup(hash,"AFM_VOLTS");
						if(!multi)
							multi = g_hash_table_lookup(hash,"DEFAULT");
					}
					else
						multi = g_hash_table_lookup(hash,"DEFAULT");

					if (!multi)
						printf("multi is null!!\n");

					lookup_current_value(multi->source,&y);
					lookup_previous_value(multi->source,&yl);
				}
				else
				{
					lookup_current_value(ve_view->z_source,&z);
					lookup_previous_value(ve_view->z_source,&zl);
				}
				/* Test Z values, redraw if needed */
				if (((fabs(z-zl)/z) > 0.001) || (forced_update))
					goto redraw;
				goto breakout;

redraw:
				gdk_window_invalidate_rect (ve_view->drawing_area->window, &ve_view->drawing_area->allocation, FALSE);
			}
breakout:
			g_hash_table_foreach(ve3d_sliders[i],rt_update_values,NULL);
		}
	}

	g_static_mutex_lock(&dash_mutex);
	if (dash_gauges)
		g_hash_table_foreach(dash_gauges,update_dash_gauge,NULL);
	g_static_mutex_unlock(&dash_mutex);

	if ((active_page == VETABLES_TAB) ||(active_page == SPARKTABLES_TAB)||(active_page == AFRTABLES_TAB)||(active_page == BOOSTTABLES_TAB)||(active_page == ROTARYTABLES_TAB) || (forced_update))
	{
		draw_ve_marker();
		update_tab_gauges();
	}
	if (rtt_hash)
		g_hash_table_foreach(rtt_hash,rtt_update_values,NULL);
	/* Update all the dynamic RT Sliders */
	if (active_page == RUNTIME_TAB)	/* Runtime display is visible */
		g_hash_table_foreach(rt_sliders,rt_update_values,NULL);
	if (active_page == ENRICHMENTS_TAB)	/* Enrichments display is up */
		g_hash_table_foreach(enr_sliders,rt_update_values,NULL);

	if (active_page == WARMUP_WIZ_TAB)	/* Warmup wizard is visible */
	{
		g_hash_table_foreach(ww_sliders,rt_update_values,NULL);

		if (!lookup_current_value("cltdeg",&coolant))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup(__FILE__": update_runtime_vars_pf()\n\t Error getting current value of \"cltdeg\" from datasource\n"));
		}
		if ((coolant != last_coolant) || (forced_update))
			warmwizard_update_status(coolant);
		last_coolant = coolant;

	}
	g_list_foreach(get_list("runtime_status"),rt_update_status,NULL);
	g_list_foreach(get_list("ww_status"),rt_update_status,NULL);

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
	/* Runtime screen */
	g_list_foreach(get_list("runtime_status"),set_widget_sensitive,GINT_TO_POINTER(FALSE));
	/* Warmup Wizard screen */
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
	extern Rtv_Map *rtv_map;

	g_return_if_fail(GTK_IS_WIDGET(widget));

	source = (gchar *)OBJ_GET(widget,"source");
	if ((g_strcasecmp(source,last_source) != 0))
	{
		object = NULL;
		object = (GObject *)g_hash_table_lookup(rtv_map->rtv_hash,source);
		if (!object)
			return;
		history = (GArray *)OBJ_GET(object,"history");
	}
	if (!connected)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
		return;
	}

	if (lookup_current_value(source,&tmpf))
		value = (gint) tmpf;
	else
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": rt_update_status()\n\t COULD NOT get current value for %s\n",source));
	}
	if (lookup_previous_value(source,&tmpf))
		previous_value = (gint) tmpf;
	else
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": rt_update_status()\n\t COULD NOT get previous value for %s\n",source));
	}

	bitval = (gint)OBJ_GET(widget,"bitval");
	bitmask = (gint)OBJ_GET(widget,"bitmask");
	bitshift = (gint)OBJ_GET(widget,"bitshift");


	/* if the value hasn't changed, don't bother continuing */
	if (((value & bitmask) == (previous_value & bitmask)) && (!forced_update))
		return;	

	if (((value & bitmask) >> bitshift) == bitval) /* enable it */
		gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
	else	/* disable it.. */
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
	Rt_Slider *slider = (Rt_Slider *)value;
	gint count = slider->count;
	gint rate = slider->rate;
	gint last_upd = slider->last_upd;
	gfloat tmpf = 0.0;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gint current_index = 0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	gfloat percentage = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;

	history = (GArray *)OBJ_GET(slider->object,"history");
	current_index = (gint)OBJ_GET(slider->object,"current_index");
	precision = (gint)OBJ_GET(slider->object,"precision");
	g_static_mutex_lock(&rtv_mutex);
	/*printf("runtime_gui history length is %i, current index %i\n",history->len,current_index);*/
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
			default:
				break;
		}


		/* If changed by more than 5% or has been at least 5 
		 * times withot an update or forced_update is set
		 * */
		if ((slider->textval) && ((abs(count-last_upd) > 2) || (forced_update)))
		{
			tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);

			gtk_entry_set_text(GTK_ENTRY(slider->textval),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}
		slider->last_percentage = percentage;
	}
	else if (slider->textval && ((abs(count-last_upd)%30) == 0))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);

		gtk_entry_set_text(GTK_ENTRY(slider->textval),tmpbuf);
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


/*!
 \brief rtt_update_values() is called for each runtime text to update
 it's label (label is periodic and not every time due to pango
 speed problems)
 \param key (gpointer) unused
 \param value (gpointer) pointer to Rt_Slider
 \param data (gpointer) unused
 */
void rtt_update_values(gpointer key, gpointer value, gpointer data)
{
	Rt_Text *rtt = (Rt_Text *)value;
	gint count = rtt->count;
	gint rate = rtt->rate;
	gint last_upd = rtt->last_upd;
	gint current_index = 0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;


	history = (GArray *)OBJ_GET(rtt->object,"history");
	current_index = (gint)OBJ_GET(rtt->object,"current_index");
	precision = (gint)OBJ_GET(rtt->object,"precision");
	g_static_mutex_lock(&rtv_mutex);
	current = g_array_index(history, gfloat, current_index);
	if (current_index > 0)
		current_index-=1;
	previous = g_array_index(history, gfloat, current_index);
	g_static_mutex_unlock(&rtv_mutex);

	if ((current != previous) || (forced_update))
	{
		/* If changed by more than 5% or has been at least 5 
		 * times withot an update or forced_update is set
		 * */
		/*if ((rtt->textval) && ((abs(count-last_upd) > 2) || (forced_update)))*/
		{
			tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);

			gtk_label_set_text(GTK_LABEL(rtt->textval),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}
	}
	else if (rtt->textval && ((abs(count-last_upd)%30) == 0))
	{
		tmpbuf = g_strdup_printf("%1$.*2$f",current,precision);

		gtk_label_set_text(GTK_LABEL(rtt->textval),tmpbuf);
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
	rtt->rate = rate;
	rtt->count = count;
	rtt->last_upd = last_upd;
	return;
}
