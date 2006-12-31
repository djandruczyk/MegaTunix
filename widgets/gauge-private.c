/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Christopher Mire, 2006
 *
 * Megasquirt gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 *  
 *
 * -------------------------------------------------------------------------
 *  Hacked and slashed to hell by David J. Andruczyk in order to bend and
 *  tweak it to my needs for MegaTunix.  Added in rendering ability using
 *  cairo and raw GDK callls for those less fortunate (OS-X)
 *  Added a HUGE number of functions to get/set every gauge attribute
 *
 *
 *  Was offered a fine contribution by Ari Karhu 
 *  "ari <at> ultimatevw <dot> com" from the msefi.com forums.
 *  His contribution made the gauges look, ohh so much nicer than I could 
 *  have come up with!
 */


#include <config.h>
#ifdef HAVE_CAIRO
#include <cairo/cairo.h>
#endif
#include <gauge.h>
#include <gauge-private.h>
#include <gauge-xml.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>


G_DEFINE_TYPE (MtxGaugeFace, mtx_gauge_face, GTK_TYPE_DRAWING_AREA);


/*!
 \brief Initializesthe mtx gauge face class and links in the primary
 signal handlers for config event, expose event, and button press/release
 \param class_name (MtxGaugeFaceClass *) pointer to the class
 */
void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_gauge_face_configure;
	widget_class->expose_event = mtx_gauge_face_expose;
	widget_class->button_press_event = mtx_gauge_face_button_press;
	widget_class->button_release_event = mtx_gauge_face_button_release;
	/* Motion event not needed, as unused currently */
//	widget_class->motion_notify_event = mtx_gauge_face_motion_event;
	widget_class->size_request = mtx_gauge_face_size_request;

	//g_type_class_add_private (obj_class, sizeof (MtxGaugeFacePrivate));
}


/*!
 \brief Initializes the gauge attributes to sane defaults
 \param gauge (MtxGaugeFace *) pointer to the gauge object
 */
void mtx_gauge_face_init (MtxGaugeFace *gauge)
{
	/* The events the gauge receives
	* Need events for button press/release AND motion EVEN THOUGH
	* we don't have a motion handler defined.  It's required for the 
	* dash designer to do drag and move placement 
	*/ 
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK |GDK_POINTER_MOTION_MASK);

	gauge->w = 0;
	gauge->h = 0;
	gauge->xc = 0.0;
	gauge->yc = 0.0;
	gauge->radius = 0.0;
	gauge->value = 0.0;//default values
	gauge->lbound = 0.0;
	gauge->ubound = 100.0;
	gauge->precision = 2;
	gauge->start_radian = 0.75 * M_PI;//M_PI is left, 0 is right
	gauge->stop_radian = 2.25 * M_PI;
	gauge->major_ticks = 9;
	gauge->font_str[MAJ_TICK] = g_strdup("Bitstream Vera Sans");
	gauge->font_scale[MAJ_TICK] = 0.135;
	gauge->minor_ticks = 3;  /* B S S S B S S S B  tick style */
	gauge->tick_inset = 0.15;    /* how much in from gauge radius fortick */
	gauge->major_tick_text_inset = 0.030;    /* how much in from gauge radius for tick text */
	gauge->major_tick_len = 0.1; /* 1 = 100% of radius, so 0.1 = 10% */
	gauge->minor_tick_len = 0.05;/* 1 = 100% of radius, so 0.1 = 10% */
	gauge->major_tick_width = 0.175; /* 1 = 10% of radius, so 0.1 = 1% */
	gauge->minor_tick_width = 0.1;/* 1 = 10% of radius, so 0.1 = 1% */
	gauge->needle_width = 0.05;  /* % of radius */
	gauge->needle_tail = 0.083;  /* % of radius */
	gauge->font_str[VALUE] = g_strdup("Bitstream Vera Sans");
	gauge->text_xpos[VALUE] = 0.0;
	gauge->text_ypos[VALUE] = 0.40;
	gauge->font_scale[VALUE] = 0.2;
	gauge->span = gauge->ubound - gauge->lbound;
#ifdef HAVE_CAIRO
	gauge->cr = NULL;
	gauge->antialias = TRUE;
#else
	gauge->antialias = FALSE;
#endif
	gauge->show_value = TRUE;
	gauge->start_deg = -((gauge->start_radian/M_PI) *180.0);
	gauge->stop_deg = -(((gauge->stop_radian-gauge->start_radian)/M_PI) *180.0);
	gauge->colormap = gdk_colormap_get_system();
	gauge->gc = NULL;
	gauge->c_ranges = g_array_new(FALSE,TRUE,sizeof(MtxColorRange *));
	gauge->t_blocks = g_array_new(FALSE,TRUE,sizeof(MtxTextBlock *));
	mtx_gauge_face_init_colors(gauge);
	mtx_gauge_face_init_name_bindings(gauge);
	mtx_gauge_face_init_xml_hash(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
}


