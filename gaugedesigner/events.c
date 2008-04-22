#include "../include/defines.h"
#include <events.h>
#include <handlers.h>
#include "../widgets/gauge.h"
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif

GtkWidget * gauge = NULL;
gboolean hold_handlers = FALSE;
GdkColor red = { 0, 65535, 0, 0};
GdkColor black = { 0, 0, 0, 0};
GdkColor white = { 0, 65535, 65535, 65535};
extern gboolean direct_path;



EXPORT gboolean create_new_gauge(GtkWidget * widget, gpointer data)
{
	GtkWidget *tmp = NULL;
	GladeXML *xml = glade_get_widget_tree(widget);
	GtkWidget *parent = glade_xml_get_widget(xml,"gauge_frame");
	gauge = mtx_gauge_face_new();
	gtk_container_add(GTK_CONTAINER(parent),gauge);
	gtk_widget_show_all(parent);
	mtx_gauge_face_redraw_canvas(MTX_GAUGE_FACE(gauge));

	tmp = glade_xml_get_widget(xml,"animate_frame");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"new_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	tmp = glade_xml_get_widget(xml,"close_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"load_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	tmp = glade_xml_get_widget(xml,"save_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"save_as_menuitem");
	gtk_widget_set_sensitive(tmp,TRUE);

	update_attributes();
	return (TRUE);
}



EXPORT gboolean close_current_gauge(GtkWidget * widget, gpointer data)
{
	GtkWidget *tmp = NULL;
	GladeXML *xml = glade_get_widget_tree(widget);
	GtkWidget *parent = glade_xml_get_widget(xml,"gauge_frame");

	if (GTK_IS_WIDGET(gauge))
		gtk_widget_destroy(gauge);
	gauge = NULL;

	tmp = glade_xml_get_widget(xml,"animate_frame");
	gtk_widget_set_sensitive(tmp,FALSE);

	tmp = glade_xml_get_widget(xml,"new_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"load_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,TRUE);

	tmp = glade_xml_get_widget(xml,"close_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	tmp = glade_xml_get_widget(xml,"save_gauge_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	tmp = glade_xml_get_widget(xml,"save_as_menuitem");
	gtk_widget_set_sensitive(tmp,FALSE);

	reset_onscreen_controls();
	direct_path = FALSE;
	gtk_widget_show_all(parent);
	return (TRUE);
}


EXPORT gboolean create_color_span_event(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *spinner = NULL;
	GtkWidget *cbutton = NULL;
	MtxColorRange *range = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gint result = 0;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "c_range_dialog", NULL);
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"c_range_dialog");
	cbutton = glade_xml_get_widget(xml,"range_colorbutton");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&white);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	/* Set the controls to sane ranges corresponding to the gauge */
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lbound);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &ubound);
	spinner = glade_xml_get_widget(xml,"range_lowpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	spinner = glade_xml_get_widget(xml,"range_highpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
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
			update_onscreen_c_ranges();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return (FALSE);
}


EXPORT gboolean create_alert_span_event(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *spinner = NULL;
	GtkWidget *cbutton = NULL;
	MtxAlertRange *range = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gint result = 0;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "a_range_dialog", NULL);
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"a_range_dialog");
	cbutton = glade_xml_get_widget(xml,"range_colorbutton");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&white);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	/* Set the controls to sane ranges corresponding to the gauge */
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lbound);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &ubound);
	spinner = glade_xml_get_widget(xml,"range_lowpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	spinner = glade_xml_get_widget(xml,"range_highpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			range = g_new0(MtxAlertRange, 1);
			range->lowpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_lowpoint_spin")));
			range->highpoint = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_highpoint_spin")));
			range->inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_inset_spin")));
			range->lwidth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"range_lwidth_spin")));
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"range_colorbutton")),&range->color);
			mtx_gauge_face_set_alert_range_struct(MTX_GAUGE_FACE(gauge),range);
			g_free(range);
			update_onscreen_a_ranges();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return (FALSE);
}


EXPORT gboolean create_polygon_event(GtkWidget * widget, gpointer wdata)
{
	GtkWidget *dialog = NULL;
	MtxPolygon *poly = NULL;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gchar * xn = NULL;
	gchar * yn = NULL;
	GtkWidget *dummy = NULL;
	gint i = 0;
	MtxPoint * points = NULL;
	void * data = NULL;
	GHashTable *hash = NULL;
	gint result = 0;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "polygon_dialog", NULL);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"polygon_dialog");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"polygon_colorbutton")),&white);
	OBJ_SET((glade_xml_get_widget(xml,"poly_combobox")),"container",glade_xml_get_widget(xml,"polygon_details_ebox"));
	OBJ_SET((glade_xml_get_widget(xml,"generic_num_points_spin")),"points_table",glade_xml_get_widget(xml,"generic_points_table"));
	hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	OBJ_SET((glade_xml_get_widget(xml,"generic_num_points_spin")),"points_hash",hash);
	g_free(filename);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			poly = g_new0(MtxPolygon, 1);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"polygon_colorbutton")),&poly->color);
			poly->filled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml,"poly_filled_cbutton")));
			tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(glade_xml_get_widget(xml,"line_style_combobox")));
			if (!tmpbuf)
				poly->line_style = GDK_LINE_SOLID;
			else if (g_strcasecmp(tmpbuf,"Solid") == 0)
				poly->line_style = GDK_LINE_SOLID;
			else if (g_strcasecmp(tmpbuf,"On Off Dash") == 0)
				poly->line_style = GDK_LINE_ON_OFF_DASH;
			else if (g_strcasecmp(tmpbuf,"Double Dash") == 0)
				poly->line_style = GDK_LINE_DOUBLE_DASH;
			if(tmpbuf)
				g_free(tmpbuf);
			tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(glade_xml_get_widget(xml,"join_style_combobox")));
			if (!tmpbuf)
				poly->line_style = GDK_JOIN_MITER;
			else if (g_strcasecmp(tmpbuf,"Miter") == 0)
				poly->line_style = GDK_JOIN_MITER;
			else if (g_strcasecmp(tmpbuf,"Round") == 0)
				poly->line_style = GDK_JOIN_ROUND;
			else if (g_strcasecmp(tmpbuf,"Bevel") == 0)
				poly->line_style = GDK_JOIN_BEVEL;
			if(tmpbuf)
				g_free(tmpbuf);
			poly->line_width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"line_width_spin")));
			tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(glade_xml_get_widget(xml,"poly_combobox")));
			if (!tmpbuf)
				tmpbuf = g_strdup("CIRCLE");
			if (g_strcasecmp(tmpbuf,"CIRCLE") == 0)
			{
				poly->type = MTX_CIRCLE;
				data = g_new0(MtxCircle, 1);
				poly->data = data;
				((MtxCircle *)data)->x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"circle_x_center_spin")));
				((MtxCircle *)data)->y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"circle_y_center_spin")));
				((MtxCircle *)data)->radius = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"circle_radius_spin")));
			}
			if (g_strcasecmp(tmpbuf,"ARC") == 0)
			{
				poly->type = MTX_ARC;
				data = g_new0(MtxArc, 1);
				poly->data = data;
				((MtxArc *)data)->x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"arc_x_left_spin")));
				((MtxArc *)data)->y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"arc_y_left_spin")));
				((MtxArc *)data)->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"arc_width_spin")));
				((MtxArc *)data)->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"arc_height_spin")));
				((MtxArc *)data)->start_angle = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"arc_start_spin")));
				((MtxArc *)data)->sweep_angle = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"arc_sweep_spin")));
			}
			if (g_strcasecmp(tmpbuf,"RECTANGLE") == 0)
			{
				poly->type = MTX_RECTANGLE;
				data = g_new0(MtxRectangle, 1);
				poly->data = data;
				((MtxRectangle *)data)->x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"rect_x_left_spin")));
				((MtxRectangle *)data)->y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"rect_y_left_spin")));
				((MtxRectangle *)data)->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"rect_width_spin")));
				((MtxRectangle *)data)->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"rect_height_spin")));
			}
			if (g_strcasecmp(tmpbuf,"GENERIC") == 0)
			{
				poly->type = MTX_GENPOLY;
				data = g_new0(MtxGenPoly, 1);
				poly->data = data;
				((MtxGenPoly *)data)->num_points = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"generic_num_points_spin")));
				hash = (GHashTable *)OBJ_GET((glade_xml_get_widget(xml,"generic_num_points_spin")),"points_hash");
				if (((MtxGenPoly *)data)->num_points > 0)
				{
					points = g_new0(MtxPoint, ((MtxGenPoly *)data)->num_points);
					((MtxGenPoly *)data)->points = points;
					for (i=0;i<((MtxGenPoly *)data)->num_points;i++)
					{
						xn = g_strdup_printf("generic_x_%i_spin",i);
						yn = g_strdup_printf("generic_y_%i_spin",i);
						dummy = g_hash_table_lookup(hash,xn);
						g_free(xn);
						if (GTK_IS_SPIN_BUTTON(dummy))
							points[i].x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dummy));
						dummy = g_hash_table_lookup(hash,yn);
						g_free(yn);
						if (GTK_IS_SPIN_BUTTON(dummy))
							points[i].y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dummy));
					}
				}
			}
			g_free(tmpbuf);
			if (hash)
				g_hash_table_destroy(hash);

			mtx_gauge_face_set_polygon_struct(MTX_GAUGE_FACE(gauge),poly);
			if ((poly->type == MTX_GENPOLY) && (((MtxGenPoly *)(poly->data))->num_points > 0))
				g_free(((MtxGenPoly *)(poly->data))->points);
			if (poly->data)
				g_free(poly->data);
			g_free(poly);
			update_onscreen_polygons();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return TRUE;
}


