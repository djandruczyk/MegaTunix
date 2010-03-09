/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire (czb)
 *
 * MegaTunix gauge widget
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

#ifndef __GAUGE_PRIVATE_H__
#define  __GAUGE_PRIVATE_H__

#include <gtk/gtk.h>
#include <gauge.h>


#define MTX_GAUGE_FACE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFacePrivate))

typedef struct _MtxGaugeFacePrivate     MtxGaugeFacePrivate;

struct _MtxGaugeFacePrivate
{
        /* Private data */
        GdkBitmap *bitmap;      /*! for shape_combine stuff */
        GdkPixmap *pixmap;      /*! Update/backing pixmap */
        GdkPixmap *bg_pixmap;   /*! Static rarely changing pixmap */
        GdkPixmap *tmp_pixmap;  /*! Tmp pixmap for alerts for speed boost */
        gint w;                 /*! width */
        gint h;                 /*! height */
	gint max_layers;	/*! maximum layers */
        gdouble xc;             /*! X Center */
        gdouble yc;             /*! Y Center */
        gdouble radius;         /*! Radius of display */
	gdouble last_click_x;	/*! Last mouse click X in -1 <-> +1 scaling */
	gdouble last_click_y;	/*! Last mouse click Y in -1 <-> +1 scaling */
        cairo_t *cr;            /*! Cairo context,  not sure if this is good
                                   too hold onto or not */
        cairo_font_options_t * font_options;
        PangoLayout *layout;    /*! Pango TextLayout object */
        PangoFontDescription *font_desc;/*! Pango Font description */
        GdkGC * bm_gc;          /*! Graphics Context for bitmap */
        GdkGC * gc;             /*! Graphics Context for drawing */
        GdkColormap *colormap;  /*! Colormap for GC's */
        gchar *value_font;      /*! Array of Font name strings */
        GArray *xmlfunc_array; /*! Array list mapping varnames to xml */
        GHashTable * xmlfunc_hash; /*! Hashtable mapping varnames to xml
                                   *  parsing functions */
        GArray *t_blocks;       /*! Array of MtxTextBlock structs */
        GArray *w_ranges;       /*! Array of MtxWarningRange structs */
        GArray *a_ranges;       /*! Array of MtxAlertRange structs */
        GArray *tick_groups;    /*! Array to contain the tick groups */
        GArray *polygons;       /*! Array to contain polygon defs */
        gchar * xml_filename;   /*! Filename of XML for this gauge  */
        gboolean show_drag_border;      /*! Show drag border flag */
        MtxClampType clamped;   /*! Isthe display clamped? */
        guint last_alert_index;  /*! index of last active alert struct */
        GdkColor colors[GAUGE_NUM_COLORS]; /*! Array of colors for specific
                                             parts of a gauge object */
	MtxDirection direction; /*! Direction of motion on the gauge */
	gboolean reenable_tattletale;	/*! Special tattle flag */
	gboolean show_tattletale;	/*! Show a Tattletale? */
	gfloat tattletale_alpha;/*! TattleTale transparency */
        gfloat value_font_scale;/*! Array of font scales */
        gfloat value_xpos;      /*! Array of X offsets for strings */
        gfloat value_ypos;      /*! Array of X offsets for strings */
        gint precision;         /*! number of decimal places for val */
        gfloat start_angle;     /*! Start point, (Cairo, CW rotation) */
        gfloat sweep_angle;     /*! Sweep of gauge (cairo, CW increasing) */
        gfloat value;           /*! Value represneting needle position */
	gfloat peak;		/*! Peak Value */
        gfloat lbound;          /*! Lower Bound to clamp at */
        gfloat ubound;          /*! Upper Bound to Clamp at */
        MtxRotType rotation;    /*! Rotation enumeration */
        gfloat span;            /*! Span from lbound to ubound */
        gboolean antialias;     /*! AA Flag (used in Cairo ONLY */
        gboolean show_value;    /*! Show the Valueon screen or not */
        gfloat needle_width;    /*! % of radius Needle width @ spin axis */
        gfloat needle_tail;     /*! % of rad Length of "backside" of needle */
        gfloat needle_length;   /*! % of rad length of main needle */
        gfloat needle_tip_width;/*! % of rad width of needle tip */
        gfloat needle_tail_width;/*! % of rad width of needle tip */
        gint needle_polygon_points;
	GdkRectangle needle_bounding_box;	/*! needle bounding box */
	GdkRectangle value_bounding_box;	/*! value text bounding box */
        MtxPoint needle_coords[6];      /*! 6 point needle for now */
        MtxPoint tattle_coords[6];      /*! 6 point needle for now */
	MtxDayNite daytime_mode;	/*! Color enum */
};


gboolean mtx_gauge_face_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_gauge_face_expose (GtkWidget *, GdkEventExpose *);
gboolean mtx_gauge_face_button_press (GtkWidget *,GdkEventButton *);
gboolean mtx_gauge_face_button_release (GtkWidget *,GdkEventButton *);
gboolean mtx_gauge_face_key_event (GtkWidget *,GdkEventKey *);
gboolean mtx_gauge_face_motion_event (GtkWidget *,GdkEventMotion *);
void mtx_gauge_face_size_request (GtkWidget *, GtkRequisition *);
void mtx_gauge_face_init (MtxGaugeFace *gauge);
void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name);
void generate_gauge_background(MtxGaugeFace *);
void update_gauge_position (MtxGaugeFace *);
void mtx_gauge_face_init_colors(MtxGaugeFace *);
void mtx_gauge_face_init_name_bindings(MtxGaugeFace *);
void mtx_gauge_face_init_xml_hash(MtxGaugeFace *);
void mtx_gauge_face_init_default_tick_group(MtxGaugeFace *);
void calc_bounding_box(MtxPoint *, gint, GdkRectangle *);


#endif
