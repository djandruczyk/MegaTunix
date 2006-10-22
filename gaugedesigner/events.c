#include "../include/defines.h"
#include <events.h>
#include "../widgets/gauge.h"
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif

static GtkWidget * gauge = NULL;
static gboolean hold_handlers = FALSE;


EXPORT gboolean create_new_gauge(GtkWidget * widget, gpointer data)
{
	GtkWidget *tmp = NULL;
	GladeXML *xml = glade_get_widget_tree(widget);
	GtkWidget *parent = glade_xml_get_widget(xml,"gauge_frame");
	gauge = mtx_gauge_face_new();
	gtk_container_add(GTK_CONTAINER(parent),gauge);
	gtk_widget_show_all(parent);
	gtk_widget_set_sensitive(widget,FALSE);
	tmp = glade_xml_get_widget(xml,"font_attributes_frame");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"gauge_attributes_frame");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"color_ranges_frame");
	gtk_widget_set_sensitive(tmp,TRUE);

	update_attributes(xml);
	return (TRUE);
}

EXPORT gboolean create_color_span(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *spinner = NULL;
	MtxColorRange *range = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	GladeXML *xml = glade_xml_new("gaugedesigner.glade", "range_dialog", NULL);
	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"range_dialog");
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	/* Set the controls to sane ranges corresponding to the gauge */
	mtx_gauge_face_get_bounds(MTX_GAUGE_FACE(gauge),&lbound,&ubound);
	spinner = glade_xml_get_widget(xml,"range_lowpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	spinner = glade_xml_get_widget(xml,"range_highpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			range = g_new0(MtxColorRange, 1);
			range->lowpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_lowpoint_spin")));
			range->highpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_highpoint_spin")));
			range->inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_inset_spin")));
			range->lwidth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_lwidth_spin")));
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"range_colorbutton")),&range->color);
			mtx_gauge_face_set_color_range_struct(MTX_GAUGE_FACE(gauge),range);
			g_free(range);
			update_onscreen_ranges(widget);

			break;
		default:
			break;
	}
	gtk_widget_destroy(dialog);

	return (FALSE);
}

EXPORT gboolean spin_button_handler(GtkWidget *widget, gpointer data)
{
	gint tmpi;
	gfloat tmpf;
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	tmpf = (gfloat)gtk_spin_button_get_value((GtkSpinButton *)widget);
	tmpi = (gint)(tmpf+0.00001);
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if (hold_handlers)
		return TRUE;

	switch (handler)
	{
		case MAJ_TICKS:
			mtx_gauge_face_set_major_ticks(g,tmpi);
			printf("major tick change\n");
			break;
		case MIN_TICKS:
			mtx_gauge_face_set_minor_ticks(g,tmpi);
			printf("minor tick change\n");
			break;
		case MAJ_TICK_LEN:
			mtx_gauge_face_set_major_tick_len(g,tmpf);
			printf("major tick LEN change\n");
			break;
		case MIN_TICK_LEN:
			mtx_gauge_face_set_minor_tick_len(g,tmpf);
			printf("minor tick LEN change\n");
			break;
		case START_ANGLE:
			mtx_gauge_face_set_lspan_rad(g,tmpf);
			printf("lspan change\n");
			break;
		case STOP_ANGLE:
			mtx_gauge_face_set_uspan_rad(g,tmpf);
			printf("uspan change\n");
			break;
		case LBOUND:
			mtx_gauge_face_set_lbound(g,tmpf);
			printf("lbound change\n");
			break;
		case UBOUND:
			mtx_gauge_face_set_ubound(g,tmpf);
			printf("ubound change\n");
			break;
		case PRECISION:
			mtx_gauge_face_set_precision(g,tmpf);
			printf("precision change\n");
			break;
		case TICK_INSET:
			mtx_gauge_face_set_tick_inset(g,tmpf);
			printf("tick inset change\n");
			break;
		case NEEDLE_WIDTH:
			mtx_gauge_face_set_needle_width(g,tmpf);
			printf("needle width change\n");
			break;
		case NEEDLE_TAIL:
			mtx_gauge_face_set_needle_tail(g,tmpf);
			printf("needle tail change\n");
			break;
		case NAME_SCALE:
			mtx_gauge_face_set_name_font_scale(g,tmpf);
			printf("name font scale change\n");
			break;
		case UNITS_SCALE:
			mtx_gauge_face_set_units_font_scale(g,tmpf);
			printf("units font scale change\n");
			break;
		case VALUE_SCALE:
			mtx_gauge_face_set_value_font_scale(g,tmpf);
			printf("value font scale change\n");
			break;

	}
	return (TRUE);
}


