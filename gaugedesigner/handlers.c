#include "../include/defines.h"
#include <events.h>
#include <handlers.h>
#include <loadsave.h>
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

extern GladeXML *topxml;
extern GdkColor black;
extern GdkColor white;
extern GtkWidget *gauge;
extern gboolean hold_handlers;
extern gboolean changed;
extern gboolean gauge_loaded;


EXPORT gboolean text_attributes_menu_handler(GtkWidget * widget, gpointer data)
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget(topxml,"tab_notebook")),TEXTBLOCK_TAB);

	return TRUE;
}


EXPORT gboolean tick_groups_menu_handler(GtkWidget * widget, gpointer data)
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget(topxml,"tab_notebook")),TICKGROUP_TAB);
	return TRUE;
}


EXPORT gboolean polygon_menu_handler(GtkWidget * widget, gpointer data)
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget(topxml,"tab_notebook")),POLYGON_TAB);
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

	if (!topxml)
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(topxml,"precision_spin");
	mtx_gauge_face_get_attribute(g, PRECISION, &tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(gint)tmp1);

	mtx_gauge_face_get_attribute(g, VALUE_FONTSCALE, &tmp1);
	widget = glade_xml_get_widget(topxml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	mtx_gauge_face_get_attribute(g, VALUE_XPOS, &tmp1);
	mtx_gauge_face_get_attribute(g, VALUE_YPOS, &tmp2);
	widget = glade_xml_get_widget(topxml,"value_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = glade_xml_get_widget(topxml,"value_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	widget = glade_xml_get_widget(topxml,"value_font_button");
	tmpbuf0 = mtx_gauge_face_get_value_font(g);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = glade_xml_get_widget(topxml,"show_value_check");
	mtx_gauge_face_get_attribute(g, SHOW_VALUE, &tmp1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	widget = glade_xml_get_widget(topxml,"value_color_day_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_VALUE_FONT_DAY));

	widget = glade_xml_get_widget(topxml,"value_color_nite_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_VALUE_FONT_NITE));

	hold_handlers = FALSE;
}


void reset_text_controls()
{
	GtkWidget * widget = NULL;

	if ((!topxml) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(topxml,"precision_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"value_font_scale_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"value_xpos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"value_ypos_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"value_font_button");
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),g_strdup(""));

	widget = glade_xml_get_widget(topxml,"show_value_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(topxml,"value_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	hold_handlers = FALSE;
}



EXPORT gboolean general_attributes_menu_handler(GtkWidget * widget, gpointer data)
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget(topxml,"tab_notebook")),GENERAL_TAB);
	return TRUE;
}


void update_general_controls()
{
	gfloat tmp1 = 0.0;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = NULL;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return;

	if (!topxml)
		return;

	hold_handlers = TRUE;

	mtx_gauge_face_get_attribute(g,ROTATION,&tmp1);
	if (tmp1 == MTX_ROT_CW)
		widget = glade_xml_get_widget(topxml,"cw_rbutton");
	else if (tmp1 == MTX_ROT_CCW)
		widget = glade_xml_get_widget(topxml,"ccw_rbutton");
	else
		widget = glade_xml_get_widget(topxml,"cw_rbutton");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),TRUE);

	mtx_gauge_face_get_attribute(g,ANTIALIAS,&tmp1);
	widget = glade_xml_get_widget(topxml,"antialiased_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	mtx_gauge_face_get_attribute(g,TATTLETALE,&tmp1);
	widget = glade_xml_get_widget(topxml,"tattletale_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	widget = glade_xml_get_widget(topxml,"tattletale_alpha_spin");
	mtx_gauge_face_get_attribute(g,TATTLETALE_ALPHA,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"needle_length_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_LENGTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"needle_width_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"needle_tip_width_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_TIP_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"needle_tail_width_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_TAIL_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"needle_tail_spin");
	mtx_gauge_face_get_attribute(g,NEEDLE_TAIL,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"start_angle_spin");
	mtx_gauge_face_get_attribute(g,START_ANGLE,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"sweep_angle_spin");
	mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"lbound_spin");
	mtx_gauge_face_get_attribute(g,LBOUND,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"ubound_spin");
	mtx_gauge_face_get_attribute(g,UBOUND,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = glade_xml_get_widget(topxml,"background_color_day_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_BG_DAY));

	widget = glade_xml_get_widget(topxml,"background_color_nite_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_BG_NITE));

	widget = glade_xml_get_widget(topxml,"needle_color_day_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_NEEDLE_DAY));

	widget = glade_xml_get_widget(topxml,"needle_color_nite_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_NEEDLE_NITE));

	widget = glade_xml_get_widget(topxml,"gradient_begin_color_day_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_BEGIN_DAY));
	
	widget = glade_xml_get_widget(topxml,"gradient_begin_color_nite_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_BEGIN_NITE));
	
	widget = glade_xml_get_widget(topxml,"gradient_end_color_day_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_END_DAY));

	widget = glade_xml_get_widget(topxml,"gradient_end_color_nite_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_END_NITE));

	widget = glade_xml_get_widget(topxml,"daytime_radiobutton");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_daytime_mode(g));
	hold_handlers = FALSE;
}


void reset_general_controls()
{
	GtkWidget * widget = NULL;

	if ((!topxml) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = glade_xml_get_widget(topxml,"cw_button");
	gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(topxml,"ccw_button");
	gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(topxml,"antialiased_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(topxml,"tattletale_check");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = glade_xml_get_widget(topxml,"tattletale_alpha_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"needle_width_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"needle_tail_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"start_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"sweep_angle_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"lbound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"ubound_spin");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = glade_xml_get_widget(topxml,"background_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&black);

	widget = glade_xml_get_widget(topxml,"needle_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	widget = glade_xml_get_widget(topxml,"gradient_begin_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	widget = glade_xml_get_widget(topxml,"gradient_end_color_button");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&black);


	hold_handlers = FALSE;
}


EXPORT gboolean warning_ranges_menu_handler(GtkWidget * widget, gpointer data)
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget(topxml,"tab_notebook")),WARNING_TAB);
	return TRUE;
}


EXPORT gboolean alert_ranges_menu_handler(GtkWidget * widget, gpointer data)
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget(topxml,"tab_notebook")),ALERT_TAB);
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


EXPORT gboolean quit_gaugedesigner(GtkWidget *widget, gpointer data)
{
	if ((gauge_loaded) && (changed))
		prompt_to_save();
	gtk_main_quit();
	return TRUE;
}