EXPORT gboolean create_text_block_event(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	MtxTextBlock *tblock = NULL;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gint result = 0;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "tblock_dialog", NULL);
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"tblock_dialog");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tblock_colorbutton")),&white);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			tblock = g_new0(MtxTextBlock, 1);
			tblock->font_scale = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tblock_font_scale_spin")));
			tblock->x_pos = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tblock_xpos_spin")));
			tblock->y_pos = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tblock_ypos_spin")));
			tblock->text = gtk_editable_get_chars(GTK_EDITABLE(glade_xml_get_widget(xml,"tblock_text_entry")),0,-1);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tblock_colorbutton")),&tblock->color);
			tblock->font = (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON(glade_xml_get_widget(xml,"tblock_fontbutton")));
			tblock->font = g_strchomp(g_strdelimit(tblock->font,"0123456789",' '));
			mtx_gauge_face_set_text_block_struct(MTX_GAUGE_FACE(gauge),tblock);
			g_free(tblock->text);
			g_free(tblock);
			update_onscreen_tblocks();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return (FALSE);
}


EXPORT gboolean create_tick_group_event(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *dummy = NULL;
	MtxTickGroup *tgroup = NULL;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	gint result = 0;
	MtxGaugeFace *g = NULL;
	
	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "tgroup_dialog", NULL);
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"tgroup_dialog");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_text_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_colorbutton")),&white);
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"lowpartner",glade_xml_get_widget(xml,"tg_lowpoint_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"highpartner",glade_xml_get_widget(xml,"tg_highpoint_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"high_angle",glade_xml_get_widget(xml,"tg_sweep_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"handler", GINT_TO_POINTER(ADJ_LOW_UNIT_PARTNER));
	OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"highpartner",glade_xml_get_widget(xml,"tg_highpoint_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"low_angle",glade_xml_get_widget(xml,"tg_start_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"handler", GINT_TO_POINTER(ADJ_HIGH_UNIT_PARTNER));
	OBJ_SET((glade_xml_get_widget(xml,"tg_lowpoint_spin")),"lowpartner",glade_xml_get_widget(xml,"tg_start_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_lowpoint_spin")),"handler", GINT_TO_POINTER(ADJ_START_ANGLE_PARTNER));
	OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"highpartner",glade_xml_get_widget(xml,"tg_sweep_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"start_angle",glade_xml_get_widget(xml,"tg_start_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"handler", GINT_TO_POINTER(ADJ_SWEEP_ANGLE_PARTNER));
	if (MTX_IS_GAUGE_FACE(g))
	{
		mtx_gauge_face_get_attribute(g,START_ANGLE,&tmp1);
		mtx_gauge_face_get_attribute(g,SWEEP_ANGLE,&tmp2);
		dummy = glade_xml_get_widget(xml,"tg_start_angle_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tmp1);
		dummy = glade_xml_get_widget(xml,"tg_sweep_angle_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tmp2);
	}


	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
		case GTK_RESPONSE_APPLY:
			tgroup = g_new0(MtxTickGroup, 1);
			tgroup->font = (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON(glade_xml_get_widget(xml,"tg_tick_fontbutton")));
			tgroup->font = g_strchomp(g_strdelimit(tgroup->font,"0123456789",' '));
			tgroup->text = gtk_editable_get_chars(GTK_EDITABLE(glade_xml_get_widget(xml,"tg_tick_textentry")),0,-1);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_text_colorbutton")),&tgroup->text_color);
			tgroup->font_scale = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_font_scale_spin")));
			tgroup->text_inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_text_inset_spin")));
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_colorbutton")),&tgroup->maj_tick_color);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_colorbutton")),&tgroup->min_tick_color);
			tgroup->maj_tick_inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_inset_spin")));
			tgroup->min_tick_inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_inset_spin")));
			tgroup->maj_tick_width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_width_spin")));
			tgroup->min_tick_width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_width_spin")));
			tgroup->maj_tick_length = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_length_spin")));
			tgroup->min_tick_length = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_length_spin")));
			tgroup->start_angle = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_start_angle_spin")));
			tgroup->sweep_angle = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_sweep_angle_spin")));
			tgroup->num_maj_ticks = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_num_maj_ticks_spin")));
			tgroup->num_min_ticks = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_num_min_ticks_spin")));

			mtx_gauge_face_set_tick_group_struct(MTX_GAUGE_FACE(gauge),tgroup);
			g_free(tgroup->text);
			g_free(tgroup);
			update_onscreen_tgroups();

			break;
		default:
			break;
	}
	if (GTK_IS_WIDGET(dialog))
		gtk_widget_destroy(dialog);

	return (FALSE);
}


