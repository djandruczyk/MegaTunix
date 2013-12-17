#include <events.h>
#include <loadsave.h>
#include <stdio.h>

extern GtkBuilder *toplevel;
extern GdkColor black;
extern GdkColor white;
extern GtkWidget *gauge;
extern gboolean hold_handlers;
extern gboolean changed;
extern gboolean gauge_loaded;


G_MODULE_EXPORT gboolean text_attributes_menu_handler(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(toplevel,"tab_notebook")),TEXTBLOCK_TAB);

	return TRUE;
}


G_MODULE_EXPORT gboolean tick_groups_menu_handler(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(toplevel,"tab_notebook")),TICKGROUP_TAB);
	return TRUE;
}


G_MODULE_EXPORT gboolean polygon_menu_handler(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(toplevel,"tab_notebook")),POLYGON_TAB);
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
	GdkColor color;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return;

	if (!toplevel)
		return;

	hold_handlers = TRUE;

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"precision_spin"));
	mtx_gauge_face_get_attribute(g, PRECISION, &tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(gint)tmp1);

	mtx_gauge_face_get_attribute(g, VALUE_FONTSCALE, &tmp1);
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_font_scale_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	mtx_gauge_face_get_attribute(g, VALUE_XPOS, &tmp1);
	mtx_gauge_face_get_attribute(g, VALUE_YPOS, &tmp2);
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_xpos_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_ypos_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp2);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_font_button"));
	tmpbuf0 = mtx_gauge_face_get_value_font(g);
	tmpbuf = g_strdup_printf("%s 13",tmpbuf0);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	g_free(tmpbuf0);
	g_free(tmpbuf);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"show_value_check"));
	mtx_gauge_face_get_attribute(g, SHOW_VALUE, &tmp1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_color_day_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_VALUE_FONT_DAY, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_color_nite_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_VALUE_FONT_NITE, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	hold_handlers = FALSE;
}


void reset_text_controls()
{
	GtkWidget * widget = NULL;

	if ((!toplevel) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"precision_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_font_scale_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_xpos_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_ypos_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_font_button"));
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),g_strdup(""));

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"show_value_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"value_color_button"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	hold_handlers = FALSE;
}



G_MODULE_EXPORT gboolean general_attributes_menu_handler(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(toplevel,"tab_notebook")),GENERAL_TAB);
	return TRUE;
}


void update_general_controls()
{
	gfloat tmp1 = 0.0;
	GtkWidget * widget = NULL;
	MtxGaugeFace *g = NULL;
	GdkColor color;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return;

	if (!toplevel)
		return;

	hold_handlers = TRUE;

	mtx_gauge_face_get_attribute(g,ROTATION,&tmp1);
	if (tmp1 == MTX_ROT_CW)
		widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"cw_rbutton"));
	else if (tmp1 == MTX_ROT_CCW)
		widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"ccw_rbutton"));
	else
		widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"cw_rbutton"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(widget),TRUE);

	mtx_gauge_face_get_attribute(g,ANTIALIAS,&tmp1);
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"antialiased_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	mtx_gauge_face_get_attribute(g,TATTLETALE,&tmp1);
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"tattletale_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(gint)tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"tattletale_alpha_spin"));
	mtx_gauge_face_get_attribute(g,TATTLETALE_ALPHA,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_length_spin"));
	mtx_gauge_face_get_attribute(g,NEEDLE_LENGTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_width_spin"));
	mtx_gauge_face_get_attribute(g,NEEDLE_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_tip_width_spin"));
	mtx_gauge_face_get_attribute(g,NEEDLE_TIP_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_tail_width_spin"));
	mtx_gauge_face_get_attribute(g,NEEDLE_TAIL_WIDTH,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_tail_spin"));
	mtx_gauge_face_get_attribute(g,NEEDLE_TAIL,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"start_angle_spin"));
	mtx_gauge_face_get_attribute(g,START_ANGLE,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"sweep_angle_spin"));
	mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"lbound_spin"));
	mtx_gauge_face_get_attribute(g,LBOUND,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"ubound_spin"));
	mtx_gauge_face_get_attribute(g,UBOUND,&tmp1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tmp1);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"background_color_day_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_BG_DAY, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"background_color_nite_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_BG_NITE, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_color_day_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_NEEDLE_DAY, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_color_nite_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_NEEDLE_NITE, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"gradient_begin_color_day_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_BEGIN_DAY, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);
	
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"gradient_begin_color_nite_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_BEGIN_NITE, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);
	
	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"gradient_end_color_day_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_END_DAY, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"gradient_end_color_nite_button"));
	(void)mtx_gauge_face_get_color(g,GAUGE_COL_GRADIENT_END_NITE, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &color);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"daytime_radiobutton"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),mtx_gauge_face_get_daytime_mode(g));
	hold_handlers = FALSE;
}


