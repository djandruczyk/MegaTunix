#include <config.h>
#include <defines.h>
#include <events.h>
#include <loadsave.h>
#include <gauge.h>
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <polygons.h>

extern GtkWidget * gauge;
extern GdkColor black;
extern GdkColor white;
extern gboolean direct_path;
extern gboolean changed;
extern gboolean gauge_loaded;
extern GtkWidget *main_window;
extern GtkBuilder *toplevel;
static gboolean poly_event_active = FALSE;


G_MODULE_EXPORT gboolean create_polygon_event(GtkWidget * widget, gpointer data)
{
	GtkBuilder *polygons = NULL;
	GtkWidget *dialog = NULL;
	gchar * filename = NULL;
	GHashTable *hash = NULL;
	GError * error = NULL;
	gint result = 0;

	if ((!GTK_IS_WIDGET(gauge)) || poly_event_active)
		return FALSE;

	filename = get_file(g_build_filename(GAUGEDESIGNER_GLADE_DIR,"polygon.glade",NULL),NULL);
	if (filename)
	{
		polygons = gtk_builder_new();
		if(!gtk_builder_add_from_file(polygons,filename, &error))
		{
			g_warning ("Couldn't load builder file: %s", error->message);
			g_error_free(error);
			exit(-1);
		}
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	gtk_builder_connect_signals(polygons,NULL);
	dialog = GTK_WIDGET (gtk_builder_get_object(polygons,"polygon_dialog"));
	if (!GTK_IS_WIDGET(dialog))
		return FALSE;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(gtk_builder_get_object(polygons,"polygon_day_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(gtk_builder_get_object(polygons,"polygon_nite_colorbutton")),&black);
	OBJ_SET((gtk_builder_get_object(polygons,"poly_combobox")),"container",gtk_builder_get_object(polygons,"polygon_details_ebox"));
	OBJ_SET(gtk_builder_get_object(polygons,"poly_combobox"),"builder",polygons);
	OBJ_SET(gtk_builder_get_object(polygons,"generic_num_points_spin"),"points_table",gtk_builder_get_object(polygons,"generic_points_table"));
	hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	OBJ_SET(gtk_builder_get_object(polygons,"generic_num_points_spin"),"points_hash",hash);
	/* Circle Handlers */
	OBJ_SET(gtk_builder_get_object(polygons,"circle_origin_edit_button"),"x_spin",gtk_builder_get_object(polygons,"circle_x_center_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"circle_origin_edit_button"),"y_spin",gtk_builder_get_object(polygons,"circle_y_center_spin"));
	/* Rectangle Origin Handlers */
	OBJ_SET(gtk_builder_get_object(polygons,"rectangle_origin_edit_button"),"x_spin",gtk_builder_get_object(polygons,"rect_x_left_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"rectangle_origin_edit_button"),"y_spin",gtk_builder_get_object(polygons,"rect_y_left_spin"));
	/* Rectangle W/H Handlers */
	OBJ_SET(gtk_builder_get_object(polygons,"rectangle_opposite_edit_button"),"x_spin",gtk_builder_get_object(polygons,"rect_width_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"rectangle_opposite_edit_button"),"y_spin",gtk_builder_get_object(polygons,"rect_height_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"rect_width_spin"),"x_o_spin",gtk_builder_get_object(polygons,"rect_x_left_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"rect_width_spin"),"y_o_spin",gtk_builder_get_object(polygons,"rect_y_left_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"rect_width_spin"),"relative",GINT_TO_POINTER(TRUE));
	/* Arc Origin Handlers */
	OBJ_SET(gtk_builder_get_object(polygons,"arc_origin_edit_button"),"x_spin",gtk_builder_get_object(polygons,"arc_x_left_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"arc_origin_edit_button"),"y_spin",gtk_builder_get_object(polygons,"arc_y_left_spin"));
	/* Arc W/H Handlers */
	OBJ_SET(gtk_builder_get_object(polygons,"arc_opposite_edit_button"),"x_spin",gtk_builder_get_object(polygons,"arc_width_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"arc_opposite_edit_button"),"y_spin",gtk_builder_get_object(polygons,"arc_height_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"arc_width_spin"),"x_o_spin",gtk_builder_get_object(polygons,"arc_x_left_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"arc_width_spin"),"y_o_spin",gtk_builder_get_object(polygons,"arc_y_left_spin"));
	OBJ_SET(gtk_builder_get_object(polygons,"arc_width_spin"),"relative",GINT_TO_POINTER(TRUE));

	g_free(filename);

	gtk_widget_show_all(dialog);
	poly_event_active = TRUE;
	g_signal_connect(G_OBJECT(dialog),"response", G_CALLBACK(new_poly_response),polygons);
	return TRUE;
}


gboolean new_poly_response(GtkDialog *dialog, gint response, gpointer user_data)
{
	MtxPolygon *poly = NULL;
	gchar * xn = NULL;
	gchar * yn = NULL;
	GtkWidget *dummy = NULL;
	gint i = 0;
	MtxPoint * points = NULL;
	void * data = NULL;
	GHashTable *hash = NULL;
	gchar * tmpbuf = NULL;
	GtkBuilder *polygons = (GtkBuilder *)user_data;

	switch (response)
	{
		case GTK_RESPONSE_APPLY:
			poly = g_new0(MtxPolygon, 1);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(gtk_builder_get_object(polygons,"polygon_day_colorbutton")),&poly->color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(gtk_builder_get_object(polygons,"polygon_nite_colorbutton")),&poly->color[MTX_NITE]);
			poly->filled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(polygons,"poly_filled_cbutton")));
			tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(gtk_builder_get_object(polygons,"line_style_combobox")));
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
			tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(gtk_builder_get_object(polygons,"join_style_combobox")));
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
			poly->line_width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"line_width_spin")));
			tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(gtk_builder_get_object(polygons,"poly_combobox")));
			if (!tmpbuf)
				tmpbuf = g_strdup("CIRCLE");
			if (g_strcasecmp(tmpbuf,"CIRCLE") == 0)
			{
				poly->type = MTX_CIRCLE;
				data = g_new0(MtxCircle, 1);
				poly->data = data;
				((MtxCircle *)data)->x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"circle_x_center_spin")));
				((MtxCircle *)data)->y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"circle_y_center_spin")));
				((MtxCircle *)data)->radius = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"circle_radius_spin")));
			}
			if (g_strcasecmp(tmpbuf,"ARC") == 0)
			{
				poly->type = MTX_ARC;
				data = g_new0(MtxArc, 1);
				poly->data = data;
				((MtxArc *)data)->x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"arc_x_left_spin")));
				((MtxArc *)data)->y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"arc_y_left_spin")));
				((MtxArc *)data)->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"arc_width_spin")));
				((MtxArc *)data)->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"arc_height_spin")));
				((MtxArc *)data)->start_angle = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"arc_start_spin")));
				((MtxArc *)data)->sweep_angle = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"arc_sweep_spin")));
			}
			if (g_strcasecmp(tmpbuf,"RECTANGLE") == 0)
			{
				poly->type = MTX_RECTANGLE;
				data = g_new0(MtxRectangle, 1);
				poly->data = data;
				((MtxRectangle *)data)->x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"rect_x_left_spin")));
				((MtxRectangle *)data)->y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"rect_y_left_spin")));
				((MtxRectangle *)data)->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"rect_width_spin")));
				((MtxRectangle *)data)->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"rect_height_spin")));
			}
			if (g_strcasecmp(tmpbuf,"GENERIC") == 0)
			{
				poly->type = MTX_GENPOLY;
				data = g_new0(MtxGenPoly, 1);
				poly->data = data;
				((MtxGenPoly *)data)->num_points = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(polygons,"generic_num_points_spin")));
				hash = (GHashTable *)OBJ_GET((gtk_builder_get_object(polygons,"generic_num_points_spin")),"points_hash");
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

			changed = TRUE;
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
		gtk_widget_destroy(GTK_WIDGET(dialog));

	return TRUE;
}


