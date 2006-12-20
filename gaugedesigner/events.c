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



EXPORT gboolean create_new_gauge(GtkWidget * widget, gpointer data)
{
	GtkWidget *tmp = NULL;
	GladeXML *xml = glade_get_widget_tree(widget);
	GtkWidget *parent = glade_xml_get_widget(xml,"gauge_frame");
	gauge = mtx_gauge_face_new();
	gtk_container_add(GTK_CONTAINER(parent),gauge);
	gtk_widget_show_all(parent);
	gtk_widget_set_sensitive(widget,FALSE);

	tmp = glade_xml_get_widget(xml,"animate_frame");
	gtk_widget_set_sensitive(tmp,TRUE);

	update_attributes(xml);
	return (TRUE);
}


EXPORT gboolean create_color_span(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *spinner = NULL;
	GtkWidget *cbutton = NULL;
	MtxColorRange *range = NULL;
	gfloat lbound = 0.0;
	gfloat ubound = 0.0;
	GladeXML *xml = NULL;
	gchar * filename = NULL;

	filename = get_file(g_build_filename(GAUGE_DATA_DIR,"gaugedesigner.glade",NULL),NULL);
	if (filename)
	{
		xml = glade_xml_new(filename, "range_dialog", NULL);
		g_free(filename);
	}
	else
	{
		printf("Can't locate primary glade file!!!!\n");
		exit(-1);
	}

	glade_xml_signal_autoconnect(xml);
	dialog = glade_xml_get_widget(xml,"range_dialog");
	cbutton = glade_xml_get_widget(xml,"range_colorbutton");
	gtk_color_button_set_color(GTK_COLOR_BUTTON(cbutton),&white);
	if (!GTK_IS_WIDGET(dialog))
	{
		return FALSE;
	}

	/* Set the controls to sane ranges corresponding to the gauge */
	mtx_gauge_face_get_bounds(MTX_GAUGE_FACE(gauge),&lbound,&ubound);
	spinner = glade_xml_get_widget(xml,"range_lowpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);
	spinner = glade_xml_get_widget(xml,"range_highpoint_spin");
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner),lbound,ubound);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
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
			update_onscreen_ranges();

			break;
		default:
			break;
	}
	gtk_widget_destroy(dialog);

	return (FALSE);
}

EXPORT gboolean create_text_block(GtkWidget * widget, gpointer data)
{
	GtkWidget *dialog = NULL;
	MtxTextBlock *tblock = NULL;
	GladeXML *xml = NULL;
	gchar * filename = NULL;

	filename = get_file(g_build_filename(GAUGE_DATA_DIR,"gaugedesigner.glade",NULL),NULL);
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

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
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
	gtk_widget_destroy(dialog);

	return (FALSE);
}


EXPORT gboolean spin_button_handler(GtkWidget *widget, gpointer data)
{
	gint tmpi;
	gfloat tmpf;
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	tmpf = (gfloat)gtk_spin_button_get_value((GtkSpinButton *)widget);
	tmpi = (gint)(tmpf+0.00001);
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if (hold_handlers)
		return TRUE;

	switch (handler)
	{
		case MAJ_TICKS:
			mtx_gauge_face_set_major_ticks(g,tmpi);
			break;
		case MIN_TICKS:
			mtx_gauge_face_set_minor_ticks(g,tmpi);
			break;
		case MAJ_TICK_LEN:
			mtx_gauge_face_set_major_tick_len(g,tmpf);
			break;
		case MIN_TICK_LEN:
			mtx_gauge_face_set_minor_tick_len(g,tmpf);
			break;
		case MAJ_TICK_WIDTH:
			mtx_gauge_face_set_major_tick_width(g,tmpf);
			break;
		case MIN_TICK_WIDTH:
			mtx_gauge_face_set_minor_tick_width(g,tmpf);
			break;
		case VALUE_XPOS:
			mtx_gauge_face_set_str_xpos(g, VALUE, tmpf);
			break;
		case VALUE_YPOS:
			mtx_gauge_face_set_str_ypos(g, VALUE, tmpf);
			break;
		case START_ANGLE:
			mtx_gauge_face_set_lspan_rad(g,tmpf);
			break;
		case STOP_ANGLE:
			mtx_gauge_face_set_uspan_rad(g,tmpf);
			break;
		case LBOUND:
			mtx_gauge_face_set_lbound(g,tmpf);
			break;
		case UBOUND:
			mtx_gauge_face_set_ubound(g,tmpf);
			break;
		case PRECISION:
			mtx_gauge_face_set_precision(g,tmpf);
			break;
		case TICK_INSET:
			mtx_gauge_face_set_tick_inset(g,tmpf);
			break;
		case MAJOR_TICK_TEXT_INSET:
			mtx_gauge_face_set_major_tick_text_inset(g,tmpf);
			break;
		case NEEDLE_WIDTH:
			mtx_gauge_face_set_needle_width(g,tmpf);
			break;
		case NEEDLE_TAIL:
			mtx_gauge_face_set_needle_tail(g,tmpf);
			break;
		case VALUE_SCALE:
			mtx_gauge_face_set_font_scale(g, VALUE, tmpf);
			break;
		case MAJ_TICK_SCALE:
			mtx_gauge_face_set_font_scale(g, MAJ_TICK, tmpf);
			break;

	}
	return (TRUE);
}


