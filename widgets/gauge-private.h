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

G_DEFINE_TYPE (MtxGaugeFace, mtx_gauge_face, GTK_TYPE_DRAWING_AREA);

typedef struct
{
	gint dragging;
}
MtxGaugeFacePrivate;


void mtx_gauge_face_class_init (MtxGaugeFaceClass *);
void mtx_gauge_face_init (MtxGaugeFace *);
void generate_gauge_background(GtkWidget *);
void update_gauge_position (GtkWidget *);
gboolean mtx_gauge_face_configure (GtkWidget *, GdkEventConfigure *);
gboolean mtx_gauge_face_expose (GtkWidget *, GdkEventExpose *);
gboolean mtx_gauge_face_button_press (GtkWidget *,GdkEventButton *);
void mtx_gauge_face_redraw_canvas (MtxGaugeFace *);
gboolean mtx_gauge_face_button_release (GtkWidget *,GdkEventButton *);
void cairo_generate_gauge_background(GtkWidget *);
void cairo_update_gauge_position (GtkWidget *);
void gdk_generate_gauge_background(GtkWidget *);
void gdk_update_gauge_position (GtkWidget *);
void mtx_gauge_face_init_colors(MtxGaugeFace *);
void mtx_gauge_face_init_name_bindings(MtxGaugeFace *);
void mtx_gauge_face_init_xml_hash(MtxGaugeFace *);
void mtx_gauge_color_import(gchar *, gpointer);
void mtx_gauge_gint_import(gchar *, gpointer);
void mtx_gauge_gfloat_import(gchar *, gpointer);
void mtx_gauge_gchar_import(gchar *, gpointer);
xmlChar * mtx_gauge_color_export(gpointer);
xmlChar * mtx_gauge_gint_export(gpointer);
xmlChar * mtx_gauge_gfloat_export(gpointer);
xmlChar * mtx_gauge_gchar_export(gpointer);


#endif
