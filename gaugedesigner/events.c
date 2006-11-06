#include "../include/defines.h"
#include <events.h>
#include "../widgets/gauge.h"
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif

GtkWidget * gauge = NULL;
static gboolean hold_handlers = FALSE;
GdkColor red = { 0, 65535, 0, 0};
GdkColor black = { 0, 0, 0, 0};



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

	tmp = glade_xml_get_widget(xml,"import_export_frame");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"animate_frame");
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
			break;
		case MIN_TICKS:
			mtx_gauge_face_set_minor_ticks(g,tmpi);
			break;
		case MAJ_TICK_LEN:
			mtx_gauge_face_set_major_tick_len(g,tmpf);
			break;
		case MIN_TICK_LEN:
			mtx_gauge_face_set_minor_tick_len(g,tmpf);
			break;
		case MAJ_TICK_WIDTH:
			mtx_gauge_face_set_major_tick_width(g,tmpf);
			break;
		case MIN_TICK_WIDTH:
			mtx_gauge_face_set_minor_tick_width(g,tmpf);
			break;
		case NAME_XPOS:
			mtx_gauge_face_set_str_xpos(g, NAME, tmpf);
			break;
		case NAME_YPOS:
			mtx_gauge_face_set_str_ypos(g, NAME, tmpf);
			break;
		case UNITS_XPOS:
			mtx_gauge_face_set_str_xpos(g, UNITS, tmpf);
			break;
		case UNITS_YPOS:
			mtx_gauge_face_set_str_ypos(g, UNITS, tmpf);
			break;
		case VALUE_XPOS:
			mtx_gauge_face_set_str_xpos(g, VALUE, tmpf);
			break;
		case VALUE_YPOS:
			mtx_gauge_face_set_str_ypos(g, VALUE, tmpf);
			break;
		case START_ANGLE:
			mtx_gauge_face_set_lspan_rad(g,tmpf);
			break;
		case STOP_ANGLE:
			mtx_gauge_face_set_uspan_rad(g,tmpf);
			break;
		case LBOUND:
			mtx_gauge_face_set_lbound(g,tmpf);
			break;
		case UBOUND:
			mtx_gauge_face_set_ubound(g,tmpf);
			break;
		case PRECISION:
			mtx_gauge_face_set_precision(g,tmpf);
			break;
		case TICK_INSET:
			mtx_gauge_face_set_tick_inset(g,tmpf);
			break;
		case MAJOR_TICK_TEXT_INSET:
			mtx_gauge_face_set_major_tick_text_inset(g,tmpf);
			break;
		case NEEDLE_WIDTH:
			mtx_gauge_face_set_needle_width(g,tmpf);
			break;
		case NEEDLE_TAIL:
			mtx_gauge_face_set_needle_tail(g,tmpf);
			break;
		case NAME_SCALE:
			mtx_gauge_face_set_font_scale(g, NAME, tmpf);
			break;
		case UNITS_SCALE:
			mtx_gauge_face_set_font_scale(g, UNITS, tmpf);
			break;
		case VALUE_SCALE:
			mtx_gauge_face_set_font_scale(g, VALUE, tmpf);
			break;
		case MAJ_TICK_SCALE:
			mtx_gauge_face_set_font_scale(g, MAJ_TICK, tmpf);
			break;

	}
	return (TRUE);
}