void reset_general_controls()
{
	GtkWidget * widget = NULL;

	if ((!toplevel) || (!gauge))
		return;

	hold_handlers = TRUE;

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"cw_button"));
	gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"ccw_button"));
	gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"antialiased_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"tattletale_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"tattletale_alpha_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_width_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_tail_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"start_angle_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"sweep_angle_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"lbound_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"ubound_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),0);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"background_color_button"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&black);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"needle_color_button"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"gradient_begin_color_button"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&white);

	widget = GTK_WIDGET (gtk_builder_get_object(toplevel,"gradient_end_color_button"));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget),&black);


	hold_handlers = FALSE;
}


G_MODULE_EXPORT gboolean warning_ranges_menu_handler(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(toplevel,"tab_notebook")),WARNING_TAB);
	return TRUE;
}


G_MODULE_EXPORT gboolean alert_ranges_menu_handler(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(toplevel,"tab_notebook")),ALERT_TAB);
	return TRUE;
}

G_MODULE_EXPORT gboolean about_menu_handler(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{
	extern GtkWidget *main_window;
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		const gchar *authors[] = {"David Andruczyk",NULL};
		const gchar *artists[] = {"Dale Anderson",NULL};
		gtk_show_about_dialog(GTK_WINDOW(main_window),
				"name","MegaTunix Gauge Designer",
				"version",VERSION,
				"copyright","David J. Andruczyk(2012)",
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


G_MODULE_EXPORT gboolean quit_gaugedesigner(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{
	if ((gauge_loaded) && (changed))
		prompt_to_save();
	gtk_main_quit();
	return TRUE;
}


G_MODULE_EXPORT gboolean generic_spin_button_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	gfloat tmpf = 0.0;
	MtxGaugeFace *g = NULL;
	gint handler = 0;

	tmpf = (gfloat)gtk_spin_button_get_value((GtkSpinButton *)widget);
	if (!OBJ_GET((widget),"handler"))
	{
		printf("control %s has no handler\n",(gchar *)gtk_widget_get_name(widget));
		return FALSE;
	}
	handler = (GINT)OBJ_GET((widget),"handler");

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else
		return FALSE;

	if (hold_handlers)
		return TRUE;
	changed = TRUE;
	mtx_gauge_face_set_attribute(g,(MtxGenAttr)handler,tmpf);
	if ((handler == UBOUND) || (handler == LBOUND))
		update_attributes();
	return TRUE;
}


G_MODULE_EXPORT gboolean tg_spin_button_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	gint tmpi = 0;
	gfloat tmpf = 0.0;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	gfloat angle = 0.0;
	gfloat sweep = 0.0;
	gfloat tmp3 = 0.0;
	gfloat percent = 0.0;
	gfloat newval = 0.0;
	GtkWidget *lowpartner = NULL;
	GtkWidget *highpartner = NULL;
	MtxGaugeFace *g = NULL;
	gint handler = (GINT)OBJ_GET((widget),"spin_handler");
	tmpf = (gfloat)gtk_spin_button_get_value((GtkSpinButton *)widget);
	tmpi = (gint)(tmpf+0.00001);

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else
		return FALSE;

	if (hold_handlers)
		return TRUE;

	switch (handler)
	{
		case ADJ_LOW_UNIT_PARTNER:
			lowpartner = (GtkWidget *)OBJ_GET((widget),"lowpartner");
			highpartner = (GtkWidget *)OBJ_GET((widget),"highpartner");
			if ((!GTK_IS_WIDGET(lowpartner)) || 
					(!GTK_IS_WIDGET(highpartner)))
				break;
			mtx_gauge_face_get_attribute(g,START_ANGLE,&angle);
			mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&sweep);
			percent = (tmpf-angle)/(sweep);
			mtx_gauge_face_get_attribute(g,LBOUND,&lbound);
			mtx_gauge_face_get_attribute(g,UBOUND,&ubound);
			newval = ((ubound-lbound)*percent)+lbound;
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(lowpartner),newval);
			tmp3 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(OBJ_GET((widget),"high_angle")));
			percent = tmp3/sweep+((tmpf-angle)/sweep);
			newval = ((ubound-lbound)*percent)+lbound;
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(highpartner),newval);
			break;
		case ADJ_HIGH_UNIT_PARTNER:
			highpartner = (GtkWidget *)OBJ_GET((widget),"highpartner");
			tmp3 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(OBJ_GET((widget),"low_angle")));
			if (!GTK_IS_WIDGET(highpartner))
				break;
			mtx_gauge_face_get_attribute(g,START_ANGLE,&angle);
			mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&sweep);
			percent = tmpf/sweep+((tmp3-angle)/sweep);
			mtx_gauge_face_get_attribute(g,LBOUND,&lbound);
			mtx_gauge_face_get_attribute(g,UBOUND,&ubound);
			newval = ((ubound-lbound)*percent)+lbound;
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(highpartner),newval);
			break;
		case ADJ_START_ANGLE_PARTNER:
			lowpartner = (GtkWidget *)OBJ_GET((widget),"lowpartner");
			if (!GTK_IS_WIDGET(lowpartner))
				break;
			mtx_gauge_face_get_attribute(g,LBOUND,&lbound);
			mtx_gauge_face_get_attribute(g,UBOUND,&ubound);
			percent = (tmpf-lbound)/(ubound-lbound);
			mtx_gauge_face_get_attribute(g,START_ANGLE,&angle);
			mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&sweep);
			newval = ((sweep)*percent)+angle;
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(lowpartner),newval);
			break;
		case ADJ_SWEEP_ANGLE_PARTNER:
			highpartner = (GtkWidget *)OBJ_GET((widget),"highpartner");
			if (!GTK_IS_WIDGET(highpartner))
				break;
			tmp3 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(OBJ_GET((widget),"start_angle")));
			mtx_gauge_face_get_attribute(g,LBOUND,&lbound);
			mtx_gauge_face_get_attribute(g,UBOUND,&ubound);
			percent = (tmpf-lbound)/(ubound-lbound);
			mtx_gauge_face_get_attribute(g,START_ANGLE,&angle);
			mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&sweep);
			newval = ((percent*sweep)+angle)-tmp3;
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(highpartner),newval);
			break;
	}
	return (TRUE);
}


G_MODULE_EXPORT gboolean day_nite_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	MtxGaugeFace *g = NULL;
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return FALSE;
	if (state)
		mtx_gauge_face_set_daytime_mode(g,TRUE);
	else
		mtx_gauge_face_set_daytime_mode(g,FALSE);

	return TRUE;
}



G_MODULE_EXPORT gboolean radio_button_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gint value = (GINT)OBJ_GET((widget),"special_value");
	gint handler = (GINT)OBJ_GET((widget),"handler");
	MtxGaugeFace *g = NULL;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return FALSE;

	if (hold_handlers)
		return TRUE;

	changed = TRUE;
	if (state)
		mtx_gauge_face_set_attribute(g,(MtxGenAttr)handler, value);

	return TRUE;
}

G_MODULE_EXPORT gboolean checkbutton_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gint handler = (GINT)OBJ_GET((widget),"handler");
	MtxGaugeFace *g = NULL;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return FALSE;

	if (hold_handlers)
		return TRUE;

	changed = TRUE;
	mtx_gauge_face_set_attribute(g,(MtxGenAttr)handler, state);

	return TRUE;
}
