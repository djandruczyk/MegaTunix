#include <defines.h>
#include <events.h>
#include <tgroups.h>
#include <gauge.h>
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <handlers.h>


extern GtkWidget * gauge;
extern GdkColor black;
extern GdkColor white;
extern gboolean changed;

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
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_text_day_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_text_nite_colorbutton")),&black);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_day_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_nite_colorbutton")),&black);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_day_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_nite_colorbutton")),&black);
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"lowpartner",glade_xml_get_widget(xml,"tg_lowpoint_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"highpartner",glade_xml_get_widget(xml,"tg_highpoint_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"high_angle",glade_xml_get_widget(xml,"tg_sweep_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_start_angle_spin")),"spin_handler", GINT_TO_POINTER(ADJ_LOW_UNIT_PARTNER));
	OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"highpartner",glade_xml_get_widget(xml,"tg_highpoint_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"low_angle",glade_xml_get_widget(xml,"tg_start_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_sweep_angle_spin")),"spin_handler", GINT_TO_POINTER(ADJ_HIGH_UNIT_PARTNER));
	OBJ_SET((glade_xml_get_widget(xml,"tg_lowpoint_spin")),"lowpartner",glade_xml_get_widget(xml,"tg_start_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_lowpoint_spin")),"spin_handler", GINT_TO_POINTER(ADJ_START_ANGLE_PARTNER));
	OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"highpartner",glade_xml_get_widget(xml,"tg_sweep_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"start_angle",glade_xml_get_widget(xml,"tg_start_angle_spin"));
	OBJ_SET((glade_xml_get_widget(xml,"tg_highpoint_spin")),"spin_handler", GINT_TO_POINTER(ADJ_SWEEP_ANGLE_PARTNER));
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
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_text_day_colorbutton")),&tgroup->text_color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_text_nite_colorbutton")),&tgroup->text_color[MTX_NITE]);
			tgroup->font_scale = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_font_scale_spin")));
			tgroup->text_inset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"tg_text_inset_spin")));
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_day_colorbutton")),&tgroup->maj_tick_color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_maj_tick_nite_colorbutton")),&tgroup->maj_tick_color[MTX_NITE]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_day_colorbutton")),&tgroup->min_tick_color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tg_min_tick_nite_colorbutton")),&tgroup->min_tick_color[MTX_NITE]);
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

			changed = TRUE;
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


void update_onscreen_tgroups()
{
	GtkWidget *toptable = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *table = NULL;
	GtkWidget *dummy = NULL;
	guint i = 0;
	MtxTickGroup *tgroup = NULL;
	GArray * array = NULL;
	GtkAdjustment *adj = NULL;
	extern GladeXML *topxml;

	if ((!topxml) || (!gauge))
		return;

	array = mtx_gauge_face_get_tick_groups(MTX_GAUGE_FACE(gauge));

	toptable = glade_xml_get_widget(topxml,"tick_groups_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	/* Get it and blow it away for re-creation */
	table = OBJ_GET((toptable), "layout_table");
	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,1,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_attach_defaults(GTK_TABLE(toptable),table,0,1,1,2);
	OBJ_SET((toptable),"layout_table",table);
	/* Repopulate the table with the current tgroups... */
	for (i=0;i<array->len; i++)
	{
		tgroup = g_array_index(array,MtxTickGroup *, i);
		subtable = build_tgroup(tgroup,i);
		gtk_table_attach(GTK_TABLE(table),subtable,0,1,i,i+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,5,0);

	}
	/* Scroll to end */
	dummy = glade_xml_get_widget(topxml,"tgroup_swin");
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dummy));
	adj->value = adj->upper;
	gtk_widget_show_all(toptable);
}


