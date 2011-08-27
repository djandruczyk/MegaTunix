#include <alerts.h>
#include <events.h>
#include <getfiles.h>
#include <stdio.h>
#include <stdlib.h>

extern GtkWidget *gauge;
extern GdkColor black;
extern GdkColor white;
extern gboolean changed;
extern GtkBuilder *toplevel;

G_MODULE_EXPORT gboolean create_alert_span_event(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *spinner = NULL;
	GtkWidget *cbutton = NULL;
	MtxAlertRange *range = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	GtkBuilder *alerts = NULL;
	gchar * filename = NULL;
	GError *error = NULL;
	gint result = 0;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"a_range.ui",NULL),NULL);
	if (filename)
	{
		alerts = gtk_builder_new();
		if (!gtk_builder_add_from_file(alerts,filename, &error))
		{
			g_warning("Could't load builder file: %s", error->message);
			g_error_free(error);
			exit(-1);
		}
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary ui file!!!!\n");
		exit(-1);
	}

	gtk_builder_connect_signals(alerts,NULL);
	dialog = GTK_WIDGET (gtk_builder_get_object (alerts,"a_range_dialog"));
	cbutton = GTK_WIDGET (gtk_builder_get_object (alerts,"range_day_colorbutton"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&white);
	cbutton = GTK_WIDGET (gtk_builder_get_object (alerts,"range_nite_colorbutton"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&black);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	/* Set the controls to sane ranges corresponding to the gauge */
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lbound);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &ubound);
	spinner = GTK_WIDGET (gtk_builder_get_object (alerts,"range_lowpoint_spin"));
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner),(ubound-lbound)/2.0);
	spinner = GTK_WIDGET (gtk_builder_get_object (alerts,"range_highpoint_spin"));
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner),ubound);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			range = g_new0(MtxAlertRange, 1);
			range->lowpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(alerts,"range_lowpoint_spin")));
			range->highpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(alerts,"range_highpoint_spin")));
			range->inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(alerts,"range_inset_spin")));
			range->x_offset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(alerts,"range_xoffset_spin")));
			range->y_offset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(alerts,"range_yoffset_spin")));
			range->lwidth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(alerts,"range_lwidth_spin")));
			gtk_color_button_get_color(GTK_COLOR_BUTTON(gtk_builder_get_object(alerts,"range_day_colorbutton")),&range->color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(gtk_builder_get_object(alerts,"range_nite_colorbutton")),&range->color[MTX_NITE]);
			changed = TRUE;
			mtx_gauge_face_set_alert_range_struct(MTX_GAUGE_FACE(gauge),range);
			g_free(range);
			update_onscreen_a_ranges();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return (FALSE);
}


void update_onscreen_a_ranges()
{
	GtkWidget *container = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *dummy = NULL;
	GtkAdjustment *adj = NULL;
	guint i = 0;
	MtxAlertRange *range = NULL;
	const GArray * array = NULL;

	if ((!toplevel) || (!GTK_IS_WIDGET(gauge)))
		return;
	array = mtx_gauge_face_get_alert_ranges(MTX_GAUGE_FACE(gauge));
	container = GTK_WIDGET (gtk_builder_get_object(toplevel,"alert_range_viewport"));
	if (!GTK_IS_WIDGET(container))
	{
		printf("alert range viewport invalid!!\n");
		return;
	}
	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);
	table = gtk_table_new(2,7,FALSE);
	gtk_container_add(GTK_CONTAINER(container),table);
	/* Repopulate the table with the current ranges... */
	for (i=0;i<array->len; i++)
	{
		range = g_array_index(array,MtxAlertRange *, i);
		subtable = build_a_range(range,i);
		gtk_table_attach(GTK_TABLE(table),subtable,0,1,i,i+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
	}
	/* Scroll to end */
	dummy = GTK_WIDGET (gtk_builder_get_object(toplevel,"arange_swin"));
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dummy));
	gtk_adjustment_set_value(adj,gtk_adjustment_get_upper(adj));
	gtk_widget_show_all(container);
}


void reset_onscreen_a_ranges()
{
	GtkWidget *container = NULL;
	GtkWidget *widget = NULL;

	if ((!toplevel))
		return;
	container = GTK_WIDGET (gtk_builder_get_object(toplevel,"alert_range_viewport"));
	if (!GTK_IS_WIDGET(container))
	{
		printf("alert range viewport invalid!!\n");
		return;
	}
	widget= gtk_bin_get_child(GTK_BIN(container));
	if (GTK_IS_WIDGET(widget))
		gtk_widget_destroy(widget);

	gtk_widget_show_all(container);
}


gboolean remove_a_range(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (GINT)OBJ_GET((widget),"range_index");
	mtx_gauge_face_remove_alert_range(MTX_GAUGE_FACE(gauge),index);
	changed = TRUE;
	update_onscreen_a_ranges();

	return TRUE;
}


