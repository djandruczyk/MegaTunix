/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <enums.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <math.h>
#include <t-logger.h>
#include <logviewer_gui.h>
#include <structures.h>


static GHashTable *trigtooth_hash = NULL;

#define CTR 187
#define UNITS 188

/*!
 \brief 
 */

EXPORT void setup_logger_display(GtkWidget * src_widget)
{
	struct TTMon_Data *ttm_data = g_new0(struct TTMon_Data, 1);

	ttm_data->page = (gint)g_object_get_data(G_OBJECT(src_widget),"page");
	ttm_data->pixmap = NULL;
	ttm_data->darea = src_widget;
	ttm_data->min_time = 65535;
	ttm_data->max_time = 0;
	ttm_data->est_teeth = 0;
	ttm_data->wrap_pt = 0;
	ttm_data->font_height = 0;
	ttm_data->usable_begin = 0;
	ttm_data->current = g_array_sized_new(FALSE,TRUE,sizeof (gushort),93);
	ttm_data->last = g_array_sized_new(FALSE,TRUE,sizeof (gushort),93);
	ttm_data->font_desc = pango_font_description_from_string("courier");

	g_object_set_data(G_OBJECT(src_widget),"ttmon_data",(gpointer)ttm_data);
	if (!trigtooth_hash)
		trigtooth_hash = g_hash_table_new(g_direct_hash,g_direct_equal);
	g_hash_table_replace(trigtooth_hash,GINT_TO_POINTER(ttm_data->page),ttm_data);
	return;
}