EXPORT gboolean generic_spin_button_handler(GtkWidget *widget, gpointer data)
{
	gfloat tmpf = 0.0;
	MtxGaugeFace *g = NULL;
	gint handler = 0;

	tmpf = (gfloat)gtk_spin_button_get_value((GtkSpinButton *)widget);
	if (!OBJ_GET((widget),"handler"))
	{
		printf("control %s has no handler\n",(gchar *)glade_get_widget_name(widget));
		return FALSE;
	}
	handler = (gint)OBJ_GET((widget),"handler");

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else
		return FALSE;

	if (hold_handlers)
		return TRUE;
	mtx_gauge_face_set_attribute(g,handler,tmpf);
	return TRUE;
}


EXPORT gboolean tg_spin_button_handler(GtkWidget *widget, gpointer data)
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
	gint handler = (gint)OBJ_GET((widget),"handler");
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
			lowpartner = OBJ_GET((widget),"lowpartner");
			highpartner = OBJ_GET((widget),"highpartner");
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
			highpartner = OBJ_GET((widget),"highpartner");
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
			lowpartner = OBJ_GET((widget),"lowpartner");
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
			highpartner = OBJ_GET((widget),"highpartner");
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


void reset_onscreen_controls(void)
{
	reset_text_controls();
	reset_general_controls();
	reset_onscreen_c_ranges();
	reset_onscreen_a_ranges();
	reset_onscreen_tblocks();
	reset_onscreen_tgroups();
	reset_onscreen_polygons();
	return;
}

void update_attributes(void)
{
	update_text_controls();
	update_general_controls();
	update_onscreen_c_ranges();
	update_onscreen_a_ranges();
	update_onscreen_tblocks();
	update_onscreen_tgroups();
	update_onscreen_polygons();
	return;
}

EXPORT gboolean entry_change_color(GtkWidget * widget, gpointer data)
{
        gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	return TRUE;

}

EXPORT gboolean change_font(GtkWidget *widget, gpointer data)
{
	gchar * tmpbuf = NULL;
	MtxGaugeFace *g = NULL;
	tmpbuf = (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget));
	/* Strip out the font size as the gauge lib uses a different scaling
	 * method that scales with the size of the gauge
	 */
	tmpbuf = g_strchomp(g_strdelimit(tmpbuf,"0123456789",' '));

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return FALSE;

	if (hold_handlers)
		return TRUE;

	mtx_gauge_face_set_value_font(g, tmpbuf);
	return TRUE;
}


EXPORT gboolean checkbutton_handler(GtkWidget *widget, gpointer data)
{
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gint handler = (gint)OBJ_GET((widget),"handler");
	MtxGaugeFace *g = NULL;

	if (GTK_IS_WIDGET(gauge))
		g = MTX_GAUGE_FACE(gauge);
	else 
		return FALSE;

	if (hold_handlers)
		return TRUE;

	mtx_gauge_face_set_attribute(g,handler, state);

	return TRUE;
}

EXPORT gboolean color_button_color_set(GtkWidget *widget, gpointer data)
{
	GdkColor color;
	gint handler = (gint)OBJ_GET((widget),"handler");

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	if (hold_handlers)
		return TRUE;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
	mtx_gauge_face_set_color(MTX_GAUGE_FACE(gauge),handler,color);

	return TRUE;
}


void update_onscreen_c_ranges()
{
	GtkWidget *container = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *label = NULL;
	GtkWidget *button = NULL;
	gint i = 0;
	gint y = 1;
	gfloat low = 0.0;
	gfloat high = 0.0;
	MtxColorRange *range = NULL;
	GArray * array = NULL;
	extern GladeXML *c_ranges_xml;

	if ((!c_ranges_xml) || (!GTK_IS_WIDGET(gauge)))
		return;
	array = mtx_gauge_face_get_color_ranges(MTX_GAUGE_FACE(gauge));
	container = glade_xml_get_widget(c_ranges_xml,"color_range_viewport");
	if (!GTK_IS_WIDGET(container))
	{
		printf("color range viewport invalid!!\n");
		return;
	}
	y=1;
	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,6,FALSE);
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
		label = gtk_label_new("Color");
		gtk_table_attach(GTK_TABLE(table),label,5,6,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
	}
	/* Repopulate the table with the current ranges... */
	for (i=0;i<array->len; i++)
	{
		range = g_array_index(array,MtxColorRange *, i);
		button = gtk_check_button_new();
		OBJ_SET((button),"range_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_c_range),NULL);
		gtk_table_attach(GTK_TABLE(table),button,0,1,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &low);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &high);
		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lowpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_c_range_data),GINT_TO_POINTER(CR_LOWPOINT));

		gtk_table_attach(GTK_TABLE(table),dummy,1,2,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->highpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_c_range_data),GINT_TO_POINTER(CR_HIGHPOINT));
		gtk_table_attach(GTK_TABLE(table),dummy,2,3,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->inset);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_c_range_data),GINT_TO_POINTER(CR_INSET));
		gtk_table_attach(GTK_TABLE(table),dummy,3,4,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lwidth);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_c_range_data),GINT_TO_POINTER(CR_LWIDTH));
		gtk_table_attach(GTK_TABLE(table),dummy,4,5,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&range->color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_c_range_data),GINT_TO_POINTER(CR_COLOR));

		gtk_table_attach(GTK_TABLE(table),dummy,5,6,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		y++;
	}
	gtk_widget_show_all(container);
}


void update_onscreen_a_ranges()
{
	GtkWidget *container = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *label = NULL;
	GtkWidget *button = NULL;
	gint i = 0;
	gint y = 1;
	gfloat low = 0.0;
	gfloat high = 0.0;
	MtxAlertRange *range = NULL;
	GArray * array = NULL;
	extern GladeXML *a_ranges_xml;

	if ((!a_ranges_xml) || (!GTK_IS_WIDGET(gauge)))
		return;
	array = mtx_gauge_face_get_alert_ranges(MTX_GAUGE_FACE(gauge));
	container = glade_xml_get_widget(a_ranges_xml,"alert_range_viewport");
	if (!GTK_IS_WIDGET(container))
	{
		printf("alert range viewport invalid!!\n");
		return;
	}
	y=1;
	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,6,FALSE);
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
		label = gtk_label_new("Color");
		gtk_table_attach(GTK_TABLE(table),label,5,6,0,1,GTK_EXPAND,GTK_SHRINK,0,0);
	}
	/* Repopulate the table with the current ranges... */
	for (i=0;i<array->len; i++)
	{
		range = g_array_index(array,MtxAlertRange *, i);
		button = gtk_check_button_new();
		OBJ_SET((button),"range_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_a_range),NULL);
		gtk_table_attach(GTK_TABLE(table),button,0,1,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &low);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &high);
		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lowpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_a_range_data),GINT_TO_POINTER(CR_LOWPOINT));

		gtk_table_attach(GTK_TABLE(table),dummy,1,2,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->highpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_a_range_data),GINT_TO_POINTER(CR_HIGHPOINT));
		gtk_table_attach(GTK_TABLE(table),dummy,2,3,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->inset);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_a_range_data),GINT_TO_POINTER(CR_INSET));
		gtk_table_attach(GTK_TABLE(table),dummy,3,4,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lwidth);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_a_range_data),GINT_TO_POINTER(CR_LWIDTH));
		gtk_table_attach(GTK_TABLE(table),dummy,4,5,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&range->color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_a_range_data),GINT_TO_POINTER(CR_COLOR));

		gtk_table_attach(GTK_TABLE(table),dummy,5,6,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		y++;
	}
	gtk_widget_show_all(container);
}


