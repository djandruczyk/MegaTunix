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
#include <ms_structures.h>
#include <rtv_processor.h>
#include <runtime_gui.h>
#include <structures.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>

extern gboolean connected;
extern gint active_page;
extern GdkColor white;
extern GdkColor black;
extern GdkColor red;

GtkWidget *rt_table[4];
gboolean forced_update;

gboolean update_runtime_vars()
{
	gint i = 0;
	struct Ve_View_3D * ve_view = NULL;
	extern GList ***ve_widgets;
	extern GHashTable *rt_controls;
	extern GHashTable *ww_controls;
	GtkWidget * tmpwidget=NULL;
	extern struct Firmware_Details *firmware;
	extern GHashTable * dynamic_widgets;
	extern struct RtvMap *rtv_map;
	gfloat coolant = 0.0;
	static gfloat last_coolant = 0.0;
	static GObject *eng_obj = NULL;
	static gfloat *history = NULL;
	static gint hist_max = 0;
	gint last_engine_entry = 0;
	union engine engine_val = {0};
	static union engine last_engine_val;


	if (!eng_obj)
	{
		eng_obj = g_hash_table_lookup(rtv_map->rtv_hash,"enginebit");
		history = (gfloat *)g_object_get_data(eng_obj,"history");
		hist_max = (gint)g_object_get_data(eng_obj,"hist_max");
	}
	last_engine_entry = (gint)g_object_get_data(eng_obj,"last_entry");
	
	gdk_threads_enter();


	/* If OpenGL window is open, redraw it... */
	for (i=0;i<firmware->total_pages;i++)
	{
		tmpwidget = g_list_nth_data(ve_widgets[i][0],0);
		ve_view = (struct Ve_View_3D *)g_object_get_data(
				G_OBJECT(tmpwidget),"ve_view");
		if ((ve_view != NULL) && (ve_view->drawing_area->window != NULL)) 
			gdk_window_invalidate_rect (ve_view->drawing_area->window, &ve_view->drawing_area->allocation, FALSE);
	}

	engine_val = (union engine)(guchar)history[last_engine_entry];

	/* Update all the dynamic RT controls */
	if (active_page == RUNTIME_PAGE)	/* Runtime display is visible */
	{
		g_hash_table_foreach(rt_controls,rt_update_values,NULL);

		/* Status boxes.... */
		/* "Connected" */
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_connected_label"),connected);
				
		if ((forced_update) || (engine_val.value != last_engine_val.value))
		{
			/* Cranking */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_cranking_label"),engine_val.bit.crank);
					
			/* Running */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_running_label"),engine_val.bit.running);
					
			/* Warmup */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_warmup_label"),engine_val.bit.warmup);
					
			/* Afterstart Enrichment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_ase_label"),engine_val.bit.ase);
					
			/* Accel Enrichment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_accel_label"),engine_val.bit.tpsaccel);
					
			/* Decel Enleanment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_decel_label"),engine_val.bit.tpsdecel);
					

			
		}
	}

	if (active_page == WARMUP_WIZ_PAGE)	/* Warmup wizard is visible */
	{
		g_hash_table_foreach(ww_controls,rt_update_values,NULL);

		if (!lookup_current_value("cltdeg",&coolant))
			dbg_func(__FILE__": update_runtime_vars()\n\t Error getting current value of \"cltdeg\" from datasource\n",CRITICAL);
		if ((coolant != last_coolant) || (forced_update))
			warmwizard_update_status(coolant);

		/* Status boxes.... */
		/* "Connected" */
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_connected_label"),connected);
				
		if ((forced_update) || (engine_val.value != last_engine_val.value))
		{
			/* Cranking */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_cranking_label"),engine_val.bit.crank);
					
			/* Running */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_running_label"),engine_val.bit.running);
					
			/* Warmup */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_warmup_label"),engine_val.bit.warmup);
					
			/* Afterstart Enrichment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_ase_label"),engine_val.bit.ase);
					
			/* Accel Enrichment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_accel_label"),engine_val.bit.tpsaccel);
					
			/* Decel Enleanment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_decel_label"),engine_val.bit.tpsdecel);
					
		}
		last_coolant = coolant;
	}

	last_engine_val = engine_val;

	gdk_threads_leave();
	return TRUE;
}

void reset_runtime_status()
{
	extern GHashTable *dynamic_widgets;
	// Runtime screen
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_connected_label"),connected);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_cranking_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_running_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_warmup_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_ase_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_accel_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_decel_label"),FALSE);

	// Warmup wizard....
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_connected_label"),connected);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_cranking_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_running_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_warmup_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_ase_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_accel_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"ww_decel_label"),FALSE);

}

void rt_update_values(gpointer key, gpointer value, gpointer data)
{
	struct Rt_Control *control = (struct Rt_Control *)value;
	gint count = control->count;
	gint rate = control->rate;
	gint last_upd = control->last_upd;
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

	last_entry = (gint)g_object_get_data(control->object,"last_entry");
	hist_size = (gint)g_object_get_data(control->object,"hist_max");
	is_float = (gboolean)g_object_get_data(control->object,"is_float");
	now = control->history[last_entry];
	if (last_entry == 0)
		last = control->history[hist_size-1];
	else
		last = control->history[(last_entry-1)];

	upper = (gfloat)control->upper;
	lower = (gfloat)control->lower;
	
	if ((now != last) || (forced_update))
	{
		percentage = (now-lower)/(upper-lower);
		tmpf = percentage <= 1.0 ? percentage : 1.0;
		tmpf = tmpf >= 0.0 ? tmpf : 0.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(control->pbar),
				tmpf);

		if ((abs(count-last_upd) > 5) || (forced_update))
		{
			if (is_float)
				tmpbuf = g_strdup_printf("%.2f",now);
			else
				tmpbuf = g_strdup_printf("%i",(gint)now);
			
			gtk_label_set_text(GTK_LABEL(control->textval),tmpbuf);
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
		gtk_label_set_text(GTK_LABEL(control->textval),tmpbuf);
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
	control->rate = rate;
	control->count = count;
	control->last_upd = last_upd;
	return;
}
