/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire (czb)
 *
 * Megasquirt gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef MTX_GAUGE_FACE_H
#define MTX_GAUGE_FACE_H

#include <config.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>



G_BEGIN_DECLS

#define MTX_TYPE_GAUGE_FACE		(mtx_gauge_face_get_type ())
#define MTX_GAUGE_FACE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFace))
#define MTX_GAUGE_FACE_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), MTX_GAUGE_FACE, MtxGaugeFaceClass))
#define MTX_IS_GAUGE_FACE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTX_TYPE_GAUGE_FACE))
#define MTX_IS_GAUGE_FACE_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MTX_TYPE_GAUGE_FACE))
#define MTX_GAUGE_FACE_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFaceClass))
//#define MTX_GAUGE_FACE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFacePrivate))



typedef struct _MtxGaugeFace		MtxGaugeFace;
typedef struct _MtxGaugeFaceClass	MtxGaugeFaceClass;
typedef struct _MtxColorRange		MtxColorRange;
typedef struct _MtxXMLFuncs		MtxXMLFuncs;
typedef struct _MtxDispatchHelper	MtxDispatchHelper;


struct _MtxDispatchHelper
{
	gchar * element_name;
	gpointer src;
	xmlNodePtr root_node;
	MtxGaugeFace * gauge;
};


/*! \struct MtxColorRange
 * \brief
 * MtxColorRange is a container struct that holds all the information needed
 * for a color range span on a gauge. Any gauge can have an arbritrary number
 * of these structs as they are stored in a dynamic array and redraw on
 * gauge background generation
 */
struct _MtxColorRange
{
	gfloat lowpoint;	///< where the range starts from
	gfloat highpoint; 	///< where the range ends at
	GdkColor color;		///< The color to use
	gfloat lwidth;		///< % of radius to determine the line width
	gfloat inset;		///< % of radius to inset the line
};

/*! \struct _MtxXMLFuncs
 * \brief This small container struct is used to store a set of import and 
 * export functions use by the XML code to export or import gauge settings
 * The import function takes two args,  one is the text string from the XML
 * to be parsed, the other is the pointer to the destination variable that
 * the import function should put the parsed data. The export function takes 
 * a pointer to the destination variable and returns an xmlChar * valid to
 * stick directly into the XML file.
 */
struct _MtxXMLFuncs
{
	void (*import_func) (MtxGaugeFace *, xmlNode *, gpointer);
	void (*export_func) (MtxDispatchHelper *);
	gchar * varname;
	gpointer dest_var;
};

enum  ColorIndex
{
	COL_BG = 0,
	COL_NEEDLE,
	COL_MAJ_TICK,
	COL_MIN_TICK,
	COL_UNIT_FONT,
	COL_NAME_FONT,
	COL_VALUE_FONT,
	NUM_COLORS
};


struct _MtxGaugeFace
{//public data
	GtkDrawingArea parent;
	GdkBitmap *bitmap;	/*! for shape_combine stuff */
	GdkPixmap *pixmap;	/*! Update/backing pixmap */
	GdkPixmap *bg_pixmap;	/*! Static rarely changing pixmap */
	GArray * xmlfunc_array; /*! Array list mapping varnames to xml */
	GHashTable * xmlfunc_hash; /*! Hashtable mapping varnames to xml 
				   *  parsing functions */
	gint w;			/*! width */
	gint h;			/*! height */
	gdouble xc;		/*! X Center */
	gdouble yc;		/*! Y Center */
	gdouble radius;		/*! Radius of display */
#ifdef HAVE_CAIRO
	cairo_t *cr;		/*! Cairo context,  not sure if this is good
				   too hold onto or not */
#endif
	PangoLayout *layout;	/*! Pango TextLayout object */
	PangoFontDescription *font_desc;/*! Pango Font description */
	GdkGC * bm_gc;		/*! Graphics Context for bitmap */
	GdkGC * gc;		/*! Graphics Context for drawing */
	GdkColormap *colormap;	/*! Colormap for GC's */
	GdkColor colors[NUM_COLORS]; /*! Array of colors for specific
					     parts of a gauge object */
	gint precision;		/*! number of decimal places for val */
	gfloat start_deg; 	/*! GDK Start point in degrees (CCW) */
	gfloat stop_deg;	/*! GDK Stop point in degrees (CCW) */
	gfloat start_radian;	/*! CAIRO Start angle in radians (CW) */
	gfloat stop_radian;	/*! CAIRO Stop Angle in radians (CW) */
	gfloat value;		/*! Value represneting needle position */
	gfloat lbound;		/*! Lower Bound to clamp at */
	gfloat ubound;		/*! Upper Bound to Clamp at */
	gfloat span;		/*! Span from lbound to ubound */
	gchar * units_font;	/*! Units Textual font name */
	gchar * units_str;	/*! Units Text String */
	gfloat units_str_xpos;	/*! Name str X pos, 0<->1 range, 0.5 = center*/
	gfloat units_str_ypos;	/*! Name str X pos, 0<->1 range, 0.5 = center*/
	gfloat units_font_scale;/*! Units Font scale, % of 2xradius */
	gchar * value_font;	/*! Value Textual font name */
	gchar * value_str;	/*! Value Text String */
	gfloat value_str_xpos;	/*! Name str X pos, 0<->1 range, 0.5 = center*/
	gfloat value_str_ypos;	/*! Name str X pos, 0<->1 range, 0.5 = center*/
	gfloat value_font_scale;/*! Value Font scale, % of 2xradius */
	gchar * name_font;	/*! Name Textual font name */
	gchar * name_str;	/*! Name Text String */
	gfloat name_str_xpos;	/*! Name str X pos, 0<->1 range, 0.5 = center*/
	gfloat name_str_ypos;	/*! Name str X pos, 0<->1 range, 0.5 = center*/
	gfloat name_font_scale;	/*! Name Font scale, % of 2xradius */
	gboolean antialias;	/*! AA Flag (used in Cairo ONLY */
	gboolean show_value;	/*! Show the Valueon screen or not */
	gint major_ticks;	/*! Number of MAJOR ticks */
	gfloat major_tick_len;	/*! Major Tick length (% of radius 0-1.0) */
	gfloat major_tick_width;/*! Maj tick width as (1/10 percent of radius */
	gint minor_ticks;	/*! Number of MINOR ticks PER pair of major's*/
	gfloat minor_tick_len;	/*! Minor tick length (% of radius 0-1.0) */
	gfloat minor_tick_width;/*! Maj tick width as (1/10 percent of radius */
	gfloat tick_inset;	/*! Percentage of radius to start tickmark @ */
	gfloat needle_width;	/*! % of radius Needle width @ spin axis */
	gfloat needle_tail;	/*! % of rad Length of "backside" of needle */
	gint needle_polygon_points;
	GdkPoint needle_coords[4];	/*! 4 point needle for now */
	GdkPoint last_needle_coords[4];	/*! 4 point needle for now */
	GArray *ranges;		/*! Array to contain the ranges */
};

