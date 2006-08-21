// Christopher Mire, 2006

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <gauge.h>

////////////////////////////////////////////////////////////////////////////////
#define MTX_GAUGE_FACE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFacePrivate))

G_DEFINE_TYPE (MtxGaugeFace, mtx_gauge_face, GTK_TYPE_DRAWING_AREA);

typedef struct
{
	gboolean dragging;
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
static gboolean mtx_gauge_face_motion_notify (GtkWidget *gauge,
					      GdkEventMotion *event);
static gboolean mtx_gauge_face_button_release (GtkWidget *gauge,
					       GdkEventButton *event);

static void mtx_gauge_face_set_value_internal (MtxGaugeFace *gauge, float value)
{
	gauge->value = value;
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}

void mtx_gauge_face_set_value (MtxGaugeFace *gauge, float value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	mtx_gauge_face_set_value_internal (gauge, value);
	g_object_thaw_notify (G_OBJECT (gauge));
}

float mtx_gauge_face_get_value (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->value;
}

void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, float value1, float value2)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	gauge->lbound = value1;
	gauge->ubound = value2;
	gauge->range = gauge->ubound -gauge->lbound;
	printf ("range is %f\n", gauge->range);
}

static void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name)
{
	printf ("class init called \n");
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->expose_event = mtx_gauge_face_expose;
	widget_class->button_press_event = mtx_gauge_face_button_press;
	widget_class->button_release_event = mtx_gauge_face_button_release;
	widget_class->motion_notify_event = mtx_gauge_face_motion_notify;

	/* MtxGaugeFace signals */
	g_type_class_add_private (obj_class, sizeof (MtxGaugeFacePrivate));
}

static void mtx_gauge_face_init (MtxGaugeFace *gauge)
{
	printf ("instance init called \n");
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK
			       | GDK_POINTER_MOTION_MASK);

	gauge->value = 0.0;
	gauge->lbound = -1.0;
	gauge->ubound = 1.0;
	gauge->start_radian = 0.75 * M_PI;
	gauge->stop_radian = 0.25 * M_PI;
	gauge->num_ticks = 24;
	gauge->range = gauge->ubound -gauge->lbound;
	printf ("range is %f\n", gauge->range);
	mtx_gauge_face_redraw_canvas (gauge);
}

static void draw (GtkWidget *gauge, cairo_t *cr)
{
	double x, y;
	double radius;
	int i;
	float current_value;

	x = gauge->allocation.width / 2;
	y = gauge->allocation.height / 2;
	radius = MIN (gauge->allocation.width / 2,
		      gauge->allocation.height / 2) - 5;

	/* gauge back */
 	cairo_arc (cr, x, y, radius, MTX_GAUGE_FACE (gauge)->start_radian,
 		   MTX_GAUGE_FACE (gauge)->stop_radian);

	cairo_set_source_rgba (cr, 1, 1, 1, 1.0);//white
	cairo_fill_preserve (cr);
	cairo_set_source_rgba (cr, 0, 0, 0, 1.0);
	cairo_stroke (cr);

	printf ("radius is %f\n", radius);
	printf ("oringal x is is %f\n", x);
	printf ("oringal y is is %f\n", y);
	/* gauge ticks */
	gint left_tick = (MTX_GAUGE_FACE (gauge)->num_ticks * 2);
	for (i = 12; i <= 36; i++)
	{
		int inset;
		cairo_save (cr); /* stack-pen-size */
		if (i % 3 == 0)
		{
			inset = (int) (0.2 * radius);
		}
		else
		{
			inset = (int) (0.1 * radius);
			cairo_set_line_width (cr, 0.5 *
					      cairo_get_line_width (cr));
		}
		cairo_move_to (cr,
			       x + (radius - inset) * cos (i * M_PI / 16),
			       y + (radius - inset) * sin (i * M_PI / 16));
		cairo_line_to (cr,
			       x + (radius * cos (i * (M_PI / 16))),
			       y + (radius * sin (i * (M_PI / 16))));
		cairo_stroke (cr);
		cairo_restore (cr); /* stack-pen-size */
	}
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, radius / 10);
	char message[30];
	strncpy (message, "Cajunbot", sizeof (message) - 1);
	cairo_move_to (cr, x - (strnlen (message, sizeof(message) - 1) *
				(radius / 32)), 1.5 * y);
	cairo_show_text (cr, message);

	strncpy (message, "left", sizeof (message) - 1);
	cairo_move_to (cr, (x / 3) - (strnlen (message, sizeof(message) - 1) *
				      (radius / 32)), 1.8 * y);
	cairo_show_text (cr, message);

	strncpy (message, "right", sizeof (message) - 1);
	cairo_move_to (cr, ((2 * x) - (x / 3)) - (strnlen (message,
			   sizeof (message) - 1) * (radius / 32)), 1.8 * y);
	cairo_show_text (cr, message);

	/* gauge hands */
	current_value = MTX_GAUGE_FACE (gauge)->value;
	printf ("value is %f\n", current_value);
	cairo_save (cr);
	cairo_set_line_width (cr, 2.5 * cairo_get_line_width (cr));
	cairo_move_to (cr, x, y);
/*  	cairo_line_to (cr, x + radius * sin (M_PI /  (MTX_GAUGE_FACE (gauge)->range * 1.33) * current_value), */
/*  			   y + radius * -cos (M_PI / (MTX_GAUGE_FACE (gauge)->range * 1.33) * current_value)); */

  	cairo_line_to (cr, x + radius * sin ((current_value - MTX_GAUGE_FACE (gauge)->lbound) * (4.71) /
					     MTX_GAUGE_FACE (gauge)->range + (-2.355)),
  			   y + radius * -cos ((current_value - MTX_GAUGE_FACE (gauge)->lbound) * (4.71) /
					      MTX_GAUGE_FACE (gauge)->range + (-2.355)));

	cairo_set_font_size (cr, radius / 5);
	snprintf (message, sizeof (message) - 1, "%f", current_value);
	cairo_move_to (cr, x - (strnlen (message, sizeof(message) - 1) *
				(radius / 16)), 1.2 * y);
	cairo_show_text (cr, message);
	cairo_stroke (cr);
	cairo_restore (cr);
}

static gboolean mtx_gauge_face_expose (GtkWidget *gauge, GdkEventExpose *event)
{
	cairo_t *cr;

	/* get a cairo_t */
	cr = gdk_cairo_create (gauge->window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);
	printf ("expose called\n");
	draw (gauge, cr);

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

static gboolean mtx_gauge_face_motion_notify (GtkWidget *gauge,
					      GdkEventMotion *event)
{
	MtxGaugeFacePrivate *priv;
	printf ("notify\n");
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);
	return FALSE;
}

static gboolean mtx_gauge_face_button_release (GtkWidget *gauge,
					       GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	printf ("button release\n");
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);
	return FALSE;
}

GtkWidget *mtx_gauge_face_new (void)
{
	printf ("new face gauge\n");
	return GTK_WIDGET (g_object_new (MTX_TYPE_GAUGE_FACE, NULL));
}
