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
#include <runtime_gui.h>
#include <structures.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>

extern struct DynamicEntries entries;
struct DynamicLabels labels;
struct DynamicMisc misc;
extern gboolean connected;
extern gint active_page;
extern GdkColor white;
extern GdkColor black;
extern GdkColor red;

struct DynamicProgress progress;
GtkWidget *rt_table[4];
gboolean forced_update;
gfloat ego_pbar_divisor = 5.0;	/* Initially assume a Wideband Sensor */
gfloat map_pbar_divisor = 255.0;/* Initially assume a Turbo MAP Sensor */
const gchar *status_msgs[] = {	"CONNECTED","CRANKING","RUNNING","WARMUP",
				"AS_ENRICH","ACCEL","DECEL"};



gboolean update_runtime_vars()
{
	gint i = 0;
	extern struct Runtime_Common *runtime;
	extern gint ecu_caps;
	struct Ve_View_3D * ve_view = NULL;
	extern GList *ve_widgets[MAX_SUPPORTED_PAGES][2*MS_PAGE_SIZE];
	extern GHashTable *rt_controls;
	gchar * tmpbuf = NULL;
	gfloat tmpf = 0.0;
	GtkWidget * tmpwidget=NULL;
	static struct Runtime_Common *rt_last = NULL;	
	extern struct Firmware_Details *firmware;
	extern GHashTable * dynamic_widgets;

	if (rt_last == NULL)
		rt_last = g_malloc0(sizeof(struct Runtime_Common));

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


	/* Update all the dynamic RT controls */
	if (active_page == RUNTIME_PAGE)	/* Runtime display is visible */
	{
		g_hash_table_foreach(rt_controls,rt_update_values,rt_last);

		/* Status boxes.... */
		/* "Connected" */
		gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_connected_label"),connected);
				

		if ((forced_update) || (runtime->engine.value != rt_last->engine.value))
		{
			/* Cranking */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_cranking_label"),runtime->engine.bit.crank);
					
			/* Running */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_running_label"),runtime->engine.bit.running);
					
			/* Warmup */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_warmup_label"),runtime->engine.bit.warmup);
					
			/* Afterstart Enrichment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_as_enrich_label"),runtime->engine.bit.startw);
					
			/* Accel Enrichment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_accel_label"),runtime->engine.bit.tpsaen);
					
			/* Decel Enleanment */
			gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_decel_label"),runtime->engine.bit.tpsden);
					

		}
	}

	if (active_page == WARMUP_WIZ_PAGE) /* Warmup wizard visible... */
	{
		/* Update all the controls on the warmup wizrd page... */
		if ((runtime->ego_volts != rt_last->ego_volts) || (forced_update))
		{
			tmpbuf = g_strdup_printf("%.2f",runtime->ego_volts);
			gtk_label_set_text(GTK_LABEL(labels.ww_ego_lab),tmpbuf);
			tmpf = runtime->ego_volts/ego_pbar_divisor <= 1.0 
				? runtime->ego_volts/ego_pbar_divisor: 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.ww_ego_pbar),
					tmpf);
			g_free(tmpbuf);
		}
		if ((runtime->map != rt_last->map) || (forced_update))
		{
			tmpbuf = g_strdup_printf("%i",(int)runtime->map);
			gtk_label_set_text(GTK_LABEL(labels.ww_map_lab),tmpbuf);
			tmpf = runtime->map/map_pbar_divisor <= 1.0 
				? runtime->map/map_pbar_divisor: 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.ww_map_pbar),
					tmpf);
			g_free(tmpbuf);
		}
		if ((runtime->clt != rt_last->clt) || (forced_update))
		{
			tmpbuf = g_strdup_printf("%i",(int)runtime->clt);
			gtk_label_set_text(GTK_LABEL(labels.ww_clt_lab),tmpbuf);
			tmpf = runtime->clt/215.0 <= 1.0 ? runtime->clt/215.0 : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.ww_clt_pbar),
					tmpf);
			g_free(tmpbuf);
			warmwizard_update_status(runtime->clt);
		}
		if ((runtime->warmcorr != rt_last->warmcorr) || (forced_update))
		{
			tmpbuf = g_strdup_printf("%i",(int)runtime->warmcorr);
			gtk_label_set_text(GTK_LABEL(labels.ww_warmcorr_lab),tmpbuf);
			tmpf = runtime->warmcorr/255.0 <= 1.0 ? runtime->warmcorr/255.0: 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.ww_warmcorr_pbar),
					tmpf);
			g_free(tmpbuf);
		}
		if ((forced_update) || (runtime->engine.value != rt_last->engine.value))
		{
			/* Cranking */
			gtk_widget_set_sensitive(misc.ww_status[STAT_CRANKING],
					runtime->engine.bit.crank);
			/* Running */
			gtk_widget_set_sensitive(misc.ww_status[STAT_RUNNING],
					runtime->engine.bit.running);
			/* Warmup */
			gtk_widget_set_sensitive(misc.ww_status[STAT_WARMUP],
					runtime->engine.bit.warmup);
			/* Afterstart Enrichment */
			gtk_widget_set_sensitive(misc.ww_status[STAT_AS_ENRICH],
					runtime->engine.bit.startw);
			/* Accel Enrichment */
			gtk_widget_set_sensitive(misc.ww_status[STAT_ACCEL],
					runtime->engine.bit.tpsaen);
			/* Decel Enleanment */
			gtk_widget_set_sensitive(misc.ww_status[STAT_DECEL],
					runtime->engine.bit.tpsden);

		}
	}

	if (ecu_caps & DUALTABLE)
	{
		/* Color the boxes on the VEtable closest to the operating point */

	}
	//	if (forced_update)
	//		forced_update = FALSE;
	memcpy(rt_last, runtime,sizeof(struct Runtime_Common));
	gdk_threads_leave();
	return TRUE;
}

void reset_runtime_status()
{
	extern GHashTable *dynamic_widgets;
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_connected_label"),connected);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_cranking_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_running_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_warmup_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_as_enrich_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_accel_label"),FALSE);
	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"runtime_decel_label"),FALSE);

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
