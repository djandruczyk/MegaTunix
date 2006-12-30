#include <defines.h>
#include <getfiles.h>
#include <gauge.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <xml.h>


#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)


EXPORT gboolean import_dash_xml(GtkWidget *widget, gpointer data)
{
	printf("Import Dash XML\n");
	return TRUE;
}


EXPORT gboolean export_dash_xml_default(GtkWidget *widget, gpointer data)
{
	extern GladeXML *main_xml;
	GtkWidget *dash = NULL;
	GList *children = NULL;
	GtkFixedChild *child = NULL;
	gchar * tmpbuf = NULL;
	gint i = 0;
	xmlDocPtr doc = NULL;       /* document pointer */
	xmlNodePtr root_node = NULL;/* node pointers */
	xmlNodePtr node = NULL;/* node pointers */
	xmlDtdPtr dtd = NULL;       /* DTD pointer */
	gchar * filename = "testdash.xml";
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	gboolean state = FALSE;
	gchar * iname = NULL;


	printf("Export Dash XML default\n");

	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST "dashboard");
	xmlDocSetRootElement(doc,root_node);
	/*
	 * Creates a DTD declaration. Isn't mandatory. 
	 */
	dtd = xmlCreateIntSubset(doc, BAD_CAST "dashboard", NULL, BAD_CAST "mtxdashboard.dtd");

	dash = glade_xml_get_widget(main_xml,"dashboard");

	node = xmlNewChild(root_node,NULL,BAD_CAST "dash_geometry", NULL);
	tmpbuf = g_strdup_printf("%i",dash->allocation.width);
	xmlNewChild(node, NULL, BAD_CAST "width", BAD_CAST tmpbuf);
	g_free(tmpbuf);
	tmpbuf = g_strdup_printf("%i",dash->allocation.height);
	xmlNewChild(node, NULL, BAD_CAST "height", BAD_CAST tmpbuf);
	g_free(tmpbuf);

	children = GTK_FIXED(dash)->children;
	for(i=0;i<g_list_length(GTK_FIXED(dash)->children);i++)
	{
		child = g_list_nth_data(GTK_FIXED(dash)->children,i);
		node = xmlNewChild(root_node,NULL,BAD_CAST "gauge", NULL);

		tmpbuf = g_strdup_printf("%i",child->widget->allocation.width);
		xmlNewChild(node, NULL, BAD_CAST "width", BAD_CAST tmpbuf);
		tmpbuf = g_strdup_printf("%i",child->widget->allocation.height);
		xmlNewChild(node, NULL, BAD_CAST "height", BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%i",child->x);
		xmlNewChild(node, NULL, BAD_CAST "x_offset", BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%i",child->y);
		xmlNewChild(node, NULL, BAD_CAST "y_offset", BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%s",mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(child->widget)));
		xmlNewChild(node, NULL, BAD_CAST "gauge_xml_name", BAD_CAST tmpbuf);
		g_free(tmpbuf);
		state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(g_object_get_data(G_OBJECT(child->widget),"combo")),&iter);
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(g_object_get_data(G_OBJECT(child->widget),"combo")));
		gtk_tree_model_get(model,&iter,2,&iname,-1);
		xmlNewChild(node, NULL, BAD_CAST "datasource", BAD_CAST iname);
		printf("child %i\n",i);
	}
	xmlSaveFormatFileEnc(filename, doc, "utf-8", 1);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();

	return TRUE;
}


EXPORT gboolean export_dash_xml_as(GtkWidget *widget, gpointer data)
{
	printf("Export Dash XML as...\n");
	return TRUE;
}

#endif
