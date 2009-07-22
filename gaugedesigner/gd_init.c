
#include "../include/defines.h"
#include "../widgets/gauge.h"
#include <gd_init.h>
#include "events.h"
#include "handlers.h"
#include <gtk/gtk.h>
#include <glade/glade.h>

extern GtkWidget *gauge;


EXPORT gboolean init_text_attributes(GladeXML *xml)
{
        extern GdkColor white;

	OBJ_SET((glade_xml_get_widget(xml,"precision_spin")),"handler",GINT_TO_POINTER(PRECISION));
	OBJ_SET((glade_xml_get_widget(xml,"value_xpos_spin")),"handler",GINT_TO_POINTER(VALUE_XPOS));
	OBJ_SET((glade_xml_get_widget(xml,"value_ypos_spin")),"handler",GINT_TO_POINTER(VALUE_YPOS));
	OBJ_SET((glade_xml_get_widget(xml,"value_font_scale_spin")),"handler",GINT_TO_POINTER(VALUE_FONTSCALE));
	OBJ_SET((glade_xml_get_widget(xml,"value_color_button")),"handler",GINT_TO_POINTER(GAUGE_COL_VALUE_FONT));
	OBJ_SET((glade_xml_get_widget(xml,"show_value_check")),"handler",GINT_TO_POINTER(SHOW_VALUE));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"value_color_button")),&white);

	update_text_controls();
	update_onscreen_tblocks();
	return TRUE;
}

EXPORT gboolean init_general_attributes(GladeXML *xml)
{
	OBJ_SET((glade_xml_get_widget(xml,"cw_rbutton")),"handler",GINT_TO_POINTER(ROTATION));
	OBJ_SET((glade_xml_get_widget(xml,"cw_rbutton")),"special_value",GINT_TO_POINTER(MTX_ROT_CW));
	OBJ_SET((glade_xml_get_widget(xml,"ccw_rbutton")),"handler",GINT_TO_POINTER(ROTATION));
	OBJ_SET((glade_xml_get_widget(xml,"ccw_rbutton")),"special_value",GINT_TO_POINTER(MTX_ROT_CCW));
	OBJ_SET((glade_xml_get_widget(xml,"antialiased_check")),"handler",GINT_TO_POINTER(ANTIALIAS));
	OBJ_SET((glade_xml_get_widget(xml,"tattletale_check")),"handler",GINT_TO_POINTER(TATTLETALE));
	OBJ_SET((glade_xml_get_widget(xml,"tattletale_alpha_spin")),"handler",GINT_TO_POINTER(TATTLETALE_ALPHA));
	OBJ_SET((glade_xml_get_widget(xml,"needle_length_spin")),"handler",GINT_TO_POINTER(NEEDLE_LENGTH));
	OBJ_SET((glade_xml_get_widget(xml,"needle_tail_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL));
	OBJ_SET((glade_xml_get_widget(xml,"needle_tip_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_TIP_WIDTH));
	OBJ_SET((glade_xml_get_widget(xml,"needle_tail_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL_WIDTH));
	OBJ_SET((glade_xml_get_widget(xml,"needle_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_WIDTH));
	OBJ_SET((glade_xml_get_widget(xml,"start_angle_spin")),"handler",GINT_TO_POINTER(START_ANGLE));
	OBJ_SET((glade_xml_get_widget(xml,"sweep_angle_spin")),"handler",GINT_TO_POINTER(SWEEP_ANGLE));
	OBJ_SET((glade_xml_get_widget(xml,"lbound_spin")),"handler",GINT_TO_POINTER(LBOUND));
	OBJ_SET((glade_xml_get_widget(xml,"ubound_spin")),"handler",GINT_TO_POINTER(UBOUND));
	OBJ_SET((glade_xml_get_widget(xml,"background_color_button")),"handler",GINT_TO_POINTER(GAUGE_COL_BG));
	OBJ_SET((glade_xml_get_widget(xml,"needle_color_button")),"handler",GINT_TO_POINTER(GAUGE_COL_NEEDLE));
	OBJ_SET((glade_xml_get_widget(xml,"gradient_begin_color_button")),"handler",GINT_TO_POINTER(GAUGE_COL_GRADIENT_BEGIN));
	OBJ_SET((glade_xml_get_widget(xml,"gradient_end_color_button")),"handler",GINT_TO_POINTER(GAUGE_COL_GRADIENT_END));
	update_general_controls();
	return TRUE;
}


