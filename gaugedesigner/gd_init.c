
#include <handlers.h>
#include <tblocks.h>

extern GtkWidget *gauge;


G_MODULE_EXPORT gboolean init_text_attributes(GtkBuilder *builder)
{
        extern GdkColor white;
        extern GdkColor black;
	GtkWidget *notebook = NULL;
	GtkWidget *child = NULL;

	notebook = GTK_WIDGET (gtk_builder_get_object(builder,"value_notebook"));
	child = GTK_WIDGET (gtk_builder_get_object(builder,"value_text_table"));
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),child,TRUE,TRUE,GTK_PACK_START);
	child = GTK_WIDGET (gtk_builder_get_object(builder,"value_font_table"));
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),child,TRUE,TRUE,GTK_PACK_START);
	child = GTK_WIDGET (gtk_builder_get_object(builder,"value_location_table"));
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),child,TRUE,TRUE,GTK_PACK_START);
	OBJ_SET((gtk_builder_get_object(builder,"precision_spin")),"handler",GINT_TO_POINTER(PRECISION));
	OBJ_SET((gtk_builder_get_object(builder,"value_xpos_spin")),"handler",GINT_TO_POINTER(VALUE_XPOS));
	OBJ_SET((gtk_builder_get_object(builder,"value_ypos_spin")),"handler",GINT_TO_POINTER(VALUE_YPOS));
	OBJ_SET((gtk_builder_get_object(builder,"value_grab_button")),"x_spin",gtk_builder_get_object(builder,"value_xpos_spin"));
	OBJ_SET((gtk_builder_get_object(builder,"value_grab_button")),"y_spin",gtk_builder_get_object(builder,"value_ypos_spin"));
	OBJ_SET((gtk_builder_get_object(builder,"value_font_scale_spin")),"handler",GINT_TO_POINTER(VALUE_FONTSCALE));
	OBJ_SET((gtk_builder_get_object(builder,"value_color_day_button")),"handler",GINT_TO_POINTER(GAUGE_COL_VALUE_FONT_DAY));
	OBJ_SET((gtk_builder_get_object(builder,"value_color_nite_button")),"handler",GINT_TO_POINTER(GAUGE_COL_VALUE_FONT_NITE));
	OBJ_SET((gtk_builder_get_object(builder,"left_justify_radio")),"handler",GINT_TO_POINTER(VALUE_JUSTIFICATION));
	OBJ_SET((gtk_builder_get_object(builder,"left_justify_radio")),"special_value",GINT_TO_POINTER(MTX_JUSTIFY_LEFT));
	OBJ_SET((gtk_builder_get_object(builder,"right_justify_radio")),"handler",GINT_TO_POINTER(VALUE_JUSTIFICATION));
	OBJ_SET((gtk_builder_get_object(builder,"right_justify_radio")),"special_value",GINT_TO_POINTER(MTX_JUSTIFY_RIGHT));
	OBJ_SET((gtk_builder_get_object(builder,"center_justify_radio")),"handler",GINT_TO_POINTER(VALUE_JUSTIFICATION));
	OBJ_SET((gtk_builder_get_object(builder,"center_justify_radio")),"special_value",GINT_TO_POINTER(MTX_JUSTIFY_CENTER));
	OBJ_SET((gtk_builder_get_object(builder,"show_value_check")),"handler",GINT_TO_POINTER(SHOW_VALUE));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(gtk_builder_get_object(builder,"value_color_day_button")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(gtk_builder_get_object(builder,"value_color_nite_button")),&black);

	update_text_controls();
	update_onscreen_tblocks();
	return TRUE;
}

G_MODULE_EXPORT gboolean init_general_attributes(GtkBuilder *builder)
{
	OBJ_SET((gtk_builder_get_object(builder,"cw_rbutton")),"handler",GINT_TO_POINTER(ROTATION));
	OBJ_SET((gtk_builder_get_object(builder,"cw_rbutton")),"special_value",GINT_TO_POINTER(MTX_ROT_CW));
	OBJ_SET((gtk_builder_get_object(builder,"ccw_rbutton")),"handler",GINT_TO_POINTER(ROTATION));
	OBJ_SET((gtk_builder_get_object(builder,"ccw_rbutton")),"special_value",GINT_TO_POINTER(MTX_ROT_CCW));
	OBJ_SET((gtk_builder_get_object(builder,"antialiased_check")),"handler",GINT_TO_POINTER(ANTIALIAS));
	OBJ_SET((gtk_builder_get_object(builder,"tattletale_check")),"handler",GINT_TO_POINTER(TATTLETALE));
	OBJ_SET((gtk_builder_get_object(builder,"tattletale_alpha_spin")),"handler",GINT_TO_POINTER(TATTLETALE_ALPHA));
	OBJ_SET((gtk_builder_get_object(builder,"needle_length_spin")),"handler",GINT_TO_POINTER(NEEDLE_LENGTH));
	OBJ_SET((gtk_builder_get_object(builder,"needle_tail_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL));
	OBJ_SET((gtk_builder_get_object(builder,"needle_tip_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_TIP_WIDTH));
	OBJ_SET((gtk_builder_get_object(builder,"needle_tail_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL_WIDTH));
	OBJ_SET((gtk_builder_get_object(builder,"needle_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_WIDTH));
	OBJ_SET((gtk_builder_get_object(builder,"start_angle_spin")),"handler",GINT_TO_POINTER(START_ANGLE));
	OBJ_SET((gtk_builder_get_object(builder,"sweep_angle_spin")),"handler",GINT_TO_POINTER(SWEEP_ANGLE));
	OBJ_SET((gtk_builder_get_object(builder,"lbound_spin")),"handler",GINT_TO_POINTER(LBOUND));
	OBJ_SET((gtk_builder_get_object(builder,"ubound_spin")),"handler",GINT_TO_POINTER(UBOUND));
	OBJ_SET((gtk_builder_get_object(builder,"background_color_day_button")),"handler",GINT_TO_POINTER(GAUGE_COL_BG_DAY));
	OBJ_SET((gtk_builder_get_object(builder,"background_color_nite_button")),"handler",GINT_TO_POINTER(GAUGE_COL_BG_NITE));
	OBJ_SET((gtk_builder_get_object(builder,"needle_color_day_button")),"handler",GINT_TO_POINTER(GAUGE_COL_NEEDLE_DAY));
	OBJ_SET((gtk_builder_get_object(builder,"needle_color_nite_button")),"handler",GINT_TO_POINTER(GAUGE_COL_NEEDLE_NITE));
	OBJ_SET((gtk_builder_get_object(builder,"gradient_begin_color_day_button")),"handler",GINT_TO_POINTER(GAUGE_COL_GRADIENT_BEGIN_DAY));
	OBJ_SET((gtk_builder_get_object(builder,"gradient_begin_color_nite_button")),"handler",GINT_TO_POINTER(GAUGE_COL_GRADIENT_BEGIN_NITE));
	OBJ_SET((gtk_builder_get_object(builder,"gradient_end_color_day_button")),"handler",GINT_TO_POINTER(GAUGE_COL_GRADIENT_END_DAY));
	OBJ_SET((gtk_builder_get_object(builder,"gradient_end_color_nite_button")),"handler",GINT_TO_POINTER(GAUGE_COL_GRADIENT_END_NITE));
	update_general_controls();
	return TRUE;
}


