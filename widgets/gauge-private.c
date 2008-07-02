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


G_DEFINE_TYPE (MtxGaugeFace, mtx_gauge_face, GTK_TYPE_DRAWING_AREA)


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
	/*widget_class->motion_notify_event = mtx_gauge_face_motion_event;*/
	widget_class->size_request = mtx_gauge_face_size_request;

	g_type_class_add_private (obj_class, sizeof (MtxGaugeFacePrivate));
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
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK |GDK_POINTER_MOTION_MASK);

	priv->w = 0;
	priv->h = 0;
	priv->xc = 0.0;
	priv->yc = 0.0;
	priv->radius = 0.0;
	priv->value = 0.0;		/* default values */
	priv->lbound = 0.0;
	priv->ubound = 100.0;
	priv->precision = 2;
	priv->clamped = CLAMP_NONE;
	priv->start_angle = 135; 	/* lower left quadrant */
	priv->sweep_angle = 270; 	/* CW sweep */
	priv->needle_width = 0.05;  	/* % of radius */
	priv->needle_tip_width = 0.0;
	priv->needle_tail_width = 0.0;
	priv->needle_tail = 0.083;  	/* % of radius */
	priv->needle_length = 0.850; 	/* % of radius */
	priv->value_font = g_strdup("Bitstream Vera Sans");
	priv->value_xpos = 0.0;
	priv->value_ypos = 0.40;
	priv->value_font_scale = 0.2;
	priv->span = priv->ubound - priv->lbound;
#ifdef HAVE_CAIRO
	priv->cr = NULL;
	priv->antialias = TRUE;
#else
	priv->antialias = FALSE;
#endif
	priv->show_value = TRUE;
	priv->colormap = gdk_colormap_get_system();
	priv->gc = NULL;
	priv->a_ranges = g_array_new(FALSE,TRUE,sizeof(MtxAlertRange *));
	priv->c_ranges = g_array_new(FALSE,TRUE,sizeof(MtxColorRange *));
	priv->t_blocks = g_array_new(FALSE,TRUE,sizeof(MtxTextBlock *));
	priv->tick_groups = g_array_new(FALSE,TRUE,sizeof(MtxTickGroup *));
	priv->polygons = g_array_new(FALSE,TRUE,sizeof(MtxPolygon *));
	mtx_gauge_face_init_default_tick_group(gauge);
	mtx_gauge_face_init_colors(gauge);
	mtx_gauge_face_init_name_bindings(gauge);
	mtx_gauge_face_init_xml_hash(gauge);
	mtx_gauge_face_redraw_canvas (gauge);
}


void mtx_gauge_face_init_name_bindings(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	g_object_set_data(G_OBJECT(gauge),"bg_color", &priv->colors[COL_BG]);
	g_object_set_data(G_OBJECT(gauge),"needle_color", &priv->colors[COL_NEEDLE]);
	g_object_set_data(G_OBJECT(gauge),"value_font_color", &priv->colors[COL_VALUE_FONT]);
	g_object_set_data(G_OBJECT(gauge),"gradient_begin_color", &priv->colors[COL_GRADIENT_BEGIN]);
	g_object_set_data(G_OBJECT(gauge),"gradient_end_color", &priv->colors[COL_GRADIENT_END]);
	g_object_set_data(G_OBJECT(gauge),"needle_length", &priv->needle_length);
	g_object_set_data(G_OBJECT(gauge),"needle_tip_width", &priv->needle_tip_width);
	g_object_set_data(G_OBJECT(gauge),"needle_tail_width", &priv->needle_tail_width);
	g_object_set_data(G_OBJECT(gauge),"needle_width", &priv->needle_width);
	g_object_set_data(G_OBJECT(gauge),"needle_tail", &priv->needle_tail);
	g_object_set_data(G_OBJECT(gauge),"precision", &priv->precision);
	g_object_set_data(G_OBJECT(gauge),"width", &priv->w);
	g_object_set_data(G_OBJECT(gauge),"height", &priv->h);
	g_object_set_data(G_OBJECT(gauge),"main_start_angle", &priv->start_angle);
	g_object_set_data(G_OBJECT(gauge),"main_sweep_angle", &priv->sweep_angle);
	g_object_set_data(G_OBJECT(gauge),"lbound", &priv->lbound);
	g_object_set_data(G_OBJECT(gauge),"ubound", &priv->ubound);
	g_object_set_data(G_OBJECT(gauge),"value_font", &priv->value_font);
	g_object_set_data(G_OBJECT(gauge),"value_font_scale", &priv->value_font_scale);
	g_object_set_data(G_OBJECT(gauge),"value_str_xpos", &priv->value_xpos);
	g_object_set_data(G_OBJECT(gauge),"value_str_ypos", &priv->value_ypos);
	g_object_set_data(G_OBJECT(gauge),"antialias", &priv->antialias);
	g_object_set_data(G_OBJECT(gauge),"show_value", &priv->show_value);
}

/*!
 * \brief  initializes and populates the xml_functions hashtable 
 */
void mtx_gauge_face_init_xml_hash(MtxGaugeFace *gauge)
{
	gint i = 0;
	MtxXMLFuncs * funcs = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	gint num_xml_funcs = sizeof(xml_functions) / sizeof(xml_functions[0]);
	priv->xmlfunc_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);

	priv->xmlfunc_array = g_array_sized_new(FALSE,TRUE,sizeof (MtxXMLFuncs *),num_xml_funcs);

	for (i=0;i<num_xml_funcs;i++)
	{
		funcs = g_new0(MtxXMLFuncs, 1);
		funcs->import_func = xml_functions[i].import_func;
		funcs->export_func = xml_functions[i].export_func;;
		funcs->varname = xml_functions[i].varname;
		funcs->dest_var = (gpointer)g_object_get_data(G_OBJECT(gauge),xml_functions[i].varname);
		g_hash_table_insert (priv->xmlfunc_hash,g_strdup(xml_functions[i].varname),funcs);
		g_array_append_val(priv->xmlfunc_array,funcs);
	}

}

/*!
 \brief Allocates the default colors for a gauge with no options 
 \param widget (GtkWidget *) pointer to the gauge object
 */
void mtx_gauge_face_init_colors(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	/* Defaults for the gauges,  user over-ridable */

	/*! Background */
	priv->colors[COL_BG].red=0*65535;
	priv->colors[COL_BG].green=0*65535;
	priv->colors[COL_BG].blue=0*65535;
	/*! Needle */
	priv->colors[COL_NEEDLE].red=1.0*65535;
	priv->colors[COL_NEEDLE].green=1.0*65535;
	priv->colors[COL_NEEDLE].blue=1.0*65535;
	/*! Units Font*/
	priv->colors[COL_VALUE_FONT].red=0.8*65535;
	priv->colors[COL_VALUE_FONT].green=0.8*65535;
	priv->colors[COL_VALUE_FONT].blue=0.8*65535;
	/*! Gradient Color Begin */
	priv->colors[COL_GRADIENT_BEGIN].red=0.85*65535;
	priv->colors[COL_GRADIENT_BEGIN].green=0.85*65535;
	priv->colors[COL_GRADIENT_BEGIN].blue=0.85*65535;
	/*! Gradient Color End */
	priv->colors[COL_GRADIENT_END].red=0.15*65535;
	priv->colors[COL_GRADIENT_END].green=0.15*65535;
	priv->colors[COL_GRADIENT_END].blue=0.15*65535;

}