void mtx_gauge_face_init_name_bindings(MtxGaugeFace *gauge)
{
	g_object_set_data(G_OBJECT(gauge),"bg_color", &gauge->colors[COL_BG]);
	g_object_set_data(G_OBJECT(gauge),"needle_color", &gauge->colors[COL_NEEDLE]);
	g_object_set_data(G_OBJECT(gauge),"majtick_color", &gauge->colors[COL_MAJ_TICK]);
	g_object_set_data(G_OBJECT(gauge),"mintick_color", &gauge->colors[COL_MIN_TICK]);
	g_object_set_data(G_OBJECT(gauge),"value_font_color", &gauge->colors[COL_VALUE_FONT]);
	g_object_set_data(G_OBJECT(gauge),"major_tick_text_color", &gauge->colors[COL_MAJ_TICK_TEXT_FONT]);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color", &gauge->colors[COL_GRADIENT_BEGIN]);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color", &gauge->colors[COL_GRADIENT_END]);
	g_object_set_data(G_OBJECT(gauge),"needle_width", &gauge->needle_width);
	g_object_set_data(G_OBJECT(gauge),"needle_tail", &gauge->needle_tail);
	g_object_set_data(G_OBJECT(gauge),"precision", &gauge->precision);
	g_object_set_data(G_OBJECT(gauge),"width", &gauge->w);
	g_object_set_data(G_OBJECT(gauge),"height", &gauge->h);
	g_object_set_data(G_OBJECT(gauge),"start_deg", &gauge->start_deg);
	g_object_set_data(G_OBJECT(gauge),"stop_deg", &gauge->stop_deg);
	g_object_set_data(G_OBJECT(gauge),"start_radian", &gauge->start_radian);
	g_object_set_data(G_OBJECT(gauge),"stop_radian", &gauge->stop_radian);
	g_object_set_data(G_OBJECT(gauge),"lbound", &gauge->lbound);
	g_object_set_data(G_OBJECT(gauge),"ubound", &gauge->ubound);
	g_object_set_data(G_OBJECT(gauge),"value_font", &gauge->font_str[VALUE]);
	g_object_set_data(G_OBJECT(gauge),"value_font_scale", &gauge->font_scale[VALUE]);
	g_object_set_data(G_OBJECT(gauge),"value_str_xpos", &gauge->text_xpos[VALUE]);
	g_object_set_data(G_OBJECT(gauge),"value_str_ypos", &gauge->text_ypos[VALUE]);
	g_object_set_data(G_OBJECT(gauge),"antialias", &gauge->antialias);
	g_object_set_data(G_OBJECT(gauge),"show_value", &gauge->show_value);
	g_object_set_data(G_OBJECT(gauge),"tick_inset", &gauge->tick_inset);
	g_object_set_data(G_OBJECT(gauge),"major_ticks", &gauge->major_ticks);
	g_object_set_data(G_OBJECT(gauge),"major_tick_len", &gauge->major_tick_len);
	g_object_set_data(G_OBJECT(gauge),"major_tick_width", &gauge->major_tick_width);
	g_object_set_data(G_OBJECT(gauge),"major_tick_text_font", &gauge->font_str[MAJ_TICK]);
	g_object_set_data(G_OBJECT(gauge),"major_tick_text_scale", &gauge->font_scale[MAJ_TICK]);
	g_object_set_data(G_OBJECT(gauge),"major_tick_text", &gauge->txt_str[MAJ_TICK]);
	g_object_set_data(G_OBJECT(gauge),"major_tick_text_inset", &gauge->major_tick_text_inset);

	g_object_set_data(G_OBJECT(gauge),"minor_ticks", &gauge->minor_ticks);
	g_object_set_data(G_OBJECT(gauge),"minor_tick_len", &gauge->minor_tick_len);
	g_object_set_data(G_OBJECT(gauge),"minor_tick_width", &gauge->minor_tick_width);
}

/*!
 * \brief  initializes and populates the xml_functions hashtable 
 */
void mtx_gauge_face_init_xml_hash(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxXMLFuncs * funcs = NULL;
	gauge->xmlfunc_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	 gint num_xml_funcs = sizeof(xml_functions) / sizeof(xml_functions[0]);

	gauge->xmlfunc_array = g_array_sized_new(FALSE,TRUE,sizeof (MtxXMLFuncs *),num_xml_funcs);

	for (i=0;i<num_xml_funcs;i++)
	{
		funcs = g_new0(MtxXMLFuncs, 1);
		funcs->import_func = xml_functions[i].import_func;
		funcs->export_func = xml_functions[i].export_func;;
		funcs->varname = xml_functions[i].varname;
		funcs->dest_var = (gpointer)g_object_get_data(G_OBJECT(gauge),xml_functions[i].varname);
		g_hash_table_insert (gauge->xmlfunc_hash,g_strdup(xml_functions[i].varname),funcs);
		g_array_append_val(gauge->xmlfunc_array,funcs);
	}

}

/*!
 \brief Allocates the default colors for a gauge with no options 
 \param widget (GtkWidget *) pointer to the gauge object
 */
void mtx_gauge_face_init_colors(MtxGaugeFace *gauge)
{
	/* Defaults for the gauges,  user over-ridable */

	/*! Background */
	gauge->colors[COL_BG].red=0*65535;
	gauge->colors[COL_BG].green=0*65535;
	gauge->colors[COL_BG].blue=0*65535;
	/*! Needle */
	gauge->colors[COL_NEEDLE].red=1.0*65535;
	gauge->colors[COL_NEEDLE].green=1.0*65535;
	gauge->colors[COL_NEEDLE].blue=1.0*65535;
	/*! Major Ticks */
	gauge->colors[COL_MAJ_TICK].red=0.8*65535;
	gauge->colors[COL_MAJ_TICK].green=0.8*65535;
	gauge->colors[COL_MAJ_TICK].blue=0.8*65535;
	/*! Minor Ticks */
	gauge->colors[COL_MIN_TICK].red=0.8*65535;
	gauge->colors[COL_MIN_TICK].green=0.8*65535;
	gauge->colors[COL_MIN_TICK].blue=0.8*65535;
	/*! Units Font*/
	gauge->colors[COL_VALUE_FONT].red=0.8*65535;
	gauge->colors[COL_VALUE_FONT].green=0.8*65535;
	gauge->colors[COL_VALUE_FONT].blue=0.8*65535;
	/*! Major Tick Text Strings Font */
	gauge->colors[COL_MAJ_TICK_TEXT_FONT].red=0.85*65535;
	gauge->colors[COL_MAJ_TICK_TEXT_FONT].green=0.85*65535;
	gauge->colors[COL_MAJ_TICK_TEXT_FONT].blue=0.85*65535;
	/*! Gradient Color Begin */
	gauge->colors[COL_GRADIENT_BEGIN].red=0.85*65535;
	gauge->colors[COL_GRADIENT_BEGIN].green=0.85*65535;
	gauge->colors[COL_GRADIENT_BEGIN].blue=0.85*65535;
	/*! Gradient Color End */
	gauge->colors[COL_GRADIENT_END].red=0.15*65535;
	gauge->colors[COL_GRADIENT_END].green=0.15*65535;
	gauge->colors[COL_GRADIENT_END].blue=0.15*65535;

}


/*!
 \brief updates the gauge position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param widget (GtkWidget *) pointer to the gauge object
 */
