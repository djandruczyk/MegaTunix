/* Originally designed by Christopher Mire, 2006
 *
 * Modded and rewritten to a large extent by David Andruczy with help from 
 * Chris, and Arik (msefi.com forums) for better, looks and performance.
 * This is a hte PRIVATE implementation header file for INTERNAL functions
 * of the widget.  Public functions as well the the gauge structure are 
 * defined in the gauge.h header file
 *
 */

#ifndef __GAUGE_PRIVATE_H__
#define  __GAUGE_PRIVATE_H__

#include <gtk/gtk.h>
#include <gauge.h>



#define MTX_GAUGE_FACE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MTX_TYPE_GAUGE_FACE, MtxGaugeFacePrivate))

G_DEFINE_TYPE (MtxGaugeFace, mtx_gauge_face, GTK_TYPE_DRAWING_AREA);

typedef struct
{
	gint dragging;
}
MtxGaugeFacePrivate;

void mtx_gauge_face_set_value_internal (MtxGaugeFace *, float );
void mtx_gauge_face_class_init (MtxGaugeFaceClass *);
void mtx_gauge_face_init (MtxGaugeFace *);
void generate_gauge_background(GtkWidget *);
void update_gauge_position (GtkWidget *);
gboolean mtx_gauge_face_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_gauge_face_expose (GtkWidget *, GdkEventExpose *);
gboolean mtx_gauge_face_button_press (GtkWidget *,GdkEventButton *);
void mtx_gauge_face_redraw_canvas (MtxGaugeFace *);
gboolean mtx_gauge_face_button_release (GtkWidget *,GdkEventButton *);
void mtx_gauge_face_set_name_str_internal (MtxGaugeFace *, gchar *);
void mtx_gauge_face_set_units_str_internal (MtxGaugeFace *, gchar *);

#endif
