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
#include <enums.h>
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

void build_runtime(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *pbar;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *ebox;
	extern GtkTooltips *tip;
	extern GList * dt_widgets;
	gint i=0;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	frame = gtk_frame_new("Real-Time Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(7,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	/* Seconds Counter Label */
	label = gtk_label_new("Seconds");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* Seconds Counter from MS */
	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.secl_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.secl_pbar = pbar;

	/* O2 Voltage Label*/
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),"O<sub>2</sub> (Volts)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	/* O2 Voltage value*/
	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.ego_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.ego_pbar = pbar;

	/* Battery Voltage */
	label = gtk_label_new("Batt (Volts)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.batt_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);

	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.batt_pbar = pbar;


	/* TPS */
	label = gtk_label_new("TPS (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.tps_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 3, 4,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.tps_pbar = pbar;

	/* RPM */
	label = gtk_label_new("RPM");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.rpm_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 4, 5,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.rpm_pbar = pbar;

	/* PW 1 */
	label = gtk_label_new("Ch. 1 PW (ms)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.pw1_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 5, 6,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.pw1_pbar = pbar;

	/* Duty Cycle 2 */
	label = gtk_label_new("Ch. 1 DC (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 6, 7,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.dcycle1_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 6, 7,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.dcycle1_pbar = pbar;


	/* Second column */
	table = gtk_table_new(7,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	/* GammaE (sum of enrichments) */
	label = gtk_label_new("GammaE (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.gammae_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.gammae_pbar = pbar;

	/* MAP Sensor */	
	label = gtk_label_new("MAP (kPa)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.map_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.map_pbar = pbar;

	/* Coolant Temp */
	label = gtk_label_new("Coolant (F)");
	labels.runtime_clt_lab = label;
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.clt_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.clt_pbar = pbar;


	/* Manifold Air Temp */
	label = gtk_label_new("MAT (F)");
	labels.runtime_mat_lab = label;
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.mat_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 3, 4,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.mat_pbar = pbar;

	/* IdleDC */
	label = gtk_label_new("Idle DC (%)");
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.idledc_lab = label;

	pbar = gtk_progress_bar_new();
	dt_widgets = g_list_append(dt_widgets, (gpointer)pbar);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 4, 5,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.idledc_pbar = pbar;

	/* PW 2 */
	label = gtk_label_new("Ch. 2 PW (ms)");
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.pw2_lab = label;

	pbar = gtk_progress_bar_new();
	dt_widgets = g_list_append(dt_widgets, (gpointer)pbar);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 5, 6,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.pw2_pbar = pbar;

	/* Duty Cycle 2 */
	label = gtk_label_new("Ch. 2 DC (%)");
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	dt_widgets = g_list_append(dt_widgets, (gpointer)label);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 6, 7,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.dcycle2_lab = label;

	pbar = gtk_progress_bar_new();
	dt_widgets = g_list_append(dt_widgets, (gpointer)pbar);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 6, 7,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.dcycle2_pbar = pbar;

	/* Corrections/Enrichments frame */

	frame = gtk_frame_new("Corrections/Enrichments (Percent)");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 0);

	hbox = gtk_hbox_new(TRUE,0);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(3,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	/* EGOcorr  */
	label = gtk_label_new("EGO (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.egocorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.egocorr_pbar = pbar;


	/* Baro Correction  */
	label = gtk_label_new("Baro (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.barocorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.barocorr_pbar = pbar;


	/* Warmup Correction  */
	label = gtk_label_new("Warmup (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.warmcorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.warmcorr_pbar = pbar;

	table = gtk_table_new(3,3,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	/* Air Den Cycle */
	label = gtk_label_new("Air Density (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.aircorr_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 0, 1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.aircorr_pbar = pbar;

	/* Ve  */
	label = gtk_label_new("VE (%)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.vecurr1_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 1, 2,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.vecurr1_pbar = pbar;

	/* Accel Boost  */
	label = gtk_label_new("Accel (ms)");
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	labels.tpsaccel_lab = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table), pbar, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (0), 0, 0);
	progress.tpsaccel_pbar = pbar;

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
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(hbox),table,FALSE,TRUE,20);

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
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);

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
	gchar *tmpbuf;
	gfloat tmpf;
	static gint count = 0;
	extern struct Runtime_Common *runtime;
	extern struct Runtime_Common *runtime_last;
	extern unsigned int ecu_flags;
	struct Ve_View_3D * ve_view0 = NULL;
	struct Ve_View_3D * ve_view1 = NULL;
	extern GtkWidget *ve_widgets[];

	gdk_threads_enter();

	ve_view0 = (struct Ve_View_3D *)g_object_get_data(
				G_OBJECT(ve_widgets[0]),"data");
	ve_view1 = (struct Ve_View_3D *)g_object_get_data(
				G_OBJECT(ve_widgets[0+MS_PAGE_SIZE]),"data");
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
	
	
	/* Color the boxes on the VEtable closest to the operating point */

	/* test to see if data changed 
	 * Why bother wasting CPU to update the GUI when 
	 * you'd just print the same damn thing?
	 * Makes the code a little uglier, but the gui won't
	 * flicker the text widgets at high update rates
	 */

	if ((runtime->secl != runtime_last->secl) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->secl);
		gtk_label_set_text(GTK_LABEL(labels.secl_lab),tmpbuf);
		tmpf = runtime->secl/255.0 <= 1.0 ? runtime->secl/255.0 : 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.secl_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->ego_volts != runtime_last->ego_volts) || (forced_update) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%.2f",runtime->ego_volts);
		gtk_label_set_text(GTK_LABEL(labels.ego_lab),tmpbuf);
		gtk_label_set_text(GTK_LABEL(labels.ww_ego_lab),tmpbuf);
		tmpf = runtime->ego_volts/ego_pbar_divisor <= 1.0 
			? runtime->ego_volts/ego_pbar_divisor: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.ego_pbar),
				tmpf);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.ww_ego_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->map != runtime_last->map) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",(int)runtime->map);
		gtk_label_set_text(GTK_LABEL(labels.ww_map_lab),tmpbuf);
		gtk_label_set_text(GTK_LABEL(labels.map_lab),tmpbuf);
		tmpf = runtime->map/map_pbar_divisor <= 1.0 
			? runtime->map/map_pbar_divisor: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.ww_map_pbar),
				tmpf);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.map_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->clt != runtime_last->clt) || (forced_update) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",(int)runtime->clt);
		gtk_label_set_text(GTK_LABEL(labels.clt_lab),tmpbuf);
		gtk_label_set_text(GTK_LABEL(labels.ww_clt_lab),tmpbuf);
		tmpf = runtime->clt/255.0 <= 1.0 ? runtime->clt/255.0 : 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.clt_pbar),
				tmpf);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.ww_clt_pbar),
				tmpf);
		g_free(tmpbuf);
		warmwizard_update_status(runtime->clt);
	}
	if ((runtime->batt_volts != runtime_last->batt_volts) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%.2f",runtime->batt_volts);
		gtk_label_set_text(GTK_LABEL(labels.batt_lab),tmpbuf);
		tmpf = runtime->batt_volts/18.0 <= 1.0 ? runtime->batt_volts/18.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.batt_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->gammae != runtime_last->gammae) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->gammae);
		gtk_label_set_text(GTK_LABEL(labels.gammae_lab),tmpbuf);
		tmpf = runtime->gammae/200.0 <= 1.0 ? runtime->gammae/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.gammae_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->mat != runtime_last->mat) || (forced_update) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",(int)runtime->mat);
		gtk_label_set_text(GTK_LABEL(labels.mat_lab),tmpbuf);
		tmpf = runtime->mat/255.0 <= 1.0 ? runtime->mat/255.0 : 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.mat_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->tps != runtime_last->tps) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%.1f",runtime->tps);
		gtk_label_set_text(GTK_LABEL(labels.tps_lab),tmpbuf);
		tmpf = runtime->tps/100.0 <= 1.0 ? runtime->tps/100.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.tps_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->rpm != runtime_last->rpm) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->rpm);
		gtk_label_set_text(GTK_LABEL(labels.rpm_lab),tmpbuf);
		tmpf = runtime->rpm/8000.0 <= 1.0 ? runtime->rpm/8000.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.rpm_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->pw1 != runtime_last->pw1) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%.1f",runtime->pw1);
		gtk_label_set_text(GTK_LABEL(labels.pw1_lab),tmpbuf);
		tmpf = runtime->pw1/25.5 <= 1.0 ? runtime->pw1/25.5: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.pw1_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->egocorr != runtime_last->egocorr) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->egocorr);
		gtk_label_set_text(GTK_LABEL(labels.egocorr_lab),tmpbuf);
		tmpf = runtime->egocorr/200.0 <= 1.0 ? runtime->egocorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.egocorr_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->barocorr != runtime_last->barocorr) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->barocorr);
		gtk_label_set_text(GTK_LABEL(labels.barocorr_lab),tmpbuf);
		tmpf = runtime->barocorr/200.0 <= 1.0 ? runtime->barocorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.barocorr_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->warmcorr != runtime_last->warmcorr) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->warmcorr);
		gtk_label_set_text(GTK_LABEL(labels.ww_warmcorr_lab),tmpbuf);
		gtk_label_set_text(GTK_LABEL(labels.warmcorr_lab),tmpbuf);
		tmpf = runtime->warmcorr/200.0 <= 1.0 ? runtime->warmcorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.warmcorr_pbar),
				tmpf);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.ww_warmcorr_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->aircorr != runtime_last->aircorr) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->aircorr);
		gtk_label_set_text(GTK_LABEL(labels.aircorr_lab),tmpbuf);
		tmpf = runtime->aircorr/200.0 <= 1.0 ? runtime->aircorr/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.aircorr_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->vecurr1 != runtime_last->vecurr1) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%i",runtime->vecurr1);
		gtk_label_set_text(GTK_LABEL(labels.vecurr1_lab),tmpbuf);
		tmpf = runtime->vecurr1/200.0 <= 1.0 ? runtime->vecurr1/200.0: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.vecurr1_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->tpsaccel != runtime_last->tpsaccel) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%.1f",runtime->tpsaccel/10.0);
		gtk_label_set_text(GTK_LABEL(labels.tpsaccel_lab),tmpbuf);
		tmpf = (runtime->tpsaccel/10.0)/25.5 <= 1.0 
			? (runtime->tpsaccel/10.0)/25.5: 1.0;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.tpsaccel_pbar),
				tmpf);
		g_free(tmpbuf);
	}
	if ((runtime->dcycle1 != runtime_last->dcycle1) || (count > 5))
	{
		tmpbuf = g_strdup_printf("%.1f",runtime->dcycle1);
		tmpf = runtime->dcycle1/100.0 <= 1.0 ? runtime->dcycle1/100.0: 1.0;
		gtk_label_set_text(GTK_LABEL(labels.dcycle1_lab),tmpbuf);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
				(progress.dcycle1_pbar),
				tmpf);
		g_free(tmpbuf);
	}

	if (ecu_flags & DUALTABLE)
	{
		/* Color the boxes on the VEtable closest to the operating point */

		if ((runtime->dcycle2 != runtime_last->dcycle2) || (count > 5))
		{
			tmpbuf = g_strdup_printf("%.1f",runtime->dcycle2);
			tmpf = runtime->dcycle2/100.0 <= 1.0 ? runtime->dcycle2/100.0: 1.0;
			gtk_label_set_text(GTK_LABEL(labels.dcycle2_lab),tmpbuf);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.dcycle2_pbar),
					tmpf);
			g_free(tmpbuf);
		}
		if ((runtime->pw2 != runtime_last->pw2) || (count > 5))
		{
			tmpbuf = g_strdup_printf("%.1f",runtime->pw2);
			tmpf = runtime->pw2/100.0 <= 1.0 ? runtime->pw2/100.0: 1.0;
			gtk_label_set_text(GTK_LABEL(labels.pw2_lab),tmpbuf);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.pw2_pbar),
					tmpf);
			g_free(tmpbuf);
		}
		if ((runtime->idledc != runtime_last->idledc) || (count > 5))
		{
			tmpbuf = g_strdup_printf("%i",runtime->idledc);
			tmpf = (float)runtime->idledc/100.0 <= 1.0 ? (float)runtime->idledc/100.0: 1.0;
			gtk_label_set_text(GTK_LABEL(labels.idledc_lab),tmpbuf);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
					(progress.idledc_pbar),
					tmpf);
			g_free(tmpbuf);
		}
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