void cairo_update_gauge_position (GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	gfloat radian_span = 0.0;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * message = NULL;
	gint i = 0;
	gfloat n_width = 0.0;
	gfloat n_tail = 0.0;
	gfloat n_tip = 0.0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	/* Copy background pixmap to intermediary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);


	cr = gdk_cairo_create (gauge->pixmap);

	/* Update the VALUE text */
	if (gauge->show_value)
	{
		cairo_set_source_rgb (cr, gauge->colors[COL_VALUE_FONT].red/65535.0,
				gauge->colors[COL_VALUE_FONT].green/65535.0,
				gauge->colors[COL_VALUE_FONT].blue/65535.0);
		cairo_select_font_face (cr, gauge->font_str[VALUE], CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (cr, (gauge->radius * gauge->font_scale[VALUE]));

		message = g_strdup_printf("%.*f", gauge->precision,gauge->value);

		cairo_text_extents (cr, message, &extents);

		cairo_move_to (cr, 
				gauge->xc-(extents.width/2 + extents.x_bearing)+(gauge->text_xpos[VALUE]*gauge->radius),
			       	gauge->yc-(extents.height/2 + extents.y_bearing)+(gauge->text_ypos[VALUE]*gauge->radius));
		cairo_show_text (cr, message);
		g_free(message);

		cairo_stroke (cr);
	}

	/* gauge hands */
	radian_span = (gauge->stop_radian - gauge->start_radian);
	tmpf = (gauge->value-gauge->lbound)/(gauge->ubound-gauge->lbound);
	needle_pos = gauge->start_radian+(tmpf*radian_span)+M_PI/2.0;

	cairo_set_source_rgb (cr, gauge->colors[COL_NEEDLE].red/65535.0,
				gauge->colors[COL_NEEDLE].green/65535.0,
				gauge->colors[COL_NEEDLE].blue/65535.0);
	cairo_set_line_width (cr, 1);

	n_width = gauge->needle_width * gauge->radius;
	n_tail = gauge->needle_tail * gauge->radius;
	n_tip = gauge->radius * 0.850;
	xc = gauge->xc;
	yc = gauge->yc;

	/* STORE needle coordinates to make the expese event a LOT more 
	 * efficient */
	for (i=0;i<4;i++)
	{
		gauge->last_needle_coords[i].x = gauge->needle_coords[i].x;
		gauge->last_needle_coords[i].y = gauge->needle_coords[i].y;
	}
	gauge->needle_coords[0].x = xc + (n_tip) * sin (needle_pos);
	gauge->needle_coords[0].y = yc + (n_tip) * -cos (needle_pos);

	gauge->needle_coords[1].x = xc + (n_width) *cos(needle_pos);
	gauge->needle_coords[1].y = yc + (n_width) *sin(needle_pos);

	gauge->needle_coords[2].x = xc + (n_tail) * -sin (needle_pos);
	gauge->needle_coords[2].y = yc + (n_tail) * cos (needle_pos);
	gauge->needle_coords[3].x = xc - (n_width) * cos (needle_pos);
	gauge->needle_coords[3].y = yc - (n_width) * sin (needle_pos);
	gauge->needle_polygon_points = 4;

	cairo_move_to (cr, gauge->needle_coords[0].x,gauge->needle_coords[0].y);
	cairo_line_to (cr, gauge->needle_coords[1].x,gauge->needle_coords[1].y);
	cairo_line_to (cr, gauge->needle_coords[2].x,gauge->needle_coords[2].y);
	cairo_line_to (cr, gauge->needle_coords[3].x,gauge->needle_coords[3].y);
	cairo_fill_preserve (cr);
	cairo_destroy(cr);
#endif
}


/*!
 \brief updates the gauge position,  This is the GDK implementation that
 looks doesn't do antialiasing,  but is the fastest one.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void gdk_update_gauge_position (GtkWidget *widget)
{
	gint i= 0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gfloat tmpf = 0.0;
	gfloat radian_span = 0.0;
	gfloat needle_pos = 0.0;
	gint n_width = 0;
	gint n_tail = 0;
	gint n_tip = 0;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	PangoRectangle logical_rect;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	/* Copy bacground pixmap to intermediaary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);


	/* the text */
	if (gauge->show_value)
	{
		gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_VALUE_FONT]);
		message = g_strdup_printf("%.*f", gauge->precision,gauge->value);

		tmpbuf = g_strdup_printf("%s %i",gauge->font_str[VALUE],(gint)(gauge->radius *gauge->font_scale[VALUE]));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
		pango_layout_set_text(gauge->layout,message,-1);
		pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);
		g_free(message);

		gdk_draw_layout(gauge->pixmap,gauge->gc,
				gauge->xc-(logical_rect.width/2)+(gauge->text_xpos[VALUE]*gauge->radius),
				gauge->yc-(logical_rect.height/2)+(gauge->text_ypos[VALUE]*gauge->radius),gauge->layout);
	}

	gdk_gc_set_line_attributes(gauge->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_ROUND,
			GDK_JOIN_ROUND);

	/* gauge hands */
	radian_span = (gauge->stop_radian - gauge->start_radian);
	tmpf = (gauge->value-gauge->lbound)/(gauge->ubound-gauge->lbound);
	needle_pos = gauge->start_radian+(tmpf*radian_span)+M_PI/2.0;
	//	printf("start_rad %f, stop_rad %f\n",gauge->start_radian, gauge->stop_radian);
	//	printf("tmpf %f, radian span %f, position, %f\n",tmpf,radian_span,needle_pos);

	xc= gauge->xc;
	yc= gauge->yc;
	n_width = gauge->needle_width * gauge->radius;
	n_tail = gauge->needle_tail * gauge->radius;
	n_tip = gauge->radius * 0.850;

	/* Four POINT needle,  point 0 is the tip (easiest to find) */
	for (i=0;i<4;i++)
	{
		gauge->last_needle_coords[i].x = gauge->needle_coords[i].x;
		gauge->last_needle_coords[i].y = gauge->needle_coords[i].y;
	}
	gauge->needle_coords[0].x = xc + (n_tip) * sin (needle_pos);
	gauge->needle_coords[0].y = yc + (n_tip) * -cos (needle_pos);

	gauge->needle_coords[1].x = xc + (n_width) *cos(needle_pos);
	gauge->needle_coords[1].y = yc + (n_width) *sin(needle_pos);

	gauge->needle_coords[2].x = xc + (n_tail) * -sin (needle_pos);
	gauge->needle_coords[2].y = yc + (n_tail) * cos (needle_pos);
	gauge->needle_coords[3].x = xc - (n_width) * cos (needle_pos);
	gauge->needle_coords[3].y = yc - (n_width) * sin (needle_pos);
	gauge->needle_polygon_points = 4;

	/* Draw the needle */
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_NEEDLE]);
	gdk_draw_polygon(gauge->pixmap,
			widget->style->white_gc,
			TRUE,gauge->needle_coords,4);

}