void update_attributes(GladeXML * xml)
{
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	gchar *tmpbuf0 = NULL;
	gchar *tmpbuf = NULL;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	hold_handlers = TRUE;
	widget = glade_xml_get_widget(xml,"major_ticks_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_ticks(g));
	
	widget = glade_xml_get_widget(xml,"minor_ticks_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_ticks(g));
	
	widget = glade_xml_get_widget(xml,"major_tick_len_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_len(g));
	
	widget = glade_xml_get_widget(xml,"major_tick_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_width(g));
	
	widget = glade_xml_get_widget(xml,"minor_tick_len_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_tick_len(g));

	widget = glade_xml_get_widget(xml,"minor_tick_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_tick_width(g));

	widget = glade_xml_get_widget(xml,"tick_inset_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_tick_inset(g));

	widget = glade_xml_get_widget(xml,"major_tick_text_inset_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_text_inset(g));

	widget = glade_xml_get_widget(xml,"needle_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_needle_width(g));

	widget = glade_xml_get_widget(xml,"needle_tail_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_needle_tail(g));

	widget = glade_xml_get_widget(xml,"name_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_font_scale(g, NAME));

	widget = glade_xml_get_widget(xml,"units_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_font_scale(g, UNITS));

	widget = glade_xml_get_widget(xml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_font_scale(g, VALUE));

	widget = glade_xml_get_widget(xml,"major_tick_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_font_scale(g, MAJ_TICK));

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

	mtx_gauge_face_get_str_pos(g,NAME, &tmp1,&tmp2);
	widget = glade_xml_get_widget(xml,"name_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(xml,"name_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	mtx_gauge_face_get_str_pos(g,UNITS,&tmp1,&tmp2);
	widget = glade_xml_get_widget(xml,"units_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(xml,"units_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	mtx_gauge_face_get_str_pos(g,VALUE,&tmp1,&tmp2);
	widget = glade_xml_get_widget(xml,"value_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(xml,"value_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	widget = glade_xml_get_widget(xml,"precision_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_precision(g));

	widget = glade_xml_get_widget(xml,"name_string_entry");
	tmpbuf = mtx_gauge_face_get_text(g, NAME);
	gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"units_string_entry");
	tmpbuf = mtx_gauge_face_get_text(g, UNITS);
	gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"major_tick_string_entry");
	tmpbuf = mtx_gauge_face_get_text(g, MAJ_TICK);
	if (tmpbuf)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"name_font_button");
	tmpbuf0 = mtx_gauge_face_get_font(g,NAME);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"units_font_button");
	tmpbuf0 = mtx_gauge_face_get_font(g,UNITS);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"value_font_button");
	tmpbuf0 = mtx_gauge_face_get_font(g,VALUE);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"major_tick_font_button");
	tmpbuf0 = mtx_gauge_face_get_font(g,MAJ_TICK);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(xml,"antialiased_check");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_antialias(g));

	widget = glade_xml_get_widget(xml,"show_value_check");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_show_value(g));

	widget = glade_xml_get_widget(xml,"name_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_NAME_FONT));

	widget = glade_xml_get_widget(xml,"units_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_UNIT_FONT));

	widget = glade_xml_get_widget(xml,"value_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_VALUE_FONT));

	widget = glade_xml_get_widget(xml,"major_tick_text_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MAJ_TICK_TEXT_FONT));

	widget = glade_xml_get_widget(xml,"background_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_BG));

	widget = glade_xml_get_widget(xml,"needle_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_NEEDLE));

	widget = glade_xml_get_widget(xml,"major_tick_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MAJ_TICK));

	widget = glade_xml_get_widget(xml,"minor_tick_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MIN_TICK));

	widget = glade_xml_get_widget(xml,"gradient_begin_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_GRADIENT_BEGIN));

	widget = glade_xml_get_widget(xml,"gradient_end_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_GRADIENT_END));

	hold_handlers = FALSE;
}

EXPORT gboolean entry_change_color(GtkWidget * widget, gpointer data)
{
        gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	return TRUE;

}
EXPORT gboolean entry_changed_handler(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	gchar * tmpbuf = NULL;

	if (hold_handlers)
		return TRUE;

        gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);

	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	if (tmpbuf == NULL)
		tmpbuf = g_strdup("");

	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);


	switch ((func)handler)
	{
		case NAME_STR:
			mtx_gauge_face_set_text(g, NAME, tmpbuf);
			break;
		case UNITS_STR:
			mtx_gauge_face_set_text(g, UNITS, tmpbuf);
			break;
		case MAJ_TICK_STR:
			mtx_gauge_face_set_text(g, MAJ_TICK, tmpbuf);
			break;
		default:
			break;
	}
	g_free(tmpbuf);
	return (TRUE);

}

EXPORT gboolean change_font(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	gchar * tmpbuf = NULL;
	tmpbuf = (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget));
	/* Strip out the font size as the gauge lib uses a different scaling
	 * method that scales with the size of the gauge
	 */
	tmpbuf = g_strchomp(g_strdelimit(tmpbuf,"0123456789",' '));
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if (hold_handlers)
		return TRUE;

	switch ((func)handler)
	{
		case NAME_FONT:
			mtx_gauge_face_set_font(g, NAME, tmpbuf);
			break;
		case VALUE_FONT:
			mtx_gauge_face_set_font(g, VALUE, tmpbuf);
			break;
		case UNITS_FONT:
			mtx_gauge_face_set_font(g, UNITS, tmpbuf);
			break;
		case MAJ_TICK_FONT:
			mtx_gauge_face_set_font(g, MAJ_TICK, tmpbuf);
			break;
		default:
			break;
	}
	return TRUE;
}


EXPORT gboolean checkbutton_handler(GtkWidget *widget, gpointer data)
{
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);
	if (hold_handlers)
		return TRUE;

	switch (handler)
	{
		case ANTIALIAS:
			mtx_gauge_face_set_antialias(g,state);
			break;
		case SHOW_VALUE:
			mtx_gauge_face_set_show_value(g,state);
			break;

	}

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

EXPORT gboolean xml_button_handler(GtkWidget *widget, gpointer data)
{
	gchar *tmpbuf = NULL;
	int handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	GladeXML *xml = glade_get_widget_tree(widget);
	gchar * name = NULL;
	GtkWidget *dialog = NULL;
	if (hold_handlers)
		return TRUE;


	switch ((StdButton)handler)
	{
		case IMPORT_XML:
			dialog = gtk_file_chooser_dialog_new ("Open File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
			break;
		case EXPORT_XML:
			dialog = gtk_file_chooser_dialog_new ("Save File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
#if GTK_MINOR_VERSION >= 8
			gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
#endif
			tmpbuf = mtx_gauge_face_get_text(MTX_GAUGE_FACE(gauge), NAME);
			name = g_strdup_printf("%s_Gauge.xml",tmpbuf);
			g_free(tmpbuf);
			gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), name);
			g_free(name);
			break;
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (handler == IMPORT_XML)
		{
			mtx_gauge_face_import_xml(gauge,filename);
			update_attributes(xml);
			update_onscreen_ranges(widget);
		}
		if (handler == EXPORT_XML)
			mtx_gauge_face_export_xml(gauge,filename);

		g_free (filename);
	}
	gtk_widget_destroy (dialog);
	return TRUE;

}

EXPORT gboolean animate_gauge(GtkWidget *widget, gpointer data)
{
	gtk_widget_set_sensitive(widget,FALSE);
	gtk_timeout_add(20,(GtkFunction)sweep_gauge, (gpointer)gauge);
	return TRUE;
}

gboolean sweep_gauge(gpointer data)
{
	static gfloat lower = 0.0;
	static gfloat upper = 0.0;
	gfloat interval = 0.0;
	gfloat cur_val = 0.0;
	GladeXML *xml = NULL;
	GtkWidget *button = NULL;
	static gboolean rising = TRUE;

	GtkWidget * gauge = data;
	mtx_gauge_face_get_bounds(MTX_GAUGE_FACE (gauge),&lower,&upper);
	interval = (upper-lower)/100.0;
	cur_val = mtx_gauge_face_get_value(MTX_GAUGE_FACE (gauge));
	if (cur_val >= upper)
		rising = FALSE;
	if (cur_val <= lower)
		rising = TRUE;

	if (rising)
		cur_val+=interval;
	else
		cur_val-=interval;

	mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge),cur_val);
	if (cur_val <= lower)
	{
		/* This cancels the timeout once one full complete sweep
		 * of the gauge
		 */
		xml = glade_get_widget_tree(gauge->parent);
		button = glade_xml_get_widget(xml,"animate_button");
		gtk_widget_set_sensitive(button,TRUE);
		mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge),lower);
		return FALSE;
	}
	else
		return TRUE;

}
