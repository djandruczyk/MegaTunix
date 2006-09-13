// Christopher Mire, 2006

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <gauge.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
#define MTX_GAUGE_FACE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFacePrivate))

G_DEFINE_TYPE (MtxGaugeFace, mtx_gauge_face, GTK_TYPE_DRAWING_AREA);

typedef struct
{
	gint dragging;
}
MtxGaugeFacePrivate;

static void mtx_gauge_face_set_value_internal (MtxGaugeFace *gauge, float value);
static void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name);
static void mtx_gauge_face_init (MtxGaugeFace *gauge);
static void draw (GtkWidget *gauge, cairo_t *cr);
static gboolean mtx_gauge_face_expose (GtkWidget *gauge, GdkEventExpose *event);
static gboolean mtx_gauge_face_button_press (GtkWidget *gauge,
					     GdkEventButton *event);
static void mtx_gauge_face_redraw_canvas (MtxGaugeFace *gauge);
static gboolean mtx_gauge_face_button_release (GtkWidget *gauge,
					       GdkEventButton *event);

static void mtx_gauge_face_set_value_internal (MtxGaugeFace *gauge, float value)
{
	gauge->value = value;
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}

static void mtx_gauge_face_set_units_str_internal (MtxGaugeFace *gauge, gchar * units_str)
{
	if (gauge->units_str)
		g_free(gauge->units_str);
	gauge->units_str = g_strdup(units_str);;
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}

/* Changes value stored in widget, and gets widget redrawn to show change */
void mtx_gauge_face_set_value (MtxGaugeFace *gauge, float value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	mtx_gauge_face_set_value_internal (gauge, value);
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Sets antialias mode */
void mtx_gauge_face_set_antialias(MtxGaugeFace *gauge, gboolean state)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	gauge->antialias = state;
}

/* Changes value stored in widget, and gets widget redrawn to show change */
void mtx_gauge_face_set_units_str (MtxGaugeFace *gauge, gchar * units_str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	mtx_gauge_face_set_units_str_internal (gauge, units_str);
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Returns value that needle currently points to */
float mtx_gauge_face_get_value (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->value;
}

/* Returns antialias status */
gboolean mtx_gauge_face_get_antialias (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return gauge->antialias;
}

/* Returns value that needle currently points to */
gchar * mtx_gauge_face_get_units_str (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->units_str;
}

/* Changes the lower and upper value bounds */
void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, float value1, float value2)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->lbound = value1;
	gauge->ubound = value2;
	gauge->range = gauge->ubound -gauge->lbound;
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* returns the lower and upper value bounds */
gboolean mtx_gauge_face_get_bounds(MtxGaugeFace *gauge, float *value1, float *value2)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	*value1 = gauge->lbound;
	*value2 = gauge->ubound;
	return TRUE;
}
/* Set number of ticks to be shown in the range of the drawn gauge */
void mtx_gauge_face_set_resolution (MtxGaugeFace *gauge, int ticks)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->num_ticks = ticks;
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Returns current number of ticks drawn in span of gauge */
int mtx_gauge_face_get_resolution (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->num_ticks;
}

/* Changes the span of the gauge, specified in radians the start and stop position. */
/* Right is 0 going clockwise to M_PI (180 Degrees) back to 0 (2 * M_PI) */
void mtx_gauge_face_set_span (MtxGaugeFace *gauge, float start_radian, float stop_radian)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->start_radian = start_radian;
	gauge->stop_radian = stop_radian;
	g_object_thaw_notify (G_OBJECT (gauge));
}

static void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->expose_event = mtx_gauge_face_expose;
	widget_class->button_press_event = mtx_gauge_face_button_press;
	widget_class->button_release_event = mtx_gauge_face_button_release;

	g_type_class_add_private (obj_class, sizeof (MtxGaugeFacePrivate));
}

static void mtx_gauge_face_init (MtxGaugeFace *gauge)
{
	//which events widget receives
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK);

	gauge->value = 0.0;//default values
	gauge->lbound = -1.0;
	gauge->ubound = 1.0;
	gauge->start_radian = 0.75 * M_PI;//M_PI is left, 0 is right
	gauge->stop_radian = 2.25 * M_PI;
	gauge->num_ticks = 24;
	gauge->antialias = FALSE;
	gauge->range = gauge->ubound -gauge->lbound;
	mtx_gauge_face_redraw_canvas (gauge);
}