/*!
 \brief handles configure events whe nthe gauge gets created or resized.
 Takes care of creating/destroying graphics contexts, backing pixmaps (two 
 levels are used to split the rendering for speed reasons) colormaps are 
 also created here as well
 \param widget (GtkWidget *) pointer to the gauge object
 \param event (GdkEventConfigure *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_gauge_face_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	GdkColormap *colormap;
	GdkColor black;
	GdkColor white;
	GdkGC *gc;

	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	if(widget->window)
	{
		gauge->w = widget->allocation.width;
		gauge->h = widget->allocation.height;

		if (gauge->gc)
			g_object_unref(gauge->gc);
		if (gauge->bm_gc)
			g_object_unref(gauge->bm_gc);
		if (gauge->layout)
			g_object_unref(gauge->layout);
		/* Shape combine bitmap */
		if (gauge->bitmap)
			g_object_unref(gauge->bitmap);
		gauge->bitmap = gdk_pixmap_new(NULL,gauge->w,gauge->h,1);
		/* Backing pixmap (copy of window) */
		if (gauge->pixmap)
			g_object_unref(gauge->pixmap);
		gauge->pixmap=gdk_pixmap_new(widget->window,
				gauge->w,gauge->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(gauge->pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				gauge->w,gauge->h);
		/* Static Background pixmap */
		if (gauge->bg_pixmap)
			g_object_unref(gauge->bg_pixmap);
		gauge->bg_pixmap=gdk_pixmap_new(widget->window,
				gauge->w,gauge->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(gauge->bg_pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				gauge->w,gauge->h);

		gdk_window_set_back_pixmap(widget->window,gauge->pixmap,0);
		gauge->layout = gtk_widget_create_pango_layout(GTK_WIDGET(&gauge->parent),NULL);	
		gauge->gc = gdk_gc_new(gauge->bg_pixmap);
		gdk_gc_set_colormap(gauge->gc,gauge->colormap);

		gauge->xc = gauge->w / 2;
		gauge->yc = gauge->h / 2;
		gauge->radius = MIN (gauge->w/2, gauge->h/2); // - 5;


		/* Shape combine mask bitmap */
		colormap = gdk_colormap_get_system ();
		gdk_color_parse ("black", & black);
		gdk_colormap_alloc_color(colormap, &black,TRUE,TRUE);
		gdk_color_parse ("white", & white);
		gdk_colormap_alloc_color(colormap, &white,TRUE,TRUE);
		gc = gdk_gc_new (gauge->bitmap);
		gdk_gc_set_foreground (gc, &black);
		gdk_draw_rectangle (gauge->bitmap,
				gc,
				TRUE,  // filled
				0,     // x
				0,     // y
				gauge->w,
				gauge->h);

		gdk_gc_set_foreground (gc, & white);
		/* Drag border boxes... */

		if (gauge->show_drag_border)
		{
			gdk_draw_rectangle (gauge->bitmap,
					gc,
					TRUE,  // filled
					0,     // x
					0,     // y
					DRAG_BORDER,
					DRAG_BORDER);
			gdk_draw_rectangle (gauge->bitmap,
					gc,
					TRUE,  // filled
					gauge->w-DRAG_BORDER,     // x
					0,     // y
					DRAG_BORDER,
					DRAG_BORDER);
			gdk_draw_rectangle (gauge->bitmap,
					gc,
					TRUE,  // filled
					gauge->w-DRAG_BORDER,     // x
					gauge->h-DRAG_BORDER,     // y
					DRAG_BORDER,
					DRAG_BORDER);
			gdk_draw_rectangle (gauge->bitmap,
					gc,
					TRUE,  // filled
					0,     // x
					gauge->h-DRAG_BORDER,     // y
					DRAG_BORDER,
					DRAG_BORDER);
		}
		gdk_draw_arc (gauge->bitmap,
				gc,
				TRUE,     // filled
				gauge->xc-gauge->radius,
				gauge->yc-gauge->radius,
				2*(gauge->radius),
				2*(gauge->radius),
				0,        // angle 1
				360*64);  // angle 2: full circle

	}
	generate_gauge_background(widget);
	update_gauge_position(widget);

	return TRUE;
}


/*!
 \brief handles exposure events when the screen is covered and then 
 exposed. Works by copying from a backing pixmap to screen,
 \param widget (GtkWidget *) pointer to the gauge object
 \param event (GdkEventExpose *) pointer to GDK event datastructure that
 encodes important info like window dimensions and depth.
 */
