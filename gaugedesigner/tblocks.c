#include <config.h>
#include <defines.h>
#include <tblocks.h>
#include <events.h>
#include <gauge.h>
#include <getfiles.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

extern GtkWidget * gauge;
extern GdkColor black;
extern GdkColor white;
extern gboolean changed;


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
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tblock_day_colorbutton")),&white);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tblock_nite_colorbutton")),&black);
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
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tblock_day_colorbutton")),&tblock->color[MTX_DAY]);
			gtk_color_button_get_color(GTK_COLOR_BUTTON(glade_xml_get_widget(xml,"tblock_nite_colorbutton")),&tblock->color[MTX_NITE]);
			tblock->font = (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON(glade_xml_get_widget(xml,"tblock_fontbutton")));
			tblock->font = g_strchomp(g_strdelimit(tblock->font,"0123456789",' '));
			changed = TRUE;
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


void update_onscreen_tblocks()
{
	GtkWidget *toptable = NULL;
	static GtkWidget *table = NULL;
	GtkWidget *subtable = NULL;
	GtkWidget *dummy = NULL;
	guint i = 0;
	gint y = 1;
	MtxTextBlock *tblock = NULL;
	GtkAdjustment *adj = NULL;
	GArray * array = NULL;
	extern GladeXML *topxml;

	if ((!topxml) || (!gauge))
		return;

	array = mtx_gauge_face_get_text_blocks(MTX_GAUGE_FACE(gauge));
	toptable = glade_xml_get_widget(topxml,"text_blocks_layout_table");
	if (!GTK_IS_WIDGET(toptable))
	{
		printf("toptable is NOT a valid widget!!\n");
		return;
	}

	if (GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);

	table = gtk_table_new(2,1,FALSE);
	gtk_table_attach(GTK_TABLE(toptable),table,0,1,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
	OBJ_SET((toptable),"layout_table",table);
	/* Repopulate the table with the current tblocks... */
	y=1;
	for (i=0;i<array->len; i++)
	{
		tblock = g_array_index(array,MtxTextBlock *, i);
		subtable = build_tblock(tblock,i);
		gtk_table_attach(GTK_TABLE(table),subtable,0,1,y,y+1,GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
		gtk_widget_show_all(subtable);
		y+=1;
	}
	/* Scroll to end */
	dummy = glade_xml_get_widget(topxml,"tblock_swin");
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dummy));
	adj->value = adj->upper;

	gtk_widget_show_all(toptable);
}


