#include "../include/defines.h"
#include <events.h>
#include <handlers.h>
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

static GladeXML *gen_xml;
GladeXML *text_xml;
GladeXML *tick_groups_xml;
GladeXML *polygons_xml;
GladeXML *c_ranges_xml;
GladeXML *a_ranges_xml;
extern GdkColor black;
extern GdkColor white;
extern GtkWidget *gauge;
extern gboolean hold_handlers;


EXPORT gboolean text_attributes_menu_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	extern GdkColor white;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (created)
	{
		window = glade_xml_get_widget(text_xml,"text_settings_window");
		if (GTK_IS_WIDGET(window))
		{
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
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_xpos_spin")),"handler",GINT_TO_POINTER(VALUE_XPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_ypos_spin")),"handler",GINT_TO_POINTER(VALUE_YPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_scale_spin")),"handler",GINT_TO_POINTER(VALUE_FONTSCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_color_button")),"handler",GINT_TO_POINTER(COL_VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"show_value_check")),"handler",GINT_TO_POINTER(SHOW_VALUE));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"value_color_button")),&white);

	text_xml = xml;
	created = TRUE;
	update_text_controls();
	update_onscreen_tblocks();
	return TRUE;
}


EXPORT gboolean tick_groups_menu_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (created)
	{
		window = glade_xml_get_widget(tick_groups_xml,"tick_group_settings_window");
		if (GTK_IS_WIDGET(window))
		{
			gtk_widget_show_all(window);
			update_onscreen_tgroups();
			return TRUE;
		}

	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "tick_group_settings_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);
	tick_groups_xml = xml;
	created = TRUE;
	update_onscreen_tgroups();
	return TRUE;
}


EXPORT gboolean polygon_menu_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (created)
	{
		window = glade_xml_get_widget(polygons_xml,"gauge_polygons_window");
		if (GTK_IS_WIDGET(window))
		{
			gtk_widget_show_all(window);
			update_onscreen_polygons();
			return TRUE;
		}

	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "gauge_polygons_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);
	polygons_xml = xml;
	created = TRUE;
	update_onscreen_polygons();
	return TRUE;
}


void update_text_controls()
{
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	gchar *tmpbuf0 = NULL;
	gchar *tmpbuf = NULL;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = NULL;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return;

	if (!text_xml)
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(text_xml,"precision_spin");
	mtx_gauge_face_get_attribute(g, PRECISION, &tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(gint)tmp1);

	mtx_gauge_face_get_attribute(g, VALUE_FONTSCALE, &tmp1);
	widget = glade_xml_get_widget(text_xml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	mtx_gauge_face_get_attribute(g, VALUE_XPOS, &tmp1);
	mtx_gauge_face_get_attribute(g, VALUE_YPOS, &tmp2);
	widget = glade_xml_get_widget(text_xml,"value_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(text_xml,"value_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	widget = glade_xml_get_widget(text_xml,"value_font_button");
	tmpbuf0 = mtx_gauge_face_get_value_font(g);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(text_xml,"show_value_check");
	mtx_gauge_face_get_attribute(g, SHOW_VALUE, &tmp1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	widget = glade_xml_get_widget(text_xml,"value_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,COL_VALUE_FONT));

	hold_handlers = FALSE;
}


void reset_text_controls()
{
	GtkWidget * widget = NULL;

	if ((!text_xml) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(text_xml,"precision_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(text_xml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(text_xml,"value_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(text_xml,"value_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(text_xml,"value_font_button");
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),g_strdup(""));

	widget = glade_xml_get_widget(text_xml,"show_value_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(text_xml,"value_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	hold_handlers = FALSE;
}



EXPORT gboolean general_attributes_menu_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	extern GdkColor white;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (created)
	{
		window = glade_xml_get_widget(gen_xml,"general_settings_window");
		if (GTK_IS_WIDGET(window))
		{
			gtk_widget_show_all(window);
			update_text_controls();
			update_general_controls();
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

	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_length_spin")),"handler",GINT_TO_POINTER(NEEDLE_LENGTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_tail_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_tip_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_TIP_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_tail_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"start_angle_spin")),"handler",GINT_TO_POINTER(START_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"sweep_angle_spin")),"handler",GINT_TO_POINTER(SWEEP_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"lbound_spin")),"handler",GINT_TO_POINTER(LBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"ubound_spin")),"handler",GINT_TO_POINTER(UBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"antialiased_check")),"handler",GINT_TO_POINTER(ANTIALIAS));
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
	MtxGaugeFace *g = NULL;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return;

	if (!gen_xml)
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(gen_xml,"needle_length_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_LENGTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(gen_xml,"needle_width_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(gen_xml,"needle_tip_width_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_TIP_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(gen_xml,"needle_tail_width_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_TAIL_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(gen_xml,"needle_tail_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_TAIL,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	mtx_gauge_face_get_attribute(g,START_ANGLE,&tmp1);
	mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&tmp2);
	widget = glade_xml_get_widget(gen_xml,"start_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(gen_xml,"sweep_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	mtx_gauge_face_get_attribute(g,LBOUND,&tmp1);
	mtx_gauge_face_get_attribute(g,UBOUND,&tmp2);
	widget = glade_xml_get_widget(gen_xml,"lbound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(gen_xml,"ubound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	mtx_gauge_face_get_attribute(g,ANTIALIAS,&tmp1);
	widget = glade_xml_get_widget(gen_xml,"antialiased_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

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


void reset_general_controls()
{
	GtkWidget * widget = NULL;

	if ((!gen_xml) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(gen_xml,"needle_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(gen_xml,"needle_tail_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(gen_xml,"start_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(gen_xml,"sweep_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(gen_xml,"lbound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(gen_xml,"ubound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(gen_xml,"antialiased_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(gen_xml,"background_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&black);

	widget = glade_xml_get_widget(gen_xml,"needle_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	widget = glade_xml_get_widget(gen_xml,"gradient_begin_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	widget = glade_xml_get_widget(gen_xml,"gradient_end_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&black);


	hold_handlers = FALSE;
}


EXPORT gboolean warning_ranges_menu_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (created)
	{
		window = glade_xml_get_widget(c_ranges_xml,"warning_ranges_window");
		if (GTK_IS_WIDGET(window))
		{
			gtk_widget_show_all(window);
			update_text_controls();
			update_onscreen_c_ranges();
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

	c_ranges_xml = xml;
	created = TRUE;
	update_onscreen_c_ranges();
	return TRUE;
}


EXPORT gboolean alert_ranges_menu_handler(GtkWidget * widget, gpointer data)
{
	static gboolean created = FALSE;
	gchar * filename = NULL;
	GtkWidget *window = NULL;
	GladeXML *xml = NULL;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (created)
	{
		window = glade_xml_get_widget(a_ranges_xml,"alert_ranges_window");
		if (GTK_IS_WIDGET(window))
		{
			gtk_widget_show_all(window);
			update_text_controls();
			update_onscreen_a_ranges();
			return TRUE;
		}

	}
	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "alert_ranges_window", NULL);
		g_free(filename);
	}
	else
	{
		printf("can't load XML, ERROR!\n");
		exit(-2);
	}

	glade_xml_signal_autoconnect(xml);

	a_ranges_xml = xml;
	created = TRUE;
	update_onscreen_a_ranges();
	return TRUE;
}

EXPORT gboolean about_menu_handler(GtkWidget *widget, gpointer data)
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