EXPORT gboolean logger_display_config_event(GtkWidget * widget, GdkEventConfigure *event , gpointer data)
{
	struct TTMon_Data *ttm_data = NULL;
	gint w = 0;
	gint h = 0;

	ttm_data = (struct TTMon_Data *)g_object_get_data(G_OBJECT(widget),"ttmon_data");
	if(widget->window)
	{
		w=widget->allocation.width;
		h=widget->allocation.height;

		if (ttm_data->pixmap)
			g_object_unref(ttm_data->pixmap);
		ttm_data->pixmap=gdk_pixmap_new(widget->window,
				w,h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(ttm_data->pixmap,
				widget->style->white_gc,
				TRUE, 0,0,
				w,h);
		gdk_window_set_back_pixmap(widget->window,ttm_data->pixmap,0);
		ttm_data->axis_gc = initialize_gc(ttm_data->pixmap,TTM_AXIS);
		ttm_data->trace_gc = initialize_gc(ttm_data->pixmap,TTM_TRACE);

	}
	crunch_trigtooth_data(ttm_data->page);
	if (ttm_data->peak > 0)
		update_trigtooth_display(ttm_data->page);
	return TRUE;
}

EXPORT gboolean logger_display_expose_event(GtkWidget * widget, GdkEventExpose *event , gpointer data)
{
	struct TTMon_Data *ttm_data = NULL;
	ttm_data = (struct TTMon_Data *)g_object_get_data(G_OBJECT(widget),"ttmon_data");
	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			ttm_data->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return TRUE;
}


void crunch_trigtooth_data(gint page)
{
	extern gint ** ms_data;
	gint i = 0;
	gint tmp = 0;
	gint min = -1;
	gint max = -1;
	gushort total = 0;
	struct TTMon_Data *ttm_data = NULL;
	gint position = ms_data[page][CTR];
	gint count = 0;
	
	ttm_data = g_hash_table_lookup(trigtooth_hash,GINT_TO_POINTER(page));
	if (!ttm_data)
		g_printf("could NOT find ttm_data in hashtable,  BIG ASS PROBLEM!\n");

	/*
	g_printf("Counter position on page %i is %i\n",page,position);
	if (position > 0)
		g_printf("data block from position %i to 185, then wrapping to 0 to %i\n",position,position-1);
	else
		g_printf("data block from position 0 to 185 (93 words)\n");
		*/

	count=0;
	for (i=0;i<93;i++)
	{
		g_array_insert_val(ttm_data->last,i,g_array_index(ttm_data->current,gushort,i));
	}
	for (i=position;i<185;i+=2)
	{
		total = (ms_data[page][i]*256)+ms_data[page][i+1];
		g_array_insert_val(ttm_data->current,count,total);
		count++;
	}
	if (position != 0)
	{
		for (i=0;i<position;i+=2)
		{
			total = (ms_data[page][i]*256)+ms_data[page][i+1];
			g_array_insert_val(ttm_data->current,count,total);
			count++;
		}
	}
//	g_printf("\n");

	if (ms_data[page][UNITS] == 1)
	{
	//	g_printf("0.1 ms units\n");
	}
	else
	{
	//	g_printf("1uS units\n");
	}

	min = 65535;
	max = 0;
	for (i=0;i<93;i++)
	{
		if (g_array_index(ttm_data->current,gushort, i) < min)
			min = g_array_index(ttm_data->current,gushort, i);
		if (g_array_index(ttm_data->current,gushort, i) > max)
			max = g_array_index(ttm_data->current,gushort, i);
	}
	ttm_data->min_time = min;
	ttm_data->max_time = max;
	//g_printf ("Minimum tooth time: %i, max tooth time %i\n",min,max);

	/* vertical scale calcs:
	 * PROBLEM:  max_time can be anywhere from 0-65535, need to 
	 * develop a way to have nice seven scale along hte Y axis so you
	 * know what the values are
	 *  for values of 0-100000
	 */
	ttm_data->peak = ttm_data->max_time *1.1; // Add 10% padding 
	tmp = ttm_data->peak;

	if (tmp < 750)
		ttm_data->vdivisor = 100;
	else if ((tmp >= 750) && (tmp < 1500))
		ttm_data->vdivisor = 250;
	else if ((tmp >= 1500) && (tmp < 3000))
		ttm_data->vdivisor = 500;
	else if ((tmp >= 3000) && (tmp < 6000))
		ttm_data->vdivisor = 1000;
	else if ((tmp >= 6000) && (tmp < 12000))
		ttm_data->vdivisor = 1500;
	else if ((tmp >= 12000) && (tmp < 24000))
		ttm_data->vdivisor = 3000;
	else if ((tmp >= 24000) && (tmp < 48000))
		ttm_data->vdivisor = 6000;
	else if (tmp >= 48000)
		ttm_data->vdivisor = 12000;

}


void update_trigtooth_display(gint page)
{
#ifdef HAVE_CAIRO
	cairo_update_trigtooth_display(page);
#else
	gdk_update_trigtooth_display(page);
#endif
}

#ifdef HAVE_CAIRO
void cairo_update_trigtooth_display(gint page)
{
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gfloat tmpx = 0.0;
	gfloat ctr = 0.0;
	gfloat cur_pos = 0.0;
	gfloat y_shift = 0.0;
	gushort val = 0;
	gchar * message = NULL;
	cairo_t *cr;
	cairo_text_extents_t extents;
	GdkRegion *region = NULL;
	struct TTMon_Data *ttm_data = NULL;

	ttm_data = g_hash_table_lookup(trigtooth_hash,GINT_TO_POINTER(page));
	if (!ttm_data)
		g_printf("could NOT find ttm_data in hashtable,  BIG ASS PROBLEM!\n");

	w=ttm_data->darea->allocation.width;
	h=ttm_data->darea->allocation.height;

	gdk_draw_rectangle(ttm_data->pixmap,
			ttm_data->darea->style->white_gc,
			TRUE, 0,0,
			w,h);

	cr = gdk_cairo_create(ttm_data->pixmap);

	/* get our font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,h/16);

	//	g_printf("peak %f, divisor, %i\n",ttm_data->peak, ttm_data->vdivisor);
	/* Get width of largest value and save it */
	message = g_strdup_printf("%i",(gint)ttm_data->peak);
	cairo_text_extents (cr, message, &extents);
	tmpx = extents.x_advance;
	y_shift = extents.height;
	ttm_data->font_height=extents.height;
	g_free(message);

	cairo_set_line_width(cr,1);
	/* Draw left side axis scale */
	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		message = g_strdup_printf("%i",(gint)ctr);
		//		g_printf("marker \"%s\"\n",message);
		cairo_text_extents (cr, message, &extents);
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak))+y_shift;
		//		g_printf("drawing at %f\n",cur_pos);
		//cairo_move_to(cr,0,cur_pos);
		cairo_move_to(cr,tmpx-extents.x_advance,cur_pos);
		cairo_show_text(cr,message);
		g_free(message);
	}
	/* Horizontal Axis lines */
	cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1.0);//grey

	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak))+(y_shift/2);
		cairo_move_to(cr,extents.x_advance+4,cur_pos);
		cairo_line_to(cr,w,cur_pos);
	}
	ttm_data->usable_begin=extents.x_advance+9;
	/* Left Side vertical axis line */
	cairo_move_to(cr,extents.x_advance+7,0);
	cairo_line_to(cr,extents.x_advance+7,h);
	cairo_stroke(cr);

	cairo_set_source_rgba (cr, 0, 0, 0, 1.0);//black
	w = ttm_data->darea->allocation.width-ttm_data->usable_begin;
	h = ttm_data->darea->allocation.height;
	y_shift=ttm_data->font_height;
	cairo_set_line_width(cr,w/186.0);
	//g_printf("ttm_data->peak is %f line width %f\n",ttm_data->peak,w/186.0);
	/* Draw the bars, left to right */
	for (i=0;i<93;i++)
	{
		//		g_printf("moved to %f %i\n",ttm_data->usable_begin+(i*w/93.0),0);
		cairo_move_to(cr,ttm_data->usable_begin+(i*w/93.0),h-(y_shift/2));
		val = g_array_index(ttm_data->current,gushort,i);
		cur_pos = (h-y_shift)*(1.0-(val/ttm_data->peak))+(y_shift/2);
		cairo_line_to(cr,ttm_data->usable_begin+(i*w/93.0),cur_pos);
	}
	cairo_stroke(cr);
	cairo_destroy(cr);

	/* Trigger redraw to main screen */
	if (!ttm_data->darea->window) 
		return;

	region = gdk_drawable_get_clip_region (ttm_data->darea->window);
	/* redraw the cairo canvas completely by exposing it */
	gdk_window_invalidate_region (ttm_data->darea->window, region, TRUE);
	gdk_window_process_updates (ttm_data->darea->window, TRUE);

	gdk_region_destroy (region);

}
#else
void gdk_update_trigtooth_display(gint page)
{
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint lwidth = 0;
	gint line_width = 0;
	gint x_pos = 0;
	gfloat tmpx = 0.0;
	gfloat ctr = 0.0;
	gfloat cur_pos = 0.0;
	gfloat y_shift = 0.0;
	gfloat frag = 0.0;
	gfloat fragcount  = 0.0;
	gfloat count  = 0.0;
	gushort val = 0;
	gchar * message = NULL;
	PangoLayout * layout = NULL;
	PangoRectangle ink_rect;
	PangoRectangle logical_rect;
	GdkRegion *region = NULL;
	struct TTMon_Data *ttm_data = NULL;

	ttm_data = g_hash_table_lookup(trigtooth_hash,GINT_TO_POINTER(page));
	if (!ttm_data)
		g_printf("could NOT find ttm_data in hashtable,  BIG ASS PROBLEM!\n");

	w=ttm_data->darea->allocation.width;
	h=ttm_data->darea->allocation.height;

	gdk_draw_rectangle(ttm_data->pixmap,
			ttm_data->darea->style->white_gc,
			TRUE, 0,0,
			w,h);


	/* get our font */
	layout = gtk_widget_create_pango_layout(ttm_data->darea,g_strdup_printf("%i",(gint)ttm_data->peak));
	pango_layout_set_font_description(layout,ttm_data->font_desc);
	pango_layout_get_pixel_extents(layout,&ink_rect,&logical_rect);

	tmpx = logical_rect.width;
	y_shift = logical_rect.height;
	ttm_data->font_height=logical_rect.height;

	/* Draw left side axis scale */
	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		message = g_strdup_printf("%i",(gint)ctr);
		layout = gtk_widget_create_pango_layout(ttm_data->darea,message);
		pango_layout_set_font_description(layout,ttm_data->font_desc);
		pango_layout_get_pixel_extents(layout,&ink_rect,&logical_rect);
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak)); 

		gdk_draw_layout(ttm_data->pixmap,ttm_data->trace_gc,tmpx-logical_rect.width,cur_pos,layout);
		g_free(message);
	}
	/* Horizontal Axis lines */

	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak))+(y_shift/2);
		gdk_draw_line(ttm_data->pixmap,ttm_data->axis_gc,tmpx+4,cur_pos,w,cur_pos);
	}
	ttm_data->usable_begin=tmpx+9;
	/* Left Side vertical axis line */
	gdk_draw_line(ttm_data->pixmap,ttm_data->axis_gc,tmpx+7,0,tmpx+7,h);

	w = ttm_data->darea->allocation.width-ttm_data->usable_begin;
	h = ttm_data->darea->allocation.height;
	y_shift=ttm_data->font_height;

	/* Code from eXtace's 2D eq for nice even spacing algorithm using a
	 * balanced interpolation scheme
	 * */

	x_pos = ttm_data->usable_begin;
	lwidth = (w-ttm_data->usable_begin)/186;
	frag = ((gfloat)(w-ttm_data->usable_begin)/186.0) - (gfloat)lwidth;
	if (frag == 0)
		frag += 0.00001;

	for (i=0;i<93;i++)
	{
		count = count + 2*frag;
		if (count > 1.0 )
		{
			count -= 1.0;
			line_width = lwidth+1;
			fragcount++;
		}
		else
			line_width = lwidth;

		val = g_array_index(ttm_data->current,gushort,i);
		cur_pos = (h-y_shift)*(1.0-(val/ttm_data->peak))+(y_shift/2);
		gdk_draw_rectangle(ttm_data->pixmap,ttm_data->trace_gc,TRUE,
				x_pos,(gint)cur_pos,
				line_width,h-(y_shift/2)-cur_pos);

		x_pos += 2*line_width;
	}

	/* Trigger redraw to main screen */
	if (!ttm_data->darea->window) 
		return;

	region = gdk_drawable_get_clip_region (ttm_data->darea->window);
	/* redraw the cairo canvas completely by exposing it */
	gdk_window_invalidate_region (ttm_data->darea->window, region, TRUE);
	gdk_window_process_updates (ttm_data->darea->window, TRUE);

	gdk_region_destroy (region);

}
#endif
