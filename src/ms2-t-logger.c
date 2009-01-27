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
#include <cairo/cairo.h>
#include <datamgmt.h>
#include <enums.h>
#include <firmware.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <math.h>
#include <ms1-t-logger.h>
#include <ms2-t-logger.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <logviewer_gui.h>
#include <rtv_processor.h>
#include <watches.h>
#include <widgetmgmt.h>


MS2_TTMon_Data *ttm_data;
extern GObject *global_data;


EXPORT void ms2_setup_logger_display(GtkWidget * src_widget)
{
	extern Firmware_Details *firmware;

	ttm_data = g_new0(MS2_TTMon_Data,1);
	ttm_data->page = -1;
	ttm_data->units = 1;
	ttm_data->pixmap = NULL;
	ttm_data->darea = src_widget;
	ttm_data->min_time = 65535;
	ttm_data->max_time = 0;
	ttm_data->est_teeth = 0;
	ttm_data->wrap_pt = 0;
	ttm_data->font_height = 0;
	ttm_data->usable_begin = 0;
	ttm_data->font_desc = NULL;
	ttm_data->missing = 0;
	ttm_data->sample_time = 0;
	ttm_data->capabilities = firmware->capabilities;
	ttm_data->captures = g_new0(gulong, 341);
	ttm_data->current = g_new0(gulong,341);
	ttm_data->last = g_new0(gulong,341);

	OBJ_SET(src_widget,"ttmon_data",(gpointer)ttm_data);
	return;
}

EXPORT gboolean ms2_logger_display_config_event(GtkWidget * widget, GdkEventConfigure *event , gpointer data)
{
	gint w = 0;
	gint h = 0;

	if(widget->window)
	{
		w=widget->allocation.width;
		h=widget->allocation.height;
		if (ttm_data->layout)
			g_object_unref(ttm_data->layout);
		if (ttm_data->axis_gc)
			g_object_unref(ttm_data->axis_gc);
		if (ttm_data->trace_gc)
			g_object_unref(ttm_data->trace_gc);

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
		ttm_data->layout = gtk_widget_create_pango_layout(ttm_data->darea,NULL);

	}
	/* Don't try to update if the page isn't bound YET */
	if (ttm_data->page < 0)
		return TRUE;

	_ms2_crunch_trigtooth_data(ttm_data->page);
	if (ttm_data->peak > 0)
		update_trigtooth_display(ttm_data->page);
	return TRUE;
}


void _ms2_crunch_trigtooth_data(gint page)
{
	extern Firmware_Details *firmware;
	gint canID = firmware->canID;
	gint i = 0;
	guint8 high = 0;
	guint8 mid = 0;
	guint8 low = 0;
	gint tmp = 0;
	gint min = -1;
	gint max = -1;
	gint cap_idx = 0;
	gfloat ratio = 0.0;
	gint lower = 0;
	gint upper = 0;
	gint index = 0;

	index=0;

	min = 1048576;
	max = 1;
	for (i=0;i<1024;i+=3)
	{

		ttm_data->last[index] = ttm_data->current[index];
		high = get_ecu_data(canID,page,i,MTX_U08);
		mid = get_ecu_data(canID,page,i+1,MTX_U08);
		low = get_ecu_data(canID,page,i+2,MTX_U08);
		ttm_data->current[index] = ((high & 0x0f) << 16) + (mid << 8) +low;
		if ((ttm_data->current[index] < min) && (ttm_data->current[index] != 0))
			min = ttm_data->current[index];
		if (ttm_data->current[index] > max)
			max = ttm_data->current[index];
		index++;
	}
	ttm_data->min_time = min;
	ttm_data->max_time = max;

	/* Ratio of min to max,  does not work for complex wheel
	 * patterns
	 */
	ratio = (float)max/(float)min;
	lookup_current_value("rpm",&ttm_data->rpm);
//printf("Current RPM %f\n",ttm_data->rpm);
	if (page == 5) /* TOOTH logger, we should search for min/max's */
	{
		/* ttm_data->current is the array containing the entire
		 * sample of data organized so the beginning of the array
		 * corresponds to the wrap point in the ECU.  Thus we should
		 * search from here to find then number of "max" pips to see
		 * how many wheel rotations we captured, and then try and
		 * count the minor pips between those maxes and crunch on them
		 * to determine the "quality" of the signal.
		 */

		/* Problems,  the wheel styles can be very complex, not just
		 * n-m styles..
		 */

		cap_idx = 0;
		for (i=0;i<341;i++)
		{
			ttm_data->captures[i] = 0;
			/* Crude test,  ok for m-n wheels, but not complex*/
			if ((ttm_data->current[i] > (1.5*min)) && (min != 0))
				ttm_data->captures[cap_idx++] = i;
		}
		upper = (gint)ceil(ratio);
		lower = (gint)floor(ratio);
		if ((ratio-lower) < 0.5)
			ttm_data->missing = lower - 1;
		else 
			ttm_data->missing = upper - 1;
	}
	/*
	printf("Data for this block\n");
	for (i=0;i<341;i++)
	{
		printf("%.4x ", ttm_data->current[i]);
		if (!((i+1)%16))
			printf("\n");
	}
	printf("\n");
	*/
	/* vertical scale calcs:
	 * PROBLEM:  max_time can be anywhere from 0-1048576, need to 
	 * develop a way to have nice even scale along the Y axis so you
	 * know what the values are
	 * for values of 0-10000000
	 */
	ttm_data->peak = ttm_data->max_time *1.25; /* Add 25% padding */
	tmp = ttm_data->peak;

	if (tmp < 7500)
		ttm_data->vdivisor = 1000;
	else if ((tmp >= 7500) && (tmp < 15000))
		ttm_data->vdivisor = 2500;
	else if ((tmp >= 15000) && (tmp < 30000))
		ttm_data->vdivisor = 5000;
	else if ((tmp >= 30000) && (tmp < 60000))
		ttm_data->vdivisor = 10000;
	else if ((tmp >= 60000) && (tmp < 120000))
		ttm_data->vdivisor = 15000;
	else if ((tmp >= 120000) && (tmp < 240000))
		ttm_data->vdivisor = 30000;
	else if ((tmp >= 240000) && (tmp < 480000))
		ttm_data->vdivisor = 60000;
	else if (tmp >= 480000) 
		ttm_data->vdivisor = 120000;

}