void reset_onscreen_tgroups()
{
	GtkWidget *toptable = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *topxml;

	if ((!topxml))
		return;
	toptable = glade_xml_get_widget(topxml,"tick_groups_layout_table");
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


gboolean alter_tgroup_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	gchar * tmpbuf = NULL;
	GdkColor color;
	TgField field = (TgField)OBJ_GET(widget,"handler");
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
		case TG_MAJ_TICK_COLOR_DAY:
		case TG_MAJ_TICK_COLOR_NITE:
		case TG_MIN_TICK_COLOR_DAY:
		case TG_MIN_TICK_COLOR_NITE:
		case TG_TEXT_COLOR_DAY:
		case TG_TEXT_COLOR_NITE:
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


gboolean remove_tgroup(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (gint)OBJ_GET((widget),"tgroup_index");
	mtx_gauge_face_remove_tick_group(MTX_GAUGE_FACE(gauge),index);
	changed = TRUE;
	update_onscreen_tgroups();

	return TRUE;
}


GtkWidget * build_tgroup(MtxTickGroup *tgroup, gint index)
{
	/* MUCH faster that the glade way unfortunately */
	GtkWidget *notebook = NULL;
	GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *widget = NULL;
	GtkWidget *x_spin = NULL;
	GtkWidget *y_spin = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *img = NULL;
	GtkWidget *label = NULL;
	GtkWidget *start = NULL;
	GtkWidget *sweep = NULL;
	GtkWidget *low = NULL;
	GtkWidget *high = NULL;
	gchar * tmpbuf = NULL;

	table = gtk_table_new(2,2,FALSE);

	/* Close button */
	widget = gtk_button_new();
	img = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
	gtk_container_add(GTK_CONTAINER(widget),img);
	OBJ_SET((widget),"tgroup_index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"clicked", G_CALLBACK(remove_tgroup),NULL);
	gtk_table_attach(GTK_TABLE(table),widget,0,1,0,4,0,0,0,0);

	notebook = gtk_notebook_new();
	gtk_table_attach(GTK_TABLE(table),notebook,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
	/* Text/Color Tab */
	subtable = gtk_table_new(2,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Text & Color");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);
	widget = gtk_label_new("Text:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,2,GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	widget = gtk_entry_new();
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_TEXT));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	gtk_entry_set_width_chars(GTK_ENTRY(widget),12);
	gtk_entry_set_text(GTK_ENTRY(widget),tgroup->text);
	g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,2,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);
	widget = gtk_label_new("Day");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,0,GTK_FILL,0,0);

	widget = gtk_color_button_new_with_color(&tgroup->text_color[MTX_DAY]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_TEXT_COLOR_DAY));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,GTK_FILL,GTK_EXPAND,0,0);

	widget = gtk_label_new("Nite");
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,0,GTK_FILL,0,0);
	widget = gtk_color_button_new_with_color(&tgroup->text_color[MTX_NITE]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_TEXT_COLOR_NITE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,GTK_FILL,GTK_EXPAND,0,0);

	/* Font Tab */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Font");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Font:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_FILL,GTK_EXPAND,0,0);

	widget = gtk_font_button_new();
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_FONT));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	tmpbuf = g_strdup_printf("%s 12",tgroup->font);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(widget),FALSE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(widget),FALSE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(widget),FALSE);
	g_free(tmpbuf);
	g_signal_connect(G_OBJECT(widget),"font_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_EXPAND,0,0);

	widget = gtk_label_new("Font\nScale");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.001,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_FONT_SCALE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->font_scale, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,0,GTK_EXPAND,0,0);

	widget = gtk_label_new("Text\nInset");
	gtk_table_attach(GTK_TABLE(subtable),widget,4,5,0,1,0,GTK_EXPAND,0,0);
	widget = gtk_spin_button_new_with_range(0.001,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_TEXT_INSET));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->text_inset, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,5,6,0,1,0,GTK_EXPAND,0,0);

	/* Major Ticks Tab */
	subtable = gtk_table_new(2,6,FALSE);
	/*gtk_table_set_col_spacings(GTK_TABLE(subtable),5);*/
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),"<b>Major</b> Ticks");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	/* Labels */
	widget = gtk_label_new("Total Ticks");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Length");
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Width");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Inset");
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Day");
	gtk_table_attach(GTK_TABLE(subtable),widget,4,5,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Nite");
	gtk_table_attach(GTK_TABLE(subtable),widget,5,6,0,1,GTK_EXPAND,GTK_EXPAND,0,0);

	/* Spinners */
	widget = gtk_spin_button_new_with_range(0.0,100.0,1);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_NUM_MAJ_TICKS));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 1, "digits", 0, "numeric", TRUE, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tgroup->num_maj_ticks);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MAJ_TICK_LENGTH));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->maj_tick_length, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MAJ_TICK_WIDTH));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->maj_tick_width, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MAJ_TICK_INSET));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->maj_tick_inset, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_color_button_new_with_color(&tgroup->maj_tick_color[MTX_DAY]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MAJ_TICK_COLOR_DAY));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,4,5,1,2,GTK_FILL,GTK_EXPAND,0,0);

	widget = gtk_color_button_new_with_color(&tgroup->maj_tick_color[MTX_NITE]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MAJ_TICK_COLOR_NITE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,5,6,1,2,GTK_FILL,GTK_EXPAND,0,0);

	/* Minor Ticks Tab */
	subtable = gtk_table_new(2,6,FALSE);
	/*gtk_table_set_col_spacings(GTK_TABLE(subtable),5);*/
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),"<b>Minor</b> Ticks");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	/* Labels */
	widget = gtk_label_new("Total Ticks");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Length");
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Width");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Inset");
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Day");
	gtk_table_attach(GTK_TABLE(subtable),widget,4,5,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Nite");
	gtk_table_attach(GTK_TABLE(subtable),widget,5,6,0,1,GTK_EXPAND,GTK_EXPAND,0,0);

	/* Spinners */
	widget = gtk_spin_button_new_with_range(0.0,100.0,1);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_NUM_MIN_TICKS));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 1, "digits", 0, "numeric", TRUE, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),tgroup->num_min_ticks);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MIN_TICK_LENGTH));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->min_tick_length, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MIN_TICK_WIDTH));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->min_tick_width, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(0.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MIN_TICK_INSET));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tgroup->min_tick_inset, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,0,GTK_EXPAND,0,0);

	widget = gtk_color_button_new_with_color(&tgroup->min_tick_color[MTX_DAY]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MIN_TICK_COLOR_DAY));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,4,5,1,2,GTK_FILL,GTK_EXPAND,0,0);

	widget = gtk_color_button_new_with_color(&tgroup->min_tick_color[MTX_NITE]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_MIN_TICK_COLOR_NITE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tgroup_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,5,6,1,2,GTK_FILL,GTK_EXPAND,0,0);

	/* Tick Span Tab */
	subtable = gtk_table_new(3,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Tick Span");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Angular Span (deg.)");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,2,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Span (Gauge Units)");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,4,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Start Angle");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Sweep Angle");
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("Low Point");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
	widget = gtk_label_new("High Point");
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
	/* Span Spinners */
	widget = gtk_spin_button_new_with_range(-360.0,360.0,0.1);
	start = widget;
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_START_ANGLE));
	OBJ_SET(widget,"spin_handler",GINT_TO_POINTER(ADJ_LOW_UNIT_PARTNER));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,2,3,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(-360.0,360.0,0.1);
	sweep = widget;
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TG_SWEEP_ANGLE));
	OBJ_SET(widget,"spin_handler",GINT_TO_POINTER(ADJ_HIGH_UNIT_PARTNER));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,2,3,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(-99999.0,99999.0,0.1);
	low = widget;
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ADJ_START_ANGLE_PARTNER));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(low),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, NULL);
	g_signal_connect(G_OBJECT(low),"value-changed",G_CALLBACK(tg_spin_button_handler),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,2,3,0,GTK_EXPAND,0,0);

	widget = gtk_spin_button_new_with_range(-99999.0,99999.0,0.1);
	high = widget;
	OBJ_SET(widget,"handler",GINT_TO_POINTER(ADJ_SWEEP_ANGLE_PARTNER));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(high),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, NULL);
	g_signal_connect(G_OBJECT(high),"value-changed",G_CALLBACK(tg_spin_button_handler),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,2,3,0,GTK_EXPAND,0,0);

	/* Start/Sweep <-> Low/High interconnectedness 
	 * This is done like shit and I don't like it..
	 */
	OBJ_SET(start,"lowpartner",low);
	OBJ_SET(start,"highpartner",high);
	OBJ_SET(start,"high_angle",sweep);
	OBJ_SET(sweep,"highpartner",high);
	OBJ_SET(sweep,"low_angle",start);
	OBJ_SET(low,"lowpartner",start);
	OBJ_SET(high,"highpartner",sweep);
	OBJ_SET(high,"start_angle",start);
	/* Connect the signals, then set the values to trigger the linked
	 * spinners to update, THEN enable change handlers, as the gauge
	 * ALREADY has the info, no need to set it again..
	 */
	g_signal_connect(G_OBJECT(start),"value-changed",G_CALLBACK(tg_spin_button_handler),NULL);
	g_signal_connect(G_OBJECT(sweep),"value-changed",G_CALLBACK(tg_spin_button_handler),NULL);
	g_object_set(G_OBJECT(start),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, "value", tgroup->start_angle, NULL);
	g_object_set(G_OBJECT(sweep),"climb-rate", 0.1, "digits", 1, "numeric", TRUE, "value", tgroup->sweep_angle, NULL);
	g_signal_connect(G_OBJECT(start),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);
	g_signal_connect(G_OBJECT(sweep),"value-changed",G_CALLBACK(alter_tgroup_data),NULL);

	widget = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),widget,0,2,1,2,GTK_FILL,0,0,0);
	return table;
}