static void draw (GtkWidget *gauge, cairo_t *cr)
{
	gdouble x, y;
	gdouble xc, yc;
	gdouble radius;
	gint last_height;
	gchar * message = NULL;
	gfloat current_value;
	cairo_text_extents_t extents;

	xc = gauge->allocation.width / 2;
	yc = gauge->allocation.height / 2;
	radius = MIN (gauge->allocation.width / 2,
		      gauge->allocation.height / 2) - 5;

	/* gauge back */
 	cairo_arc (cr, xc, yc, radius, MTX_GAUGE_FACE (gauge)->start_radian,
 		   MTX_GAUGE_FACE (gauge)->stop_radian);

	cairo_set_source_rgba (cr, 1, 1, 1, 1.0);//white
	cairo_fill_preserve (cr);
	cairo_set_source_rgba (cr, 0, 0, 0, 1.0);
	cairo_stroke (cr);

	/* gauge ticks */
	gfloat arc = (MTX_GAUGE_FACE (gauge)->stop_radian - MTX_GAUGE_FACE (gauge)->start_radian) / (2 * M_PI);
	gint total_ticks = MTX_GAUGE_FACE (gauge)->num_ticks / arc;//amount of ticks if entire gauge was used
	gint left_tick = total_ticks * (MTX_GAUGE_FACE (gauge)->start_radian / (2 * M_PI));//start tick
	gint right_tick = total_ticks * (MTX_GAUGE_FACE (gauge)->stop_radian / (2 * M_PI));//end tick

	gint counter;
	for (counter = left_tick ; counter <= right_tick; counter++)
	{
		gint inset;
		cairo_save (cr); /* stack-pen-size */
		if (counter % 3 == 0)
		{
			inset = (gint) (0.2 * radius);
		}
		else
		{
			inset = (gint) (0.1 * radius);
			cairo_set_line_width (cr, 0.5 *
					      cairo_get_line_width (cr));
		}
		cairo_move_to (cr,
			       xc + (radius - inset) * cos (counter * M_PI / (total_ticks / 2)),
			       yc + (radius - inset) * sin (counter * M_PI / (total_ticks / 2)));
		cairo_line_to (cr,
			       xc + (radius * cos (counter * (M_PI / (total_ticks / 2)))),
			       yc + (radius * sin (counter * (M_PI / (total_ticks / 2)))));
		cairo_stroke (cr);
		cairo_restore (cr); /* stack-pen-size */
	}


	/* gauge hands */
	current_value = MTX_GAUGE_FACE (gauge)->value;
	cairo_save (cr);
	cairo_set_line_width (cr, 2.5 * cairo_get_line_width (cr));
	cairo_move_to (cr, xc, yc);

	gfloat arc_rad = (2 * M_PI) * arc;//angle in radians of total arc
  	cairo_line_to (cr, xc + radius * sin ((current_value - MTX_GAUGE_FACE (gauge)->lbound) * (arc_rad) /
					     MTX_GAUGE_FACE (gauge)->range +//needle neutral is 1.5 M_PI
					     (MTX_GAUGE_FACE (gauge)->start_radian) - (1.5 * M_PI)),
		       yc + radius * -cos ((current_value - MTX_GAUGE_FACE (gauge)->lbound) * (arc_rad) /
					  MTX_GAUGE_FACE (gauge)->range +//needle neutral is 1.5 M_PI
					  (MTX_GAUGE_FACE (gauge)->start_radian) - (1.5 * M_PI)));

	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size (cr, radius / 5);
	message = g_strdup_printf("%.2f", current_value);

	cairo_text_extents (cr, message, &extents);
	x = 0.5-(extents.width/2 + extents.x_bearing);
	y = 0.5-(extents.height/2 + extents.y_bearing);

	cairo_move_to (cr, xc+x, (1.2*yc)+y);
	cairo_show_text (cr, message);
	g_free(message);
	last_height = extents.height;

	if (MTX_GAUGE_FACE(gauge)->units_str)
	{
		cairo_set_font_size (cr, radius / 8);

		cairo_text_extents (cr, MTX_GAUGE_FACE(gauge)->units_str, &extents);
		x = 0.5-(extents.width/2 + extents.x_bearing);
		y = 0.5-(extents.height/2 + extents.y_bearing);

		cairo_move_to (cr, xc+x, (1.2*yc)+last_height+y);
		cairo_show_text (cr, MTX_GAUGE_FACE(gauge)->units_str);
	}
	cairo_stroke (cr);
	cairo_restore (cr);
}

static gboolean mtx_gauge_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
	cairo_t *cr;
	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	/* get a cairo_t */
	cr = gdk_cairo_create (widget->window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);
	if (gauge->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
	draw (widget, cr);

	cairo_destroy (cr);

	return FALSE;
}

static gboolean mtx_gauge_face_button_press (GtkWidget *gauge,
					     GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);

	return FALSE;
}

static void mtx_gauge_face_redraw_canvas (MtxGaugeFace *gauge)
{
	GtkWidget *widget;
	GdkRegion *region;

	widget = GTK_WIDGET (gauge);

	if (!widget->window) return;

	region = gdk_drawable_get_clip_region (widget->window);
	/* redraw the cairo canvas completely by exposing it */
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_window_process_updates (widget->window, TRUE);

	gdk_region_destroy (region);
}

static gboolean mtx_gauge_face_button_release (GtkWidget *gauge,
					       GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	printf ("button release\n");
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);
	return FALSE;
}

GtkWidget *mtx_gauge_face_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_GAUGE_FACE, NULL));
}