void update_attributes(GladeXML * xml)
{
	update_tickmark_controls();
	update_text_controls();
	update_general_controls();
	update_onscreen_ranges();
	update_onscreen_tblocks();

}

EXPORT gboolean entry_change_color(GtkWidget * widget, gpointer data)
{
        gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&red);
	return TRUE;

}
EXPORT gboolean entry_changed_handler(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	gchar * tmpbuf = NULL;

	if (hold_handlers)
		return TRUE;

        gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);

	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	if (tmpbuf == NULL)
		tmpbuf = g_strdup("");

	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);


	switch ((func)handler)
	{
		case MAJ_TICK_STR:
			mtx_gauge_face_set_text(g, MAJ_TICK, tmpbuf);
			break;
		default:
			break;
	}
	g_free(tmpbuf);
	return (TRUE);

}

EXPORT gboolean change_font(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	gchar * tmpbuf = NULL;
	tmpbuf = (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget));
	/* Strip out the font size as the gauge lib uses a different scaling
	 * method that scales with the size of the gauge
	 */
	tmpbuf = g_strchomp(g_strdelimit(tmpbuf,"0123456789",' '));
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);

	if (hold_handlers)
		return TRUE;

	switch ((func)handler)
	{
		case VALUE_FONT:
			mtx_gauge_face_set_font(g, VALUE, tmpbuf);
			break;
		case MAJ_TICK_FONT:
			mtx_gauge_face_set_font(g, MAJ_TICK, tmpbuf);
			break;
		default:
			break;
	}
	return TRUE;
}


EXPORT gboolean checkbutton_handler(GtkWidget *widget, gpointer data)
{
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");
	MtxGaugeFace *g = MTX_GAUGE_FACE(gauge);
	if (hold_handlers)
		return TRUE;

	switch (handler)
	{
		case ANTIALIAS:
			mtx_gauge_face_set_antialias(g,state);
			break;
		case SHOW_VALUE:
			mtx_gauge_face_set_show_value(g,state);
			break;

	}

	return TRUE;
}

EXPORT gboolean color_button_color_set(GtkWidget *widget, gpointer data)
{
	gint handler = (gint)g_object_get_data(G_OBJECT(widget),"handler");

	if (hold_handlers)
		return TRUE;
	GdkColor color;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget),&color);
	mtx_gauge_face_set_color(MTX_GAUGE_FACE(gauge),handler,color);

	return TRUE;
}

void update_onscreen_ranges()
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
	extern GladeXML *ranges_xml;

	if ((!ranges_xml) || (!gauge))
		return;
	array = mtx_gauge_face_get_color_ranges(MTX_GAUGE_FACE(gauge));
	container = glade_xml_get_widget(ranges_xml,"color_range_viewport");
	if (!GTK_IS_WIDGET(container))
	{
		printf("color range viewport invalid!!\n");
		return;
	}
	y=1;
	if (table != NULL)
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
		g_object_set_data(G_OBJECT(button),"range_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_crange),NULL);
		gtk_table_attach(GTK_TABLE(table),button,0,1,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		mtx_gauge_face_get_bounds(MTX_GAUGE_FACE(gauge),&low,&high);
		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lowpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_crange_data),GINT_TO_POINTER(CR_LOWPOINT));

		gtk_table_attach(GTK_TABLE(table),dummy,1,2,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(low,high,(high-low)/100);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->highpoint);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_crange_data),GINT_TO_POINTER(CR_HIGHPOINT));
		gtk_table_attach(GTK_TABLE(table),dummy,2,3,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->inset);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_crange_data),GINT_TO_POINTER(CR_INSET));
		gtk_table_attach(GTK_TABLE(table),dummy,3,4,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_spin_button_new_with_range(0,1,0.001);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dummy),range->lwidth);
		g_signal_connect(G_OBJECT(dummy),"value_changed", G_CALLBACK(alter_crange_data),GINT_TO_POINTER(CR_LWIDTH));
		gtk_table_attach(GTK_TABLE(table),dummy,4,5,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&range->color);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_crange_data),GINT_TO_POINTER(CR_COLOR));

		gtk_table_attach(GTK_TABLE(table),dummy,5,6,y,y+1,GTK_SHRINK,GTK_SHRINK,0,0);
		y++;
	}
	gtk_widget_show_all(container);
}