struct _MtxGaugeFaceClass
{
	GtkDrawingAreaClass parent_class;
};

GType mtx_gauge_face_get_type (void) G_GNUC_CONST;
void generate_gauge_background(GtkWidget *);
void update_gauge_position (GtkWidget *);
GtkWidget* mtx_gauge_face_new ();
void mtx_gauge_face_set_antialias (MtxGaugeFace *gauge, gboolean value);
gboolean mtx_gauge_face_get_antialias (MtxGaugeFace *gauge);
void mtx_gauge_face_set_show_value (MtxGaugeFace *gauge, gboolean value);
gboolean mtx_gauge_face_get_show_value (MtxGaugeFace *gauge);
void mtx_gauge_face_set_value (MtxGaugeFace *gauge, gfloat value);
float mtx_gauge_face_get_value (MtxGaugeFace *gauge);
void mtx_gauge_face_set_color_range(MtxGaugeFace *gauge, gfloat, gfloat, GdkColor, gfloat, gfloat);
gint mtx_gauge_face_set_color_range_struct(MtxGaugeFace *gauge, MtxColorRange *);
GArray * mtx_gauge_face_get_color_ranges(MtxGaugeFace *gauge);
void mtx_gauge_face_remove_color_range(MtxGaugeFace *gauge, gint index);
void mtx_gauge_face_remove_all_color_ranges(MtxGaugeFace *gauge);
void mtx_gauge_face_set_name_str (MtxGaugeFace *gauge, gchar * str);
void mtx_gauge_face_set_units_str (MtxGaugeFace *gauge, gchar * str);
void mtx_gauge_face_set_precision(MtxGaugeFace *gauge, gint);
gint mtx_gauge_face_get_precision(MtxGaugeFace *gauge);
gchar * mtx_gauge_face_get_units_str (MtxGaugeFace *gauge);
gchar * mtx_gauge_face_get_name_str (MtxGaugeFace *gauge);
void mtx_gauge_face_set_name_str_pos (MtxGaugeFace *gauge, gfloat value1, gfloat value2);
void mtx_gauge_face_set_name_str_xpos (MtxGaugeFace *gauge, gfloat );
void mtx_gauge_face_set_name_str_ypos (MtxGaugeFace *gauge, gfloat );
gboolean mtx_gauge_face_get_name_str_pos (MtxGaugeFace *gauge, gfloat *value1, gfloat *value2);
void mtx_gauge_face_set_units_str_pos (MtxGaugeFace *gauge, gfloat value1, gfloat value2);
void mtx_gauge_face_set_units_str_xpos (MtxGaugeFace *gauge, gfloat );
void mtx_gauge_face_set_units_str_ypos (MtxGaugeFace *gauge, gfloat );
gboolean mtx_gauge_face_get_units_str_pos (MtxGaugeFace *gauge, gfloat *value1, gfloat *value2);
void mtx_gauge_face_set_value_str_pos (MtxGaugeFace *gauge, gfloat value1, gfloat value2);
void mtx_gauge_face_set_value_str_xpos (MtxGaugeFace *gauge, gfloat );
void mtx_gauge_face_set_value_str_ypos (MtxGaugeFace *gauge, gfloat );
gboolean mtx_gauge_face_get_value_str_pos (MtxGaugeFace *gauge, gfloat *value1, gfloat *value2);
void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, gfloat value1, gfloat value2);
gboolean mtx_gauge_face_get_bounds (MtxGaugeFace *gauge, gfloat *value1, gfloat *value2);
void mtx_gauge_face_set_lbound (MtxGaugeFace *gauge, gfloat );
void mtx_gauge_face_set_ubound (MtxGaugeFace *gauge, gfloat );
void mtx_gauge_face_set_major_tick_len (MtxGaugeFace *gauge, gfloat );
gfloat mtx_gauge_face_get_major_tick_len (MtxGaugeFace *gauge);
void mtx_gauge_face_set_major_tick_width (MtxGaugeFace *gauge, gfloat );
gfloat mtx_gauge_face_get_major_tick_width (MtxGaugeFace *gauge);
void mtx_gauge_face_set_minor_tick_len (MtxGaugeFace *gauge, gfloat );
gfloat mtx_gauge_face_get_minor_tick_len (MtxGaugeFace *gauge);
void mtx_gauge_face_set_minor_tick_width (MtxGaugeFace *gauge, gfloat );
gfloat mtx_gauge_face_get_minor_tick_width (MtxGaugeFace *gauge);
void mtx_gauge_face_set_major_ticks (MtxGaugeFace *gauge, int ticks);
int mtx_gauge_face_get_major_ticks (MtxGaugeFace *gauge);
void mtx_gauge_face_set_minor_ticks (MtxGaugeFace *gauge, int ticks);
int mtx_gauge_face_get_minor_ticks (MtxGaugeFace *gauge);
gboolean mtx_gauge_face_get_span_rad (MtxGaugeFace *gauge, gfloat *start_radian, gfloat *stop_radian);
gboolean mtx_gauge_face_get_span_deg (MtxGaugeFace *gauge, gfloat *start_deg, gfloat *stop_deg);
void mtx_gauge_face_set_span_rad (MtxGaugeFace *gauge, gfloat start_radian, gfloat stop_radian);
void mtx_gauge_face_set_span_deg (MtxGaugeFace *gauge, gfloat start_deg, gfloat stop_deg);
void mtx_gauge_face_set_lspan_rad (MtxGaugeFace *gauge, gfloat start_radian);
void mtx_gauge_face_set_lspan_deg (MtxGaugeFace *gauge, gfloat start_deg);
void mtx_gauge_face_set_uspan_rad (MtxGaugeFace *gauge, gfloat stop_radian);
void mtx_gauge_face_set_uspan_deg (MtxGaugeFace *gauge, gfloat stop_deg);
void mtx_gauge_face_set_tick_inset (MtxGaugeFace *gauge, gfloat inset);
gfloat mtx_gauge_face_get_tick_inset (MtxGaugeFace *gauge);
void mtx_gauge_face_set_needle_width (MtxGaugeFace *gauge, gfloat width);
gfloat mtx_gauge_face_get_needle_width (MtxGaugeFace *gauge);
void mtx_gauge_face_set_needle_tail (MtxGaugeFace *gauge, gfloat width);
gfloat mtx_gauge_face_get_needle_tail (MtxGaugeFace *gauge);
void mtx_gauge_face_set_units_font (MtxGaugeFace *gauge, gchar * font_name);
gchar * mtx_gauge_face_get_units_font (MtxGaugeFace *gauge);
void mtx_gauge_face_set_value_font (MtxGaugeFace *gauge, gchar * font_name);
gchar * mtx_gauge_face_get_value_font (MtxGaugeFace *gauge);
void mtx_gauge_face_set_name_font (MtxGaugeFace *gauge, gchar * font_name);
gchar * mtx_gauge_face_get_name_font (MtxGaugeFace *gauge);
void mtx_gauge_face_set_units_font_scale (MtxGaugeFace *gauge, gfloat scale);
gfloat mtx_gauge_face_get_units_font_scale (MtxGaugeFace *gauge);
void mtx_gauge_face_set_name_font_scale (MtxGaugeFace *gauge, gfloat scale);
gfloat mtx_gauge_face_get_name_font_scale (MtxGaugeFace *gauge);
void mtx_gauge_face_set_value_font_scale (MtxGaugeFace *gauge, gfloat scale);
gfloat mtx_gauge_face_get_value_font_scale (MtxGaugeFace *gauge);
void mtx_gauge_face_set_color (MtxGaugeFace *gauge, gint index, GdkColor color);
GdkColor *mtx_gauge_face_get_color (MtxGaugeFace *gauge, gint index);
void mtx_gauge_face_import_xml(GtkWidget *, gchar *);
void mtx_gauge_face_export_xml(GtkWidget *, gchar *);

G_END_DECLS

#endif
