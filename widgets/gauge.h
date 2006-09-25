// Christopher Mire, 2006

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

struct _MtxGaugeFace
{//public data
	GtkDrawingArea parent;
	GdkPixmap *pixmap;
	GdkPixmap *bg_pixmap;
	cairo_t *cr;
	gfloat value;//very basic now, a single float value to display
	gchar * units_str;
	gfloat lbound;
	gfloat ubound;
	gfloat range;
	gdouble xc;
	gdouble yc;
	gdouble radius;
	gfloat start_radian;
	gfloat stop_radian;
	gboolean antialias;
	gint num_ticks;
	GdkGC * axis_gc;
	GdkGC * font_gc;
	GdkGC * needle_gc;
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
void mtx_gauge_face_set_units_str (MtxGaugeFace *gauge, gchar * str);
gchar * mtx_gauge_face_get_units_str (MtxGaugeFace *gauge);
void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, float value1, float value2);
gboolean mtx_gauge_face_get_bounds (MtxGaugeFace *gauge, float *value1, float *value2);
void mtx_gauge_face_set_resolution (MtxGaugeFace *gauge, int ticks);
int mtx_gauge_face_get_resolution (MtxGaugeFace *gauge);
void mtx_gauge_face_set_span (MtxGaugeFace *gauge, float start_radian, float stop_radian);
//should also have functions to set scale, maybe autoscale

G_END_DECLS

#endif