void update_onscreen_polygons()
{
	GtkWidget *toptable = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *button = NULL;
	GtkWidget *img = NULL;
	GtkWidget *x_origin = NULL;
	GtkWidget *y_origin = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *tmpwidget = NULL;
	GtkAdjustment *adj = NULL;
	GHashTable *hash = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	guint i = 0;
	gint j = 0;
	MtxPolygon *poly = NULL;
	GArray * array = NULL;

	if ((!toplevel) || (!gauge))
		return;

	array = mtx_gauge_face_get_polygons(MTX_GAUGE_FACE(gauge));

	toptable = GTK_WIDGET (gtk_builder_get_object(toplevel,"polygons_layout_table"));
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
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
		poly = g_array_index(array,MtxPolygon *, i);
		subtable = build_polygon(poly,i);
		gtk_table_attach(GTK_TABLE(table),subtable,0,1,i,i+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
	}
	gtk_widget_show_all(table);
	g_free(filename);
	/* Scroll to end */
	dummy = GTK_WIDGET (gtk_builder_get_object(toplevel,"poly_swin"));
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dummy));
	adj->value = adj->upper;
}


void reset_onscreen_polygons()
{
	GtkWidget *toptable = NULL;
	GtkWidget *widget = NULL;

	if ((!toplevel))
		return;
	toptable = GTK_WIDGET (gtk_builder_get_object(toplevel,"polygons_layout_table"));
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


gboolean alter_polygon_data(GtkWidget *widget, gpointer data)
{
	gint index = (GINT)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	GHashTable *hash = NULL;
	GtkWidget *tmpwidget = NULL;
	gint i = 0;
	gint num_points = 0;
	MtxPoint *points = NULL;
	GtkWidget *dummy = NULL;
	gchar * tmpbuf = NULL;
	GdkColor color;
	PolyField field = 0;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	field = (PolyField)OBJ_GET(widget,"handler");;

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
		case POLY_LAYER:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case POLY_FILLED:
			value = (float)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			mtx_gauge_face_alter_polygon(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case POLY_COLOR_DAY:
		case POLY_COLOR_NITE:
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


gboolean remove_polygon(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (GINT)OBJ_GET((widget),"polygon_index");
	mtx_gauge_face_remove_polygon(MTX_GAUGE_FACE(gauge),index);
	changed = TRUE;
	update_onscreen_polygons();

	return TRUE;
}


gboolean polygon_type_changed_event(GtkWidget *widget, gpointer data)
{
	gchar * tmpbuf = NULL;
	gchar * up = NULL;
	GtkBuilder *polygons = NULL;
	GtkWidget *container = NULL;
	GtkWidget *circle_ctrls = NULL;
	GtkWidget *arc_ctrls = NULL;
	GtkWidget *rect_ctrls = NULL;
	GtkWidget *generic_ctrls = NULL;
	MtxPolyType type = -1;

	polygons = OBJ_GET(widget,"builder");
	tmpbuf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
	container = (GtkWidget *)OBJ_GET((widget),"container");
	up = g_ascii_strup(tmpbuf,-1);
	arc_ctrls = GTK_WIDGET (gtk_builder_get_object(polygons,"arc_polygon_table"));
	circle_ctrls = GTK_WIDGET (gtk_builder_get_object(polygons,"circle_polygon_table"));
	rect_ctrls = GTK_WIDGET (gtk_builder_get_object(polygons,"rectangle_polygon_table"));
	generic_ctrls = GTK_WIDGET (gtk_builder_get_object(polygons,"generic_polygon_table"));
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
	GtkWidget *x_spin = NULL;
	GtkWidget *y_spin = NULL;
	GtkWidget *img = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	GHashTable *hash = NULL;
	gchar *xn = NULL;
	gchar *yn = NULL;
	gchar *ln = NULL;
	gchar *en = NULL;
	gint num_points = -1;
	gint index = 0;
	gint i = 0;
	gint rows = 0;
	gboolean live = FALSE;

	table = (GtkWidget *)OBJ_GET((widget),"points_table");
	hash = (GHashTable *)OBJ_GET((widget),"points_hash");
	live = (GBOOLEAN)OBJ_GET((widget),"live");
	index = (GINT)OBJ_GET((widget),"index");
	num_points = (GINT)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));

	rows = ((GtkTable *)table)->nrows;

	if (num_points == 0)
		return TRUE;
	if (num_points == rows)
	{
		for (i=0;i<rows;i++)
		{
			ln = g_strdup_printf("index_%i_label",i);
			dummy = g_hash_table_lookup(hash,ln);
			if (!GTK_IS_LABEL(dummy))
			{
				dummy = gtk_label_new(g_strdup_printf("%i",i+1));
				gtk_table_attach(GTK_TABLE(table),dummy,0,1,i,i+1,0,0,2,0);
				g_hash_table_insert(hash,g_strdup(ln),dummy);
			}
			xn = g_strdup_printf("generic_x_%i_spin",i);
			dummy = g_hash_table_lookup(hash,xn);
			if (!GTK_IS_SPIN_BUTTON(dummy))
			{
				dummy = gtk_spin_button_new_with_range(-1,1,0.001);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy),3);
				OBJ_SET((dummy),"num_points_spin",widget);
				OBJ_SET((dummy),"index",GINT_TO_POINTER(index));
				if (live)
					g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_POINTS));
				gtk_table_attach(GTK_TABLE(table),dummy,1,2,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(xn),dummy);
			}
			x_spin = dummy;
			yn = g_strdup_printf("generic_y_%i_spin",i);
			dummy = g_hash_table_lookup(hash,yn);
			if (!GTK_IS_SPIN_BUTTON(dummy))
			{
				dummy = gtk_spin_button_new_with_range(-1,1,0.001);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy),3);
				OBJ_SET((dummy),"num_points_spin",widget);
				OBJ_SET((dummy),"index",GINT_TO_POINTER(index));
				if (live)
					g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),GINT_TO_POINTER(POLY_POINTS));
				gtk_table_attach(GTK_TABLE(table),dummy,2,3,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(yn),dummy);
			}
			y_spin = dummy;
			en = g_strdup_printf("generic_edit_button_%i",i);
			dummy = g_hash_table_lookup(hash,en);
			if (!GTK_IS_BUTTON(dummy))
			{
				dummy = gtk_button_new();
				hbox = gtk_hbox_new(FALSE,0);
				gtk_container_add(GTK_CONTAINER(dummy),hbox);
				img = gtk_image_new_from_stock("gtk-edit",GTK_ICON_SIZE_MENU);
				gtk_box_pack_start(GTK_BOX(hbox),img,FALSE,FALSE,0);
				label = gtk_label_new("Edit");
				gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
				g_signal_connect(G_OBJECT(dummy),"clicked",G_CALLBACK(grab_coords_event),NULL);
				OBJ_SET(dummy,"num_points_spin",widget);
				OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				OBJ_SET(dummy,"x_spin",x_spin);
				OBJ_SET(dummy,"y_spin",y_spin);
				gtk_table_attach(GTK_TABLE(table),dummy,3,4,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(en),dummy);
			}
			g_free(ln);
			g_free(xn);
			g_free(yn);
			g_free(en);

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

			en = g_strdup_printf("generic_edit_button_%i",i-1);
			dummy = g_hash_table_lookup(hash,en);
			if (GTK_IS_BUTTON(dummy))
				gtk_widget_destroy(dummy);

			g_hash_table_remove(hash,xn);
			g_hash_table_remove(hash,yn);
			g_hash_table_remove(hash,ln);
			g_hash_table_remove(hash,en);
			g_free(xn);
			g_free(yn);
			g_free(ln);
			g_free(en);
			/*printf("removing controls on row %i\n",i);*/
		}
		gtk_table_resize(GTK_TABLE(table),num_points,4);
		/*printf("table shoulda been resized to %i,3\n",num_points);*/
	}
	else if (num_points > rows)
	{
		/*printf("num_points > rows\n");*/
		gtk_table_resize(GTK_TABLE(table),num_points,4);
		for (i=0;i<num_points;i++)
		{
			/*printf("creating new sets of spinners for row %i\n",i+1);*/
			ln = g_strdup_printf("index_%i_label",i);
			if (!GTK_IS_LABEL(g_hash_table_lookup(hash,ln)))
			{
				dummy = gtk_label_new(g_strdup_printf("%i",i+1));
				gtk_table_attach(GTK_TABLE(table),dummy,0,1,i,i+1,0,0,2,0);
				g_hash_table_insert(hash,g_strdup(ln),dummy);
			}
			xn = g_strdup_printf("generic_x_%i_spin",i);
			if (!GTK_IS_SPIN_BUTTON(g_hash_table_lookup(hash,xn)))
			{
				dummy = gtk_spin_button_new_with_range(-1,1,0.001);
				x_spin = dummy;
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy),3);
				OBJ_SET(dummy,"num_points_spin",widget);
				OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				OBJ_SET(dummy,"handler",GINT_TO_POINTER(POLY_POINTS));
				if (live)
					g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),NULL);
				gtk_table_attach(GTK_TABLE(table),dummy,1,2,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(xn),dummy);
			}

			yn = g_strdup_printf("generic_y_%i_spin",i);
			if (!GTK_IS_SPIN_BUTTON(g_hash_table_lookup(hash,yn)))
			{
				dummy = gtk_spin_button_new_with_range(-1,1,0.001);
				y_spin = dummy;
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),0.0);
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dummy),3);
				OBJ_SET(dummy,"num_points_spin",widget);
				OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				OBJ_SET(dummy,"handler",GINT_TO_POINTER(POLY_POINTS));
				if (live)
					g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_polygon_data),NULL);
				gtk_table_attach(GTK_TABLE(table),dummy,2,3,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(yn),dummy);
			}

			en = g_strdup_printf("generic_edit_button_%i",i);
			if (!GTK_IS_BUTTON(g_hash_table_lookup(hash,en)))
			{
				dummy = gtk_button_new();
				hbox = gtk_hbox_new(FALSE,0);
				gtk_container_add(GTK_CONTAINER(dummy),hbox);
				img = gtk_image_new_from_stock("gtk-edit",GTK_ICON_SIZE_MENU);
				gtk_box_pack_start(GTK_BOX(hbox),img,FALSE,FALSE,0);
				label = gtk_label_new("Edit");
				gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
				g_signal_connect(G_OBJECT(dummy),"clicked",G_CALLBACK(grab_coords_event),NULL);
				OBJ_SET(dummy,"num_points_spin",widget);
				OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				OBJ_SET(dummy,"x_spin",x_spin);
				OBJ_SET(dummy,"y_spin",y_spin);
				gtk_table_attach(GTK_TABLE(table),dummy,3,4,i,i+1,0,0,0,0);
				g_hash_table_insert(hash,g_strdup(en),dummy);
			}
			g_free(xn);
			g_free(yn);
			g_free(ln);
			g_free(en);
		}

	}
	gtk_widget_show_all(table);

	return TRUE;
}