/*!
 \brief creates a default tick group (replacing old tick system)
 \param widget (GtkWidget *) pointer to the gauge object
 */
void mtx_gauge_face_init_default_tick_group(MtxGaugeFace *gauge)
{
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);
	MtxTickGroup *tgroup = NULL;
	GdkColor white = { 0, 65535, 65535, 65535};

	tgroup = g_new0(MtxTickGroup, 1);
	tgroup->num_maj_ticks = 9;
	tgroup->num_min_ticks = 4;
	tgroup->start_angle = priv->start_angle;
	tgroup->sweep_angle = priv->sweep_angle;
	tgroup->maj_tick_inset = 0.15;
	tgroup->maj_tick_width = 0.175;
	tgroup->maj_tick_length = 0.110;
	tgroup->min_tick_inset = 0.175;
	tgroup->min_tick_length = 0.05;
	tgroup->min_tick_width = 0.10;
	tgroup->maj_tick_color = white;
	tgroup->min_tick_color = white;
	tgroup->font = g_strdup("Arial");
	tgroup->font_scale = 0.135;
	tgroup->text_inset = 0.255;
	tgroup->text = g_strdup("");
	tgroup->text_color = white;
	g_array_append_val(priv->tick_groups,tgroup);

}
/*!
 \brief updates the gauge position,  This is the CAIRO implementation that
 looks a bit nicer, though is a little bit slower
 \param widget (MtxGaugeFace *) pointer to the gauge object
 */
