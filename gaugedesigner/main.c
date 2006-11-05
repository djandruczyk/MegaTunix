#include "events.h"
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib.h>
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>

int main (int argc, char ** argv )
{
	extern GtkWidget *gauge;
	GtkWidget *window;
	GtkWidget *widget;
	GtkWidget *toptable;
	GladeXML *xml = NULL;
	gchar *filename = NULL;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_main_quit),NULL);


	filename = get_file(g_build_filename(GAUGE_DATA_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
		xml = glade_xml_new(filename, "toptable", NULL);
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}
	g_free(filename);
	
	glade_xml_signal_autoconnect(xml);
	toptable = glade_xml_get_widget(xml, "toptable");
	/* Bind the appropriate handlers */
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_ticks_spin")),"handler",GINT_TO_POINTER(MAJ_TICKS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_ticks_spin")),"handler",GINT_TO_POINTER(MIN_TICKS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_len_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_LEN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_len_spin")),"handler",GINT_TO_POINTER(MIN_TICK_LEN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_width_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_width_spin")),"handler",GINT_TO_POINTER(MIN_TICK_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"tick_inset_spin")),"handler",GINT_TO_POINTER(TICK_INSET));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_text_inset_spin")),"handler",GINT_TO_POINTER(MAJOR_TICK_TEXT_INSET));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_tail_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"start_angle_spin")),"handler",GINT_TO_POINTER(START_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"stop_angle_spin")),"handler",GINT_TO_POINTER(STOP_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"lbound_spin")),"handler",GINT_TO_POINTER(LBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"ubound_spin")),"handler",GINT_TO_POINTER(UBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_string_entry")),"handler",GINT_TO_POINTER(NAME_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_xpos_spin")),"handler",GINT_TO_POINTER(NAME_XPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_ypos_spin")),"handler",GINT_TO_POINTER(NAME_YPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_string_entry")),"handler",GINT_TO_POINTER(UNITS_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_xpos_spin")),"handler",GINT_TO_POINTER(UNITS_XPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_ypos_spin")),"handler",GINT_TO_POINTER(UNITS_YPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_string_entry")),"handler",GINT_TO_POINTER(MAJ_TICK_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"precision_spin")),"handler",GINT_TO_POINTER(PRECISION));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"antialiased_check")),"handler",GINT_TO_POINTER(AA));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_string_entry")),"handler",GINT_TO_POINTER(NAME_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_string_entry")),"handler",GINT_TO_POINTER(UNITS_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_font_button")),"handler",GINT_TO_POINTER(NAME_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_font_button")),"handler",GINT_TO_POINTER(UNITS_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_button")),"handler",GINT_TO_POINTER(VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_font_button")),"handler",GINT_TO_POINTER(MAJ_TICK_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_xpos_spin")),"handler",GINT_TO_POINTER(VALUE_XPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_ypos_spin")),"handler",GINT_TO_POINTER(VALUE_YPOS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_font_scale_spin")),"handler",GINT_TO_POINTER(NAME_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_font_scale_spin")),"handler",GINT_TO_POINTER(UNITS_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_scale_spin")),"handler",GINT_TO_POINTER(VALUE_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_font_scale_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_color_button")),"handler",GINT_TO_POINTER(COL_NAME_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_color_button")),"handler",GINT_TO_POINTER(COL_UNIT_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_color_button")),"handler",GINT_TO_POINTER(COL_VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_text_color_button")),"handler",GINT_TO_POINTER(COL_MAJ_TICK_TEXT_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"background_color_button")),"handler",GINT_TO_POINTER(COL_BG));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_color_button")),"handler",GINT_TO_POINTER(COL_NEEDLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_color_button")),"handler",GINT_TO_POINTER(COL_MAJ_TICK));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_color_button")),"handler",GINT_TO_POINTER(COL_MIN_TICK));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"gradient_begin_color_button")),"handler",GINT_TO_POINTER(COL_GRADIENT_BEGIN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"gradient_end_color_button")),"handler",GINT_TO_POINTER(COL_GRADIENT_END));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"import_button")),"handler",GINT_TO_POINTER(IMPORT_XML));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"export_button")),"handler",GINT_TO_POINTER(EXPORT_XML));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"antialiased_check")),"handler",GINT_TO_POINTER(ANTIALIAS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"show_value_check")),"handler",GINT_TO_POINTER(SHOW_VALUE));


	gtk_container_add(GTK_CONTAINER(window),toptable);

	gtk_widget_show_all(window);
	if (argc > 1)  /* User specified xml file */
	{
		widget = glade_xml_get_widget(xml,"import_button");
		create_new_gauge(widget,NULL);
		mtx_gauge_face_import_xml(gauge,argv[1]);
		update_attributes(xml);
		update_onscreen_ranges(widget);
		gtk_widget_set_sensitive(widget,TRUE);
	}

	gtk_main();
	return (0);
}