gboolean mtx_gauge_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	/* Win32 doesn't like this... */
//#ifndef __WIN32__
	if (GTK_IS_WINDOW(widget->parent))
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gtk_widget_input_shape_combine_mask(widget->parent,gauge->bitmap,0,0);
#endif
		gtk_widget_shape_combine_mask(widget->parent,gauge->bitmap,0,0);
	}
	else
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gdk_window_input_shape_combine_mask(widget->window,gauge->bitmap,0,0);
#endif
		gdk_window_shape_combine_mask(widget->window,gauge->bitmap,0,0);
	}
	/*
#else // win32 specific way 

#if GTK_MINOR_VERSION >= 10
	if (gtk_minor_version >= 10)
		gtk_widget_input_shape_combine_mask(widget->parent,gauge->bitmap,0,0);
#endif
	gtk_widget_shape_combine_mask(widget->parent,gauge->bitmap,0,0);
#if GTK_MINOR_VERSION >= 10
	if (gtk_minor_version >= 10)
		gdk_window_input_shape_combine_mask(widget->window,gauge->bitmap,0,0);
#endif
	gdk_window_shape_combine_mask(widget->window,gauge->bitmap,0,0);
#endif
*/


	return FALSE;
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void cairo_generate_gauge_background(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_t *cr = NULL;
	gfloat arc = 0.0;
	gfloat radians_per_major_tick = 0.0;
	gfloat radians_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gint count = 0;
	gfloat counter = 0;
	gfloat rad = 0.0;
	gfloat subcounter = 0;
	gchar ** vector = NULL;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat lwidth = 0.0;
	gfloat angle1, angle2;
	cairo_pattern_t *gradient = NULL;
	cairo_text_extents_t extents;
	MtxColorRange *range = NULL;
	MtxTextBlock *tblock = NULL;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	w = widget->allocation.width;
	h = widget->allocation.height;

	/* get a cairo_t */
	cr = gdk_cairo_create (gauge->bg_pixmap);
	/* Background set to black */
	if (gauge->show_drag_border)
	{
		cairo_rectangle (cr,
				0,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				gauge->w-DRAG_BORDER,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				0,gauge->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				gauge->w-DRAG_BORDER,gauge->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
	}
	cairo_arc(cr, gauge->xc, gauge->yc, gauge->radius, 0, 2 * M_PI);
	cairo_set_source_rgb (cr, 0,0,0);

	cairo_fill(cr);
	if (gauge->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);


	/* Filled Arcs */

	/* Outside gradient ring */
	gradient = cairo_pattern_create_linear(gauge->xc+(0.707*gauge->xc),
		 	gauge->yc-(0.707*gauge->yc),
			gauge->xc-(0.707*gauge->xc),
			gauge->yc+(0.707*gauge->yc));
	cairo_pattern_add_color_stop_rgb(gradient, 0, 
			gauge->colors[COL_GRADIENT_BEGIN].red/65535.0, 
			gauge->colors[COL_GRADIENT_BEGIN].green/65535.0, 
			gauge->colors[COL_GRADIENT_BEGIN].blue/65535.0);
	cairo_pattern_add_color_stop_rgb(gradient, 2*gauge->radius, 
			gauge->colors[COL_GRADIENT_END].red/65535.0, 
			gauge->colors[COL_GRADIENT_END].green/65535.0, 
			gauge->colors[COL_GRADIENT_END].blue/65535.0);
	cairo_set_source(cr, gradient);
	cairo_arc(cr, gauge->xc, gauge->yc, gauge->radius, 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);

	/* Inside gradient ring */
	gradient = cairo_pattern_create_linear(gauge->xc-(0.707*gauge->xc),
		 	gauge->yc+(0.707*gauge->yc),
			gauge->xc+(0.707*gauge->xc),
			gauge->yc-(0.707*gauge->yc));
	cairo_pattern_add_color_stop_rgb(gradient, 0, 
			gauge->colors[COL_GRADIENT_BEGIN].red/65535.0, 
			gauge->colors[COL_GRADIENT_BEGIN].green/65535.0, 
			gauge->colors[COL_GRADIENT_BEGIN].blue/65535.0);
	cairo_pattern_add_color_stop_rgb(gradient, 2*gauge->radius, 
			gauge->colors[COL_GRADIENT_END].red/65535.0, 
			gauge->colors[COL_GRADIENT_END].green/65535.0, 
			gauge->colors[COL_GRADIENT_END].blue/65535.0);
	cairo_set_source(cr, gradient);
	cairo_arc(cr, gauge->xc, gauge->yc, (0.950 * gauge->radius), 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);

	/* Gauge background inside the bezel */
	cairo_set_source_rgb (cr, gauge->colors[COL_BG].red/65535.0,
			gauge->colors[COL_BG].green/65535.0,
			gauge->colors[COL_BG].blue/65535.0);
	cairo_arc(cr, gauge->xc, gauge->yc, (0.900 * gauge->radius), 0, 2 * M_PI);
	cairo_fill(cr);

	/* The warning color ranges */
	arc = (gauge->stop_radian - gauge->start_radian) / (2 * M_PI);
	for (i=0;i<gauge->c_ranges->len;i++)
	{
		range = g_array_index(gauge->c_ranges,MtxColorRange *, i);
		cairo_set_source_rgb(cr,range->color.red/65535.0,
				range->color.green/65535.0,
				range->color.blue/65535.0);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		angle1 = (range->lowpoint-gauge->lbound)/(gauge->ubound-gauge->lbound);
		angle2= (range->highpoint-gauge->lbound)/(gauge->ubound-gauge->lbound);
		//printf("gauge color range should be from %f, to %f of full scale\n",angle1, angle2);
		lwidth = gauge->radius*range->lwidth < 1 ? 1: gauge->radius*range->lwidth;
		cairo_set_line_width (cr, lwidth);
		cairo_arc(cr, gauge->xc, gauge->yc, (range->inset * gauge->radius),gauge->start_radian+(angle1*(gauge->stop_radian-gauge->start_radian)), gauge->start_radian+(angle2*(gauge->stop_radian-gauge->start_radian)));

		cairo_stroke(cr);

	}

	/* gauge ticks */
	cairo_set_source_rgb (cr, gauge->colors[COL_MAJ_TICK].red/65535.0,
			gauge->colors[COL_MAJ_TICK].green/65535.0,
			gauge->colors[COL_MAJ_TICK].blue/65535.0);
	radians_per_major_tick = (gauge->stop_radian - gauge->start_radian)/(float)(gauge->major_ticks-1);
	radians_per_minor_tick = radians_per_major_tick/(float)(1+gauge->minor_ticks);
	/* Major ticks first */
	insetfrom = gauge->radius * gauge->tick_inset;

	counter = gauge->start_radian;
	count = 0;
	i = 0;
	if (gauge->txt_str[MAJ_TICK])
	{
		vector = g_strsplit(gauge->txt_str[MAJ_TICK],",",-1);
		while (vector[i])
			i++;
		count=i;
		cairo_select_font_face (cr, gauge->font_str[MAJ_TICK], CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, (gauge->radius * gauge->font_scale[MAJ_TICK]));
	}

	for (i=0;i<gauge->major_ticks;i++)
	{
		inset = (gint) (gauge->major_tick_len * gauge->radius);

		lwidth = (gauge->radius/10)*gauge->major_tick_width < 1 ? 1: (gauge->radius/10)*gauge->major_tick_width;
		cairo_set_line_width (cr, lwidth);
		cairo_move_to (cr,
				gauge->xc + (gauge->radius - insetfrom) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom) * sin (counter));
		cairo_line_to (cr,
				gauge->xc + (gauge->radius - insetfrom - inset) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom - inset) * sin (counter));
		cairo_stroke (cr);
		if ((vector) && (i < count) && (vector[i])) /* If not null */
		{
			cairo_save(cr);
			cairo_set_source_rgb (cr, 
				gauge->colors[COL_MAJ_TICK_TEXT_FONT].red/65535.0,
				gauge->colors[COL_MAJ_TICK_TEXT_FONT].green/65535.0,
				gauge->colors[COL_MAJ_TICK_TEXT_FONT].blue/65535.0);
			cairo_text_extents (cr, vector[i], &extents);
			/* Gets the radius of a circle that encompasses the 
			 * rectangle of text on screen */
			rad = sqrt(pow(extents.width,2)+pow(extents.height,2))/2.0;
			cairo_move_to (cr,
					gauge->xc + (gauge->radius - insetfrom - inset - gauge->major_tick_text_inset*gauge->radius - rad) * cos (counter) - extents.width/2.0,
					gauge->yc + (gauge->radius - insetfrom - inset -  gauge->major_tick_text_inset*gauge->radius - rad) * sin (counter) + extents.height/2.0);
			cairo_show_text (cr, vector[i]);
			cairo_restore(cr);
		}
		/* minor ticks */
		if ((gauge->minor_ticks > 0) && (i < (gauge->major_ticks-1)))
		{
			cairo_save (cr); /* stack-pen-size */
			cairo_set_source_rgb (cr, 
					gauge->colors[COL_MIN_TICK].red/65535.0,
					gauge->colors[COL_MIN_TICK].green/65535.0,
					gauge->colors[COL_MIN_TICK].blue/65535.0);
			inset = (gint) (gauge->minor_tick_len * gauge->radius);
			lwidth = (gauge->radius/10)*gauge->minor_tick_width < 1 ? 1: (gauge->radius/10)*gauge->minor_tick_width;
			cairo_set_line_width (cr, lwidth);
			for (j=1;j<=gauge->minor_ticks;j++)
			{
				subcounter = j*radians_per_minor_tick;
				cairo_move_to (cr,
						gauge->xc + (gauge->radius - insetfrom) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom) * sin (counter+subcounter));
				cairo_line_to (cr,
						gauge->xc + (gauge->radius - insetfrom - inset) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom - inset) * sin (counter+subcounter));
				cairo_stroke (cr);
			}
			cairo_restore (cr); /* stack-pen-size */
		}
		counter += radians_per_major_tick;
	}
	g_strfreev(vector);

	/* Render all the text blocks */
	for (i=0;i<gauge->t_blocks->len;i++)
	{
		tblock = g_array_index(gauge->t_blocks,MtxTextBlock *, i);
		cairo_set_source_rgb (cr, 
				tblock->color.red/65535.0,
				tblock->color.green/65535.0,
				tblock->color.blue/65535.0);

		cairo_select_font_face (cr, tblock->font, CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (cr, (gauge->radius * tblock->font_scale));
		cairo_text_extents (cr, tblock->text, &extents);
		cairo_move_to (cr, 
				gauge->xc-(extents.width/2 + extents.x_bearing)+(tblock->x_pos*gauge->radius),
			       	gauge->yc-(extents.height/2 + extents.y_bearing)+(tblock->y_pos*gauge->radius));
		cairo_show_text (cr, tblock->text);
	}

	cairo_destroy (cr);
#endif
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the gdk version.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void gdk_generate_gauge_background(GtkWidget *widget)
{
	gfloat radians_per_major_tick = 0.0;
	gfloat radians_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gint count = 0;
	gfloat rad = 0.0;
	gchar **vector = NULL;
	gint lwidth = 0;
	gfloat arc = 0.0;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat tmpf = 0.0;
	gfloat counter = 0.0;
	gfloat subcounter = 0.0;
	gchar * tmpbuf = NULL;
	gint redstep = 0;
	gint greenstep = 0;
	gint bluestep = 0;
	gfloat angle1 = 0.0;
	gfloat angle2 = 0.0;
	gfloat start_pos = 0.0;
	gfloat stop_pos = 0.0;
	gfloat start_angle = 0.0;
	gfloat span = 0.0;
	gint r_sign = 0;
	gint g_sign = 0;
	gint b_sign = 0;
	MtxColorRange *range = NULL;
	MtxTextBlock *tblock = NULL;
	GdkColor color;
	GdkColor *b_color;
	GdkColor *e_color;
	PangoRectangle logical_rect;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;
	w = widget->allocation.width;
	h = widget->allocation.height;


	/* Wipe the display, black */
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			DRAG_BORDER,
			DRAG_BORDER);
	/* Wipe the display, black */
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, gauge->w-DRAG_BORDER,0,
			DRAG_BORDER,
			DRAG_BORDER);
	/* Wipe the display, black */
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,gauge->h-DRAG_BORDER,
			DRAG_BORDER,
			DRAG_BORDER);
	/* Wipe the display, black */
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, gauge->w-DRAG_BORDER,gauge->h-DRAG_BORDER,
			DRAG_BORDER,
			DRAG_BORDER);
	gdk_draw_arc(gauge->bg_pixmap,widget->style->black_gc,TRUE,
			gauge->xc-gauge->radius,
			gauge->yc-gauge->radius,
			2*(gauge->radius),
			2*(gauge->radius),
			0,360*64);


	/* The main grey (will have a thin overlay on top of it) 
	 * This is a FILLED circle */

	/* Gradients */

	lwidth = MIN (gauge->xc,gauge->yc)/20 < 1 ? 1: MIN (gauge->xc,gauge->yc)/20;
	gdk_gc_set_line_attributes(gauge->gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);

	tmpf = (gfloat)lwidth/(gfloat)(2*gauge->radius);
	tmpf = (1.0-(tmpf));

	/* This is a HORRENDOUSLY UGLY hack to get pretty gradients
	 * like cairo does. (cairo does the gradient in something like
	 * 3 calls instead of all the follow ugliness, but they render 
	 * nearly identical which is what counts....
	 */
	b_color = &gauge->colors[COL_GRADIENT_BEGIN];
	e_color = &gauge->colors[COL_GRADIENT_END];

	redstep = abs(b_color->red-e_color->red)/36;
	greenstep = abs(b_color->green-e_color->green)/36;
	bluestep = abs(b_color->blue-e_color->blue)/36;

	/* Outer Gradient */
	if (b_color->red > e_color->red)
		r_sign = -1;
	else
		r_sign = 1;
	if (b_color->green > e_color->green)
		g_sign = -1;
	else
		g_sign = 1;
	if (b_color->blue > e_color->blue)
		b_sign = -1;
	else
		b_sign = 1;
	for(i=0;i<36;i++)
	{
		color.red=b_color->red + (i * r_sign * redstep);
		color.green=b_color->green + (i * g_sign * greenstep);
		color.blue=b_color->blue + (i * b_sign * bluestep);
		gdk_gc_set_rgb_fg_color(gauge->gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
				gauge->xc-gauge->radius*tmpf,
				gauge->yc-gauge->radius*tmpf,
				2*(gauge->radius*tmpf),
				2*(gauge->radius*tmpf),
				(45+(i*5))*64,5*64);
	}
	if (b_color->red > e_color->red)
		r_sign = -1;
	else
		r_sign = 1;
	if (b_color->green > e_color->green)
		g_sign = -1;
	else
		g_sign = 1;
	if (b_color->blue > e_color->blue)
		b_sign = -1;
	else
		b_sign = 1;
	for(i=0;i<36;i++)
	{
		color.red=e_color->red - (i * r_sign * redstep);
		color.green=e_color->green - (i * g_sign * greenstep);
		color.blue=e_color->blue - (i * b_sign * bluestep);
		gdk_gc_set_rgb_fg_color(gauge->gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
				gauge->xc-gauge->radius*tmpf,
				gauge->yc-gauge->radius*tmpf,
				2*(gauge->radius*tmpf),
				2*(gauge->radius*tmpf),
				(225+(i*5))*64,5*64);
	}
	/* Inner Gradient */
	if (b_color->red > e_color->red)
		r_sign = -1;
	else
		r_sign = 1;
	if (b_color->green > e_color->green)
		g_sign = -1;
	else
		g_sign = 1;
	if (b_color->blue > e_color->blue)
		b_sign = -1;
	else
		b_sign = 1;
	tmpf = (gfloat)lwidth/(gfloat)(2*gauge->radius);
	tmpf = (1.0-(3*tmpf));
	for(i=0;i<36;i++)
	{
		color.red=b_color->red + (i * r_sign * redstep);
		color.green=b_color->green + (i * g_sign * greenstep);
		color.blue=b_color->blue + (i * b_sign * bluestep);
		gdk_gc_set_rgb_fg_color(gauge->gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
				gauge->xc-gauge->radius*tmpf,
				gauge->yc-gauge->radius*tmpf,
				2*(gauge->radius*tmpf),
				2*(gauge->radius*tmpf),
				(225+(i*5))*64,5*64);
	}
	if (b_color->red > e_color->red)
		r_sign = -1;
	else
		r_sign = 1;
	if (b_color->green > e_color->green)
		g_sign = -1;
	else
		g_sign = 1;
	if (b_color->blue > e_color->blue)
		b_sign = -1;
	else
		b_sign = 1;
	for(i=0;i<36;i++)
	{
		color.red=e_color->red - (i * r_sign * redstep);
		color.green=e_color->green - (i * g_sign * greenstep);
		color.blue=e_color->blue - (i * b_sign * bluestep);
		gdk_gc_set_rgb_fg_color(gauge->gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE,
				gauge->xc-gauge->radius*tmpf,
				gauge->yc-gauge->radius*tmpf,
				2*(gauge->radius*tmpf),
				2*(gauge->radius*tmpf),
				(45+(i*5))*64,5*64);
	}

	/* Create the INNER filled black arc to draw the ticks and everything
	 * else onto
	 */
	tmpf = (gfloat)lwidth/(gfloat)(2*gauge->radius);
	tmpf = (1.0-(4*tmpf));
	gdk_gc_set_line_attributes(gauge->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);
	gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_BG]);

	gdk_draw_arc(gauge->bg_pixmap,gauge->gc,TRUE,
			gauge->xc-gauge->radius*tmpf,
			gauge->yc-gauge->radius*tmpf,
			2*(gauge->radius*tmpf),
			2*(gauge->radius*tmpf),
			0,360*64);

	/* The warning color ranges */
	arc = (gauge->stop_radian - gauge->start_radian) / (2 * M_PI);
	for (i=0;i<gauge->c_ranges->len;i++)
	{
		range = g_array_index(gauge->c_ranges,MtxColorRange *, i);
		gdk_gc_set_rgb_fg_color(gauge->gc,&range->color);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		span = gauge->stop_radian - gauge->start_radian;
		angle1 = (range->lowpoint-gauge->lbound)/(gauge->span);
		angle2 = (range->highpoint-gauge->lbound)/(gauge->span);

		/* positions of the range in radians */
		start_pos = gauge->start_radian+(angle1*span);
		stop_pos = gauge->start_radian+(angle2*span);
		/* Converted to funky GDK units */
		start_angle = -start_pos*(180/M_PI)*64;
		span = - (stop_pos-start_pos)*(180/M_PI)*64;

		lwidth = gauge->radius*range->lwidth < 1 ? 1: gauge->radius*range->lwidth;
		gdk_gc_set_line_attributes(gauge->gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);
		gdk_draw_arc(gauge->bg_pixmap,gauge->gc,FALSE, 
				gauge->xc-gauge->radius*range->inset, 
				gauge->yc-gauge->radius*range->inset,
				2*(gauge->radius*range->inset),
				2*(gauge->radius*range->inset),
				start_angle,
				span);
	}


	/* gauge ticks */
	radians_per_major_tick = (gauge->stop_radian - gauge->start_radian)/(float)(gauge->major_ticks-1);
	radians_per_minor_tick = radians_per_major_tick/(float)(1+gauge->minor_ticks);
	/* Major ticks first */
	insetfrom = gauge->radius * gauge->tick_inset;
	count = 0;
	i=0;
	if (gauge->txt_str[MAJ_TICK])
	{
		vector = g_strsplit(gauge->txt_str[MAJ_TICK],",",-1);
		while (vector[i])
			i++;
		count=i;
		tmpbuf = g_strdup_printf("%s %i",gauge->font_str[MAJ_TICK],(gint)(gauge->radius*gauge->font_scale[MAJ_TICK]));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
	}


	counter = gauge->start_radian;
	for (i=0;i<gauge->major_ticks;i++)
	{
		inset = (gint) (gauge->major_tick_len * gauge->radius);
		lwidth = (gauge->radius/10)*gauge->major_tick_width < 1 ? 1: (gauge->radius/10)*gauge->major_tick_width;
		gdk_gc_set_line_attributes(gauge->gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);

		gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_MAJ_TICK]);
		gdk_draw_line(gauge->bg_pixmap,gauge->gc,

				gauge->xc + (gauge->radius - insetfrom) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom) * sin (counter),
				gauge->xc + ((gauge->radius - insetfrom - inset) * cos (counter)),
				gauge->yc + ((gauge->radius - insetfrom - inset) * sin (counter)));
		if ((vector) && (i < count) && (vector[i])) /* If not null */
		{
			gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_MAJ_TICK_TEXT_FONT]);
			pango_layout_set_text(gauge->layout,vector[i],-1);
			pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);

			rad = sqrt(pow(logical_rect.width,2)+pow(logical_rect.height,2))/2.0;
			/* Fudge factor due to differenced ins pango vs
			 * cairo font extents/rectangles
			 */
			rad*=0.55;

			gdk_draw_layout(gauge->bg_pixmap,gauge->gc,
					gauge->xc + (gauge->radius - insetfrom - inset - gauge->major_tick_text_inset*gauge->radius - rad) * cos (counter) - (logical_rect.width/2),
					gauge->yc + (gauge->radius - insetfrom - inset -  gauge->major_tick_text_inset*gauge->radius - rad) * sin (counter) - (logical_rect.height/2),gauge->layout);
		}

		/* Now the minor ticks... */
		if ((gauge->minor_ticks > 0) && (i < (gauge->major_ticks-1)))
		{
			gdk_gc_set_rgb_fg_color(gauge->gc,&gauge->colors[COL_MIN_TICK]);
			inset = (gint) (gauge->minor_tick_len * gauge->radius);
			lwidth = (gauge->radius/10)*gauge->minor_tick_width < 1 ? 1: (gauge->radius/10)*gauge->minor_tick_width;
			gdk_gc_set_line_attributes(gauge->gc,lwidth,
					GDK_LINE_SOLID,
					GDK_CAP_BUTT,
					GDK_JOIN_BEVEL);
			for (j=1;j<=gauge->minor_ticks;j++)
			{
				subcounter = j*radians_per_minor_tick;
				gdk_draw_line(gauge->bg_pixmap,gauge->gc,

						gauge->xc + (gauge->radius - insetfrom) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom) * sin (counter+subcounter),
						gauge->xc + ((gauge->radius - insetfrom - inset) * cos (counter+subcounter)),
						gauge->yc + ((gauge->radius - insetfrom - inset) * sin (counter+subcounter)));

			}

		}
		counter += radians_per_major_tick;

	}

	/* text Blocks */
	for (i=0;i<gauge->t_blocks->len;i++)
	{
		tblock = g_array_index(gauge->t_blocks,MtxTextBlock *, i);
		gdk_gc_set_rgb_fg_color(gauge->gc,&tblock->color);
		tmpbuf = g_strdup_printf("%s %i",tblock->font,(gint)(gauge->radius*tblock->font_scale));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
		pango_layout_set_text(gauge->layout,tblock->text,-1);
		pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);

		gdk_draw_layout(gauge->bg_pixmap,gauge->gc,
				gauge->xc-(logical_rect.width/2)+(tblock->x_pos*gauge->radius),
				gauge->yc-(logical_rect.height/2)+(tblock->y_pos*gauge->radius),gauge->layout);
	}

}