void cairo_update_gauge_position (MtxGaugeFace *gauge)
{
#ifdef HAVE_CAIRO
	GtkWidget *widget = NULL;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	gint i = 0;
	gfloat n_width = 0.0;
	gfloat n_tail = 0.0;
	gfloat n_tip = 0.0;
	gfloat tip_width = 0.0;
	gfloat tail_width = 0.0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gfloat lwidth = 0.0;
	gfloat val = 0.0;
	gboolean alert = FALSE;
	MtxAlertRange *range = NULL;
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	widget = GTK_WIDGET(gauge);

	/* Check if in alert bounds and alert as necessary */
	alert = FALSE;
	for (i=0;i<priv->a_ranges->len;i++)
	{
		range = g_array_index(priv->a_ranges,MtxAlertRange *, i);
		if ((priv->value >= range->lowpoint)  &&
				(priv->value <= range->highpoint))
		{
			alert = TRUE;
			if (priv->last_alert_index == i)
				goto cairo_jump_out_of_alerts;

			/* If we alert, in order to save CPU, we copy the 
			 * background pixmap to a temp pixmap and render on 
			 * that and STORE the index of this alert.  Next time
			 * acount we'll detect we ALREADY drew the alert and 
			 * just copy hte pixmap (saving all the render time)
			 * as pixmap copies are fast.
			 */
			priv->last_alert_index = i;
			widget = GTK_WIDGET(gauge);
			gdk_draw_drawable(priv->tmp_pixmap,
					widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					priv->bg_pixmap,
					0,0,
					0,0,
					widget->allocation.width,widget->allocation.height);
			cr = gdk_cairo_create (priv->tmp_pixmap);
			cairo_set_source_rgb(cr,range->color.red/65535.0,
					range->color.green/65535.0,
					range->color.blue/65535.0);
			lwidth = priv->radius*range->lwidth < 1 ? 1: priv->radius*range->lwidth;
			cairo_set_line_width (cr, lwidth);
			cairo_arc(cr, priv->xc, priv->yc, (range->inset * priv->radius),0, 2*M_PI);
			cairo_stroke(cr);
			cairo_destroy(cr);
			break;
		}
	}
cairo_jump_out_of_alerts:
	/* Copy background pixmap to intermediary for final rendering */
	if (!alert)
		gdk_draw_drawable(priv->pixmap,
				widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				priv->bg_pixmap,
				0,0,
				0,0,
				widget->allocation.width,widget->allocation.height);
	else
		gdk_draw_drawable(priv->pixmap,
				widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				priv->tmp_pixmap,
				0,0,
				0,0,
				widget->allocation.width,widget->allocation.height);


	cr = gdk_cairo_create (priv->pixmap);
	cairo_set_font_options(cr,priv->font_options);

	if (priv->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
	/* Update the VALUE text */
	if (priv->show_value)
	{
		cairo_set_source_rgb (cr, priv->colors[COL_VALUE_FONT].red/65535.0,
				priv->colors[COL_VALUE_FONT].green/65535.0,
				priv->colors[COL_VALUE_FONT].blue/65535.0);
		tmpbuf = g_utf8_strup(priv->value_font,-1);
		if (g_strrstr(tmpbuf,"BOLD"))
			weight = CAIRO_FONT_WEIGHT_BOLD;
		else
			weight = CAIRO_FONT_WEIGHT_NORMAL;
		if (g_strrstr(tmpbuf,"OBLIQUE"))
			slant = CAIRO_FONT_SLANT_OBLIQUE;
		else if (g_strrstr(tmpbuf,"ITALIC"))
			slant = CAIRO_FONT_SLANT_ITALIC;
		else
			slant = CAIRO_FONT_SLANT_NORMAL;
		g_free(tmpbuf);
		cairo_select_font_face (cr, priv->value_font,  slant, weight);

		cairo_set_font_size (cr, (priv->radius * priv->value_font_scale));

		message = g_strdup_printf("%.*f", priv->precision,priv->value);

		cairo_text_extents (cr, message, &extents);

		cairo_move_to (cr, 
				priv->xc-(extents.width/2 + extents.x_bearing)+(priv->value_xpos*priv->radius),
				priv->yc-(extents.height/2 + extents.y_bearing)+(priv->value_ypos*priv->radius));
		cairo_show_text (cr, message);
		g_free(message);

		cairo_stroke (cr);
	}

	/* gauge hands */
	if (priv->clamped == CLAMP_UPPER)
		val = priv->ubound;
	else if (priv->clamped == CLAMP_LOWER)
		val = priv->lbound;
	else
		val = priv->value;
	tmpf = (val-priv->lbound)/(priv->ubound-priv->lbound);
	needle_pos = (priv->start_angle+(tmpf*priv->sweep_angle))*(M_PI/180);


	cairo_set_source_rgb (cr, priv->colors[COL_NEEDLE].red/65535.0,
			priv->colors[COL_NEEDLE].green/65535.0,
			priv->colors[COL_NEEDLE].blue/65535.0);
	cairo_set_line_width (cr, 1);

	n_width = priv->needle_width * priv->radius;
	n_tail = priv->needle_tail * priv->radius;
	n_tip = priv->needle_length * priv->radius;
	tip_width = priv->needle_tip_width * priv->radius;
	tail_width = priv->needle_tail_width * priv->radius;
	xc = priv->xc;
	yc = priv->yc;

	priv->needle_coords[0].x = xc + ((n_tip) * cos (needle_pos))+((tip_width) * -sin(needle_pos));
	priv->needle_coords[0].y = yc + ((n_tip) * sin (needle_pos))+((tip_width) * cos(needle_pos));
	priv->needle_coords[1].x = xc + ((n_tip) * cos (needle_pos))+((tip_width) * sin(needle_pos));
	priv->needle_coords[1].y = yc + ((n_tip) * sin (needle_pos))+((tip_width) * -cos(needle_pos));

	priv->needle_coords[2].x = xc + (n_width) * sin(needle_pos);
	priv->needle_coords[2].y = yc + (n_width) * -cos(needle_pos);

	priv->needle_coords[3].x = xc + ((n_tail) * -cos (needle_pos))+((tail_width) * sin (needle_pos));
	priv->needle_coords[3].y = yc + ((n_tail) * -sin (needle_pos))+((tail_width) * -cos (needle_pos));
	priv->needle_coords[4].x = xc + ((n_tail) * -cos (needle_pos))+((tail_width) * -sin (needle_pos));
	priv->needle_coords[4].y = yc + ((n_tail) * -sin (needle_pos))+((tail_width) * cos (needle_pos));
	priv->needle_coords[5].x = xc + (n_width) * -sin (needle_pos);
	priv->needle_coords[5].y = yc + (n_width) * cos (needle_pos);
	priv->needle_polygon_points = 6;

	cairo_move_to (cr, priv->needle_coords[0].x,priv->needle_coords[0].y);
	cairo_line_to (cr, priv->needle_coords[1].x,priv->needle_coords[1].y);
	cairo_line_to (cr, priv->needle_coords[2].x,priv->needle_coords[2].y);
	cairo_line_to (cr, priv->needle_coords[3].x,priv->needle_coords[3].y);
	cairo_line_to (cr, priv->needle_coords[4].x,priv->needle_coords[4].y);
	cairo_line_to (cr, priv->needle_coords[5].x,priv->needle_coords[5].y);
	cairo_fill_preserve (cr);
	cairo_stroke(cr);



	cairo_destroy(cr);
#endif
}


/*!
 \brief updates the gauge position,  This is the GDK implementation that
 looks doesn't do antialiasing,  but is the fastest one.
 \param widget (MtxGaugeFace *) pointer to the gauge object
 */
void gdk_update_gauge_position (MtxGaugeFace *gauge)
{
#ifndef HAVE_CAIRO
	GtkWidget *widget = NULL;
	gint i= 0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gint n_width = 0;
	gint n_tail = 0;
	gint n_tip = 0;
	gint tip_width = 0;
	gint tail_width = 0;
	gboolean alert = FALSE;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	gint lwidth = 0;
	gfloat val = 0.0;
	MtxAlertRange* range = NULL;
	PangoRectangle logical_rect;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);


	widget = GTK_WIDGET(gauge);
	/* Check if in alert bounds and alert as necessary */
	alert = FALSE;
	for (i=0;i<priv->a_ranges->len;i++)
	{
		range = g_array_index(priv->a_ranges,MtxAlertRange *, i);
		if ((priv->value >= range->lowpoint)  &&
				(priv->value <= range->highpoint))
		{
			alert = TRUE;
			if (priv->last_alert_index == i)
				goto gdk_jump_out_of_alerts;

			/* If we alert, in order to save CPU, we copy the 
			 * background pixmap to a temp pixmap and render on 
			 * that and STORE the index of this alert.  Next time
			 * acount we'll detect we ALREADY drew the alert and 
			 * just copy hte pixmap (saving all the render time)
			 * as pixmap copies are fast.
			 */
			priv->last_alert_index = i;
			gdk_draw_drawable(priv->tmp_pixmap,
				widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				priv->bg_pixmap,
				0,0,
				0,0,
				widget->allocation.width,widget->allocation.height);
			gdk_gc_set_rgb_fg_color(priv->gc,&range->color);
			lwidth = priv->radius*range->lwidth < 1 ? 1: priv->radius*range->lwidth;
			gdk_gc_set_line_attributes(priv->gc,lwidth,
					GDK_LINE_SOLID,
					GDK_CAP_BUTT,
					GDK_JOIN_BEVEL);
			gdk_draw_arc(priv->tmp_pixmap,priv->gc,FALSE,
					priv->xc-priv->radius*range->inset,
					priv->yc-priv->radius*range->inset,
					2*(priv->radius*range->inset),
					2*(priv->radius*range->inset),
					0,
					360*64);
			break;
		}
	}
gdk_jump_out_of_alerts:
	/* Copy background pixmap to intermediary for final rendering */
	if (!alert)
		gdk_draw_drawable(priv->pixmap,
				widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				priv->bg_pixmap,
				0,0,
				0,0,
				widget->allocation.width,widget->allocation.height);
	else
		gdk_draw_drawable(priv->pixmap,
				widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				priv->tmp_pixmap,
				0,0,
				0,0,
				widget->allocation.width,widget->allocation.height);

	/* the text */
	if (priv->show_value)
	{
		gdk_gc_set_rgb_fg_color(priv->gc,&priv->colors[COL_VALUE_FONT]);
		message = g_strdup_printf("%.*f", priv->precision,priv->value);

		tmpbuf = g_strdup_printf("%s %i",priv->value_font,(gint)(priv->radius *priv->value_font_scale*0.82));
		priv->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(priv->layout,priv->font_desc);
		pango_layout_set_text(priv->layout,message,-1);
		pango_layout_get_pixel_extents(priv->layout,NULL,&logical_rect);
		g_free(message);

		gdk_draw_layout(priv->pixmap,priv->gc,
				priv->xc-(logical_rect.width/2)+(priv->value_xpos*priv->radius),
				priv->yc-(logical_rect.height/2)+(priv->value_ypos*priv->radius),priv->layout);
	}

	gdk_gc_set_line_attributes(priv->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_ROUND,
			GDK_JOIN_ROUND);

	/* gauge hands */
	if (priv->clamped == CLAMP_UPPER)
		val = priv->ubound;
	else if (priv->clamped == CLAMP_LOWER)
		val = priv->lbound;
	else
		val = priv->value;
	tmpf = (val-priv->lbound)/(priv->ubound-priv->lbound);
	needle_pos = (priv->start_angle+(tmpf*priv->sweep_angle))*(M_PI/180.0);
	xc= priv->xc;
	yc= priv->yc;
	n_width = priv->needle_width * priv->radius;
	n_tail = priv->needle_tail * priv->radius;
	n_tip = priv->needle_length * priv->radius;
	tip_width = priv->needle_tip_width * priv->radius;
	tail_width = priv->needle_tail_width * priv->radius;

	priv->needle_coords[0].x = xc + ((n_tip) * cos (needle_pos))+((tip_width) * -sin(needle_pos));
	priv->needle_coords[0].y = yc + ((n_tip) * sin (needle_pos))+((tip_width) * cos(needle_pos));
	priv->needle_coords[1].x = xc + ((n_tip) * cos (needle_pos))+((tip_width) * sin(needle_pos));
	priv->needle_coords[1].y = yc + ((n_tip) * sin (needle_pos))+((tip_width) * -cos(needle_pos));
	
	 priv->needle_coords[2].x = xc + (n_width) * sin(needle_pos);
	 priv->needle_coords[2].y = yc + (n_width) * -cos(needle_pos);

	 priv->needle_coords[3].x = xc + ((n_tail) * -cos (needle_pos))+((tail_width) * sin (needle_pos));
	 priv->needle_coords[3].y = yc + ((n_tail) * -sin (needle_pos))+((tail_width) * -cos (needle_pos));
	 priv->needle_coords[4].x = xc + ((n_tail) * -cos (needle_pos))+((tail_width) * -sin (needle_pos));
	 priv->needle_coords[4].y = yc + ((n_tail) * -sin (needle_pos))+((tail_width) * cos (needle_pos));
	 priv->needle_coords[5].x = xc + (n_width) * -sin (needle_pos);
	 priv->needle_coords[5].y = yc + (n_width) * cos (needle_pos);
	 priv->needle_polygon_points = 6;

	/* Draw the needle */
	gdk_gc_set_rgb_fg_color(priv->gc,&priv->colors[COL_NEEDLE]);
	gdk_draw_polygon(priv->pixmap,
			priv->gc,
			TRUE,priv->needle_coords,
			priv->needle_polygon_points);
	
#endif
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
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(widget);

	if(widget->window)
	{
		priv->w = widget->allocation.width;
		priv->h = widget->allocation.height;

		if (priv->gc)
			g_object_unref(priv->gc);
		if (priv->bm_gc)
			g_object_unref(priv->bm_gc);
		if (priv->layout)
			g_object_unref(priv->layout);
		/* Shape combine bitmap */
		if (priv->bitmap)
			g_object_unref(priv->bitmap);
		priv->bitmap = gdk_pixmap_new(NULL,priv->w,priv->h,1);
		/* Backing pixmap (copy of window) */
		if (priv->pixmap)
			g_object_unref(priv->pixmap);
		priv->pixmap=gdk_pixmap_new(widget->window,
				priv->w,priv->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(priv->pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				priv->w,priv->h);
		/* Static Background pixmap */
		if (priv->bg_pixmap)
			g_object_unref(priv->bg_pixmap);
		priv->bg_pixmap=gdk_pixmap_new(widget->window,
				priv->w,priv->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(priv->bg_pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				priv->w,priv->h);
		/* Tmp Background pixmap */
		if (priv->tmp_pixmap)
			g_object_unref(priv->tmp_pixmap);
		priv->tmp_pixmap=gdk_pixmap_new(widget->window,
				priv->w,priv->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(priv->tmp_pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				priv->w,priv->h);
		priv->last_alert_index = -1;

		gdk_window_set_back_pixmap(widget->window,priv->pixmap,0);
		priv->layout = gtk_widget_create_pango_layout(GTK_WIDGET(&gauge->parent),NULL);	
		priv->gc = gdk_gc_new(priv->bg_pixmap);
		gdk_gc_set_colormap(priv->gc,priv->colormap);

		priv->xc = priv->w / 2;
		priv->yc = priv->h / 2;
		priv->radius = MIN (priv->w/2, priv->h/2); 

#ifdef HAVE_CAIRO
		if (priv->font_options)
			cairo_font_options_destroy(priv->font_options);
		priv->font_options = cairo_font_options_create();
		cairo_font_options_set_antialias(priv->font_options,
				CAIRO_ANTIALIAS_GRAY);
#endif

		/* Shape combine mask bitmap */
		colormap = gdk_colormap_get_system ();
		gdk_color_parse ("black", & black);
		gdk_colormap_alloc_color(colormap, &black,TRUE,TRUE);
		gdk_color_parse ("white", & white);
		gdk_colormap_alloc_color(colormap, &white,TRUE,TRUE);
		gc = gdk_gc_new (priv->bitmap);
		gdk_gc_set_foreground (gc, &black);
		gdk_draw_rectangle (priv->bitmap,
				gc,
				TRUE,  /* filled */
				0,     /* x */
				0,     /* y */
				priv->w,
				priv->h);

		gdk_gc_set_foreground (gc, & white);
		/* Drag border boxes... */

		if (priv->show_drag_border)
		{
			gdk_draw_rectangle (priv->bitmap,
					gc,
					TRUE,  /* filled */
					0,     /* x */
					0,     /* y */
					DRAG_BORDER,
					DRAG_BORDER);
			gdk_draw_rectangle (priv->bitmap,
					gc,
					TRUE,  /* filled */
					priv->w-DRAG_BORDER,     /* x */
					0,     /* y */
					DRAG_BORDER,
					DRAG_BORDER);
			gdk_draw_rectangle (priv->bitmap,
					gc,
					TRUE,  /* filled */
					priv->w-DRAG_BORDER,     /* x */
					priv->h-DRAG_BORDER,     /* y */
					DRAG_BORDER,
					DRAG_BORDER);
			gdk_draw_rectangle (priv->bitmap,
					gc,
					TRUE,  /* filled */
					0,     /* x */
					priv->h-DRAG_BORDER,     /* y */
					DRAG_BORDER,
					DRAG_BORDER);
		}
		gdk_draw_arc (priv->bitmap,
				gc,
				TRUE,     /* filled */
				priv->xc-priv->radius,
				priv->yc-priv->radius,
				2*(priv->radius),
				2*(priv->radius),
				0,        /* angle 1 */
				360*64);  /* angle 2: full circle */

	}
	if (priv->radius > 0)
	{
		generate_gauge_background(gauge);
		update_gauge_position(gauge);
	}

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
	MtxGaugeFacePrivate * priv = MTX_GAUGE_FACE_GET_PRIVATE(widget);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	if (GTK_IS_WINDOW(widget->parent))
	{
#ifdef HAVE_CAIRO

#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gtk_widget_input_shape_combine_mask(widget->parent,priv->bitmap,0,0);
#endif
#endif
		gtk_widget_shape_combine_mask(widget->parent,priv->bitmap,0,0);
	}
	else
	{
#ifdef HAVE_CAIRO
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
			gdk_window_input_shape_combine_mask(widget->window,priv->bitmap,0,0);
#endif
#endif
		gdk_window_shape_combine_mask(widget->window,priv->bitmap,0,0);
	}


	return FALSE;
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the cairo version.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void cairo_generate_gauge_background(MtxGaugeFace *gauge)
{
#ifdef HAVE_CAIRO
	GtkWidget * widget = NULL;
	cairo_t *cr = NULL;
	double dashes[2] = {4.0,4.0};
	gfloat deg_per_major_tick = 0.0;
	gfloat deg_per_minor_tick = 0.0;
	cairo_font_weight_t weight;
	cairo_font_slant_t slant;
	gchar * tmpbuf = NULL;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint num_points = 0;
	gint count = 0;
	gfloat counter = 0;
	gfloat rad = 0.0;
	gfloat subcounter = 0;
	gchar ** vector = NULL;
	gfloat inset = 0.0;
	gfloat insetfrom = 0.0;
	gfloat mintick_inset = 0.0;
	gfloat lwidth = 0.0;
	gfloat angle1, angle2;
	cairo_pattern_t *gradient = NULL;
	cairo_text_extents_t extents;
	MtxPolygon *poly = NULL;
	MtxColorRange *range = NULL;
	MtxTextBlock *tblock = NULL;
	MtxTickGroup *tgroup = NULL;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	w = GTK_WIDGET(gauge)->allocation.width;
	h = GTK_WIDGET(gauge)->allocation.height;

	if (!priv->bg_pixmap)
		return;
	/* get a cairo_t */
	cr = gdk_cairo_create (priv->bg_pixmap);
	cairo_set_font_options(cr,priv->font_options);
	/* Background set to black */
	if (priv->show_drag_border)
	{
		cairo_rectangle (cr,
				0,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				priv->w-DRAG_BORDER,0,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				0,priv->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
		cairo_rectangle (cr,
				priv->w-DRAG_BORDER,priv->h-DRAG_BORDER,
				DRAG_BORDER, DRAG_BORDER);
	}
	cairo_arc(cr, priv->xc, priv->yc, priv->radius, 0, 2 * M_PI);
	cairo_set_source_rgb (cr, 0,0,0);

	cairo_fill(cr);
	if (priv->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);


	/* Filled Arcs */
	/* Outside gradient ring */
	gradient = cairo_pattern_create_linear(priv->xc+(0.707*priv->xc),
			priv->yc-(0.707*priv->yc),
			priv->xc-(0.707*priv->xc),
			priv->yc+(0.707*priv->yc));
	cairo_pattern_add_color_stop_rgb(gradient, 0, 
			priv->colors[COL_GRADIENT_BEGIN].red/65535.0, 
			priv->colors[COL_GRADIENT_BEGIN].green/65535.0, 
			priv->colors[COL_GRADIENT_BEGIN].blue/65535.0);
	cairo_pattern_add_color_stop_rgb(gradient, 2*priv->radius, 
			priv->colors[COL_GRADIENT_END].red/65535.0, 
			priv->colors[COL_GRADIENT_END].green/65535.0, 
			priv->colors[COL_GRADIENT_END].blue/65535.0);
	cairo_set_source(cr, gradient);
	cairo_arc(cr, priv->xc, priv->yc, priv->radius, 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);

	/* Inside gradient ring */
	gradient = cairo_pattern_create_linear(priv->xc-(0.707*priv->xc),
			priv->yc+(0.707*priv->yc),
			priv->xc+(0.707*priv->xc),
			priv->yc-(0.707*priv->yc));
	cairo_pattern_add_color_stop_rgb(gradient, 0, 
			priv->colors[COL_GRADIENT_BEGIN].red/65535.0, 
			priv->colors[COL_GRADIENT_BEGIN].green/65535.0, 
			priv->colors[COL_GRADIENT_BEGIN].blue/65535.0);
	cairo_pattern_add_color_stop_rgb(gradient, 2*priv->radius, 
			priv->colors[COL_GRADIENT_END].red/65535.0, 
			priv->colors[COL_GRADIENT_END].green/65535.0, 
			priv->colors[COL_GRADIENT_END].blue/65535.0);
	cairo_set_source(cr, gradient);
	cairo_arc(cr, priv->xc, priv->yc, (0.950 * priv->radius), 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);

	/* Gauge background inside the bezel */
	cairo_set_source_rgb (cr, priv->colors[COL_BG].red/65535.0,
			priv->colors[COL_BG].green/65535.0,
			priv->colors[COL_BG].blue/65535.0);
	cairo_arc(cr, priv->xc, priv->yc, (0.900 * priv->radius), 0, 2 * M_PI);
	cairo_fill(cr);

	/* The warning color ranges */
	for (i=0;i<priv->c_ranges->len;i++)
	{
		range = g_array_index(priv->c_ranges,MtxColorRange *, i);
		cairo_set_source_rgb(cr,range->color.red/65535.0,
				range->color.green/65535.0,
				range->color.blue/65535.0);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		angle1 = (range->lowpoint-priv->lbound)/(priv->ubound-priv->lbound);
		angle2 = (range->highpoint-priv->lbound)/(priv->ubound-priv->lbound);
		/*printf("gauge color range should be from %f, to %f of full scale\n",angle1, angle2);*/
		lwidth = priv->radius*range->lwidth < 1 ? 1: priv->radius*range->lwidth;
		cairo_set_line_width (cr, lwidth);
		cairo_arc(cr, priv->xc, priv->yc, (range->inset * priv->radius),(priv->start_angle+(angle1*(priv->sweep_angle)))*(M_PI/180.0), (priv->start_angle+(angle2*(priv->sweep_angle)))*(M_PI/180.0));

		cairo_stroke(cr);

	}

	/* NEW STYLE Gauge tick groups */
	for (i=0;i<priv->tick_groups->len;i++)
	{
		tgroup = g_array_index(priv->tick_groups,MtxTickGroup *, i);
		cairo_set_source_rgb (cr, 
				tgroup->maj_tick_color.red/65535.0,
				tgroup->maj_tick_color.green/65535.0,
				tgroup->maj_tick_color.blue/65535.0);

		deg_per_major_tick = (tgroup->sweep_angle)/(float)(tgroup->num_maj_ticks-1);
		deg_per_minor_tick = deg_per_major_tick/(float)(1+tgroup->num_min_ticks);

		insetfrom = priv->radius * tgroup->maj_tick_inset;
		counter = tgroup->start_angle *(M_PI/180.0);
		if (tgroup->text)
		{
			vector = g_strsplit(tgroup->text,",",-1);
			count = g_strv_length(vector);
			tmpbuf = g_utf8_strup(tgroup->font,-1);
			if (g_strrstr(tmpbuf,"BOLD"))
				weight = CAIRO_FONT_WEIGHT_BOLD;
			else
				weight = CAIRO_FONT_WEIGHT_NORMAL;
			if (g_strrstr(tmpbuf,"OBLIQUE"))
				slant = CAIRO_FONT_SLANT_OBLIQUE;
			else if (g_strrstr(tmpbuf,"ITALIC"))
				slant = CAIRO_FONT_SLANT_ITALIC;
			else
				slant = CAIRO_FONT_SLANT_NORMAL;
			g_free(tmpbuf);
			cairo_select_font_face (cr, tgroup->font, slant, weight);
			cairo_set_font_size (cr, (priv->radius * tgroup->font_scale));
		}
		for (j=0;j<tgroup->num_maj_ticks;j++)
		{
			inset = tgroup->maj_tick_length * priv->radius;

			lwidth = (priv->radius/10)*tgroup->maj_tick_width < 1 ? 1: (priv->radius/10)*tgroup->maj_tick_width;
			cairo_set_line_width (cr, lwidth);
			cairo_move_to (cr,
					priv->xc + (priv->radius - insetfrom) * cos (counter),
					priv->yc + (priv->radius - insetfrom) * sin (counter));
			cairo_line_to (cr,
					priv->xc + (priv->radius - insetfrom - inset) * cos (counter),
					priv->yc + (priv->radius - insetfrom - inset) * sin (counter));
			cairo_stroke (cr);
			if ((vector) && (j < count)) /* If not null */
			{
				cairo_save(cr);
				cairo_set_source_rgb (cr, 
						tgroup->text_color.red/65535.0,
						tgroup->text_color.green/65535.0,
						tgroup->text_color.blue/65535.0);
				cairo_text_extents (cr, vector[j], &extents);
				/* Gets the radius of a circle that encompasses the 
				 * rectangle of text on screen */
				rad = sqrt(pow(extents.width,2)+pow(extents.height,2))/2.0;
				cairo_move_to (cr,
						priv->xc + (priv->radius - tgroup->text_inset*priv->radius - rad) * cos (counter) - extents.width/2.0-extents.x_bearing,
						priv->yc + (priv->radius - tgroup->text_inset*priv->radius - rad) * sin (counter) + extents.height/2.0);
				cairo_show_text (cr, vector[j]);
				cairo_restore(cr);
			}
			/* minor ticks */
			if ((tgroup->num_min_ticks > 0) && (j < (tgroup->num_maj_ticks-1)))
			{
				cairo_save (cr); /* stack-pen-size */
				cairo_set_source_rgb (cr,
						tgroup->min_tick_color.red/65535.0,
						tgroup->min_tick_color.green/65535.0,
						tgroup->min_tick_color.blue/65535.0);
				inset = tgroup->min_tick_length * priv->radius;
				mintick_inset = priv->radius * tgroup->min_tick_inset;
				lwidth = (priv->radius/10)*tgroup->min_tick_width < 1 ? 1: (priv->radius/10)*tgroup->min_tick_width;
				cairo_set_line_width (cr, lwidth);
				for (k=1;k<=tgroup->num_min_ticks;k++)
				{
					subcounter = (k*deg_per_minor_tick)*(M_PI/180.0);
					cairo_move_to (cr,
							priv->xc + (priv->radius - mintick_inset) * cos (counter+subcounter),
							priv->yc + (priv->radius - mintick_inset) * sin (counter+subcounter));
					cairo_line_to (cr,
							priv->xc + (priv->radius - mintick_inset - inset) * cos (counter+subcounter),
							priv->yc + (priv->radius - mintick_inset - inset) * sin (counter+subcounter));
					cairo_stroke (cr);
				}
				cairo_restore (cr); /* stack-pen-size */
			}
			counter += (deg_per_major_tick)*(M_PI/180);
		}
		g_strfreev(vector);
	}

	/* Polygons */
	for (i=0;i<priv->polygons->len;i++)
	{
		poly = g_array_index(priv->polygons,MtxPolygon *, i);
		cairo_set_source_rgb(cr,
				poly->color.red/65535.0,
				poly->color.green/65535.0,
				poly->color.blue/65535.0);
		lwidth = priv->radius*poly->line_width < 1 ? 1: priv->radius*poly->line_width;
		cairo_set_line_width (cr, lwidth);
		cairo_set_line_join(cr,poly->join_style);
		switch (poly->line_style)
		{
			case GDK_LINE_SOLID:
				cairo_set_dash(cr,0,0,0);
				break;
			case GDK_LINE_ON_OFF_DASH:
				cairo_set_dash(cr,dashes,2,0);
				break;
			default:
				break;
		}
		switch (poly->type)
		{
			case MTX_CIRCLE:
				cairo_arc(cr,
						priv->xc+((MtxCircle *)poly->data)->x*priv->radius,
						priv->yc+((MtxCircle *)poly->data)->y*priv->radius,
						((MtxCircle *)poly->data)->radius*priv->radius,
						0,2*M_PI);
				break;
			case MTX_RECTANGLE:
				cairo_rectangle(cr,
						priv->xc+((MtxRectangle *)poly->data)->x*priv->radius,
						priv->yc+((MtxRectangle *)poly->data)->y*priv->radius,
						((MtxRectangle *)poly->data)->width*priv->radius,
						((MtxRectangle *)poly->data)->height*priv->radius);
				break;
			case MTX_ARC:
				cairo_save(cr);
				cairo_translate(cr,
						priv->xc+(((MtxArc *)poly->data)->x*priv->radius),
						priv->yc+(((MtxArc *)poly->data)->y*priv->radius));
				cairo_scale(cr,
						((MtxArc *)poly->data)->width*priv->radius,
						((MtxArc *)poly->data)->height*priv->radius);
				cairo_arc(cr,
						0.0,
						0.0,
						1.0,
						((MtxArc *)poly->data)->start_angle * (M_PI/180.0),(((MtxArc *)poly->data)->sweep_angle+((MtxArc *)poly->data)->start_angle)*(M_PI/180));
				if (poly->filled)
				{
					cairo_line_to(cr,0,0);
					cairo_close_path(cr);
				}
				cairo_restore(cr);
				break;
			case MTX_GENPOLY:
				num_points = ((MtxGenPoly *)poly->data)->num_points;
				if (num_points < 1)
					break;
				cairo_move_to(cr,
						priv->xc + (((MtxGenPoly *)poly->data)->points[0].x * priv->radius),
						priv->yc + (((MtxGenPoly *)poly->data)->points[0].y * priv->radius));
				for (j=1;j<num_points;j++)
				{
					cairo_line_to(cr,
							priv->xc + (((MtxGenPoly *)poly->data)->points[j].x * priv->radius),
							priv->yc + (((MtxGenPoly *)poly->data)->points[j].y * priv->radius));
				}
				cairo_close_path(cr);
				break;
			default:
				break;
		}
		if (poly->filled)
			cairo_fill(cr);
		else
			cairo_stroke(cr);
	}
	/* Render all the text blocks */
	for (i=0;i<priv->t_blocks->len;i++)
	{
		tblock = g_array_index(priv->t_blocks,MtxTextBlock *, i);
		cairo_set_source_rgb (cr, 
				tblock->color.red/65535.0,
				tblock->color.green/65535.0,
				tblock->color.blue/65535.0);

		tmpbuf = g_utf8_strup(tblock->font,-1);
		if (g_strrstr(tmpbuf,"BOLD"))
			weight = CAIRO_FONT_WEIGHT_BOLD;
		else
			weight = CAIRO_FONT_WEIGHT_NORMAL;
		if (g_strrstr(tmpbuf,"OBLIQUE"))
			slant = CAIRO_FONT_SLANT_OBLIQUE;
		else if (g_strrstr(tmpbuf,"ITALIC"))
			slant = CAIRO_FONT_SLANT_ITALIC;
		else
			slant = CAIRO_FONT_SLANT_NORMAL;
		g_free(tmpbuf);
		cairo_select_font_face (cr, tblock->font, slant, weight);

		cairo_set_font_size (cr, (priv->radius * tblock->font_scale));
		cairo_text_extents (cr, tblock->text, &extents);
		cairo_move_to (cr, 
				priv->xc-(extents.width/2 + extents.x_bearing)+(tblock->x_pos*priv->radius),
				priv->yc-(extents.height/2 + extents.y_bearing)+(tblock->y_pos*priv->radius));
		cairo_show_text (cr, tblock->text);
	}
	cairo_stroke(cr);

	cairo_destroy (cr);
	/* SAVE copy of this on tmp pixmap */
	widget = GTK_WIDGET(gauge);
	gdk_draw_drawable(priv->tmp_pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);
	priv->last_alert_index = -1;
#endif
}


/*!
 \brief draws the static elements of the gauge (only on resize), This includes
 the border, units and name strings,  tick marks and warning regions
 This is the gdk version.
 \param widget (GtkWidget *) pointer to the gauge object
 */
void gdk_generate_gauge_background(MtxGaugeFace *gauge)
{
#ifndef HAVE_CAIRO
	GtkWidget * widget = NULL;
	gfloat deg_per_major_tick = 0.0;
	gfloat deg_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint count = 0;
	gfloat rad = 0.0;
	gchar **vector = NULL;
	gint lwidth = 0;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat tmpf = 0.0;
	gfloat counter = 0.0;
	gfloat subcounter = 0.0;
	gchar * tmpbuf = NULL;
	gint redstep = 0;
	gint greenstep = 0;
	gint bluestep = 0;
	gfloat mintick_inset = 0.0;
	gfloat angle1 = 0.0;
	gfloat angle2 = 0.0;
	gfloat start_pos = 0.0;
	gfloat stop_pos = 0.0;
	gfloat start_angle = 0.0;
	gfloat span = 0.0;
	gint r_sign = 0;
	gint g_sign = 0;
	gint b_sign = 0;
	gint num_points = 0;
	MtxColorRange *range = NULL;
	MtxTextBlock *tblock = NULL;
	MtxTickGroup *tgroup = NULL;
	MtxPolygon *poly = NULL;
	GdkPoint *points = NULL;
	GdkColor color;
	GdkColor *b_color;
	GdkColor *e_color;
	PangoRectangle logical_rect;
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(gauge);

	if (!priv->bg_pixmap)
		return;

	widget = GTK_WIDGET(gauge);

	w = widget->allocation.width;
	h = widget->allocation.height;


	/* Wipe the display, black */
	gdk_draw_rectangle(priv->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			DRAG_BORDER,
			DRAG_BORDER);
	/* Wipe the display, black */
	gdk_draw_rectangle(priv->bg_pixmap,
			widget->style->black_gc,
			TRUE, priv->w-DRAG_BORDER,0,
			DRAG_BORDER,
			DRAG_BORDER);
	/* Wipe the display, black */
	gdk_draw_rectangle(priv->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,priv->h-DRAG_BORDER,
			DRAG_BORDER,
			DRAG_BORDER);
	/* Wipe the display, black */
	gdk_draw_rectangle(priv->bg_pixmap,
			widget->style->black_gc,
			TRUE, priv->w-DRAG_BORDER,priv->h-DRAG_BORDER,
			DRAG_BORDER,
			DRAG_BORDER);
	gdk_draw_arc(priv->bg_pixmap,widget->style->black_gc,TRUE,
			priv->xc-priv->radius,
			priv->yc-priv->radius,
			2*(priv->radius),
			2*(priv->radius),
			0,360*64);


	/* The main grey (will have a thin overlay on top of it) 
	 * This is a FILLED circle */

	/* Gradients */

	lwidth = MIN (priv->xc,priv->yc)/20 < 1 ? 1: MIN (priv->xc,priv->yc)/20;
	gdk_gc_set_line_attributes(priv->gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);

	tmpf = (gfloat)lwidth/(gfloat)(2*priv->radius);
	tmpf = (1.0-(tmpf));

	/* This is a HORRENDOUSLY UGLY hack to get pretty gradients
	 * like cairo does. (cairo does the gradient in something like
	 * 3 calls instead of all the follow ugliness, but they render 
	 * nearly identical which is what counts....
	 */
	b_color = &priv->colors[COL_GRADIENT_BEGIN];
	e_color = &priv->colors[COL_GRADIENT_END];

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
		gdk_gc_set_rgb_fg_color(priv->gc,&color);

		gdk_draw_arc(priv->bg_pixmap,priv->gc,FALSE,
				priv->xc-priv->radius*tmpf,
				priv->yc-priv->radius*tmpf,
				2*(priv->radius*tmpf),
				2*(priv->radius*tmpf),
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
		gdk_gc_set_rgb_fg_color(priv->gc,&color);

		gdk_draw_arc(priv->bg_pixmap,priv->gc,FALSE,
				priv->xc-priv->radius*tmpf,
				priv->yc-priv->radius*tmpf,
				2*(priv->radius*tmpf),
				2*(priv->radius*tmpf),
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
	tmpf = (gfloat)lwidth/(gfloat)(2*priv->radius);
	tmpf = (1.0-(3*tmpf));
	for(i=0;i<36;i++)
	{
		color.red=b_color->red + (i * r_sign * redstep);
		color.green=b_color->green + (i * g_sign * greenstep);
		color.blue=b_color->blue + (i * b_sign * bluestep);
		gdk_gc_set_rgb_fg_color(priv->gc,&color);

		gdk_draw_arc(priv->bg_pixmap,priv->gc,FALSE,
				priv->xc-priv->radius*tmpf,
				priv->yc-priv->radius*tmpf,
				2*(priv->radius*tmpf),
				2*(priv->radius*tmpf),
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
		gdk_gc_set_rgb_fg_color(priv->gc,&color);

		gdk_draw_arc(priv->bg_pixmap,priv->gc,FALSE,
				priv->xc-priv->radius*tmpf,
				priv->yc-priv->radius*tmpf,
				2*(priv->radius*tmpf),
				2*(priv->radius*tmpf),
				(45+(i*5))*64,5*64);
	}

	/* Create the INNER filled black arc to draw the ticks and everything
	 * else onto
	 */
	tmpf = (gfloat)lwidth/(gfloat)(2*priv->radius);
	tmpf = (1.0-(4*tmpf));
	gdk_gc_set_line_attributes(priv->gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);
	gdk_gc_set_rgb_fg_color(priv->gc,&priv->colors[COL_BG]);

	gdk_draw_arc(priv->bg_pixmap,priv->gc,TRUE,
			priv->xc-priv->radius*tmpf,
			priv->yc-priv->radius*tmpf,
			2*(priv->radius*tmpf),
			2*(priv->radius*tmpf),
			0,360*64);

	/* The warning color ranges */
	for (i=0;i<priv->c_ranges->len;i++)
	{
		range = g_array_index(priv->c_ranges,MtxColorRange *, i);
		gdk_gc_set_rgb_fg_color(priv->gc,&range->color);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		span = priv->sweep_angle;
		angle1 = (range->lowpoint-priv->lbound)/(priv->ubound-priv->lbound);
		angle2 = (range->highpoint-priv->lbound)/(priv->ubound-priv->lbound);

		/* positions of the range in degrees */
		start_pos = priv->start_angle+(angle1*span);
		stop_pos = priv->start_angle+(angle2*span);
		/* Converted to funky GDK units */
		start_angle = -start_pos*64;
		span = -(stop_pos-start_pos)*64;

		lwidth = priv->radius*range->lwidth < 1 ? 1: priv->radius*range->lwidth;
		gdk_gc_set_line_attributes(priv->gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);
		gdk_draw_arc(priv->bg_pixmap,priv->gc,FALSE, 
				priv->xc-priv->radius*range->inset, 
				priv->yc-priv->radius*range->inset,
				2*(priv->radius*range->inset),
				2*(priv->radius*range->inset),
				start_angle,
				span);
	}

	/* NEW STYLE gauge ticks */
	for (i=0;i<priv->tick_groups->len;i++)
	{
		tgroup = g_array_index(priv->tick_groups,MtxTickGroup *, i);
		deg_per_major_tick = tgroup->sweep_angle/(float)(tgroup->num_maj_ticks-1);
		deg_per_minor_tick = deg_per_major_tick/(float)(1+tgroup->num_min_ticks);
		/* Major ticks first */
		insetfrom = priv->radius * tgroup->maj_tick_inset;
		count = 0;
		if (tgroup->text)
		{
			vector = g_strsplit(tgroup->text,",",-1);
			count = g_strv_length(vector);
			tmpbuf = g_strdup_printf("%s %i",tgroup->font,(gint)(priv->radius*tgroup->font_scale*0.82));
			priv->font_desc = pango_font_description_from_string(tmpbuf);
			g_free(tmpbuf);
			pango_layout_set_font_description(priv->layout,priv->font_desc);
		}

		counter = (tgroup->start_angle)*(M_PI/180);
		for (j=0;j<tgroup->num_maj_ticks;j++)
		{
			inset = tgroup->maj_tick_length * priv->radius;
			lwidth = (priv->radius/10)*tgroup->maj_tick_width < 1 ? 1: (priv->radius/10)*tgroup->maj_tick_width;
			gdk_gc_set_line_attributes(priv->gc,lwidth,
					GDK_LINE_SOLID,
					GDK_CAP_BUTT,
					GDK_JOIN_BEVEL);

			gdk_gc_set_rgb_fg_color(priv->gc,&tgroup->maj_tick_color);
			gdk_draw_line(priv->bg_pixmap,priv->gc,

					priv->xc + (priv->radius - insetfrom) * cos (counter),
					priv->yc + (priv->radius - insetfrom) * sin (counter),
					priv->xc + ((priv->radius - insetfrom - inset) * cos (counter)),
					priv->yc + ((priv->radius - insetfrom - inset) * sin (counter)));
			if ((vector) && (j < count)) /* If not null */
			{
				gdk_gc_set_rgb_fg_color(priv->gc,&tgroup->text_color);
				pango_layout_set_text(priv->layout,vector[j],-1);
				pango_layout_get_pixel_extents(priv->layout,NULL,&logical_rect);

				rad = sqrt(pow(logical_rect.width,2)+pow(logical_rect.height,2))/2.0;
				/* Fudge factor due to differenced ins pango vs
				 * cairo font extents/rectangles
				 */
				rad*=0.62;

				gdk_draw_layout(priv->bg_pixmap,priv->gc,
						priv->xc + (priv->radius - tgroup->text_inset*priv->radius - rad) * cos (counter) - (logical_rect.width/2),
						priv->yc + (priv->radius - tgroup->text_inset*priv->radius - rad) * sin (counter) - (logical_rect.height/2),priv->layout);
			}

			/* Now the minor ticks... */
			if ((tgroup->num_min_ticks > 0) && (j < (tgroup->num_maj_ticks-1)))
			{
				mintick_inset = priv->radius * tgroup->min_tick_inset;
				gdk_gc_set_rgb_fg_color(priv->gc,&tgroup->min_tick_color);
				inset = tgroup->min_tick_length * priv->radius;
				lwidth = (priv->radius/10)*tgroup->min_tick_width < 1 ? 1: (priv->radius/10)*tgroup->min_tick_width;
				gdk_gc_set_line_attributes(priv->gc,lwidth,
						GDK_LINE_SOLID,
						GDK_CAP_BUTT,
						GDK_JOIN_BEVEL);
				for (k=1;k<=tgroup->num_min_ticks;k++)
				{
					subcounter = (k*deg_per_minor_tick)*(M_PI/180);
					gdk_draw_line(priv->bg_pixmap,priv->gc,

							priv->xc + (priv->radius - mintick_inset) * cos (counter+subcounter),
							priv->yc + (priv->radius - mintick_inset) * sin (counter+subcounter),
							priv->xc + ((priv->radius - mintick_inset - inset) * cos (counter+subcounter)),
							priv->yc + ((priv->radius - mintick_inset - inset) * sin (counter+subcounter)));

				}

			}
			counter += (deg_per_major_tick)*(M_PI/180);

		}
	}

	/* Polygons */
	for (i=0;i<priv->polygons->len;i++)
	{
		poly = g_array_index(priv->polygons,MtxPolygon *, i);
		gdk_gc_set_rgb_fg_color(priv->gc,&poly->color);
		gdk_gc_set_line_attributes(priv->gc,
				poly->line_width*priv->radius,
				poly->line_style,
				GDK_CAP_BUTT,
				poly->join_style);
		switch (poly->type)
		{
			case MTX_CIRCLE:
				gdk_draw_arc(priv->bg_pixmap, priv->gc,
						poly->filled,
						priv->xc+((MtxCircle *)poly->data)->x*priv->radius-(((MtxCircle *)poly->data)->radius*priv->radius),
						priv->yc+((MtxCircle *)poly->data)->y*priv->radius-(((MtxCircle *)poly->data)->radius*priv->radius),
						2*((MtxCircle *)poly->data)->radius*priv->radius,
						2*((MtxCircle *)poly->data)->radius*priv->radius,
						0,360*64);
				break;
			case MTX_RECTANGLE:
				gdk_draw_rectangle(priv->bg_pixmap,
						priv->gc,
						poly->filled, 
						priv->xc+((MtxRectangle *)poly->data)->x*priv->radius,
						priv->yc+((MtxRectangle *)poly->data)->y*priv->radius,
						((MtxRectangle *)poly->data)->width*priv->radius,
						((MtxRectangle *)poly->data)->height*priv->radius);
				break;
			case MTX_ARC:
				gdk_draw_arc(priv->bg_pixmap, priv->gc,
						poly->filled,
						priv->xc+((MtxArc *)poly->data)->x*priv->radius-(((MtxArc *)poly->data)->width*priv->radius),
						priv->yc+((MtxArc *)poly->data)->y*priv->radius-(((MtxArc *)poly->data)->height*priv->radius),
						2*((MtxArc *)poly->data)->width*priv->radius,
						2*((MtxArc *)poly->data)->height*priv->radius,
						-((MtxArc *)poly->data)->start_angle*64,
						-((MtxArc *)poly->data)->sweep_angle*64);
				break;
			case MTX_GENPOLY:
				num_points = ((MtxGenPoly *)poly->data)->num_points;
				points = g_new0(GdkPoint, num_points);
				for (j=0;j<num_points;j++)
				{
					points[j].x = priv->xc + (((MtxGenPoly *)poly->data)->points[j].x * priv->radius);
					points[j].y = priv->yc + (((MtxGenPoly *)poly->data)->points[j].y * priv->radius);
				}
				gdk_draw_polygon(priv->bg_pixmap,
						priv->gc,
						poly->filled,
						points,
						num_points);
				g_free(points);
				break;
			default:
				break;
		}
	}
	/* text Blocks */
	for (i=0;i<priv->t_blocks->len;i++)
	{
		tblock = g_array_index(priv->t_blocks,MtxTextBlock *, i);
		gdk_gc_set_rgb_fg_color(priv->gc,&tblock->color);
		tmpbuf = g_strdup_printf("%s %i",tblock->font,(gint)(priv->radius*tblock->font_scale*0.82));
		priv->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(priv->layout,priv->font_desc);
		pango_layout_set_text(priv->layout,tblock->text,-1);
		pango_layout_get_pixel_extents(priv->layout,NULL,&logical_rect);

		gdk_draw_layout(priv->bg_pixmap,priv->gc,
				priv->xc-(logical_rect.width/2)+(tblock->x_pos*priv->radius),
				priv->yc-(logical_rect.height/2)+(tblock->y_pos*priv->radius),priv->layout);
	}
	/* SAVE copy of this on tmp pixmap */
	gdk_draw_drawable(priv->tmp_pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);
	priv->last_alert_index = -1;

#endif
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
	MtxGaugeFacePrivate *priv = MTX_GAUGE_FACE_GET_PRIVATE(widget);
	GdkWindowEdge edge = -1;
	/*printf("gauge button event\n");*/
	/* Right side of window */
	if (event->x > (priv->w-10))
	{
		/* Upper portion */
		if (event->y < 10)
			edge = GDK_WINDOW_EDGE_NORTH_EAST;
		/* Lower portion */
		else if (event->y > (priv->h-10))
			edge = GDK_WINDOW_EDGE_SOUTH_EAST;
		else 
			edge = -1;
	}
	/* Left Side of window */
	else if (event->x < 10)
	{
		/* If it's in the middle portion */
		/* Upper portion */
		if (event->y < 10) 
			edge = GDK_WINDOW_EDGE_NORTH_WEST;
		/* Lower portion */
		else if (event->y > (priv->h-10))
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
				if ((edge == -1) && (GTK_IS_WINDOW(widget->parent)))
				{
					gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
							event->button,
							event->x_root,
							event->y_root,
							event->time);
				}
				else if (GTK_IS_WINDOW(widget->parent))
				{

						gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
								edge,
								event->button,
								event->x_root,
								event->y_root,
								event->time);
				}
				break;
			case 3: /* right button */
				if (GTK_IS_WINDOW(widget->parent))
					gtk_main_quit();
				break;
		}
	}
	/*printf("gauge button event ENDING\n");*/
	return FALSE;
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
	/*printf("button release\n");*/
	return FALSE;
}


gboolean mtx_gauge_face_motion_event (GtkWidget *gauge,GdkEventMotion *event)
{
	/* We don't care, but return FALSE to propogate properly */
	/*printf("motion in gauge, returning false\n");*/
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
	requisition->width = 75;
	requisition->height = 75;
}
