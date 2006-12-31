#include "../include/defines.h"
#include <events.h>
#include <handlers.h>
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

static GladeXML *tick_xml;
static GladeXML *gen_xml;
GladeXML *text_xml;
GladeXML *ranges_xml;
extern GtkWidget *gauge;
extern gboolean hold_handlers;

EXPORT gboolean tick_attributes_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	extern GdkColor white;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;
	if (created)
	{
		window = glade_xml_get_widget(tick_xml,"tick_settings_window");
		if (GTK_IS_WIDGET(window))
		{
			gtk_widget_show_all(window);
			update_tickmark_controls();
		}

		return TRUE;
	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "tick_settings_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);

	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_ticks_spin")),"handler",GINT_TO_POINTER(MAJ_TICKS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_ticks_spin")),"handler",GINT_TO_POINTER(MIN_TICKS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_len_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_LEN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_len_spin")),"handler",GINT_TO_POINTER(MIN_TICK_LEN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_width_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_width_spin")),"handler",GINT_TO_POINTER(MIN_TICK_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"tick_inset_spin")),"handler",GINT_TO_POINTER(TICK_INSET));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_text_inset_spin")),"handler",GINT_TO_POINTER(MAJOR_TICK_TEXT_INSET));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_string_entry")),"handler",GINT_TO_POINTER(MAJ_TICK_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_text_color_button")),"handler",GINT_TO_POINTER(COL_MAJ_TICK_TEXT_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_font_button")),"handler",GINT_TO_POINTER(MAJ_TICK_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_font_scale_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_color_button")),"handler",GINT_TO_POINTER(COL_MAJ_TICK));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_color_button")),"handler",GINT_TO_POINTER(COL_MIN_TICK));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"major_tick_text_color_button")),&white);

	tick_xml = xml;
	created = TRUE;
	update_tickmark_controls();
	return TRUE;
}


void update_tickmark_controls()
{
	gchar *tmpbuf0 = NULL;
	gchar *tmpbuf = NULL;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if ((!tick_xml) || (!gauge))
		return;

	hold_handlers = TRUE;
	widget = glade_xml_get_widget(tick_xml,"major_ticks_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_ticks(g));

	widget = glade_xml_get_widget(tick_xml,"minor_ticks_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_ticks(g));

	widget = glade_xml_get_widget(tick_xml,"major_tick_len_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_len(g));

	widget = glade_xml_get_widget(tick_xml,"major_tick_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_width(g));

	widget = glade_xml_get_widget(tick_xml,"minor_tick_len_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_tick_len(g));

	widget = glade_xml_get_widget(tick_xml,"minor_tick_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_minor_tick_width(g));

	widget = glade_xml_get_widget(tick_xml,"tick_inset_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_tick_inset(g));

	widget = glade_xml_get_widget(tick_xml,"major_tick_text_inset_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_major_tick_text_inset(g));

	widget = glade_xml_get_widget(tick_xml,"major_tick_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_font_scale(g, MAJ_TICK));
	widget = glade_xml_get_widget(tick_xml,"major_tick_string_entry");
	tmpbuf = mtx_gauge_face_get_text(g, MAJ_TICK);
	if (tmpbuf)
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
	g_free(tmpbuf);


	widget = glade_xml_get_widget(tick_xml,"major_tick_font_button");
	tmpbuf0 = mtx_gauge_face_get_font(g,MAJ_TICK);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(tick_xml,"major_tick_text_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MAJ_TICK_TEXT_FONT));

	widget = glade_xml_get_widget(tick_xml,"major_tick_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MAJ_TICK));

	widget = glade_xml_get_widget(tick_xml,"minor_tick_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_MIN_TICK));
	hold_handlers = FALSE;

}


EXPORT gboolean text_attributes_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	extern GdkColor white;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;
	if (created)
	{
		window = glade_xml_get_widget(text_xml,"text_settings_window");
		if (GTK_IS_WIDGET(window))
		{
			printf("show make it re-appear\n");
			gtk_widget_show_all(window);
			update_text_controls();
			return TRUE;
		}

	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "text_settings_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);

	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"precision_spin")),"handler",GINT_TO_POINTER(PRECISION));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_button")),"handler",GINT_TO_POINTER(VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_xpos_spin")),"handler",GINT_TO_POINTER(VALUE_XPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_ypos_spin")),"handler",GINT_TO_POINTER(VALUE_YPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_scale_spin")),"handler",GINT_TO_POINTER(VALUE_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_color_button")),"handler",GINT_TO_POINTER(COL_VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"show_value_check")),"handler",GINT_TO_POINTER(SHOW_VALUE));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"value_color_button")),&white);

	text_xml = xml;
	created = TRUE;
	update_text_controls();
	update_onscreen_tblocks();
	return TRUE;
}


void update_text_controls()
{
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	gchar *tmpbuf0 = NULL;
	gchar *tmpbuf = NULL;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if ((!text_xml) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(text_xml,"precision_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_precision(g));

	widget = glade_xml_get_widget(text_xml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_font_scale(g, VALUE));

	mtx_gauge_face_get_str_pos(g,VALUE,&tmp1,&tmp2);
	widget = glade_xml_get_widget(text_xml,"value_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(text_xml,"value_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	widget = glade_xml_get_widget(text_xml,"value_font_button");
	tmpbuf0 = mtx_gauge_face_get_font(g,VALUE);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(text_xml,"show_value_check");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_show_value(g));

	widget = glade_xml_get_widget(text_xml,"value_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_VALUE_FONT));

	hold_handlers = FALSE;
}



EXPORT gboolean general_attributes_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	extern GdkColor white;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;
	if (created)
	{
		window = glade_xml_get_widget(gen_xml,"general_settings_window");
		if (GTK_IS_WIDGET(window))
		{
			printf("show make it re-appear\n");
			gtk_widget_show_all(window);
			update_text_controls();
			return TRUE;
		}

	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "general_settings_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);

	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_tail_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"start_angle_spin")),"handler",GINT_TO_POINTER(START_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"stop_angle_spin")),"handler",GINT_TO_POINTER(STOP_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"lbound_spin")),"handler",GINT_TO_POINTER(LBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"ubound_spin")),"handler",GINT_TO_POINTER(UBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"antialiased_check")),"handler",GINT_TO_POINTER(AA));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"background_color_button")),"handler",GINT_TO_POINTER(COL_BG));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_color_button")),"handler",GINT_TO_POINTER(COL_NEEDLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"gradient_begin_color_button")),"handler",GINT_TO_POINTER(COL_GRADIENT_BEGIN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"gradient_end_color_button")),"handler",GINT_TO_POINTER(COL_GRADIENT_END));


	gen_xml = xml;
	created = TRUE;
	update_general_controls();
	return TRUE;
}


