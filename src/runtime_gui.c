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
#include <progress.h>
#include <rtv_map_loader.h>
#include <rtv_processor.h>
#include <runtime_gui.h>
#include <runtime_sliders.h>
#include <runtime_text.h>
#include <stdlib.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>
#include <widgetmgmt.h>

extern gboolean connected;
extern gint active_page;
extern GdkColor white;
extern GdkColor black;
extern GdkColor red;
extern GObject *global_data;

gboolean forced_update = TRUE;
gboolean rt_forced_update = TRUE;
GStaticMutex rtv_mutex = G_STATIC_MUTEX_INIT;
GStaticMutex rtt_mutex = G_STATIC_MUTEX_INIT;


/*!
 \brief update_runtime_vars_pf() updates all of the runtime sliders on all
 visible portions of the gui
 */
EXPORT gboolean update_runtime_vars_pf()
{
	static gint count = 0;
	static gboolean conn_status = FALSE;
	extern gboolean interrogated;
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;

	if (!interrogated)
		return FALSE;

	count++;
	if (conn_status != connected)
	{
		g_list_foreach(get_list("connected_widgets"),set_widget_sensitive,GINT_TO_POINTER(connected));
		conn_status = connected;
		forced_update = TRUE;
	}
	if ((count > 60) && (!forced_update))
		forced_update = TRUE;

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
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;

	g_return_if_fail(GTK_IS_WIDGET(widget));

	source = (gchar *)OBJ_GET(widget,"source");
	if ((g_strcasecmp(source,last_source) != 0))
	{
		object = NULL;
		object = (GObject *)g_hash_table_lookup(rtv_map->rtv_hash,source);
		if (!object)
			return;
		history = (GArray *)OBJ_GET(object,"history");
		if (!history)
			return;
	}
	if (!connected)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
		return;
	}

	if (lookup_current_value(source,&tmpf))
		value = (gint) tmpf;
	else
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": rt_update_status()\n\t COULD NOT get current value for %s\n",source));
	if (lookup_previous_value(source,&tmpf))
		previous_value = (gint) tmpf;
	else
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": rt_update_status()\n\t COULD NOT get previous value for %s\n",source));

	bitval = (GINT)OBJ_GET(widget,"bitval");
	bitmask = (GINT)OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift(bitmask);


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
	gint last_upd = slider->last_upd;
	gfloat tmpf = 0.0;
	gfloat upper = 0.0;
	gfloat lower = 0.0;
	gint precision = 0;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	gfloat percentage = 0.0;
	GArray *history = NULL;
	gchar * tmpbuf = NULL;
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;

	history = (GArray *)OBJ_GET(slider->object,"history");
	if (!history)
		return;
	if ((gint)history->len-1 <= 0)
		return;
	precision = (GINT)OBJ_GET(slider->object,"precision");
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rt_update_values() before lock rtv_mutex\n"));
	g_static_mutex_lock(&rtv_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rt_update_values() after lock rtv_mutex\n"));
	/*printf("runtime_gui history length is %i, current index %i\n",history->len,history->len-1);*/
	current = g_array_index(history, gfloat, history->len-1);
	previous = slider->last;
	slider->last = current;
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rt_update_values() before UNlock rtv_mutex\n"));
	g_static_mutex_unlock(&rtv_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": rt_update_values() after UNlock rtv_mutex\n"));

	upper = (gfloat)slider->upper;
	lower = (gfloat)slider->lower;
	
	gdk_threads_enter();
	if ((current != previous) || (rt_forced_update))
	{
		percentage = (current-lower)/(upper-lower);
		tmpf = percentage <= 1.0 ? percentage : 1.0;
		tmpf = tmpf >= 0.0 ? tmpf : 0.0;
		switch (slider->class)
		{
			case MTX_PROGRESS:
				mtx_progress_bar_set_fraction(MTX_PROGRESS_BAR
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
		if ((slider->textval) && ((abs(count-last_upd) > 2) || (rt_forced_update)))
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

	gdk_threads_leave();
	if (last_upd > 5000)
		last_upd = 0;
	count++;
	if (count > 5000)
		count = 0;
	slider->count = count;
	slider->last_upd = last_upd;
	return;
}


gboolean update_rtsliders(gpointer data)
{
	gfloat coolant = 0.0;
	static gfloat last_coolant = 0.0;
	gint i = 0;
	GHashTable **ve3d_sliders;
	GHashTable *hash;
	extern Firmware_Details *firmware;
	ve3d_sliders = OBJ_GET(global_data,"ve3d_sliders");
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;

	/* Update all the dynamic RT Sliders */
	if (active_page == RUNTIME_TAB)	/* Runtime display is visible */
		if ((hash = OBJ_GET(global_data,"rt_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);
	if (active_page == ENRICHMENTS_TAB)	/* Enrichments display is up */
		if ((hash = OBJ_GET(global_data,"enr_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);
	if (active_page == ACCEL_WIZ_TAB)	/* Enrichments display is up */
		if ((hash = OBJ_GET(global_data,"aw_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);
	if (active_page == WARMUP_WIZ_TAB)	/* Warmup wizard is visible */
	{
		if ((hash = OBJ_GET(global_data,"ww_sliders")))
			g_hash_table_foreach(hash,rt_update_values,NULL);

		if (!lookup_current_value("cltdeg",&coolant))
			dbg_func(CRITICAL,g_strdup(__FILE__": update_runtime_vars_pf()\n\t Error getting current value of \"cltdeg\" from datasource\n"));
		if ((coolant != last_coolant) || (forced_update))
			warmwizard_update_status(coolant);
		last_coolant = coolant;
	}
	if (ve3d_sliders)
	{
		for (i=0;i<firmware->total_tables;i++)
		{
			if (ve3d_sliders[i])
				g_hash_table_foreach(ve3d_sliders[i],rt_update_values,NULL);
		}
	}
	return TRUE;
}


gboolean update_rttext(gpointer data)
{
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;
	g_static_mutex_lock(&rtt_mutex);
	if (OBJ_GET(global_data,"rtt_model"))
		gtk_tree_model_foreach(GTK_TREE_MODEL(OBJ_GET(global_data,"rtt_model")),rtt_foreach,NULL);
	if (OBJ_GET(global_data,"rtt_hash"))
		g_hash_table_foreach(OBJ_GET(global_data,"rtt_hash"),rtt_update_values,NULL);
	g_static_mutex_unlock(&rtt_mutex);
	return TRUE;
}


gboolean update_dashboards(gpointer data)
{
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;
	extern GStaticMutex dash_mutex;

	dbg_func(MUTEX,g_strdup_printf(__FILE__": update_dashboards() before lock dash_mutex\n"));
	g_static_mutex_lock(&dash_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": update_dashboards() after lock dash_mutex\n"));
	if (OBJ_GET(global_data,"dash_hash"))
		g_hash_table_foreach(OBJ_GET(global_data,"dash_hash"),update_dash_gauge,NULL);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": update_dashboards() before UNlock dash_mutex\n"));
	g_static_mutex_unlock(&dash_mutex);
	dbg_func(MUTEX,g_strdup_printf(__FILE__": update_dashboards() after UNlock dash_mutex\n"));
	return TRUE;
}


gboolean update_ve3ds(gpointer data)
{
	gint i = 0;
	Ve_View_3D * ve_view = NULL;
	GtkWidget * tmpwidget=NULL;
	extern Firmware_Details *firmware;
	gfloat x,y,z = 0.0;
	static gfloat xl,yl,zl = 9999.9;
	gchar * string = NULL;
	GHashTable *hash = NULL;
	extern GHashTable *sources_hash;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	MultiSource *multi = NULL;
	extern gint * algorithm;
	extern volatile gboolean leaving;
	if (leaving)
		return FALSE;

	if (!firmware)
		return TRUE;
	/* If OpenGL window is open, redraw it... */
	for (i=0;i<firmware->total_tables;i++)
	{
		string = g_strdup_printf("ve_view_%i",i);
		tmpwidget = lookup_widget(string);
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
						printf(_("multi is null!!\n"));

					lookup_current_value(multi->source,&x);
				}
				else
					lookup_current_value(ve_view->x_source,&x);

				/* Test X values, redraw if needed */
				if (((fabs(x-xl)/x) > 0.001) || (forced_update))
				{
					xl = x;
					goto redraw;
				}

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
						printf(_("multi is null!!\n"));

					lookup_current_value(multi->source,&y);
				}
				else
					lookup_current_value(ve_view->y_source,&y);

				/* Test Y values, redraw if needed */
				if (((fabs(y-yl)/y) > 0.001) || (forced_update))
				{
					yl = y;
					goto redraw;
				}

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
						printf(_("multi is null!!\n"));

					lookup_current_value(multi->source,&y);
				}
				else
					lookup_current_value(ve_view->z_source,&z);

				/* Test Z values, redraw if needed */
				if (((fabs(z-zl)/z) > 0.001) || (forced_update))
				{
					zl = z;
					goto redraw;
				}
				goto breakout;

redraw:
				gdk_threads_enter();
				gdk_window_invalidate_rect (ve_view->drawing_area->window, &ve_view->drawing_area->allocation, FALSE);
				gdk_threads_leave();
			}
		}
	}
breakout:

	gdk_threads_enter();
	if (firmware->capabilities & PIS)
	{
		if ((active_page == RUNTIME_TAB) || (active_page == SETTINGS_TAB )|| (active_page == CORRECTIONS_TAB))
			update_tab_gauges();
	}

	if ((active_page == VETABLES_TAB) || (active_page == SPARKTABLES_TAB) || (active_page == AFRTABLES_TAB) || (active_page == BOOSTTABLES_TAB) || (active_page == ROTARYTABLES_TAB) || (active_page == ALPHA_N_TAB) ||  (active_page == STAGING_TAB) || (forced_update))
	{
		draw_ve_marker();
		update_tab_gauges();
	}
	gdk_threads_leave();
	return TRUE;
}