void reset_onscreen_c_ranges()
{
	GtkWidget *container = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *c_ranges_xml;

	if ((!c_ranges_xml))
		return;
	container = glade_xml_get_widget(c_ranges_xml,"color_range_viewport");
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


void reset_onscreen_a_ranges()
{
	GtkWidget *container = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *a_ranges_xml;

	if ((!a_ranges_xml))
		return;
	container = glade_xml_get_widget(a_ranges_xml,"alert_range_viewport");
	if (!GTK_IS_WIDGET(container))
	{
		printf("alert range viewport invalid!!\n");
		return;
	}
	widget= gtk_bin_get_child(GTK_BIN(container));
	if (GTK_IS_WIDGET(widget))
		gtk_widget_destroy(widget);

	gtk_widget_show_all(container);
}


gboolean remove_c_range(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (gint)OBJ_GET((widget),"range_index");
	mtx_gauge_face_remove_color_range(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_c_ranges();

	return TRUE;
}


gboolean remove_a_range(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (gint)OBJ_GET((widget),"range_index");
	mtx_gauge_face_remove_alert_range(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_a_ranges();

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



EXPORT gboolean animate_gauge(GtkWidget *widget, gpointer data)
{
	gfloat lower = 0.0;
	gfloat upper = 0.0;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	gtk_widget_set_sensitive(widget,FALSE);

	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lower);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &upper);
	mtx_gauge_face_set_value(MTX_GAUGE_FACE (gauge),lower);
	g_timeout_add(20,(GtkFunction)sweep_gauge, (gpointer)gauge);
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
	GtkWidget * gauge = NULL;

	gauge = (GtkWidget *)data;

	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lower);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &upper);
	interval = (upper-lower)/75.0;
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


void update_onscreen_tblocks()
{
	GtkWidget *toptable = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *subtable2 = NULL;
	GtkWidget *button = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *label = NULL;
	GtkWidget *spin = NULL;
	gchar * tmpbuf = NULL;
	gint i = 0;
	gint y = 1;
	MtxTextBlock *tblock = NULL;
	GArray * array = NULL;
	extern GladeXML *text_xml;

	if ((!text_xml) || (!gauge))
		return;

	array = mtx_gauge_face_get_text_blocks(MTX_GAUGE_FACE(gauge));
	toptable = glade_xml_get_widget(text_xml,"text_blocks_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,1,FALSE);
	gtk_table_attach_defaults(GTK_TABLE(toptable),table,0,1,1,2);
	OBJ_SET((toptable),"layout_table",table);
	/* Repopulate the table with the current tblocks... */
	y=1;
	for (i=0;i<array->len; i++)
	{
		tblock = g_array_index(array,MtxTextBlock *, i);
		subtable = gtk_table_new(3,4,FALSE);
		gtk_table_set_row_spacings(GTK_TABLE(subtable),1);
		gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
		gtk_table_attach(GTK_TABLE(table),subtable,0,1,y,y+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);

		button = gtk_check_button_new();
		OBJ_SET((button),"tblock_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_tblock),NULL);
		gtk_table_attach(GTK_TABLE(subtable),button,0,1,0,3,GTK_SHRINK,GTK_FILL,0,0);
		label = gtk_label_new("Text");
		gtk_table_attach(GTK_TABLE(subtable),label,1,2,0,1,GTK_EXPAND,GTK_SHRINK,3,0);

		dummy = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(dummy),tblock->text);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_TEXT));
		gtk_table_attach(GTK_TABLE(subtable),dummy,2,3,0,1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&tblock->color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_COLOR));
		gtk_table_attach(GTK_TABLE(subtable),dummy,3,4,0,1,GTK_FILL,GTK_SHRINK,0,0);
		tmpbuf = g_strdup_printf("%s 12",tblock->font);
		dummy = gtk_font_button_new_with_font(tmpbuf);
		g_free(tmpbuf);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"font_set", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_FONT));
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(dummy),FALSE);

		gtk_table_attach(GTK_TABLE(subtable),dummy,1,3,1,2,GTK_FILL,GTK_SHRINK,0,0);
		spin = gtk_spin_button_new_with_range(0,1,0.001);
		OBJ_SET((spin),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin),tblock->font_scale);
		g_signal_connect(G_OBJECT(spin),"value_changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_FONT_SCALE));
		gtk_table_attach(GTK_TABLE(subtable),spin,3,4,1,2,GTK_FILL,GTK_SHRINK,0,0);

		subtable2 = gtk_table_new(1,4,FALSE);
		gtk_table_attach(GTK_TABLE(subtable),subtable2,1,4,2,3,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
		label = gtk_label_new("X Position");
		gtk_table_attach_defaults(GTK_TABLE(subtable2),label,0,1,0,1);

		label = gtk_label_new("Y Position");
		gtk_table_attach_defaults(GTK_TABLE(subtable2),label,2,3,0,1);
		spin = gtk_spin_button_new_with_range(-1,1,0.001);
		OBJ_SET((spin),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin),tblock->x_pos);
		g_signal_connect(G_OBJECT(spin),"value_changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_X_POS));
		gtk_table_attach(GTK_TABLE(subtable2),spin,1,2,0,1,GTK_FILL,GTK_FILL,0,0);

		spin = gtk_spin_button_new_with_range(-1,1,0.001);
		OBJ_SET((spin),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin),tblock->y_pos);
		g_signal_connect(G_OBJECT(spin),"value_changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_Y_POS));
		gtk_table_attach(GTK_TABLE(subtable2),spin,3,4,0,1,GTK_FILL,GTK_FILL,0,0);
		dummy = gtk_hseparator_new();
		gtk_table_attach(GTK_TABLE(table),dummy,0,1,y+1,y+2,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,2);

		y+=2;
	}
	gtk_widget_show_all(toptable);
}