/*!
 \brief handler called when a button is pressed,  currently unused 
 \param gauge (GtkWidget *) pointer to the gauge widget
 \param event (GdkEventButton *) struct containing details on the button(s) 
 pressed
 \returns FALSE
 */
gboolean mtx_gauge_face_button_press (GtkWidget *widget,GdkEventButton *event)
					     
{
	MtxGaugeFace *gauge = MTX_GAUGE_FACE(widget);
	GdkWindowEdge edge = -1;
//	printf("gauge button event\n");
	/* Right side of window */
	if (event->x > (0.55*gauge->w))
	{
		/* Upper portion */
		if (event->y < (0.45*gauge->h))
			edge = GDK_WINDOW_EDGE_NORTH_EAST;
		/* Lower portion */
		else if (event->y > (0.55*gauge->h))
			edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		else 
			edge = -1;
	}
	/* Left Side of window */
	else if (event->x < (0.45*gauge->w))
	{
		/* If it's in the middle portion */
		/* Upper portion */
		if (event->y < (0.45*gauge->h))
			edge = GDK_WINDOW_EDGE_NORTH_WEST;
		/* Lower portion */
		else if (event->y > (0.55*gauge->h))
			edge = GDK_WINDOW_EDGE_SOUTH_WEST;
		else 
			edge = -1;
	}
	else
		edge = -1;


	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button)
		{
			case 1: /* left button */
				if (GTK_IS_WINDOW(widget->parent))
				{
					gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
							event->button,
							event->x_root,
							event->y_root,
							event->time);
				}
				break;
			case 2: /* Middle button,  RESIZE */
				if (edge != -1)
				{
					if (GTK_IS_WINDOW(widget->parent))
					{

					gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
							edge,
							event->button,
							event->x_root,
							event->y_root,
							event->time);
					}
				}
				break;
			case 3: /* right button */
				if (GTK_IS_WINDOW(widget->parent))
					gtk_main_quit();
				break;
		}
	}