void update_general_controls()
{
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if ((!gen_xml) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(gen_xml,"needle_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_needle_width(g));

	widget = glade_xml_get_widget(gen_xml,"needle_tail_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),mtx_gauge_face_get_needle_tail(g));

	mtx_gauge_face_get_span_rad(g,&tmp1,&tmp2);
	widget = glade_xml_get_widget(gen_xml,"start_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(gen_xml,"stop_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	mtx_gauge_face_get_bounds(g,&tmp1,&tmp2);
	widget = glade_xml_get_widget(gen_xml,"lbound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(gen_xml,"ubound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	widget = glade_xml_get_widget(gen_xml,"antialiased_check");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_antialias(g));

	widget = glade_xml_get_widget(gen_xml,"background_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_BG));

	widget = glade_xml_get_widget(gen_xml,"needle_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_NEEDLE));

	widget = glade_xml_get_widget(gen_xml,"gradient_begin_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_GRADIENT_BEGIN));
	widget = glade_xml_get_widget(gen_xml,"gradient_end_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_GRADIENT_END));


	hold_handlers = FALSE;
}


EXPORT gboolean warning_ranges_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;
	if (created)
	{
		window = glade_xml_get_widget(ranges_xml,"warning_ranges_window");
		if (GTK_IS_WIDGET(window))
		{
			printf("show make it re-appear\n");
			gtk_widget_show_all(window);
			update_text_controls();
			return TRUE;
		}

	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "warning_ranges_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);

	ranges_xml = xml;
	created = TRUE;
	update_onscreen_ranges();
	return TRUE;
}

EXPORT gboolean about_handler(GtkWidget *widget, gpointer data)
{
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		gchar *authors[] = {"David Andruczyk",NULL};
		gchar *artists[] = {"Dale Anderson",NULL};
		gtk_show_about_dialog(NULL,
				"name","MegaTunix Gauge Designer",
				"version",VERSION,
				"copyright","David J. Andruczyk(2006)",
				"comments","MegaTunix Gauge Designer is a tool to design custom Gauges for the MegaTunix MegaSquirt tuning software",
				"license","GNU GPL v2",
				"website","http://megatunix.sourceforge.net",
				"authors",authors,
				"artists",artists,
				"documenters",authors,
				NULL);
	}
#endif
	return TRUE;
}