void update_onscreen_tgroups()
{
	GtkWidget *toptable = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *button = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *tg_main_table = NULL;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gint i = 0;
	MtxTickGroup *tgroup = NULL;
	GArray * array = NULL;
	extern GladeXML *tick_groups_xml;

	if ((!tick_groups_xml) || (!gauge))
		return;

	array = mtx_gauge_face_get_tick_groups(MTX_GAUGE_FACE(gauge));

	toptable = glade_xml_get_widget(tick_groups_xml,"tick_groups_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (!filename)
	{
		printf("Can't locate primary glade file!!!!\n");
		return;
	}


	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,1,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_attach_defaults(GTK_TABLE(toptable),table,0,1,1,2);
	OBJ_SET((toptable),"layout_table",table);
	/* Repopulate the table with the current tgroups... */
	for (i=0;i<array->len; i++)
	{
		frame = gtk_frame_new(NULL);
		gtk_table_attach(GTK_TABLE(table),frame,0,1,i,i+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,5,0);
		tgroup = g_array_index(array,MtxTickGroup *, i);
		subtable = gtk_table_new(1,2,FALSE);
		gtk_container_add(GTK_CONTAINER(frame),subtable);

		button = gtk_check_button_new();
		OBJ_SET((button),"tgroup_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_tgroup),NULL);
		gtk_table_attach(GTK_TABLE(subtable),button,0,1,0,1,GTK_SHRINK,GTK_FILL,0,0);
		/* Load glade template */
		xml = glade_xml_new(filename, "tgroup_main_table", NULL);
		tg_main_table = glade_xml_get_widget(xml,"tgroup_main_table");
		gtk_table_attach(GTK_TABLE(subtable),tg_main_table,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
		OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"lowpartner",glade_xml_get_widget(xml,"tg_lowpoint_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"highpartner",glade_xml_get_widget(xml,"tg_highpoint_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"high_angle",glade_xml_get_widget(xml,"tg_sweep_angle_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"handler", GINT_TO_POINTER(ADJ_LOW_UNIT_PARTNER));
		OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"highpartner",glade_xml_get_widget(xml,"tg_highpoint_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"low_angle",glade_xml_get_widget(xml,"tg_start_angle_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"handler", GINT_TO_POINTER(ADJ_HIGH_UNIT_PARTNER));
		OBJ_SET((glade_xml_get_widget(xml,"tg_lowpoint_spin")),"lowpartner",glade_xml_get_widget(xml,"tg_start_angle_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_lowpoint_spin")),"handler", GINT_TO_POINTER(ADJ_START_ANGLE_PARTNER));
		OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"highpartner",glade_xml_get_widget(xml,"tg_sweep_angle_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"start_angle",glade_xml_get_widget(xml,"tg_start_angle_spin"));
		OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"handler", GINT_TO_POINTER(ADJ_SWEEP_ANGLE_PARTNER));
		glade_xml_signal_autoconnect(xml);

		/* fontbutton */
		dummy = glade_xml_get_widget(xml,"tg_tick_fontbutton");
		tmpbuf = g_strdup_printf("%s 12",tgroup->font);
		gtk_font_button_set_font_name(GTK_FONT_BUTTON(dummy),tmpbuf);
		g_free(tmpbuf);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"font_set", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_FONT));

		/* Font Scale*/
		dummy = glade_xml_get_widget(xml,"tg_font_scale_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->font_scale);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_FONT_SCALE));

		/* Text entry */
		dummy = glade_xml_get_widget(xml,"tg_tick_textentry");
		gtk_entry_set_text(GTK_ENTRY(dummy),tgroup->text);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_TEXT));

		/* Text Color*/
		dummy = glade_xml_get_widget(xml,"tg_text_colorbutton");
		gtk_color_button_set_color(GTK_COLOR_BUTTON(dummy),&tgroup->text_color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_TEXT_COLOR));

		/* Text Inset*/
		dummy = glade_xml_get_widget(xml,"tg_text_inset_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->text_inset);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_TEXT_INSET));

		/* Major Tick Color*/
		dummy = glade_xml_get_widget(xml,"tg_maj_tick_colorbutton");
		gtk_color_button_set_color(GTK_COLOR_BUTTON(dummy),&tgroup->maj_tick_color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MAJ_TICK_COLOR));

		/* Minor Tick Color*/
		dummy = glade_xml_get_widget(xml,"tg_min_tick_colorbutton");
		gtk_color_button_set_color(GTK_COLOR_BUTTON(dummy),&tgroup->min_tick_color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MIN_TICK_COLOR));

		/* Major Tick Inset*/
		dummy = glade_xml_get_widget(xml,"tg_maj_tick_inset_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->maj_tick_inset);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MAJ_TICK_INSET));

		/* Minor Tick Inset*/
		dummy = glade_xml_get_widget(xml,"tg_min_tick_inset_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->min_tick_inset);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MIN_TICK_INSET));

		/* Major Tick Width*/
		dummy = glade_xml_get_widget(xml,"tg_maj_tick_width_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->maj_tick_width);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MAJ_TICK_WIDTH));

		/* Minor Tick Width*/
		dummy = glade_xml_get_widget(xml,"tg_min_tick_width_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->min_tick_width);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MIN_TICK_WIDTH));

		/* Major Tick Length*/
		dummy = glade_xml_get_widget(xml,"tg_maj_tick_length_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->maj_tick_length);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MAJ_TICK_LENGTH));

		/* Minor Tick Length*/
		dummy = glade_xml_get_widget(xml,"tg_min_tick_length_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->min_tick_length);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_MIN_TICK_LENGTH));

		/* Start Angle*/
		dummy = glade_xml_get_widget(xml,"tg_start_angle_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->start_angle);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_START_ANGLE));

		/* Stop Angle*/
		dummy = glade_xml_get_widget(xml,"tg_sweep_angle_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->sweep_angle);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_SWEEP_ANGLE));

		/* Num Major Ticks*/
		dummy = glade_xml_get_widget(xml,"tg_num_maj_ticks_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->num_maj_ticks);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_NUM_MAJ_TICKS));

		/* Num Minor Ticks*/
		dummy = glade_xml_get_widget(xml,"tg_num_min_ticks_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),tgroup->num_min_ticks);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_tgroup_data),GINT_TO_POINTER(TG_NUM_MIN_TICKS));

	}
	g_free(filename);
	gtk_widget_show_all(toptable);
}


void update_onscreen_polygons()
{
	GtkWidget *toptable = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *button = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *tmpwidget = NULL;
	GHashTable *hash = NULL;
	GladeXML *xml = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gint i = 0;
	gint j = 0;
	MtxPolygon *poly = NULL;
	GArray * array = NULL;
	extern GladeXML *polygons_xml;

	if ((!polygons_xml) || (!gauge))
		return;

	array = mtx_gauge_face_get_polygons(MTX_GAUGE_FACE(gauge));

	toptable = glade_xml_get_widget(polygons_xml,"polygons_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"gaugedesigner.glade",NULL),NULL);
	if (!filename)
	{
		printf("Can't locate primary glade file!!!!\n");
		return;
	}


	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,1,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_attach_defaults(GTK_TABLE(toptable),table,0,1,1,2);
	OBJ_SET((toptable),"layout_table",table);
	gtk_widget_show_all(toptable);
	gdk_flush();
	/* Repopulate the table with the current polygons... */
	for (i=0;i<array->len; i++)
	{
		frame = gtk_frame_new(NULL);
		gtk_widget_show(frame);
		gtk_table_attach(GTK_TABLE(table),frame,0,1,i,i+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,5,0);
		poly = g_array_index(array,MtxPolygon *, i);
		subtable = gtk_table_new(1,2,FALSE);
		gtk_container_add(GTK_CONTAINER(frame),subtable);
		gtk_widget_show(subtable);

		button = gtk_check_button_new();
		OBJ_SET((button),"polygon_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_polygon),NULL);
		gtk_table_attach(GTK_TABLE(subtable),button,0,1,0,1,GTK_SHRINK,GTK_FILL,0,0);
		gtk_widget_show(button);
		/* Load glade template */
		xml = glade_xml_new(filename, "polygon_notebook", NULL);
		notebook = glade_xml_get_widget(xml,"polygon_notebook");
		gtk_table_attach(GTK_TABLE(subtable),notebook,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
		gtk_widget_show(notebook);
		gdk_flush();
		OBJ_SET((glade_xml_get_widget(xml,"poly_combobox")),"container",glade_xml_get_widget(xml,"polygon_details_ebox"));
		OBJ_SET((glade_xml_get_widget(xml,"generic_num_points_spin")),"points_table",glade_xml_get_widget(xml,"generic_points_table"));
		OBJ_SET((glade_xml_get_widget(xml,"generic_num_points_spin")),"live",GINT_TO_POINTER(TRUE));
		hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		OBJ_SET((glade_xml_get_widget(xml,"generic_num_points_spin")),"points_hash",hash);
		dummy = glade_xml_get_widget(xml,"arc_polygon_table");
		gtk_widget_hide_all(dummy);
		dummy = glade_xml_get_widget(xml,"circle_polygon_table");
		gtk_widget_hide_all(dummy);
		dummy = glade_xml_get_widget(xml,"rectangle_polygon_table");
		gtk_widget_hide_all(dummy);
		dummy = glade_xml_get_widget(xml,"generic_polygon_table");
		gtk_widget_hide_all(dummy);
		glade_xml_signal_autoconnect(xml);

		/* Colorbutton*/
		dummy = glade_xml_get_widget(xml,"polygon_colorbutton");
		gtk_color_button_set_color(GTK_COLOR_BUTTON(dummy),&poly->color);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_COLOR));

		/* Filled Checkbutton*/
		dummy = glade_xml_get_widget(xml,"poly_filled_cbutton");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dummy),poly->filled);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"toggled", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_FILLED));

		/* Line Style Combobox */
		dummy = glade_xml_get_widget(xml,"line_style_combobox");
		gtk_combo_box_set_active(GTK_COMBO_BOX(dummy),poly->line_style);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_LINESTYLE));
		/* Joint Style Combobox */
		dummy = glade_xml_get_widget(xml,"join_style_combobox");
		gtk_combo_box_set_active(GTK_COMBO_BOX(dummy),poly->join_style);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_JOINSTYLE));
		/* Line Width */
		dummy = glade_xml_get_widget(xml,"line_width_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),poly->line_width);
		OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_LINEWIDTH));

		if (poly->type == MTX_CIRCLE)
		{
			dummy = glade_xml_get_widget(xml,"poly_combobox");
			gtk_widget_destroy(dummy);
			dummy = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(dummy),"<b>Circle</b>");
			tmpwidget =  glade_xml_get_widget(xml,"polygon_details_table");
			gtk_table_attach(GTK_TABLE(tmpwidget),dummy,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
			gtk_widget_show(dummy);
			gtk_widget_show_all(glade_xml_get_widget(xml,"circle_polygon_table"));
			/* Update circle controls */
			/* X Center */
			dummy = glade_xml_get_widget(xml,"circle_x_center_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxCircle *)poly->data)->x);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_X));

			/* Y Center */
			dummy = glade_xml_get_widget(xml,"circle_y_center_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxCircle *)poly->data)->y);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_Y));

			/* Radius */
			dummy = glade_xml_get_widget(xml,"circle_radius_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxCircle *)poly->data)->radius);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_RADIUS));
		}
		if (poly->type == MTX_RECTANGLE)
		{
			dummy = glade_xml_get_widget(xml,"poly_combobox");
			gtk_widget_destroy(dummy);
			dummy = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(dummy),"<b>Rectangle</b>");
			tmpwidget =  glade_xml_get_widget(xml,"polygon_details_table");
			gtk_table_attach(GTK_TABLE(tmpwidget),dummy,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
			gtk_widget_show(dummy);

			gtk_widget_show_all(glade_xml_get_widget(xml,"rectangle_polygon_table"));
			/* Update Rectangle controls */
			/* Upper left X */
			dummy = glade_xml_get_widget(xml,"rect_x_left_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxRectangle *)poly->data)->x);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_X));

			/* Upper Left Y */
			dummy = glade_xml_get_widget(xml,"rect_y_left_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxRectangle *)poly->data)->y);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_Y));

			/* Width */
			dummy = glade_xml_get_widget(xml,"rect_width_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxRectangle *)poly->data)->width);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_WIDTH));

			/* Height */
			dummy = glade_xml_get_widget(xml,"rect_height_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxRectangle *)poly->data)->height);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_HEIGHT));
		}

		if (poly->type == MTX_ARC)
		{
			dummy = glade_xml_get_widget(xml,"poly_combobox");
			gtk_widget_destroy(dummy);
			dummy = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(dummy),"<b>Arc</b>");
			tmpwidget =  glade_xml_get_widget(xml,"polygon_details_table");
			gtk_table_attach(GTK_TABLE(tmpwidget),dummy,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
			gtk_widget_show(dummy);

			gtk_widget_show_all(glade_xml_get_widget(xml,"arc_polygon_table"));
			/* Update Arc controls */
			/* Upper left X */
			dummy = glade_xml_get_widget(xml,"arc_x_left_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxArc *)poly->data)->x);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_X));

			/* Upper Left Y */
			dummy = glade_xml_get_widget(xml,"arc_y_left_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxArc *)poly->data)->y);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_Y));

			/* Width */
			dummy = glade_xml_get_widget(xml,"arc_width_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxArc *)poly->data)->width);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_WIDTH));

			/* Height */
			dummy = glade_xml_get_widget(xml,"arc_height_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxArc *)poly->data)->height);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_HEIGHT));

			/* Start Angle */
			dummy = glade_xml_get_widget(xml,"arc_start_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxArc *)poly->data)->start_angle);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_START_ANGLE));

			/* Sweep Angle */
			dummy = glade_xml_get_widget(xml,"arc_sweep_spin");
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxArc *)poly->data)->sweep_angle);
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_SWEEP_ANGLE));
		}
		if (poly->type == MTX_GENPOLY)
		{
			dummy = glade_xml_get_widget(xml,"poly_combobox");
			gtk_widget_destroy(dummy);
			dummy = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(dummy),"<b>Generic Polygon</b>");
			tmpwidget =  glade_xml_get_widget(xml,"polygon_details_table");
			gtk_table_attach(GTK_TABLE(tmpwidget),dummy,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
			gtk_widget_show(dummy);

			gtk_widget_show_all(glade_xml_get_widget(xml,"generic_polygon_table"));
			/* Update Generic Polygon controls */


			/* Number of Generic Polygon vertexes */
			dummy = glade_xml_get_widget(xml,"generic_num_points_spin");
			OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxGenPoly *)poly->data)->num_points);
			g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_NUM_POINTS));
			gdk_flush();
			while (gtk_events_pending())
				gtk_main_iteration();
			for (j=0;j<((MtxGenPoly *)poly->data)->num_points;j++)
			{
				tmpbuf = g_strdup_printf("generic_x_%i_spin",j);
				dummy = g_hash_table_lookup(hash,tmpbuf);
				if (GTK_IS_SPIN_BUTTON(dummy))
				{
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxGenPoly *)poly->data)->points[j].x);
					OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
					OBJ_SET(dummy,"num_points_spin",glade_xml_get_widget(xml,"generic_num_points_spin"));
					g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_POINTS));
				}
				else
					printf("Spinbutton %s MISSING\n",tmpbuf);
				g_free(tmpbuf);
				tmpbuf = g_strdup_printf("generic_y_%i_spin",j);
				dummy = g_hash_table_lookup(hash,tmpbuf);
				if (GTK_IS_SPIN_BUTTON(dummy))
				{
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxGenPoly *)poly->data)->points[j].y);
					OBJ_SET(dummy,"index",GINT_TO_POINTER(i));
					OBJ_SET(dummy,"num_points_spin",glade_xml_get_widget(xml,"generic_num_points_spin"));
					g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_POINTS));
				}
				else
					printf("Spinbutton %s MISSING\n",tmpbuf);
				g_free(tmpbuf);

			}
		}

	}
	g_free(filename);
}


