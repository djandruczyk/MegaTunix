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
#include <datalogging_const.h>
#include <defines.h>
#include <enums.h>
#include <glib/gprintf.h>
#include <gui_handlers.h>
#include <ms_structures.h>
#include <runtime_gui.h>
#include <structures.h>
#include <vetable_gui.h>
#include <warmwizard_gui.h>


extern struct DynamicEntries entries;
extern struct DynamicLabels labels;
struct DynamicProgress progress;
extern struct DynamicMisc misc;
const gchar *status_msgs[] = {	"CONNECTED","CRANKING","RUNNING","WARMUP",
				"AS_ENRICH","ACCEL","DECEL"};
gboolean force_status_update = TRUE;
extern gboolean connected;
extern gboolean forced_update;
extern GdkColor white;
extern GdkColor black;
extern struct DynamicLabels labels;
gfloat ego_pbar_divisor = 5.0;	/* Initially assume a Wideband Sensor */
gfloat map_pbar_divisor = 255.0;/* Initially assume a Turbo MAP Sensor */
GtkWidget *rt_table[4];


void build_runtime(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *ebox;
	extern GtkTooltips *tip;
	gint i=0;

	/* ugly hack to prevent a compiler warning,  find a better way to
	 * suppress the warning than this shit and tell me how to do it :)
	 */
	gint junk = sizeof(mt_classic_names)/sizeof(gchar *);
	junk = sizeof(mt_full_names)/sizeof(gchar *);
	junk = sizeof(logable_names)/sizeof(gchar *);
	junk = sizeof(logable_names_tips)/sizeof(gchar *);
	/* End of ugly hack, beginning of spaghetti code.. :) */

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Real-Time Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	rt_table[0] = gtk_table_new(10,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(rt_table[0]),1);
	gtk_table_set_col_spacings(GTK_TABLE(rt_table[0]),5);
	gtk_container_set_border_width (GTK_CONTAINER (rt_table[0]), 5);
	gtk_box_pack_start(GTK_BOX(hbox),rt_table[0],TRUE,TRUE,0);


	/* Second column */

	rt_table[1] = gtk_table_new(10,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(rt_table[1]),1);
	gtk_table_set_col_spacings(GTK_TABLE(rt_table[1]),5);
	gtk_container_set_border_width (GTK_CONTAINER (rt_table[1]), 5);
	gtk_box_pack_start(GTK_BOX(hbox),rt_table[1],TRUE,TRUE,0);


	/* Corrections/Enrichments frame */

	frame = gtk_frame_new("Corrections/Enrichments (Percent)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	rt_table[2] = gtk_table_new(5,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(rt_table[2]),1);
	gtk_table_set_col_spacings(GTK_TABLE(rt_table[2]),5);
	gtk_container_set_border_width (GTK_CONTAINER (rt_table[2]), 5);
	gtk_box_pack_start(GTK_BOX(hbox),rt_table[2],TRUE,TRUE,0);

	rt_table[3] = gtk_table_new(5,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(rt_table[3]),1);
	gtk_table_set_col_spacings(GTK_TABLE(rt_table[3]),5);
	gtk_container_set_border_width (GTK_CONTAINER (rt_table[3]), 5);
	gtk_box_pack_start(GTK_BOX(hbox),rt_table[3],TRUE,TRUE,0);

	ebox = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,TRUE,0);
	gtk_tooltips_set_tip(tip,ebox,
			"This block shows you statistics on the number of good reads of the VE/Constants datablocks, RealTime datablocks and the MegaSquirt hard reset and Serial I/O error counts.  Hard resets are indicative of power problems or excessive electrical noise to the MS (causing cpu resets).  Serial I/O errors are indicative of a poor cable connection between this host computer and the MS.",NULL);

	frame = gtk_frame_new("MegaSquirt I/O Status");
	gtk_container_add(GTK_CONTAINER(ebox),frame);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(3,4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,5);

	label = gtk_label_new("Good VE/Constants Reads");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new();
	entries.runtime_ve_readcount_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);


	label = gtk_label_new("Good RealTime Reads");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new();
	entries.runtime_readcount_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Hard Reset Count");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new();
	entries.runtime_reset_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Serial I/O Error Count");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new();
	entries.runtime_sioerr_entry = entry;
	gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);
	gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), entry, 3, 4, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Reset Status Counters...");
	g_signal_connect_swapped(G_OBJECT (button), "clicked",
			G_CALLBACK (reset_errcounts), \
			NULL);
	gtk_table_attach (GTK_TABLE (table), button, 0, 4, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Start Reading RT Vars");
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(START_REALTIME));
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Stop Reading RT vars");
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), \
			GINT_TO_POINTER(STOP_REALTIME));
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Runtime Status");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);
	hbox = gtk_hbox_new(TRUE,3);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,TRUE,TRUE,0);
	for (i=0;i<7;i++)
	{
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
		misc.status[i] = gtk_label_new(status_msgs[i]);
		gtk_widget_set_sensitive(misc.status[i],FALSE);
		gtk_container_add(GTK_CONTAINER(frame),misc.status[i]);
		gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);
	}

	return;
}

