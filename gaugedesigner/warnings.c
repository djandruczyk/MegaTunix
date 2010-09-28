#include <config.h>
#include <defines.h>
#include <events.h>
#include <gauge.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <warnings.h>

extern GtkWidget * gauge;
extern GdkColor black;
extern GdkColor white;
extern gboolean changed;
extern GtkBuilder *toplevel;


EXPORT gboolean create_warning_span_event(GtkWidget * widget, gpointer data)
{
	GtkBuilder *warnings;
	GtkWidget *dialog = NULL;
	GtkWidget *spinner = NULL;
	GtkWidget *tmpwidget = NULL;
	GtkWidget *cbutton = NULL;
	MtxWarningRange *range = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	gchar * filename = NULL;
	gint result = 0;
	GError *error = NULL;


	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"w_range.glade",NULL),NULL);
	if (filename)
	{
		warnings = gtk_builder_new();
		if(!gtk_builder_add_from_file(warnings,filename,&error))
		{
			g_warning ("Couldn't load builder file: %s", error->message);
			g_error_free(error);
			exit(-1);
		}
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	gtk_builder_connect_signals(warnings,NULL);
	dialog = GTK_WIDGET (gtk_builder_get_object(warnings,"w_range_dialog"));
	cbutton = GTK_WIDGET (gtk_builder_get_object(warnings,"range_day_colorbutton"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&white);
	cbutton = GTK_WIDGET (gtk_builder_get_object(warnings,"range_nite_colorbutton"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&black);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	/* Set the controls to sane ranges corresponding to the gauge */
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lbound);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &ubound);
	spinner = GTK_WIDGET (gtk_builder_get_object(warnings,"range_lowpoint_spin"));
	OBJ_SET(spinner,"builder",warnings);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner),(ubound-lbound)/2.0);
	spinner = GTK_WIDGET (gtk_builder_get_object(warnings,"range_highpoint_spin"));
	OBJ_SET(spinner,"builder",warnings);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner),ubound);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			range = g_new0(MtxWarningRange, 1);
			range->lowpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(warnings,"range_lowpoint_spin")));
			range->highpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(warnings,"range_highpoint_spin")));
			range->inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(warnings,"range_inset_spin")));
			range->lwidth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(warnings,"range_lwidth_spin")));
			gtk_color_button_get_color(GTK_COLOR_BUTTON(gtk_builder_get_object(warnings,"range_day_colorbutton")),&range->color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(gtk_builder_get_object(warnings,"range_nite_colorbutton")),&range->color[MTX_NITE]);
			changed = TRUE;
			mtx_gauge_face_set_warning_range_struct(MTX_GAUGE_FACE(gauge),range);
			g_free(range);
			update_onscreen_w_ranges();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return (FALSE);
}


void update_onscreen_w_ranges()
{
	GtkWidget *container = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *label = NULL;
	GtkWidget *button = NULL;
	GtkWidget *img = NULL;
	GtkAdjustment *adj = NULL;
	guint i = 0;
	gint y = 1;
	gfloat low = 0.0;
	gfloat high = 0.0;
	MtxWarningRange *range = NULL;
	GArray * array = NULL;

	if ((!toplevel) || (!GTK_IS_WIDGET(gauge)))
		return;
	array = mtx_gauge_face_get_warning_ranges(MTX_GAUGE_FACE(gauge));
	container = GTK_WIDGET (gtk_builder_get_object(toplevel,"warning_range_viewport"));
	if (!GTK_IS_WIDGET(container))
	{
		printf("color range viewport invalid!!\n");
		return;
	}
	y=1;
	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,7,FALSE);
	gtk_container_add(GTK_CONTAINER(container),table);
	if (array->len > 0)
	{
		/* Create headers */
		label = gtk_label_new("Low");
		gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
		label = gtk_label_new("High");
		gtk_table_attach(GTK_TABLE(table),label,2,3,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
		label = gtk_label_new("Inset");
		gtk_table_attach(GTK_TABLE(table),label,3,4,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
		label = gtk_label_new("LWidth");
		gtk_table_attach(GTK_TABLE(table),label,4,5,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
		label = gtk_label_new("Day Color");
		gtk_table_attach(GTK_TABLE(table),label,5,6,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
		label = gtk_label_new("Nite Color");
		gtk_table_attach(GTK_TABLE(table),label,6,7,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
	}
	/* Repopulate the table with the current ranges... */
	for (i=0;i<array->len; i++)
	{
		range = g_array_index(array,MtxWarningRange *, i);
		button = gtk_button_new();
		img = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
		gtk_container_add(GTK_CONTAINER(button),img);
		OBJ_SET((button),"range_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(remove_w_range),NULL);
		gtk_table_attach(GTK_TABLE(table),button,0,1,y,y+1,0,0,0,0);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &low);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &high);
		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lowpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_w_range_data),GINT_TO_POINTER(WR_LOWPOINT));

		gtk_table_attach(GTK_TABLE(table),dummy,1,2,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->highpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_w_range_data),GINT_TO_POINTER(WR_HIGHPOINT));
		gtk_table_attach(GTK_TABLE(table),dummy,2,3,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->inset);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_w_range_data),GINT_TO_POINTER(WR_INSET));
		gtk_table_attach(GTK_TABLE(table),dummy,3,4,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lwidth);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_w_range_data),GINT_TO_POINTER(WR_LWIDTH));
		gtk_table_attach(GTK_TABLE(table),dummy,4,5,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&range->color[MTX_DAY]);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_w_range_data),GINT_TO_POINTER(WR_COLOR_DAY));

		gtk_table_attach(GTK_TABLE(table),dummy,5,6,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&range->color[MTX_NITE]);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_w_range_data),GINT_TO_POINTER(WR_COLOR_NITE));

		gtk_table_attach(GTK_TABLE(table),dummy,6,7,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		y++;
	}
	/* Scroll to end */
	dummy = GTK_WIDGET (gtk_builder_get_object(toplevel,"crange_swin"));
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dummy));
	adj->value = adj->upper;
	gtk_widget_show_all(container);
}


void reset_onscreen_w_ranges()
{
	GtkWidget *container = NULL;
	GtkWidget *widget = NULL;

	if ((!toplevel))
		return;
	container = GTK_WIDGET (gtk_builder_get_object(toplevel,"warning_range_viewport"));
	if (!GTK_IS_WIDGET(container))
	{
		printf("color range viewport invalid!!\n");
		return;
	}
	widget= gtk_bin_get_child(GTK_BIN(container));
	if (GTK_IS_WIDGET(widget))
		gtk_widget_destroy(widget);

	gtk_widget_show_all(container);
}


gboolean remove_w_range(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (GINT)OBJ_GET((widget),"range_index");
	mtx_gauge_face_remove_warning_range(MTX_GAUGE_FACE(gauge),index);
	changed = TRUE;
	update_onscreen_w_ranges();

	return TRUE;
}


gboolean alter_w_range_data(GtkWidget *widget, gpointer data)
{
	gint index = (GINT)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	GdkColor color;
	WrField field = (WrField)data;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case WR_LOWPOINT:
		case WR_HIGHPOINT:
		case WR_INSET:
		case WR_LWIDTH:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_warning_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case WR_COLOR_DAY:
		case WR_COLOR_NITE:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_warning_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		default:
			break;

	}
	return TRUE;
}