void reset_onscreen_tblocks()
{
	GtkWidget *toptable = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *text_xml;

	if ((!text_xml) || (!gauge))
		return;
	toptable = glade_xml_get_widget(text_xml,"text_blocks_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	widget = OBJ_GET((toptable),"layout_table");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_destroy(widget);

	gtk_widget_show_all(toptable);
}


void reset_onscreen_tgroups()
{
	GtkWidget *toptable = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *tick_groups_xml;

	if ((!tick_groups_xml))
		return;
	toptable = glade_xml_get_widget(tick_groups_xml,"tick_groups_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	widget = OBJ_GET((toptable),"layout_table");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_destroy(widget);

	gtk_widget_show_all(toptable);
}


void reset_onscreen_polygons()
{
	GtkWidget *toptable = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *polygons_xml;

	if ((!polygons_xml))
		return;
	toptable = glade_xml_get_widget(polygons_xml,"polygons_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	widget = OBJ_GET((toptable),"layout_table");
	if (GTK_IS_WIDGET(widget))
		gtk_widget_destroy(widget);

	gtk_widget_show_all(toptable);
}


gboolean alter_tblock_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	gchar * tmpbuf = NULL;
	GdkColor color;
	TbField field = (TbField)data;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case TB_FONT_SCALE:
		case TB_X_POS:
		case TB_Y_POS:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_text_block(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case TB_COLOR:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_text_block(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		case TB_FONT:
			tmpbuf = g_strdup(gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget)));
			tmpbuf = g_strchomp(g_strdelimit(tmpbuf,"0123456789",' '));
			mtx_gauge_face_alter_text_block(MTX_GAUGE_FACE(gauge),index,field,(void *)tmpbuf);
			g_free(tmpbuf);
			break;

		case TB_TEXT:
			tmpbuf = g_strdup(gtk_entry_get_text (GTK_ENTRY(widget)));
			mtx_gauge_face_alter_text_block(MTX_GAUGE_FACE(gauge),index,field,(void *)tmpbuf);
			g_free(tmpbuf);
		default:
			break;

	}
	return TRUE;
}

gboolean alter_tgroup_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	gchar * tmpbuf = NULL;
	GdkColor color;
	TgField field = (TgField)data;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case TG_FONT_SCALE:
		case TG_TEXT_INSET:
		case TG_MAJ_TICK_INSET:
		case TG_MIN_TICK_INSET:
		case TG_MAJ_TICK_WIDTH:
		case TG_MAJ_TICK_LENGTH:
		case TG_MIN_TICK_WIDTH:
		case TG_MIN_TICK_LENGTH:
		case TG_START_ANGLE:
		case TG_SWEEP_ANGLE:
		case TG_NUM_MAJ_TICKS:
		case TG_NUM_MIN_TICKS:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_tick_group(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case TG_MAJ_TICK_COLOR:
		case TG_MIN_TICK_COLOR:
		case TG_TEXT_COLOR:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_tick_group(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		case TG_FONT:
			tmpbuf = g_strdup(gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget)));
			tmpbuf = g_strchomp(g_strdelimit(tmpbuf,"0123456789",' '));
			mtx_gauge_face_alter_tick_group(MTX_GAUGE_FACE(gauge),index,field,(void *)tmpbuf);
			g_free(tmpbuf);
			break;

		case TG_TEXT:
			tmpbuf = g_strdup(gtk_entry_get_text (GTK_ENTRY(widget)));
			mtx_gauge_face_alter_tick_group(MTX_GAUGE_FACE(gauge),index,field,(void *)tmpbuf);
			g_free(tmpbuf);
		default:
			break;

	}
	return TRUE;
}