gboolean alter_a_range_data(GtkWidget *widget, gpointer data)
{
	gint index = (GINT)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	GdkColor color;
	AlertField field = (AlertField)OBJ_GET(widget,"handler");
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case ALRT_LOWPOINT:
		case ALRT_HIGHPOINT:
		case ALRT_INSET:
		case ALRT_LWIDTH:
		case ALRT_X_OFFSET:
		case ALRT_Y_OFFSET:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_alert_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case ALRT_COLOR_DAY:
		case ALRT_COLOR_NITE:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_alert_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		default:
			break;

	}
	return TRUE;
}


GtkWidget * build_a_range(MtxAlertRange *range, gint index)
{
	/* MUCH faster that the ui way unfortunately */
	GtkWidget *notebook = NULL;
	GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *minitable = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *x_spin = NULL;
	GtkWidget *y_spin = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *img = NULL;
	GtkWidget *label = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	gchar * tmpbuf = NULL;

	table = gtk_table_new(2,2,FALSE);

	/* Close button */
	widget = gtk_button_new();
	img = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
	gtk_container_add(GTK_CONTAINER(widget),img);
	OBJ_SET((widget),"range_index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"clicked", G_CALLBACK(remove_a_range),NULL);
	gtk_table_attach(GTK_TABLE(table),widget,0,1,0,4,0,0,0,0);

	notebook = gtk_notebook_new();
	gtk_table_attach(GTK_TABLE(table),notebook,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
	/* Alert Limits Tab */
	subtable = gtk_table_new(1,2,TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Alert Limits");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	/* Low point */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
	gtk_table_attach(GTK_TABLE(subtable),minitable,0,1,0,1,GTK_EXPAND,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("Low Threshold:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lbound);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &ubound);
	widget = gtk_spin_button_new_with_range(lbound,ubound,0.1);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_LOWPOINT));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, "value", range->lowpoint, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,0,GTK_EXPAND,0,0);

	/* high point */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
	gtk_table_attach(GTK_TABLE(subtable),minitable,1,2,0,1,GTK_EXPAND,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("High Threshold:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_spin_button_new_with_range(lbound,ubound,0.1);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_HIGHPOINT));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, "value", range->highpoint, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,0,GTK_EXPAND,0,0);

	/* Width/Inset Limits Tab */
	subtable = gtk_table_new(1,2,TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Width/Inset");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	/* Width */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
	gtk_table_attach(GTK_TABLE(subtable),minitable,0,1,0,1,GTK_EXPAND,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("Width:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_LWIDTH));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", range->lwidth, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,0,GTK_EXPAND,0,0);

	/* Inset */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
	gtk_table_attach(GTK_TABLE(subtable),minitable,1,2,0,1,GTK_EXPAND,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("Inset:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_INSET));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", range->inset, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,0,GTK_EXPAND,0,0);

	/* Colors Tab */
	subtable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Colors");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	/* Daytime Color */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
	gtk_table_attach(GTK_TABLE(subtable),minitable,0,1,0,1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("Day:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_color_button_new_with_color(&range->color[MTX_DAY]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_COLOR_DAY));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL|GTK_EXPAND,GTK_EXPAND,0,0);
	/* Nitetime Color */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
	gtk_table_attach(GTK_TABLE(subtable),minitable,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("Nite:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_color_button_new_with_color(&range->color[MTX_NITE]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_COLOR_NITE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL|GTK_EXPAND,GTK_EXPAND,0,0);

	/* Center Offset Tab: Edit button, X/Y position spinners */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Center Offset");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Center\nOffset");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_EXPAND,GTK_EXPAND,0,0);

	/* X position minilayout table */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_attach(GTK_TABLE(subtable),minitable,2,3,0,1,GTK_EXPAND,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("X:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND,0,0);
	widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_X_OFFSET));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", range->x_offset, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL,GTK_EXPAND,0,0);
	x_spin = widget;

	/* Y position minilayout table */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_attach(GTK_TABLE(subtable),minitable,3,4,0,1,0,GTK_FILL,0,0);
	widget = gtk_label_new("Y:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND,0,0);
	widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ALRT_Y_OFFSET));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", range->y_offset, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_a_range_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL,GTK_EXPAND,0,0);
	y_spin = widget;

	widget = gtk_button_new();
	OBJ_SET(widget,"x_spin",x_spin);
	OBJ_SET(widget,"y_spin",y_spin);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(widget),hbox);
	img = gtk_image_new_from_stock("gtk-edit",GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(hbox),img,FALSE,FALSE,0);
	label = gtk_label_new("Edit");
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(grab_coords_event),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),widget,0,2,1,2,GTK_FILL,0,0,0);
	return table;
}