GtkWidget *build_polygon(MtxPolygon *poly, gint index)
{
	GtkWidget *notebook = NULL;
	GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *button = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *x_origin = NULL;
	GtkWidget *y_origin = NULL;
	GtkWidget *x_spin = NULL;
	GtkWidget *dummy = NULL;
	GtkWidget *y_spin = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *img = NULL;
	GtkWidget *label = NULL;
	GtkWidget *minitable = NULL;
	GtkWidget *align = NULL;
	GtkWidget *points_table = NULL;
	gchar * tmpbuf = NULL;
	gint j = 0;
	GHashTable *hash = NULL;

	table = gtk_table_new(2,2,FALSE);

	button = gtk_button_new();
	img = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
	gtk_container_add(GTK_CONTAINER(button),img);
	OBJ_SET((button),"polygon_index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(remove_polygon),NULL);
	gtk_table_attach(GTK_TABLE(table),button,0,1,0,1,0,0,0,0);

	notebook = gtk_notebook_new();
	gtk_table_attach(GTK_TABLE(table),notebook,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
	/* Basics Tab (common to all polygons) */
	subtable = gtk_table_new(2,4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(subtable),2);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Basics");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);
	minitable = gtk_table_new(3,1,FALSE);
	gtk_table_attach(GTK_TABLE(subtable),minitable,0,1,0,2,GTK_EXPAND,0,0,0);
	widget = gtk_label_new(NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,2,GTK_EXPAND,GTK_EXPAND|GTK_FILL,0,0);
	switch (poly->type)
	{
		case MTX_CIRCLE:
			gtk_label_set_markup(GTK_LABEL(widget),"<big><b>Circle</b></big>");
			break;
		case MTX_RECTANGLE:
			gtk_label_set_markup(GTK_LABEL(widget),"<big><b>Rectangle</b></big>");
			break;
		case MTX_ARC:
			gtk_label_set_markup(GTK_LABEL(widget),"<big><b>Arc</b></big>");
			break;
		case MTX_GENPOLY:
			gtk_label_set_markup(GTK_LABEL(widget),"<big><b>Generic</b></big>");
			break;
		default:
			break;
	}
	widget = gtk_check_button_new_with_label("Filled");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),poly->filled);
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_FILLED));
	g_signal_connect(G_OBJECT(widget),"toggled", G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,2,3,GTK_EXPAND|GTK_FILL,0,0,0);

	widget = gtk_label_new("Line Style");
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND,0,0,0);
	widget = gtk_label_new("Joint Style");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,GTK_EXPAND,0,0,0);
	widget = gtk_label_new("Line Width");
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,GTK_EXPAND,0,0,0);

	widget = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget),"Solid");
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget),"Dash");
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),poly->line_style);
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_LINESTYLE));
	g_signal_connect(G_OBJECT(widget),"changed", G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,GTK_FILL,GTK_FILL,0,0);

	widget = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget),"Miter");
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget),"Round");
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget),"Bevel");
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),poly->join_style);
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_JOINSTYLE));
	g_signal_connect(G_OBJECT(widget),"changed", G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,GTK_FILL,GTK_FILL,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_LINEWIDTH));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", poly->line_width, NULL);
	g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,0,0,0,0);


	/* Color Tab (common to all polygons) */
	subtable = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Colors");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_END);
	widget = gtk_label_new("Day");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_EXPAND,0,0,0);

	widget = gtk_label_new("Nite");
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND,0,0,0);

	widget = gtk_color_button_new_with_color(&poly->color[MTX_DAY]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_COLOR_DAY));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,GTK_FILL,0,0,0);
	widget = gtk_color_button_new_with_color(&poly->color[MTX_NITE]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_COLOR_NITE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,GTK_FILL,0,0,0);

	if (poly->type == MTX_CIRCLE)	/* Circles */
	{
		/* Circle Tab */
		subtable = gtk_table_new(2,5,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
		label = gtk_label_new("Circle Attributes.");
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
		gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);
		widget = gtk_label_new("X center");
		gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_label_new("Y center");
		gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_label_new("Radius");
		gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,GTK_EXPAND,0,0,0);

		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_X));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxCircle *)poly->data)->x, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,0,0,0,0);
		x_spin = widget;	

		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_Y));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxCircle *)poly->data)->y, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,0,0,0,0);
		y_spin = widget;	

		widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_RADIUS));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxCircle *)poly->data)->radius, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,0,0,0,0);

		widget = gtk_button_new();
		OBJ_SET(widget,"x_spin",x_spin);
		OBJ_SET(widget,"y_spin",y_spin);
		hbox = gtk_hbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(widget),hbox);
		img = gtk_image_new_from_stock("gtk-edit",GTK_ICON_SIZE_MENU);
		gtk_box_pack_start(GTK_BOX(hbox),img,FALSE,FALSE,0);
		label = gtk_label_new("Edit");
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(grab_coords_event),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
	}
	if (poly->type == MTX_RECTANGLE)	/* Rectangles */
	{
		/* Rectangle Tab */
		subtable = gtk_table_new(3,4,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
		label = gtk_label_new("Rectangle Attributes.");
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
		gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);
		widget = gtk_label_new("Upper left X");
		gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_label_new("Upper left Y");
		gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_label_new("Width");
		gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_label_new("Height");
		gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,GTK_EXPAND,0,0,0);

		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_X));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxRectangle *)poly->data)->x, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,0,0,0,0);
		x_spin = widget;	
		x_origin = widget;	

		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_Y));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxRectangle *)poly->data)->y, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,0,0,0,0);
		y_spin = widget;	
		y_origin = widget;	

		widget = gtk_button_new();
		OBJ_SET(widget,"x_spin",x_spin);
		OBJ_SET(widget,"y_spin",y_spin);
		hbox = gtk_hbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(widget),hbox);
		img = gtk_image_new_from_stock("gtk-edit",GTK_ICON_SIZE_MENU);
		gtk_box_pack_start(GTK_BOX(hbox),img,FALSE,FALSE,0);
		label = gtk_label_new("Edit");
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(grab_coords_event),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,0,2,2,3,0,0,0,0);


		widget = gtk_spin_button_new_with_range(0.0,2.0,0.001);
		x_spin = widget;	
		OBJ_SET(widget,"x_o_spin",x_origin);
		OBJ_SET(widget,"y_o_spin",y_origin);
		OBJ_SET(widget,"relative", GINT_TO_POINTER(TRUE));
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_WIDTH));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxRectangle *)poly->data)->width, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,0,0,0,0);

		widget = gtk_spin_button_new_with_range(0.0,2.0,0.001);
		y_spin = widget;	
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_HEIGHT));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxRectangle *)poly->data)->height, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,0,0,0,0);

		widget = gtk_button_new();
		OBJ_SET(widget,"x_spin",x_spin);
		OBJ_SET(widget,"y_spin",y_spin);
		hbox = gtk_hbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(widget),hbox);
		img = gtk_image_new_from_stock("gtk-edit",GTK_ICON_SIZE_MENU);
		gtk_box_pack_start(GTK_BOX(hbox),img,FALSE,FALSE,0);
		label = gtk_label_new("Edit");
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(grab_coords_event),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,2,4,2,3,0,0,0,0);
	}
	if (poly->type == MTX_ARC)
	{
		/* Arc Tab */
		subtable = gtk_table_new(2,2,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(subtable),10);
		gtk_table_set_row_spacings(GTK_TABLE(subtable),3);
		label = gtk_label_new("Arc Attributes.");
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
		gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

		minitable = gtk_table_new(2,3,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
		gtk_table_attach(GTK_TABLE(subtable),minitable,0,1,0,1,0,0,0,0);
		widget = gtk_label_new("Origin");
		gtk_table_attach(GTK_TABLE(minitable),widget,1,3,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_X));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxArc *)poly->data)->x, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,1,2,1,2,0,0,0,0);
		x_spin = widget;	
		x_origin = widget;	

		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_Y));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxArc *)poly->data)->y, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,2,3,1,2,0,0,0,0);
		y_spin = widget;	
		y_origin = widget;	

		widget = gtk_button_new_with_label("Edit");
		OBJ_SET(widget,"x_spin",x_spin);
		OBJ_SET(widget,"y_spin",y_spin);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(grab_coords_event),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,2,0,GTK_EXPAND|GTK_FILL,0,0);

		minitable = gtk_table_new(2,3,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
		gtk_table_attach(GTK_TABLE(subtable),minitable,1,2,0,1,0,0,0,0);
		widget = gtk_label_new("Width    and    Height");
		gtk_table_attach(GTK_TABLE(minitable),widget,1,3,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"x_o_spin",x_origin);
		OBJ_SET(widget,"y_o_spin",y_origin);
		OBJ_SET(widget,"relative", GINT_TO_POINTER(TRUE));
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_WIDTH));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxArc *)poly->data)->width, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,1,2,1,2,0,0,0,0);
		x_spin = widget;	

		widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
		OBJ_SET(widget,"relative", GINT_TO_POINTER(TRUE));
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_HEIGHT));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", ((MtxArc *)poly->data)->height, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,2,3,1,2,0,0,0,0);
		y_spin = widget;	

		widget = gtk_button_new_with_label("Edit");
		OBJ_SET(widget,"x_spin",x_spin);
		OBJ_SET(widget,"y_spin",y_spin);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(grab_coords_event),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,2,0,GTK_EXPAND|GTK_FILL,0,0);

		minitable = gtk_table_new(1,5,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
		gtk_table_attach(GTK_TABLE(subtable),minitable,0,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
		widget = gtk_label_new("Start Angle");
		g_object_set(G_OBJECT(widget),"xalign", 1.0, NULL);

		gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
		widget = gtk_label_new(NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,2,3,0,1,GTK_EXPAND,0,0,0);
		widget = gtk_label_new("Sweep Angle");
		g_object_set(G_OBJECT(widget),"xalign", 1.0, NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,3,4,0,1,GTK_EXPAND|GTK_FILL,0,0,0);

		widget = gtk_spin_button_new_with_range(-360.0,360.0,0.1);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_START_ANGLE));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, "value", ((MtxArc *)poly->data)->start_angle, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_EXPAND,0,0,0);

		widget = gtk_spin_button_new_with_range(-360.0,360.0,0.1);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_SWEEP_ANGLE));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		g_object_set(G_OBJECT(widget),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, "value", ((MtxArc *)poly->data)->sweep_angle, NULL);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(minitable),widget,4,5,0,1,GTK_EXPAND,0,0,0);
	}
	if (poly->type == MTX_GENPOLY)
	{
		/* Generic Polygon Table */
		subtable = gtk_table_new(2,2,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
		label = gtk_label_new("Generic Polygon Attrs.");
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
		gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);
		widget = gtk_label_new("Number of\nPoints");
		gtk_label_set_justify(GTK_LABEL(widget),GTK_JUSTIFY_CENTER);
		gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,0,GTK_FILL,0,0);


		minitable = gtk_table_new(1,3,FALSE);
		gtk_table_set_col_spacings(GTK_TABLE(minitable),5);
		gtk_table_attach(GTK_TABLE(subtable),minitable,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
		widget = gtk_label_new("Index");
		gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,0,0,0);
		widget = gtk_label_new("X");
		gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL,0,0,0);
		widget = gtk_label_new("Y");
		gtk_table_attach(GTK_TABLE(minitable),widget,2,3,0,1,GTK_FILL,0,0,0);

		minitable = gtk_table_new(1,4,FALSE);
		points_table = minitable;
		gtk_table_set_row_spacings(GTK_TABLE(minitable),0);
		widget = gtk_scrolled_window_new(NULL,NULL);
		gtk_widget_set_usize(widget,-1,120);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),minitable);
		gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,GTK_FILL,GTK_FILL,0,0);
		hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		/* Number of points spinner */
		widget = gtk_spin_button_new_with_range(0.0,100.0,1);
		OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_NUM_POINTS));
		OBJ_SET(widget,"index",GINT_TO_POINTER(index));
		OBJ_SET(widget,"live",GINT_TO_POINTER(TRUE));
		OBJ_SET(widget,"points_table",points_table);
		OBJ_SET(widget,"points_hash",hash);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(adj_generic_num_points),NULL);
		g_object_set(G_OBJECT(widget),"climb-rate", 1.0, "digits", 0, "numeric", TRUE, NULL);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),((MtxGenPoly *)poly->data)->num_points);
		g_signal_connect(G_OBJECT(widget),"value_changed",G_CALLBACK(alter_polygon_data),NULL);
		gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,0,0,0,0);

		/* Points table */
		for (j=0;j<((MtxGenPoly *)poly->data)->num_points;j++)
		{
			tmpbuf = g_strdup_printf("generic_x_%i_spin",j);
			dummy = g_hash_table_lookup(hash,tmpbuf);
			if (GTK_IS_SPIN_BUTTON(dummy))
			{
				OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				OBJ_SET(dummy,"num_points_spin",widget);
				OBJ_SET(dummy,"handler",GINT_TO_POINTER(POLY_POINTS));
				g_signal_handlers_block_by_func (dummy,(gpointer)alter_polygon_data, NULL);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxGenPoly *)poly->data)->points[j].x);
				g_signal_handlers_unblock_by_func (dummy,(gpointer)alter_polygon_data, NULL);
			}
			else
				printf("Spinbutton %s MISSING\n",tmpbuf);
			g_free(tmpbuf);
			tmpbuf = g_strdup_printf("generic_y_%i_spin",j);
			dummy = g_hash_table_lookup(hash,tmpbuf);
			if (GTK_IS_SPIN_BUTTON(dummy))
			{
				OBJ_SET(dummy,"index",GINT_TO_POINTER(index));
				OBJ_SET(dummy,"handler",GINT_TO_POINTER(POLY_POINTS));
				OBJ_SET(dummy,"num_points_spin",widget);
				g_signal_handlers_block_by_func (dummy,(gpointer)alter_polygon_data, NULL);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),((MtxGenPoly *)poly->data)->points[j].y);
				g_signal_handlers_unblock_by_func (dummy,(gpointer)alter_polygon_data, NULL);
			}
			else
				printf("Spinbutton %s MISSING\n",tmpbuf);
			g_free(tmpbuf);

		}
	}
	/* Layer Tab: Layer */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Layer");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Layer:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_FILL,0,0,0);
	widget = gtk_spin_button_new_with_range(0.0,10.0,1.0);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(POLY_LAYER));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 1.0, "digits", 0, "numeric", TRUE, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(gfloat)poly->layer);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_polygon_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_FILL,0,0,0);

	widget = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),widget,0,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
	return table;
}