gboolean alter_polygon_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	GHashTable *hash = NULL;
	GtkWidget *tmpwidget = NULL;
	gint i = 0;
	gint num_points = 0;
	MtxPoint *points = NULL;
	GtkWidget *dummy = NULL;
	gchar * tmpbuf = NULL;
	GdkColor color;
	PolyField field = (PolyField)data;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case POLY_LINESTYLE:
		case POLY_JOINSTYLE:
			value = (gfloat)gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
			
		case POLY_X:
		case POLY_Y:
		case POLY_WIDTH:
		case POLY_HEIGHT:
		case POLY_RADIUS:
		case POLY_START_ANGLE:
		case POLY_SWEEP_ANGLE:
		case POLY_NUM_POINTS:
		case POLY_LINEWIDTH:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case POLY_FILLED:
			value = (float)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case POLY_COLOR:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		case POLY_POINTS:
			tmpwidget = OBJ_GET((widget),"num_points_spin");
			num_points = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(tmpwidget));
			hash = (GHashTable *)OBJ_GET((tmpwidget),"points_hash");
			points = g_new0(MtxPoint, num_points);
			for (i=0;i<num_points;i++)
			{
				tmpbuf =  g_strdup_printf("generic_x_%i_spin",i);
				dummy = g_hash_table_lookup(hash,tmpbuf);
				if (GTK_IS_SPIN_BUTTON(dummy))
					points[i].x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dummy));
				else
					printf("Spinbutton %s is invalid\n",tmpbuf);
				g_free(tmpbuf);
				tmpbuf =  g_strdup_printf("generic_y_%i_spin",i);
				dummy = g_hash_table_lookup(hash,tmpbuf);
				if (GTK_IS_SPIN_BUTTON(dummy))
					points[i].y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dummy));
				else
					printf("Spinbutton %s is invalid\n",tmpbuf);
				g_free(tmpbuf);
			}
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)points);
			g_free(points);

		default:
			break;

	}
	return TRUE;
}


gboolean alter_c_range_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	GdkColor color;
	CrField field = (CrField)data;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case CR_LOWPOINT:
		case CR_HIGHPOINT:
		case CR_INSET:
		case CR_LWIDTH:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_color_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case CR_COLOR:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_color_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		default:
			break;

	}
	return TRUE;
}


gboolean alter_a_range_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	GdkColor color;
	AlertField field = (AlertField)data;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case ALRT_LOWPOINT:
		case ALRT_HIGHPOINT:
		case ALRT_INSET:
		case ALRT_LWIDTH:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_alert_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case ALRT_COLOR:
			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
			mtx_gauge_face_alter_alert_range(MTX_GAUGE_FACE(gauge),index,field,(void *)&color);
			break;
		default:
			break;

	}
	return TRUE;
}