//	printf("gauge button event ENDING\n");
	return FALSE;
}


/*!
 \brief gets called to redraw the entire display manually
 \param gauge (MtxGaugeFace *) pointer to the gauge object
 */
void mtx_gauge_face_redraw_canvas (MtxGaugeFace *gauge)
{
	GtkWidget *widget;

	widget = GTK_WIDGET (gauge);

	if (!widget->window) return;

	update_gauge_position(widget);
	gdk_window_clear(widget->window);
}


/*!
 \brief handler called when a button is released,  currently unused 
 \param gauge (GtkWidget *) pointer to the gauge widget
 \param event (GdkEventButton *) struct containing details on the button(s) 
 released
 \returns FALSE
 */
gboolean mtx_gauge_face_button_release (GtkWidget *gauge,GdkEventButton *event)
					       
{
	/*
	MtxGaugeFacePrivate *priv;
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);
	*/
//	printf("button release\n");
	return FALSE;
}


gboolean mtx_gauge_face_motion_event (GtkWidget *gauge,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */
//	printf("motion in gauge, returning false\n");
	return FALSE;
}
					       


/*!
 \brief sets the INITIAL sizeof the widget
 \param gauge (GtkWidget *) pointer to the gauge widget
 \param requisition (GdkRequisition *) struct to set the vars within
 \returns void
 */
void mtx_gauge_face_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width = 80;
	requisition->height = 80;
}
