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
#include <ms2-t-logger.h>
#include <ms1-t-logger.h>
#include <threads.h>
#include <timeout_handlers.h>
#include <logviewer_gui.h>
#include <rtv_processor.h>
#include <widgetmgmt.h>


TTMon_Data *ttm_data;
extern GObject *global_data;

#define CTR 187
#define UNITS 188

/*!
 \brief 
 */
void bind_ttm_to_page(gint page)
{
	ttm_data->page = page;
	OBJ_SET(ttm_data->darea,"page",GINT_TO_POINTER(page));
}


EXPORT void reset_ttm_buttons()
{
	GtkWidget *widget = NULL;
	widget = lookup_widget("toothlogger_disable_radio_button");
	if (GTK_IS_WIDGET(widget))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
	widget = lookup_widget("triggerlogger_disable_radio_button");
	if (GTK_IS_WIDGET(widget))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
	widget = lookup_widget("compositelogger_disable_radio_button");
	if (GTK_IS_WIDGET(widget))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
}


EXPORT void setup_logger_display(GtkWidget * src_widget)
{
	ttm_data = g_new0(TTMon_Data,1);
	ttm_data->page = -1;
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
	ttm_data->captures = g_new0(gushort, 93);
	ttm_data->current = g_new0(gushort,93);
	ttm_data->last = g_new0(gushort,93);

	OBJ_SET(src_widget,"ttmon_data",(gpointer)ttm_data);
	return;
}

EXPORT gboolean logger_display_config_event(GtkWidget * widget, GdkEventConfigure *event , gpointer data)
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

	_crunch_trigtooth_data(ttm_data->page);
	if (ttm_data->peak > 0)
		update_trigtooth_display(ttm_data->page);
	return TRUE;
}

EXPORT gboolean logger_display_expose_event(GtkWidget * widget, GdkEventExpose *event , gpointer data)
{
	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			ttm_data->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return TRUE;
}


EXPORT void crunch_trigtooth_data_pf()
{
	_crunch_trigtooth_data(ttm_data->page);
}

void _crunch_trigtooth_data(gint page)
{
	extern Firmware_Details *firmware;
	gint canID = firmware->canID;
	DataSize size = MTX_U08;
	gint i = 0;
	gint tmp = 0;
	gint min = -1;
	gint max = -1;
	gint cap_idx = 0;
	gfloat ratio = 0.0;
	gfloat suggested_sample_time= 0.0;
	gint min_sampling_time = 0;
	extern gint toothmon_id;
	gint lower = 0;
	gint upper = 0;
	gushort total = 0;
	gint position = get_ecu_data(canID,page,CTR,size);
	gint index = 0;

/*
	g_printf("Counter position on page %i is %i\n",page,position);
	if (position > 0)
		g_printf("data block from position %i to 185, then wrapping to 0 to %i\n",position,position-1);
	else
		g_printf("data block from position 0 to 185 (93 words)\n");
*/

	/*printf("position is %i\n",position);*/
	index=0;
	for (i=0;i<93;i++)
		ttm_data->last[i] = ttm_data->current[i];


	for (i=position;i<185;i+=2)
	{
		/*total = (get_ecu_data(canID,page,i,size)*256)+get_ecu_data(canID,page,i+1,size);*/
		total = get_ecu_data(canID,page,i,MTX_U16);
		ttm_data->current[index] = total;
		index++;
	}
	if (position != 0)
	{
		for (i=0;i<position;i+=2)
		{
			/*total = (get_ecu_data(canID,page,i,size)*256)+get_ecu_data(canID,page,i+1,size);*/
			total = get_ecu_data(canID,page,i,MTX_U16);
			ttm_data->current[index] = total;
			index++;
		}
	}
	/*g_printf("\n");*/

	if (get_ecu_data(canID,page,UNITS,size) == 1)
	{
		/*g_printf("0.1 ms units\n");*/
		ttm_data->units=100;
	}
	else
	{
		/*g_printf("1uS units\n");*/
		ttm_data->units=1;
	}

	min = 65535;
	max = 1;
	for (i=0;i<93;i++)
	{
		if ((ttm_data->current[i] < min) && (ttm_data->current[i] != 0))
			min = ttm_data->current[i];
		if (ttm_data->current[i] > max)
			max = ttm_data->current[i];
	}
	ttm_data->min_time = min;
	ttm_data->max_time = max;
	/* Ratio of min to max,  may not work for complex wheel
	 * patterns
	 */
	ratio = (float)max/(float)min;
	lookup_current_value("rpm",&ttm_data->rpm);
/*printf("Current RPM %f\n",ttm_data->rpm);*/
	if (page == 9) /* TOOTH logger, we should search for min/max's */
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
		for (i=0;i<93;i++)
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
		/*
		for (i=1;i<cap_idx;i++)
			printf("read %i trigger times followed by %i missing, thus %i-%i wheel\n",ttm_data->captures[i]-ttm_data->captures[i-1],ttm_data->missing,ttm_data->missing+ttm_data->captures[i]-ttm_data->captures[i-1],ttm_data->missing);
		for (i=0;i<cap_idx;i++)
			printf("Missing teeth at index %i\n",ttm_data->captures[i]);
			*/

		/*printf("max/min is %f\n ceil %f. floor %f",ratio,ceil(ratio),floor(ratio) );*/
		/*printf("wheel is a missing %i style\n",ttm_data->missing);*/

		/*
		printf("Minimum tooth time: %i, max tooth time %i\n",min,max);
		printf ("Teeth per second is %f\n",1.0/(((float)min*ttm_data->units)/1000000.0));
		*/
		suggested_sample_time = 186000/((1.0/(((float)min*ttm_data->units)/1000000.0)));
		if (suggested_sample_time < 0)
			suggested_sample_time = 0;
		min_sampling_time = 500; /* milliseconds */

		ttm_data->sample_time = suggested_sample_time < min_sampling_time ? min_sampling_time : suggested_sample_time;

		/*
		printf("Suggested Sampling time is %f ms.\n",suggested_sample_time);
		printf("Sampling time set to %i ms.\n",ttm_data->sample_time);
		*/

		if (toothmon_id != 0)
		{
			g_source_remove(toothmon_id);
			toothmon_id = g_timeout_add(ttm_data->sample_time,(GtkFunction)signal_toothtrig_read,GINT_TO_POINTER(TOOTHMON_TICKLER));
		}

	}
	/*
	printf("Data for this block\n");
	for (i=0;i<93;i++)
	{
		printf("%.4x ", ttm_data->current[i]);
		if (!((i+1)%16))
			printf("\n");
	}
	printf("\n");
	*/
	/* vertical scale calcs:
	 * PROBLEM:  max_time can be anywhere from 0-65535, need to 
	 * develop a way to have nice even scale along the Y axis so you
	 * know what the values are
	 * for values of 0-100000
	 */
	ttm_data->peak = ttm_data->max_time *1.25; /* Add 25% padding */
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