gboolean remove_tblock(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (gint)OBJ_GET((widget),"tblock_index");
	mtx_gauge_face_remove_text_block(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_tblocks();

	return TRUE;
}

gboolean remove_tgroup(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (gint)OBJ_GET((widget),"tgroup_index");
	mtx_gauge_face_remove_tick_group(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_tgroups();

	return TRUE;
}

gboolean remove_polygon(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (gint)OBJ_GET((widget),"polygon_index");
	mtx_gauge_face_remove_polygon(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_polygons();

	return TRUE;
}


gboolean polygon_type_changed_event(GtkWidget *widget, gpointer data)
{
	gchar *filename = NULL;
	GladeXML *xml = NULL;
	gchar * tmpbuf = NULL;
	gchar * up = NULL;
	GtkWidget *container = NULL;
	GtkWidget *circle_ctrls = NULL;
	GtkWidget *arc_ctrls = NULL;
	GtkWidget *rect_ctrls = NULL;
	GtkWidget *generic_ctrls = NULL;
	MtxPolyType type = -1;

	tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
	container = (GtkWidget *)OBJ_GET((widget),"container");
	filename = (gchar *)OBJ_GET((widget),"glade_file");
	up = g_ascii_strup(tmpbuf,-1);
	xml = glade_get_widget_tree(widget);
	arc_ctrls = glade_xml_get_widget(xml,"arc_polygon_table");
	circle_ctrls = glade_xml_get_widget(xml,"circle_polygon_table");
	rect_ctrls = glade_xml_get_widget(xml,"rectangle_polygon_table");
	generic_ctrls = glade_xml_get_widget(xml,"generic_polygon_table");
	gtk_widget_hide_all(arc_ctrls);
	gtk_widget_hide_all(circle_ctrls);
	gtk_widget_hide_all(rect_ctrls);
	gtk_widget_hide_all(generic_ctrls);
	if (g_strcasecmp(up,"CIRCLE") == 0 )
	{
		type = MTX_CIRCLE;
		gtk_widget_show_all(circle_ctrls);
	}
	if (g_strcasecmp(up,"ARC") == 0 )
	{
		type = MTX_ARC;
		gtk_widget_show_all(arc_ctrls);
	}
	if (g_strcasecmp(up,"RECTANGLE") == 0 )
	{
		type = MTX_RECTANGLE;
		gtk_widget_show_all(rect_ctrls);
	}
	if (g_strcasecmp(up,"GENERIC") == 0 )
	{
		type = MTX_GENPOLY;
		gtk_widget_show_all(generic_ctrls);
	}
	
	return TRUE;
}


gboolean adj_generic_num_points(GtkWidget *widget, gpointer data)
{
	GtkWidget *table = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *dummy1 = NULL;
	GtkWidget *dummy2 = NULL;
	GHashTable *hash = NULL;
	gchar *xn = NULL;
	gchar *yn = NULL;
	gchar *ln = NULL;
	gint num_points = -1;
	gint index = 0;
	gint i = 0;
	gint rows = 0;
	gboolean live = FALSE;

	table = (GtkWidget *)OBJ_GET((widget),"points_table");
	hash = (GHashTable *)OBJ_GET((widget),"points_hash");
	live = (gboolean)OBJ_GET((widget),"live");
	index = (gint)OBJ_GET((widget),"index");
	num_points = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));

	rows = ((GtkTable *)table)->nrows;

	if (num_points == 0)
		return TRUE;
	if (num_points == rows)
	{
		for (i=0;i<rows;i++)
		{
			ln = g_strdup_printf("index_%i_label",i);
			dummy = g_hash_table_lookup(hash,ln);
			xn = g_strdup_printf("generic_x_%i_spin",i);
			dummy1 = g_hash_table_lookup(hash,xn);
			yn = g_strdup_printf("generic_y_%i_spin",i);
			dummy2 = g_hash_table_lookup(hash,yn);
			if (!GTK_IS_LABEL(dummy))
			{
				dummy = gtk_label_new(g_strdup_printf("%i",i+1));
				gtk_table_attach(GTK_TABLE(table),dummy,0,1,i,i+1,0,0,15,0);
				g_hash_table_insert(hash,g_strdup(ln),dummy);
			}
			if (!GTK_IS_SPIN_BUTTON(dummy1))
			{
				dummy1 = gtk_spin_button_new_with_range(-1,1,0.001);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy1),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy1),3);
				if (live)
				{
					OBJ_SET((dummy1),"num_points_spin",widget);
					OBJ_SET((dummy1),"index",GINT_TO_POINTER(index));
					g_signal_connect(G_OBJECT(dummy1),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_POINTS));
				}
				gtk_table_attach(GTK_TABLE(table),dummy1,1,2,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(xn),dummy1);
			}
			if (!GTK_IS_SPIN_BUTTON(dummy2))
			{
				dummy2 = gtk_spin_button_new_with_range(-1,1,0.001);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy2),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy2),3);
				if (live)
				{
					OBJ_SET((dummy2),"num_points_spin",widget);
					OBJ_SET((dummy2),"index",GINT_TO_POINTER(index));
					g_signal_connect(G_OBJECT(dummy2),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_POINTS));
				}
				gtk_table_attach(GTK_TABLE(table),dummy2,2,3,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(yn),dummy2);
			}
			g_free(ln);
			g_free(xn);
			g_free(yn);

		}

	}
	/* If we are shrinking the table,  remove entries from hashtable
	 * and then resize table destroying the widgets */
	else if (rows > num_points)
	{
		/*printf("rows > num_points\n");*/
		for (i=rows;i>num_points;i--)
		{
			xn = g_strdup_printf("generic_x_%i_spin",i-1);
			dummy = g_hash_table_lookup(hash,xn);
			if (GTK_IS_SPIN_BUTTON(dummy))
				gtk_widget_destroy(dummy);

			yn = g_strdup_printf("generic_y_%i_spin",i-1);
			dummy = g_hash_table_lookup(hash,yn);
			if (GTK_IS_SPIN_BUTTON(dummy))
				gtk_widget_destroy(dummy);

			ln = g_strdup_printf("index_%i_label",i-1);
			dummy = g_hash_table_lookup(hash,ln);
			if (GTK_IS_LABEL(dummy))
				gtk_widget_destroy(dummy);

			g_hash_table_remove(hash,xn);
			g_hash_table_remove(hash,yn);
			g_hash_table_remove(hash,ln);
			g_free(xn);
			g_free(yn);
			g_free(ln);
			/*printf("removing controls on row %i\n",i);*/
		}
		gtk_table_resize(GTK_TABLE(table),num_points,3);
		/*printf("table shoulda been resized to %i,3\n",num_points);*/
	}
	else if (num_points > rows)
	{
		/*printf("num_points > rows\n");*/
		gtk_table_resize(GTK_TABLE(table),num_points,3);
		for (i=0;i<num_points;i++)
		{
			/*printf("creating new sets of spinners for row %i\n",i+1);*/
			xn = g_strdup_printf("generic_x_%i_spin",i);
			if (!GTK_IS_SPIN_BUTTON(g_hash_table_lookup(hash,xn)))
			{
				dummy = gtk_spin_button_new_with_range(-1,1,0.001);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy),3);
				if (live)
				{
					OBJ_SET(dummy,"num_points_spin",widget);
					OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				}
				gtk_table_attach(GTK_TABLE(table),dummy,1,2,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(xn),dummy);
			}

			yn = g_strdup_printf("generic_y_%i_spin",i);
			if (!GTK_IS_SPIN_BUTTON(g_hash_table_lookup(hash,yn)))
			{
				dummy = gtk_spin_button_new_with_range(-1,1,0.001);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy),3);
				if (live)
				{
					OBJ_SET(dummy,"num_points_spin",widget);
					OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				}
				gtk_table_attach(GTK_TABLE(table),dummy,2,3,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(yn),dummy);
			}

			ln = g_strdup_printf("index_%i_label",i);
			if (!GTK_IS_LABEL(g_hash_table_lookup(hash,ln)))
			{
				dummy = gtk_label_new(g_strdup_printf("%i",i+1));
				gtk_table_attach(GTK_TABLE(table),dummy,0,1,i,i+1,0,0,15,0);
				g_hash_table_insert(hash,g_strdup(ln),dummy);
			}
			g_free(xn);
			g_free(yn);
			g_free(ln);
		}

	}
	gtk_widget_show_all(table);

	return TRUE;
}