void ms2_update_trigtooth_display(gint page)
{
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint space = 0;
	gfloat tmpx = 0.0;
	gfloat ctr = 0.0;
	gfloat cur_pos = 0.0;
	gfloat y_shift = 0.0;
	gushort val = 0;
	gchar * message = NULL;
	cairo_t *cr;
	cairo_text_extents_t extents;

	w=ttm_data->darea->allocation.width;
	h=ttm_data->darea->allocation.height;

	gdk_draw_rectangle(ttm_data->pixmap,
			ttm_data->darea->style->white_gc,
			TRUE, 0,0,
			w,h);

	cr = gdk_cairo_create(ttm_data->pixmap);

	/* get our font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,h/20);

g_printf("peak %f, divisor, %i\n",ttm_data->peak, ttm_data->vdivisor);
	/* Get width of largest value and save it */
	if (ttm_data->units == 1)
		message = g_strdup_printf("%i",(gint)(ttm_data->peak));
	else
		message = g_strdup_printf("%i",(gint)((ttm_data->peak)/10.0));
	cairo_text_extents (cr, message, &extents);
	tmpx = extents.x_advance;
	y_shift = extents.height;
	ttm_data->font_height=extents.height;
	g_free(message);

	cairo_set_line_width(cr,1);
	/* Draw left side axis scale */
	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		if (ttm_data->units == 1)
			message = g_strdup_printf("%i",(gint)ctr);
		else
			message = g_strdup_printf("%i",(gint)(ctr/10.0));
		/*g_printf("marker \"%s\"\n",message);*/
		cairo_text_extents (cr, message, &extents);
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak))+y_shift;
		/*g_printf("drawing at %f\n",cur_pos);*/
		cairo_move_to(cr,tmpx-extents.x_advance,cur_pos);
		cairo_show_text(cr,message);
		g_free(message);
	}
	/* Horizontal Axis lines */
	cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1.0); /* grey */

	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak))+(y_shift/2);
		cairo_move_to(cr,extents.x_advance+3,cur_pos);
		cairo_line_to(cr,w,cur_pos);
	}
	ttm_data->usable_begin=extents.x_advance+9;
	/* Left Side vertical axis line */
	cairo_move_to(cr,extents.x_advance+7,0);
	cairo_line_to(cr,extents.x_advance+7,h);
	cairo_stroke(cr);

	cairo_set_source_rgba (cr, 0, 0, 0, 1.0); /* black */
	w = ttm_data->darea->allocation.width-ttm_data->usable_begin;
	h = ttm_data->darea->allocation.height;
	y_shift=ttm_data->font_height;
	cairo_set_line_width(cr,w/682.0);
	/*g_printf("ttm_data->peak is %f line width %f\n",ttm_data->peak,w/186.0);*/
	/* Draw the bars, left to right */
	printf("peak %f \n",ttm_data->peak);
	for (i=0;i<341;i++)
	{
		/*g_printf("moved to %f %i\n",ttm_data->usable_begin+(i*w/341.0),0);*/
		cairo_move_to(cr,ttm_data->usable_begin+(i*w/341.0),h-(y_shift/2));
		val = ttm_data->current[i];
		cur_pos = (h-y_shift)*(1.0-(val/ttm_data->peak))+(y_shift/2);
		cairo_line_to(cr,ttm_data->usable_begin+(i*w/341.0),cur_pos);
	}
	cairo_set_font_size(cr,20);
	if (ttm_data->units == 1)
		if (ttm_data->page == 5)
			message = g_strdup("Tooth times in usec.");
		else
			message = g_strdup("Trigger times in usec.");
	else
		if (ttm_data->page == 6)
			message = g_strdup("Tooth times in msec.");
		else
			message = g_strdup("Trigger times in msec.");

	cairo_text_extents (cr, message, &extents);
	cairo_move_to(cr,ttm_data->usable_begin+((w)/2)-(extents.width/2),extents.height*1.125);
	space = extents.height*1.25;
	cairo_show_text(cr,message);
	g_free(message);

	cairo_set_font_size(cr,12);
	message = g_strdup_printf("Engine RPM:  %.1f",ttm_data->rpm);
	cairo_text_extents (cr, message, &extents);
	space +=extents.height;
	cairo_move_to(cr,ttm_data->usable_begin+5,space+extents.height/8);
	cairo_show_text(cr,message);
	g_free(message);

	message = g_strdup_printf("Sample Time: %i ms.",ttm_data->sample_time);
	cairo_text_extents (cr, message, &extents);
	space +=extents.height;
	cairo_move_to(cr,ttm_data->usable_begin+5,space+extents.height/8);
	cairo_show_text(cr,message);
	g_free(message);

	cairo_stroke(cr);
	cairo_destroy(cr);

	/* Trigger redraw to main screen */
	if (!ttm_data->darea->window) 
		return;
	gdk_window_clear(ttm_data->darea->window);

}


