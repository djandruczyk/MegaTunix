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
#include <default_limits.h>
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
	gfloat *fl_ptr, *l_fl_ptr;
	gushort *sh_ptr, *l_sh_ptr;
	guchar *uc_ptr, *l_uc_ptr;	
	extern struct Runtime_Common *runtime;
	struct Rt_Control *control = (struct Rt_Control *)value;
	gint offset = control->runtime_offset;
	//gfloat lower = def_limits[control->limits_index].lower;
	gfloat upper = def_limits[control->limits_index].upper;
	gint count = control->count;
	gint rate = control->rate;
	gint last_upd = control->last_upd;
	gint ivalue = 0;
	gshort svalue = 0;
	gfloat fvalue = 0.0;
	gfloat tmpf;
	gchar * tmpbuf = NULL;
	struct Runtime_Common * rt_last = (struct Runtime_Common *)data;

	if (control->size == UCHAR)
	{
		uc_ptr = (guchar *) runtime;
		l_uc_ptr = (guchar *) rt_last;
		if ((uc_ptr[offset/UCHAR] != l_uc_ptr[offset/UCHAR]) || (forced_update))
		{
			ivalue = uc_ptr[offset/UCHAR];
			tmpf = (gfloat)ivalue/upper <= 1.0 ? (gfloat)ivalue/upper : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(control->pbar),
					tmpf);

			if ((abs(count-last_upd) > 5) || (forced_update))
			{
				tmpbuf = g_strdup_printf("%i",uc_ptr[offset/UCHAR]);
				gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
				g_free(tmpbuf);
				last_upd = count;
			}
		}
		else if ((count % 30 == 0))
		{
			tmpbuf = g_strdup_printf("%i",uc_ptr[offset/UCHAR]);
			gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}

	}
	else if (control->size == SHORT)
	{
		sh_ptr = (gushort *) runtime;
		l_sh_ptr = (gushort *) rt_last;
		if ((sh_ptr[offset/SHORT] != l_sh_ptr[offset/SHORT]) || (forced_update))
		{
			svalue = sh_ptr[offset/SHORT];
			tmpf = (gfloat)svalue/upper <= 1.0 ? (gfloat)svalue/upper : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(control->pbar),
					tmpf);

			if ((abs(count-last_upd) > 5) || (forced_update))
			{
				tmpbuf = g_strdup_printf("%i",sh_ptr[offset/SHORT]);
				gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
				g_free(tmpbuf);
				last_upd = count;
			}
		}
		else if ((count % 30 == 0))
		{
			tmpbuf = g_strdup_printf("%i",sh_ptr[offset/SHORT]);
			gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}

	}
	else if (control->size == FLOAT)
	{
		fl_ptr = (gfloat *) runtime;
		l_fl_ptr = (gfloat *) rt_last;
		if ((fl_ptr[offset/FLOAT] != l_fl_ptr[offset/FLOAT]) || (forced_update))
		{
			fvalue = fl_ptr[offset/FLOAT];
			tmpf = fvalue/upper <= 1.0 ? fvalue/upper : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(control->pbar),
					tmpf);

			if ((abs(count-last_upd) > 5) || (forced_update))
			{
				tmpbuf = g_strdup_printf("%.2f",fl_ptr[offset/FLOAT]);
				gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
				g_free(tmpbuf);
				last_upd = count;
			}
		}
		else if ((count % 30 == 0))
		{
			tmpbuf = g_strdup_printf("%.2f",fl_ptr[offset/FLOAT]);
			gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
			g_free(tmpbuf);
			last_upd = count;
		}
	}
	else
		dbg_func(g_strdup_printf(__FILE__": rt_update_values()\n\tMAJOR error, size invalid: %i\n",control->size),CRITICAL);

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
