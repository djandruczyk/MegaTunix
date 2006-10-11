// Christopher Mire, 2006

#include <config.h>
#ifndef MTX_GAUGE_FACE_H
#define MTX_GAUGE_FACE_H

G_BEGIN_DECLS

#define MTX_TYPE_GAUGE_FACE		(mtx_gauge_face_get_type ())
#define MTX_GAUGE_FACE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFace))
#define MTX_GAUGE_FACE_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), MTX_GAUGE_FACE, MtxGaugeFaceClass))
#define MTX_IS_GAUGE_FACE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTX_TYPE_GAUGE_FACE))
#define MTX_IS_GAUGE_FACE_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MTX_TYPE_GAUGE_FACE))
#define MTX_GAUGE_FACE_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFaceClass))

typedef struct _MtxGaugeFace		MtxGaugeFace;
typedef struct _MtxGaugeFaceClass	MtxGaugeFaceClass;
typedef struct _MtxColorRange		MtxColorRange;
typedef struct _MtxColorRanges		MtxColorRanges;

struct _MtxColorRange
{
	gdouble lowpoint;
	gdouble highpoint;
	GdkColor color;
};

struct _MtxColorRanges
{
	gint total_ranges;
	GHashTable *range_hash;
};

struct _MtxGaugeFace
{//public data
	GtkDrawingArea parent;
	GdkPixmap *pixmap;	/*! Update/backing pixmap */
	GdkPixmap *bg_pixmap;	/*! Static rarely changing pixmap */
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
	GdkGC * axis_gc;	/*! Axis Graphics Context, needed??? */
	GdkGC * font_gc;	/*! Font Graphics Context, needed??? */
	GdkGC * needle_gc;	/*! Needle Graphics Context, needed??? */
	GdkColormap *colormap;	/*! Colormap for GC's */
	gint precision;		/*! number of decimal places for val */
	gfloat start_deg; 	/*! GDK Start point in degrees (CCW) */
	gfloat stop_deg;	/*! GDK Stop point in degrees (CCW) */
	gfloat start_radian;	/*! CAIRO Start angle in radians (CW) */
	gfloat stop_radian;	/*! CAIRO Stop Angle in radians (CW) */
	gfloat value;		/*! Value represneting needle position */
	gfloat lbound;		/*! Lower Bound to clamp at */
	gfloat ubound;		/*! Upper Bound to Clamp at */
	gfloat range;		/*! Range from lbound to ubound */
	gchar * units_font;	/*! Units Textual font name */
	gchar * units_str;	/*! Units Text String */
	gfloat units_font_scale;/*! Units Font scale, % of 2xradius */
	gchar * value_font;	/*! Value Textual font name */
	gchar * value_str;	/*! Value Text String */
	gfloat value_font_scale;/*! Value Font scale, % of 2xradius */
	gchar * name_font;	/*! Name Textual font name */
	gchar * name_str;	/*! Name Text String */
	gfloat name_font_scale;	/*! Name Font scale, % of 2xradius */
	gboolean antialias;	/*! AA Flag (used in Cairo ONLY */
	gint num_ticks;		/*! Total Number of Ticks */
	gint majtick_divisor;	/*! Number of min ticks before major */
	gfloat tick_inset;	/*! Percentage of radius to start tickmark @ */
	gfloat major_tick_len;	/*! Major Tick length (% of radius 0-1.0) */
	gfloat minor_tick_len;	/*! Minor tick length (% of radius 0-1.0) */
	gfloat needle_width;	/*! % of radius Needle width @ spin axis */
	gfloat needle_tail;	/*! % of rad Length of "backside" of needle */
	gint needle_polygon_points;
	GdkPoint needle_coords[4];	/*! 4 point needle for now */
	GdkPoint last_needle_coords[4];	/*! 4 point needle for now */
	MtxColorRanges *ranges;	/*! Array of color limits structures */
};

struct _MtxGaugeFaceClass
{
	GtkDrawingAreaClass parent_class;
};

GType mtx_gauge_face_get_type (void) G_GNUC_CONST;
GtkWidget* mtx_gauge_face_new ();
void mtx_gauge_face_set_antialias (MtxGaugeFace *gauge, gboolean value);
gboolean mtx_gauge_face_get_antialias (MtxGaugeFace *gauge);
void mtx_gauge_face_set_value (MtxGaugeFace *gauge, float value);
float mtx_gauge_face_get_value (MtxGaugeFace *gauge);
MtxColorRange * mtx_gauge_face_set_color_range(MtxGaugeFace *gauge, gdouble, gdouble, GdkColor);
MtxColorRanges * mtx_gauge_face_get_ranges(MtxGaugeFace *gauge);
void mtx_gauge_face_set_name_str (MtxGaugeFace *gauge, gchar * str);
void mtx_gauge_face_set_units_str (MtxGaugeFace *gauge, gchar * str);
void mtx_gauge_face_set_precision(MtxGaugeFace *gauge, gint);
gint mtx_gauge_face_get_precision(MtxGaugeFace *gauge);
gchar * mtx_gauge_face_get_units_str (MtxGaugeFace *gauge);
void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, float value1, float value2);
gboolean mtx_gauge_face_get_bounds (MtxGaugeFace *gauge, float *value1, float *value2);
void mtx_gauge_face_set_resolution (MtxGaugeFace *gauge, int ticks);
int mtx_gauge_face_get_resolution (MtxGaugeFace *gauge);
void mtx_gauge_face_set_span (MtxGaugeFace *gauge, float start_radian, float stop_radian);
void cairo_generate_gauge_background(GtkWidget *);
void cairo_update_gauge_position (GtkWidget *);
void gdk_generate_gauge_background(GtkWidget *);
void gdk_update_gauge_position (GtkWidget *);
//should also have functions to set scale, maybe autoscale

G_END_DECLS

#endif