gboolean update_runtime_vars()
{
	static gint count = 0;
	extern struct Runtime_Common *runtime;
	extern struct Runtime_Common *runtime_last;
	extern unsigned int ecu_caps;
	struct Ve_View_3D * ve_view0 = NULL;
	struct Ve_View_3D * ve_view1 = NULL;
	struct Ve_View_3D * ve_view2 = NULL;
	extern GtkWidget *ve_widgets[];
	extern GtkWidget *ign_widgets[];
	extern GHashTable *rt_controls;
	gchar * tmpbuf = NULL;
	gfloat tmpf = 0.0;

	gdk_threads_enter();

	ve_view0 = (struct Ve_View_3D *)g_object_get_data(
				G_OBJECT(ve_widgets[0]),"data");
	ve_view1 = (struct Ve_View_3D *)g_object_get_data(
				G_OBJECT(ve_widgets[0+MS_PAGE_SIZE]),"data");
	if (ecu_caps & (S_N_SPARK|S_N_EDIS))
		ve_view2 = (struct Ve_View_3D *)g_object_get_data(
					G_OBJECT(ign_widgets[0]),"data");
	/* Count is used  to force an update after 5 runs EVEN IF the 
	 * value hasn't changed.  seems to fix a "stuck bar" I've seen
	 */
	count++;
	
	/* The additional NULL test is to avoid a timing-based problem
	 * where ve_view can exist, but the window doesn't yet.
	 * It's a small window, but I hit it several times.
	 */

	if ((ve_view0 != NULL) && (ve_view0->drawing_area->window != NULL)) 
	        gdk_window_invalidate_rect (ve_view0->drawing_area->window, 
					&ve_view0->drawing_area->allocation, 
					FALSE);

	if ((ve_view1 != NULL) && (ve_view1->drawing_area->window != NULL)) 
	        gdk_window_invalidate_rect (ve_view1->drawing_area->window, 
					&ve_view1->drawing_area->allocation, 
					FALSE);
	if (ecu_caps & (S_N_SPARK|S_N_EDIS))
	{
		if ((ve_view2 != NULL) && (ve_view2->drawing_area->window != NULL)) 
			gdk_window_invalidate_rect (ve_view2->drawing_area->window, 
					&ve_view2->drawing_area->allocation, 
					FALSE);
	}
	
	
	/* Color the boxes on the VEtable closest to the operating point */


	/* Update allthe dynamic RT controls */
	g_hash_table_foreach(rt_controls,rt_update_values,NULL);

	/* Update all the controls on the warmup wizrd page... */
	if ((runtime->ego_volts != runtime_last->ego_volts) || (forced_update) || (count > 5))
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
	if ((runtime->map != runtime_last->map) || (count > 5))
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
	if ((runtime->clt != runtime_last->clt) || (forced_update) || (count > 5))
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
	if ((runtime->warmcorr != runtime_last->warmcorr) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",(int)runtime->warmcorr);
		gtk_label_set_text(GTK_LABEL(labels.ww_warmcorr_lab),tmpbuf);
		tmpf = runtime->warmcorr/255.0 <= 1.0 ? runtime->warmcorr/255.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.ww_warmcorr_pbar),
				tmpf);
		g_free(tmpbuf);
	}

	if (ecu_caps & DUALTABLE)
	{
		/* Color the boxes on the VEtable closest to the operating point */

	}

	/* Connected */
	gtk_widget_set_sensitive(misc.status[CONNECTED],
			connected);

	if ((forced_update) || (runtime->engine.value != runtime_last->engine.value) || (count > 5))
	{
		/* Cranking */
		gtk_widget_set_sensitive(misc.status[CRANKING],
				runtime->engine.bit.crank);
		gtk_widget_set_sensitive(misc.ww_status[CRANKING],
				runtime->engine.bit.crank);
		/* Running */
		gtk_widget_set_sensitive(misc.status[RUNNING],
				runtime->engine.bit.running);
		gtk_widget_set_sensitive(misc.ww_status[RUNNING],
				runtime->engine.bit.running);
		/* Warmup */
		gtk_widget_set_sensitive(misc.status[WARMUP],
				runtime->engine.bit.warmup);
		gtk_widget_set_sensitive(misc.ww_status[WARMUP],
				runtime->engine.bit.warmup);
		/* Afterstart Enrichment */
		gtk_widget_set_sensitive(misc.status[AS_ENRICH],
				runtime->engine.bit.startw);
		gtk_widget_set_sensitive(misc.ww_status[AS_ENRICH],
				runtime->engine.bit.startw);
		/* Accel Enrichment */
		gtk_widget_set_sensitive(misc.status[ACCEL],
				runtime->engine.bit.tpsaen);
		gtk_widget_set_sensitive(misc.ww_status[ACCEL],
				runtime->engine.bit.tpsaen);
		/* Decel Enleanment */
		gtk_widget_set_sensitive(misc.status[DECEL],
				runtime->engine.bit.tpsden);
		gtk_widget_set_sensitive(misc.ww_status[DECEL],
				runtime->engine.bit.tpsden);

	}
	if (forced_update)
		forced_update = FALSE;
	if (count > 5)
		count = 0;
	gdk_threads_leave();
	return TRUE;
}