gboolean ms2_tlogger_button_handler(GtkWidget * widget, gpointer data)
{
	gint handler = (gint)OBJ_GET(widget,"handler");
	extern GHashTable *dynamic_widgets;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{       /* It's pressed (or checked) */
		switch ((ToggleButton)handler)
		{

			case START_TOOTHMON_LOGGER:
				ttm_data->stop = FALSE;
				ttm_data->widget = widget;
				OBJ_SET(widget,"io_cmd_function","ms2_e_read_toothmon");
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"triggerlogger_buttons_table")),FALSE);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"compositelogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)OBJ_GET(widget,"page"));
				io_cmd("ms2_e_read_toothmon",NULL);
				break;
			case START_TRIGMON_LOGGER:
				ttm_data->stop = FALSE;
				ttm_data->widget = widget;
				OBJ_SET(widget,"io_cmd_function","ms2_e_read_trigmon");
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"toothlogger_buttons_table")),FALSE);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"compositelogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)OBJ_GET(widget,"page"));
				io_cmd("ms2_e_read_trigmon",NULL);
				break;
			case START_COMPOSITEMON_LOGGER:
				ttm_data->stop = FALSE;
				ttm_data->widget = widget;
				OBJ_SET(widget,"io_cmd_function","ms2_e_read_compositemon");
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"toothlogger_buttons_table")),FALSE);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"triggerlogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)OBJ_GET(widget,"page"));
				io_cmd("ms2_e_read_compositemon",NULL);
				break;
			case STOP_TOOTHMON_LOGGER:
				ttm_data->stop = TRUE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"triggerlogger_buttons_table")),TRUE);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"compositelogger_buttons_table")),TRUE);
				break;
			case STOP_TRIGMON_LOGGER:
				ttm_data->stop = TRUE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"toothlogger_buttons_table")),TRUE);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"compositelogger_buttons_table")),TRUE);
				break;
			case STOP_COMPOSITEMON_LOGGER:
				ttm_data->stop = TRUE;
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"toothlogger_buttons_table")),TRUE);
				gtk_widget_set_sensitive(GTK_WIDGET(g_hash_table_lookup(dynamic_widgets,"triggerlogger_buttons_table")),TRUE);
				break;
			default:
				break;
		}
	}
	return TRUE;
}


/*!
 \brief ms2_ttm_update is a function called by a watch that was set looking
 for the state of a particular variable. When that var is set  this function
 is fired off to take care of updating the MS2 TTM display
 \param data (gpointer) arbritary data passed.
 */
void ms2_ttm_update(gpointer data)
{
	gint page = 0;

	page = (gint)OBJ_GET(ttm_data->widget,"page");
	_ms2_crunch_trigtooth_data(page);
	update_trigtooth_display(page);
	if (ttm_data->stop)
		return;
	io_cmd((gchar *)OBJ_GET(ttm_data->widget,"io_cmd_function"),NULL);
}


EXPORT void ms2_ttm_watch(void)
{
	create_single_bit_watch("status3",1,TRUE,"ms2_ttm_update", (gpointer)ttm_data->widget);
}