void reset_onscreen_tblocks()
{
	GtkWidget *toptable = NULL;
	GtkWidget *widget = NULL;
	extern GladeXML *topxml;

	if ((!topxml))
		return;
	toptable = glade_xml_get_widget(topxml,"text_blocks_layout_table");
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
	gint index = (GINT)OBJ_GET((widget),"index");
	gfloat value = 0.0;
	gchar * tmpbuf = NULL;
	GdkColor color;
	TbField field = (TbField)OBJ_GET(widget,"handler");
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	switch (field)
	{
		case TB_FONT_SCALE:
		case TB_LAYER:
		case TB_X_POS:
		case TB_Y_POS:
			value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			mtx_gauge_face_alter_text_block(MTX_GAUGE_FACE(gauge),index,field,(void *)&value);
			break;
		case TB_COLOR_DAY:
		case TB_COLOR_NITE:
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


gboolean remove_tblock(GtkWidget * widget, gpointer data)
{
	gint index = -1;
	if (!GTK_IS_WIDGET(gauge))
		return FALSE;

	index = (GINT)OBJ_GET((widget),"tblock_index");
	mtx_gauge_face_remove_text_block(MTX_GAUGE_FACE(gauge),index);
	changed = TRUE;
	update_onscreen_tblocks();

	return TRUE;
}


GtkWidget * build_tblock(MtxTextBlock *tblock, gint index)
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
	GtkWidget *minitable = NULL;
	gchar * tmpbuf = NULL;

	table = gtk_table_new(2,2,FALSE);

	/* Close button */
	widget = gtk_button_new();
	img = gtk_image_new_from_stock("gtk-close",GTK_ICON_SIZE_MENU);
	gtk_container_add(GTK_CONTAINER(widget),img);
	OBJ_SET((widget),"tblock_index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"clicked", G_CALLBACK(remove_tblock),NULL);
	gtk_table_attach(GTK_TABLE(table),widget,0,1,0,4,0,0,0,0);

	notebook = gtk_notebook_new();
	gtk_table_attach(GTK_TABLE(table),notebook,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
	/* text, color buttons */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Text & Color");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);
	widget = gtk_label_new("Text:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_FILL,0,0,0);

	widget = gtk_entry_new();
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_TEXT));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	gtk_entry_set_width_chars(GTK_ENTRY(widget),12);
	gtk_entry_set_text(GTK_ENTRY(widget),tblock->text);
	g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
	widget = gtk_color_button_new_with_color(&tblock->color[MTX_DAY]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_COLOR_DAY));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,GTK_FILL,0,0,0);

	widget = gtk_color_button_new_with_color(&tblock->color[MTX_NITE]);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_COLOR_NITE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_signal_connect(G_OBJECT(widget),"color_set",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,GTK_FILL,0,0,0);

	/* font, font scale spinner */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Font");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Font:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_FILL,0,0,0);

	widget = gtk_font_button_new();
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_FONT));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	tmpbuf = g_strdup_printf("%s 12",tblock->font);
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget),tmpbuf);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(widget),FALSE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(widget),FALSE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(widget),FALSE);
	g_free(tmpbuf);
	g_signal_connect(G_OBJECT(widget),"font_set",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);

	widget = gtk_label_new("Font\nScale");
	gtk_table_attach(GTK_TABLE(subtable),widget,2,3,0,1,0,0,0,0);

	widget = gtk_spin_button_new_with_range(0.001,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_FONT_SCALE));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tblock->font_scale, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,3,4,0,1,0,0,0,0);

	/* Location Tab: Edit button, X/Y position spinners */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Location");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Position:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_FILL,0,0,0);

	/* X position minilayout table */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_attach(GTK_TABLE(subtable),minitable,2,3,0,1,GTK_EXPAND,0,0,0);
	widget = gtk_label_new("X:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,0,0,0);
	widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_X_POS));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tblock->x_pos, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL,0,0,0);
	x_spin = widget;

	/* Y position minilayout table */
	minitable = gtk_table_new(1,2,FALSE);
	gtk_table_attach(GTK_TABLE(subtable),minitable,3,4,0,1,GTK_FILL|GTK_EXPAND,0,0,0);
	widget = gtk_label_new("Y:");
	gtk_table_attach(GTK_TABLE(minitable),widget,0,1,0,1,GTK_FILL,0,0,0);
	widget = gtk_spin_button_new_with_range(-1.0,1.0,0.001);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_Y_POS));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 0.001, "digits", 3, "numeric", TRUE, "value", tblock->y_pos, NULL);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(minitable),widget,1,2,0,1,GTK_FILL,0,0,0);
	y_spin = widget;

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
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
  
	/* Layer Tab: Layer */
	subtable = gtk_table_new(1,4,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(subtable),5);
	label = gtk_label_new("Layer");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),subtable,label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook),subtable,TRUE,TRUE,GTK_PACK_START);

	widget = gtk_label_new("Layer:");
	gtk_table_attach(GTK_TABLE(subtable),widget,0,1,0,1,GTK_FILL,0,0,0);
	widget = gtk_spin_button_new_with_range(0.0,10.0,1.0);
	OBJ_SET(widget,"handler",GINT_TO_POINTER(TB_LAYER));
	OBJ_SET(widget,"index",GINT_TO_POINTER(index));
	g_object_set(G_OBJECT(widget),"climb-rate", 1, "digits", 0, "numeric", TRUE, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(gfloat)tblock->layer);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(alter_tblock_data),NULL);
	gtk_table_attach(GTK_TABLE(subtable),widget,1,2,0,1,GTK_FILL,0,0,0);

	widget = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),widget,0,2,1,2,GTK_FILL,0,0,0);
	return table;
}