EXPORT void update_trigtooth_display_pf()
{
	update_trigtooth_display(ttm_data->page);
}

void update_trigtooth_display(gint page)
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

	/*g_printf("peak %f, divisor, %i\n",ttm_data->peak, ttm_data->vdivisor);*/
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
	cairo_set_line_width(cr,w/186.0);
	/*g_printf("ttm_data->peak is %f line width %f\n",ttm_data->peak,w/186.0);*/
	/* Draw the bars, left to right */
	for (i=0;i<93;i++)
	{
		/*g_printf("moved to %f %i\n",ttm_data->usable_begin+(i*w/93.0),0);*/
		cairo_move_to(cr,ttm_data->usable_begin+(i*w/93.0),h-(y_shift/2));
		val = ttm_data->current[i];
		cur_pos = (h-y_shift)*(1.0-(val/ttm_data->peak))+(y_shift/2);
		cairo_line_to(cr,ttm_data->usable_begin+(i*w/93.0),cur_pos);
	}
	cairo_set_font_size(cr,20);
	if (ttm_data->units == 1)
		if (ttm_data->page == 9)
			message = g_strdup("Tooth times in usec.");
		else
			message = g_strdup("Trigger times in usec.");
	else
		if (ttm_data->page == 9)
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

EXPORT gboolean ms1_tlogger_button_handler(GtkWidget * widget, gpointer data)
{
	gint handler = (gint)OBJ_GET(widget, "handler");

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{       /* It's pressed (or checked) */
		switch ((ToggleButton)handler)
		{

			case START_TOOTHMON_LOGGER:
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("triggerlogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)OBJ_GET(widget,"page"));
				start_tickler(TOOTHMON_TICKLER);
				break;
			case START_TRIGMON_LOGGER:
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("toothlogger_buttons_table")),FALSE);
				bind_ttm_to_page((gint)OBJ_GET(widget,"page"));
				start_tickler(TRIGMON_TICKLER);
				break;
			case STOP_TOOTHMON_LOGGER:
				stop_tickler(TOOTHMON_TICKLER);
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("triggerlogger_buttons_table")),TRUE);
				break;
			case STOP_TRIGMON_LOGGER:
				stop_tickler(TRIGMON_TICKLER);
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget("toothlogger_buttons_table")),TRUE);
				break;
			default:
				break;
		}
	}
	return TRUE;
}