void reset_runtime_status()
{
	gint i;
	for(i=1;i<7;i++)
	{
		gtk_widget_set_sensitive(misc.status[i],
				FALSE);
		gtk_widget_set_sensitive(misc.ww_status[i],
				FALSE);
	}
}

void rt_update_values(gpointer key, gpointer value, gpointer data)
{
 	gfloat *fl_ptr, *l_fl_ptr;
	unsigned short *sh_ptr, *l_sh_ptr;
	unsigned char *uc_ptr, *l_uc_ptr;	
	extern struct Runtime_Common *runtime;
	extern struct Runtime_Common *runtime_last;
	struct Rt_Control *control = (struct Rt_Control *)value;
	gint offset = control->runtime_offset;
	//gfloat lower = limits[control->limits_index].lower;
	gfloat upper = limits[control->limits_index].upper;
	gint ivalue = 0;
	gshort svalue = 0;
	gfloat fvalue = 0.0;
	gfloat tmpf;
	gchar * tmpbuf = NULL;
	gint count = control->count++;
	
	if (control->size == UCHAR)
	{
		uc_ptr = (unsigned char *) runtime;
		l_uc_ptr = (unsigned char *) runtime_last;
		if ((uc_ptr[offset/UCHAR] != l_uc_ptr[offset/UCHAR]) || (count > 5))
		{
			ivalue = uc_ptr[offset];
			tmpbuf = g_strdup_printf("%i",ivalue);
			gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
			tmpf = (float)ivalue/upper <= 1.0 ? (float)ivalue/upper : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(control->pbar),
					tmpf);
			g_free(tmpbuf);
			if (control->count > 5)
				control->count = 0;
		}
	}
	else if (control->size == SHORT)
	{
		sh_ptr = (short *) runtime;
		l_sh_ptr = (short *) runtime_last;
		if ((sh_ptr[offset/SHORT] != l_sh_ptr[offset/SHORT]) || (count > 5))
		{
			svalue = sh_ptr[offset/SHORT];
			tmpbuf = g_strdup_printf("%i",svalue);
			gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
			tmpf = (float)svalue/upper <= 1.0 ? (float)svalue/upper : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(control->pbar),
					tmpf);
			g_free(tmpbuf);
			if (control->count > 5)
				control->count = 0;
		}
	}
	else if (control->size == FLOAT)
	{
		fl_ptr = (float *) runtime;
		l_fl_ptr = (float *) runtime_last;
		if ((fl_ptr[offset/FLOAT] != l_fl_ptr[offset/FLOAT]) || (count > 5))
		{
			fvalue = fl_ptr[offset/FLOAT];
			tmpbuf = g_strdup_printf("%.2f",fvalue);
			gtk_label_set_text(GTK_LABEL(control->data),tmpbuf);
			tmpf = fvalue/upper <= 1.0 ? fvalue/upper : 1.0;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(control->pbar),
					tmpf);
			g_free(tmpbuf);
			if (control->count > 5)
				control->count = 0;
		}
	}
	else
		g_fprintf(stderr,__FILE__": MAJOR error, rt_update_values(), size invalid: %i\n",control->size);

	return;
}
