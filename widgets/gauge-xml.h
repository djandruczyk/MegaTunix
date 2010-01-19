/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * 
 * MegaTunix gauge widget XML I/O header
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 * This is a the PRIVATE implementation header file for INTERNAL functions
 * of the widget.  Public functions as well the the gauge structure are 
 * defined in the gauge.h header file
 *
 */

#ifndef __GAUGE_XML_H__
#define  __GAUGE_XML_H__

#include <gtk/gtk.h>
#include <gauge.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


typedef struct _MtxXMLFuncs             MtxXMLFuncs;

/*! \struct _MtxXMLFuncs
 * \brief This small container struct is used to store a set of import and
 * export functions use by the XML code to export or import gauge settings
 * The import function takes two args,  one is the text string from the XML
 * to be parsed, the other is the pointer to the destination pointer that
 * the import function should put the parsed data. The export function takes
 * a pointer to the source dispatch helper struct and returns an
 * xmlChar * valid to stick directly into the XML file.
 */
struct _MtxXMLFuncs
{
        void (*import_func) (MtxGaugeFace *, xmlNode *, gpointer, gboolean);
        void (*export_func) (MtxDispatchHelper *);
        gchar * varname;
        gpointer dest_var;
	gboolean api_compat;
};



/* Prototypes */
void testload(GtkWidget *);
void output_xml(GtkWidget * );
void generic_color_export(xmlNode * ,GdkColor *);
void generic_color_import(xmlNode * ,GdkColor *);
void mtx_gauge_alert_range_export(MtxDispatchHelper *);
void mtx_gauge_alert_range_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_warning_range_export(MtxDispatchHelper *);
void mtx_gauge_warning_range_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_color_export(MtxDispatchHelper *);
void mtx_gauge_color_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_text_block_export(MtxDispatchHelper *);
void mtx_gauge_text_block_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_tick_group_export(MtxDispatchHelper *);
void mtx_gauge_tick_group_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_polygon_export(MtxDispatchHelper *);
void mtx_gauge_polygon_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_gfloat_export(MtxDispatchHelper *);
void mtx_gauge_gfloat_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_gint_export(MtxDispatchHelper *);
void mtx_gauge_gint_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_gchar_export(MtxDispatchHelper *);
void mtx_gauge_gchar_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean);
void mtx_gauge_poly_generic_export(xmlNodePtr , MtxPolygon * );
void mtx_gauge_poly_generic_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean );
void mtx_gauge_poly_rectangle_export(xmlNodePtr , MtxPolygon * );
void mtx_gauge_poly_rectangle_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean );
void mtx_gauge_poly_arc_export(xmlNodePtr , MtxPolygon * );
void mtx_gauge_poly_arc_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean );
void mtx_gauge_poly_circle_export(xmlNodePtr , MtxPolygon * );
void mtx_gauge_poly_circle_import(MtxGaugeFace *, xmlNode *, gpointer, gboolean );

/** Structure used to streamline the xml import/export */
static const struct
{
	void (*import_func) (MtxGaugeFace *, xmlNode *, gpointer, gboolean);
	void (*export_func) (MtxDispatchHelper *);
	char * varname;
	gboolean api_compat;
} xml_functions[] = {
	/* Compat functions for API break, note import ONLY */
	{ mtx_gauge_color_import, NULL,"bg_color",TRUE},
	{ mtx_gauge_color_import, NULL,"needle_color",TRUE},
	{ mtx_gauge_color_import, NULL,"value_font_color",TRUE},
	{ mtx_gauge_color_import, NULL,"gradient_begin_color",TRUE},
	{ mtx_gauge_color_import, NULL,"gradient_end_color",TRUE},
	/* End compat functions */
	{ mtx_gauge_color_import, mtx_gauge_color_export,"bg_color_day",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"bg_color_nite",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"needle_color_day",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"needle_color_nite",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"value_font_color_day",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"value_font_color_nite",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"gradient_begin_color_day",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"gradient_begin_color_nite",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"gradient_end_color_day",FALSE},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"gradient_end_color_nite",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_length",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_tip_width",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_tail_width",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_width",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_tail",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"show_tattletale",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"tattletale_alpha",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"precision",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"width",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"height",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"main_start_angle",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"main_sweep_angle",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"rotation",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"lbound",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"ubound",FALSE},
	{ mtx_gauge_gchar_import, mtx_gauge_gchar_export,"value_font",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"value_font_scale",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"value_str_xpos",FALSE},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"value_str_ypos",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"antialias",FALSE},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"show_value",FALSE},
	{ mtx_gauge_alert_range_import, mtx_gauge_alert_range_export,"alert_range",FALSE},
	{ mtx_gauge_warning_range_import, mtx_gauge_warning_range_export,"color_range",FALSE},
	{ mtx_gauge_text_block_import, mtx_gauge_text_block_export,"text_block",FALSE},
	{ mtx_gauge_tick_group_import, mtx_gauge_tick_group_export,"tick_group",FALSE},
	{ mtx_gauge_polygon_import, mtx_gauge_polygon_export,"polygon",FALSE},
};


#endif