void update_attributes(GladeXML * xml)
{
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	hold_handlers = TRUE;
	widget = glade_xml_get_widget(xml,"major_ticks_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_ticks(g));
	widget = glade_xml_get_widget(xml,"minor_ticks_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_ticks(g));
	widget = glade_xml_get_widget(xml,"major_tick_len_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_len(g));
	widget = glade_xml_get_widget(xml,"minor_tick_len_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_tick_len(g));
	widget = glade_xml_get_widget(xml,"tick_inset_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_tick_inset(g));
	widget = glade_xml_get_widget(xml,"needle_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_needle_width(g));
	widget = glade_xml_get_widget(xml,"needle_tail_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_needle_tail(g));
	widget = glade_xml_get_widget(xml,"name_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_name_font_scale(g));
	widget = glade_xml_get_widget(xml,"units_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_units_font_scale(g));
	widget = glade_xml_get_widget(xml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_value_font_scale(g));
	mtx_gauge_face_get_span_rad(g,&tmp1,&tmp2);
	widget = glade_xml_get_widget(xml,"start_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(xml,"stop_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);
	mtx_gauge_face_get_bounds(g,&tmp1,&tmp2);
	widget = glade_xml_get_widget(xml,"lbound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(xml,"ubound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);
	widget = glade_xml_get_widget(xml,"precision_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_precision(g));
	widget = glade_xml_get_widget(xml,"name_string_entry");
	gtk_entry_set_text(GTK_ENTRY(widget),mtx_gauge_face_get_name_string(g));
	widget = glade_xml_get_widget(xml,"units_string_entry");
	gtk_entry_set_text(GTK_ENTRY(widget),mtx_gauge_face_get_units_string(g));
	widget = glade_xml_get_widget(xml,"name_font_entry");
	gtk_entry_set_text(GTK_ENTRY(widget),mtx_gauge_face_get_name_font(g));
	widget = glade_xml_get_widget(xml,"units_font_entry");
	gtk_entry_set_text(GTK_ENTRY(widget),mtx_gauge_face_get_units_font(g));
	widget = glade_xml_get_widget(xml,"value_font_entry");
	gtk_entry_set_text(GTK_ENTRY(widget),mtx_gauge_face_get_value_font(g));
	widget = glade_xml_get_widget(xml,"antialiased_check");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_antialias(g));
	widget = glade_xml_get_widget(xml,"name_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_NAME_FONT));
	widget = glade_xml_get_widget(xml,"units_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_UNIT_FONT));
	widget = glade_xml_get_widget(xml,"value_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_VALUE_FONT));
	widget = glade_xml_get_widget(xml,"background_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_BG));
	widget = glade_xml_get_widget(xml,"needle_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_NEEDLE));
	widget = glade_xml_get_widget(xml,"major_tick_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MAJ_TICK));
	widget = glade_xml_get_widget(xml,"minor_tick_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MIN_TICK));
	hold_handlers = FALSE;
}

EXPORT gboolean entry_changed_handler(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	gchar * tmpbuf = NULL;
	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);

	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if (hold_handlers)
		return TRUE;

	switch ((func)handler)
	{
		case NAME_STR:
			mtx_gauge_face_set_name_str(g,tmpbuf);
			printf("set name string\n");
			break;
		case UNITS_STR:
			mtx_gauge_face_set_units_str(g,tmpbuf);
			printf("set units str\n");
			break;
		case NAME_FONT:
			mtx_gauge_face_set_name_font(g,tmpbuf);
			printf("set name font \n");
			break;
		case VALUE_FONT:
			mtx_gauge_face_set_value_font(g,tmpbuf);
			printf("set value font \n");
			break;
		case UNITS_FONT:
			mtx_gauge_face_set_name_font(g,tmpbuf);
			printf("set units font \n");
			break;
		default:
			break;
	}
	return (TRUE);

}

EXPORT gboolean set_antialiased_mode(GtkWidget *widget, gpointer data)
{
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);
	if (hold_handlers)
		return TRUE;

	mtx_gauge_face_set_antialias(g,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));

	return TRUE;
}

EXPORT gboolean color_button_color_set(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");

	if (hold_handlers)
		return TRUE;
	GdkColor color;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
	mtx_gauge_face_set_color(MTX_GAUGE_FACE(gauge),handler,color);

	return TRUE;
}

void update_onscreen_ranges(GtkWidget *widget)
{
	GtkWidget *toptable = NULL;
	GtkWidget *table = NULL;
	GtkWidget *button = NULL;
	GtkWidget *label = NULL;
	gint i = 0;
	gint y = 1;
	MtxColorRange *range = NULL;
	GArray * array = NULL;

	GladeXML *xml = glade_get_widget_tree(widget);
	array = mtx_gauge_face_get_color_ranges(MTX_GAUGE_FACE(gauge));
	toptable = glade_xml_get_widget(xml,"color_ranges_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("table widget invalid!!\n");
		return;
	}
	y=1;
	if (g_object_get_data(G_OBJECT(toptable),"subtable") != NULL)
	{
		gtk_widget_destroy(g_object_get_data(G_OBJECT(toptable),"subtable"));
	}
	table = gtk_table_new(2,6,FALSE);
	gtk_table_attach(GTK_TABLE(toptable),table,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
	g_object_set_data(G_OBJECT(toptable),"subtable",table);
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
		label = gtk_label_new("Color");
		gtk_table_attach(GTK_TABLE(table),label,5,6,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
	}
	/* Repopulate the table with the current ranges... */
	for (i=0;i<array->len; i++)
	{
		range = g_array_index(array,MtxColorRange *, i);
		button = gtk_check_button_new();
		g_object_set_data(G_OBJECT(button),"range_index",GINT_TO_POINTER(i));
		g_object_set_data(G_OBJECT(button),"table_ptr",toptable);
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_range),NULL);
		gtk_table_attach(GTK_TABLE(table),button,0,1,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		label = gtk_label_new(g_strdup_printf("%.3f",range->lowpoint));
		g_object_set_data(G_OBJECT(button),"lowpt_ptr",label);
		gtk_table_attach(GTK_TABLE(table),label,1,2,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		label = gtk_label_new(g_strdup_printf("%.3f",range->highpoint));
		g_object_set_data(G_OBJECT(button),"highpt_ptr",label);
		gtk_table_attach(GTK_TABLE(table),label,2,3,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		label = gtk_label_new(g_strdup_printf("%.3f",range->inset));
		g_object_set_data(G_OBJECT(button),"inset_ptr",label);
		gtk_table_attach(GTK_TABLE(table),label,3,4,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		label = gtk_label_new(g_strdup_printf("%.3f",range->lwidth));
		g_object_set_data(G_OBJECT(button),"lwidth_ptr",label);
		gtk_table_attach(GTK_TABLE(table),label,4,5,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		label = gtk_color_button_new_with_color(&range->color);
		g_object_set_data(G_OBJECT(button),"color_ptr",label);
		gtk_table_attach(GTK_TABLE(table),label,5,6,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		y++;
	}
	gtk_widget_show_all(toptable);
}

gboolean remove_range(GtkWidget * widget, gpointer data)
{
	gint index = -1;

	index = (gint)g_object_get_data(G_OBJECT(widget),"range_index");
	mtx_gauge_face_remove_color_range(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_ranges(g_object_get_data(G_OBJECT(widget),"table_ptr"));

	return TRUE;
}

EXPORT gboolean link_range_spinners(GtkWidget *widget, gpointer data)
{
	GladeXML *xml = glade_get_widget_tree(widget);
	GtkAdjustment *adj = NULL;
	GtkWidget *upper_spin = glade_xml_get_widget(xml,"range_highpoint_spin");

	adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(upper_spin));
	adj->lower = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	if (adj->value < adj->lower)
		adj->value = adj->lower;
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(upper_spin),adj);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(upper_spin),adj->value);
	return FALSE;
}