gboolean remove_crange(GtkWidget * widget, gpointer data)
{
	gint index = -1;

	index = (gint)g_object_get_data(G_OBJECT(widget),"range_index");
	mtx_gauge_face_remove_color_range(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_ranges();

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
	gtk_widget_set_sensitive(widget,FALSE);
	gtk_timeout_add(20,(GtkFunction)sweep_gauge, (gpointer)gauge);
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

	GtkWidget * gauge = data;
	mtx_gauge_face_get_bounds(MTX_GAUGE_FACE (gauge),&lower,&upper);
	interval = (upper-lower)/100.0;
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

	if (table)
		gtk_widget_destroy(table);

	table = gtk_table_new(2,1,FALSE);
	gtk_table_attach_defaults(GTK_TABLE(toptable),table,0,1,1,2);
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
		g_object_set_data(G_OBJECT(button),"tblock_index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(button),"toggled", G_CALLBACK(remove_tblock),NULL);
		gtk_table_attach(GTK_TABLE(subtable),button,0,1,0,3,GTK_SHRINK,GTK_FILL,0,0);
		label = gtk_label_new("Text");
		gtk_table_attach(GTK_TABLE(subtable),label,1,2,0,1,GTK_EXPAND,GTK_SHRINK,3,0);

		dummy = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(dummy),tblock->text);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_TEXT));
		gtk_table_attach(GTK_TABLE(subtable),dummy,2,3,0,1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);

		dummy = gtk_color_button_new_with_color(&tblock->color);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"color_set", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_COLOR));
		gtk_table_attach(GTK_TABLE(subtable),dummy,3,4,0,1,GTK_FILL,GTK_SHRINK,0,0);
		tmpbuf = g_strdup_printf("%s 12",tblock->font);
		dummy = gtk_font_button_new_with_font(tmpbuf);
		g_free(tmpbuf);
		g_object_set_data(G_OBJECT(dummy),"index",GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(dummy),"font_set", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_FONT));
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(dummy),FALSE);

		gtk_table_attach(GTK_TABLE(subtable),dummy,1,3,1,2,GTK_FILL,GTK_SHRINK,0,0);
		spin = gtk_spin_button_new_with_range(0,1,0.001);
		g_object_set_data(G_OBJECT(spin),"index",GINT_TO_POINTER(i));
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
		g_object_set_data(G_OBJECT(spin),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin),tblock->x_pos);
		g_signal_connect(G_OBJECT(spin),"value_changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_X_POS));
		gtk_table_attach(GTK_TABLE(subtable2),spin,1,2,0,1,GTK_FILL,GTK_FILL,0,0);

		spin = gtk_spin_button_new_with_range(-1,1,0.001);
		g_object_set_data(G_OBJECT(spin),"index",GINT_TO_POINTER(i));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin),tblock->y_pos);
		g_signal_connect(G_OBJECT(spin),"value_changed", G_CALLBACK(alter_tblock_data),GINT_TO_POINTER(TB_Y_POS));
		gtk_table_attach(GTK_TABLE(subtable2),spin,3,4,0,1,GTK_FILL,GTK_FILL,0,0);
		dummy = gtk_hseparator_new();
		gtk_table_attach(GTK_TABLE(table),dummy,0,1,y+1,y+2,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,2);

		y+=2;
	}
	gtk_widget_show_all(toptable);
}


gboolean alter_tblock_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)g_object_get_data(G_OBJECT(widget),"index");
	gfloat value = 0.0;
	gchar * tmpbuf = NULL;
	GdkColor color;
	TbField field = (TbField)data;

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


gboolean alter_crange_data(GtkWidget *widget, gpointer data)
{
	gint index = (gint)g_object_get_data(G_OBJECT(widget),"index");
	gfloat value = 0.0;
	GdkColor color;
	CrField field = (CrField)data;

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


gboolean remove_tblock(GtkWidget * widget, gpointer data)
{
	gint index = -1;

	index = (gint)g_object_get_data(G_OBJECT(widget),"tblock_index");
	mtx_gauge_face_remove_text_block(MTX_GAUGE_FACE(gauge),index);
	update_onscreen_tblocks();

	return TRUE;
}

