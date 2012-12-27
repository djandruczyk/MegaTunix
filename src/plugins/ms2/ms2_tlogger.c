/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

/*!
  \file src/plugins/ms2/ms2_tlogger.c
  \ingroup MS2Plugin,Plugins
  \brief MS2 Plugin Tooth Trigger Logger functionality
  \author David Andruczyk
  */

#include <firmware.h>
#include <ms2_plugin.h>
#include <ms2_tlogger.h>

MS2_TTMon_Data *ttm_data;


/*!
  \brief trivial function to set the page variable of the ttm_data structure
  \see MS2_TTM_Data
  \param page is the page to store in the datastructure
  */
G_MODULE_EXPORT void bind_ttm_to_page(gint page)
{
	ENTER();
	ttm_data->page = page;
	OBJ_SET(ttm_data->darea,"page",GINT_TO_POINTER(page));
	EXIT();
	return;
}


/*!
  \brief resets the ms2 ttm's zoom value
  */
G_MODULE_EXPORT void ms2_ttm_reset_zoom(void)
{
	GtkWidget *widget = NULL;
	ENTER();
	widget = lookup_widget_f("ttm_zoom");
	if (GTK_IS_WIDGET(widget))
		gtk_range_set_value(GTK_RANGE(widget),1.0);
	EXIT();
	return;
}


/*!
  \brief Initializes the MS2_TTMon_Data structure and stores a pointer to 
  it in the passed src_widget
  \param src_widget is where the pointer to the TM datastructure is stored
  \see MS2_TTMon_Data
  */
G_MODULE_EXPORT void ms2_setup_logger_display(GtkWidget * src_widget)
{
	ENTER();
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
	ttm_data->sample_time = 0;
	ttm_data->current = g_new0(gulong,341);
	ttm_data->flags = g_new0(guchar,341);
	ttm_data->sync_loss = g_new0(gboolean,341);
	ttm_data->last = g_new0(gulong,341);
	ttm_data->zoom = 1.0;

	OBJ_SET(src_widget,"ttmon_data",(gpointer)ttm_data);
	EXIT();
	return;
}


/*!
  \brief the TM drawingarea configure event
  \param widget is the pointer to the drawingarea for the TTM
  \param event is the pointer to the GdkEventConfigure structure
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ms2_logger_display_config_event(GtkWidget * widget, GdkEventConfigure *event , gpointer data)
{
	cairo_t *cr = NULL;
	GtkAllocation allocation;
	GdkWindow *window = gtk_widget_get_window(widget);
	gtk_widget_get_allocation(widget,&allocation);

	ENTER();
	if (!ttm_data)
	{
		EXIT();
		return FALSE;
	}
	if(window)
	{
		gint w = allocation.width;
		gint h = allocation.height;
		if (ttm_data->layout)
			g_object_unref(ttm_data->layout);
		if (ttm_data->pixmap)
			g_object_unref(ttm_data->pixmap);
		ttm_data->pixmap=gdk_pixmap_new(window,
				w,h,
				-1);
		cr = gdk_cairo_create(ttm_data->pixmap);
		cairo_set_source_rgb(cr,1.0,1.0,1.0);
		cairo_paint(cr);
		cairo_destroy(cr);
		gdk_window_set_back_pixmap(window,ttm_data->pixmap,0);
		ttm_data->layout = gtk_widget_create_pango_layout(ttm_data->darea,NULL);

	}
	/* Don't try to update if the page isn't bound YET */
	if (ttm_data->page < 0)
	{
		EXIT();
		return TRUE;
	}

	ms2_crunch_trigtooth_data();
	if (ttm_data->peak > 0)
		ms2_update_trigtooth_display();
	EXIT();
	return TRUE;
}


/*!
  \brief crunches the TTM data in prep for display
  */
void ms2_crunch_trigtooth_data()
{
	static GTimeVal last;
	GTimeVal current;
	gint canID = 0;
	gint i = 0;
	guint8 high = 0;
	guint8 mid = 0;
	guint8 low = 0;
	gint tmp = 0;
	gulong min = 0;
	gulong max = 0;
	gint cap_idx = 0;
	gfloat ratio = 0.0;
	extern gconstpointer *global_data;
	Firmware_Details *firmware;

	ENTER();
	min = 1048576;
	max = 1;
	g_get_current_time(&current);
	ttm_data->sample_time = ((current.tv_sec-last.tv_sec)*1000) + ((current.tv_usec-last.tv_usec)/1000.0);
	last = current;

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	canID = firmware->canID;

	for (i=0;i<341;i++)
	{
		ttm_data->last[i] = ttm_data->current[i];
		high = ms_get_ecu_data_f(canID,ttm_data->page,(i*3),MTX_U08);
		mid = ms_get_ecu_data_f(canID,ttm_data->page,(i*3)+1,MTX_U08);
		low = ms_get_ecu_data_f(canID,ttm_data->page,(i*3)+2,MTX_U08);
		ttm_data->current[i] = (((high & 0x0f) << 16) + (mid << 8) +low)*0.66;
		ttm_data->flags[i] = (high & 0xf0) >> 4;
		if ((ttm_data->current[i] < min) && (ttm_data->current[i] != 0))
			min = ttm_data->current[i];
		if (ttm_data->current[i] > max)
			max = ttm_data->current[i];
	}
	ttm_data->min_time = min;
	ttm_data->max_time = max;

	/* Ratio of min to max,  does not work for complex wheel
	 * patterns
	 */
	ratio = (float)max/(float)min;
	lookup_current_value_f("rpm",&ttm_data->rpm);
	/*printf("Current RPM %f\n",ttm_data->rpm);*/
	if (ttm_data->page == firmware->toothmon_page) /* TOOTH logger, we should search for min/max's */
	{
		for (i=0;i<341;i++)
		{
			if (!(ttm_data->flags[i] & 0x01))
				ttm_data->sync_loss[i] = TRUE;
			else
				ttm_data->sync_loss[i] = FALSE;
			/*
			   if (!(ttm_data->flags[i] & 0x02))
			   printf("Cam event happened at sample %i\n",i);
			 */
		}
	}
	/* vertical scale calcs:
	 * PROBLEM:  max_time can be anywhere from 0-1048576, need to 
	 * develop a way to have nice even scale along the Y axis so you
	 * know what the values are
	 * for values of 0-10000000
	 */
	ttm_data->peak = ttm_data->max_time *1.25; /* Add 25% padding */
	tmp = ttm_data->peak;

	if (tmp < 500)
		ttm_data->vdivisor = 50;
	else if ((tmp >= 500) && (tmp < 1000))
		ttm_data->vdivisor = 100;
	else if ((tmp >= 1000) && (tmp < 2500))
		ttm_data->vdivisor = 200;
	else if ((tmp >= 2500) && (tmp < 7500))
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
	else if ((tmp >= 480000)  && (tmp < 960000))
		ttm_data->vdivisor = 120000;
	else
		ttm_data->vdivisor = 240000;

	EXIT();
	return;
}


/*!
  \brief renders the new TTM data to the screen
  */
void ms2_update_trigtooth_display()
{
	gint w = 0;
	gint h = 0;
	gint i = 0;
	cairo_t *cr = NULL;
	gfloat tmpf = 0;
	gint space = 0;
	gfloat tmpx = 0.0;
	gfloat ctr = 0.0;
	gfloat cur_pos = 0.0;
	gfloat y_shift = 0.0;
	gushort val = 0;
	gchar * message = NULL;
	cairo_text_extents_t extents;
	extern gconstpointer *global_data;
	Firmware_Details *firmware;
	GtkAllocation allocation;
	GdkWindow *window = gtk_widget_get_window(ttm_data->darea);

	ENTER();
	gtk_widget_get_allocation(ttm_data->darea,&allocation);
	
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	w=allocation.width;
	h=allocation.height;

	cr = gdk_cairo_create(ttm_data->pixmap);
	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr,0.0,0.0,0.0);
	/* get our font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,h/30);
	cairo_set_line_width(cr,1);

	/* Get width of largest value and save it */
	message = g_strdup_printf("%i",(GINT)(ttm_data->peak));
	cairo_text_extents (cr, message, &extents);
	tmpx = extents.x_advance;
	y_shift = extents.height;
	ttm_data->font_height=extents.height;
	g_free(message);

	cairo_set_line_width(cr,1);
	/* Draw left side axis scale */
	for (ctr=0.0;ctr < ttm_data->peak;ctr+=ttm_data->vdivisor)
	{
		message = g_strdup_printf("%i",(GINT)ctr);
		/*g_printf("marker \"%s\"\n",message);*/
		cairo_text_extents (cr, message, &extents);
		cur_pos = (h-y_shift)*(1-(ctr/ttm_data->peak))+y_shift;
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
	w = allocation.width-ttm_data->usable_begin;
	h = allocation.height;
	y_shift=ttm_data->font_height;
	tmpf = w/(682.0/ttm_data->zoom) >= 2.0 ? 2.0 : w/(682.0/ttm_data->zoom);
	cairo_set_line_width(cr,tmpf);
	
	/*g_printf("ttm_data->peak is %f line width %f\n",ttm_data->peak,w/186.0);*/
	/* Draw the bars, left to right */
	for (i=0;i<(GINT)(341.0/ttm_data->zoom);i++)
	{
		/*g_printf("moved to %f %i\n",ttm_data->usable_begin+(i*w/341.0),0);*/
		cairo_move_to(cr,ttm_data->usable_begin+(i*w/(341.0/ttm_data->zoom)),h-(y_shift/2));
		val = ttm_data->current[i];
		cur_pos = (h-y_shift)*(1.0-(val/ttm_data->peak))+(y_shift/2);
		cairo_line_to(cr,ttm_data->usable_begin+(i*w/(341.0/ttm_data->zoom)),cur_pos);
	}
	cairo_set_font_size(cr,h/20);
	if (ttm_data->page == firmware->toothmon_page)
		message = g_strdup("Tooth times in microseconds.");
	else if (ttm_data->page == firmware->trigmon_page)
		message = g_strdup("Trigger times in microseconds.");
	else
		message = g_strdup("Composite, not sure yet..");

	cairo_text_extents (cr, message, &extents);
	cairo_move_to(cr,ttm_data->usable_begin+((w)/2)-(extents.width/2),extents.height*1.125);
	space = extents.height*1.25;
	cairo_show_text(cr,message);
	g_free(message);

	cairo_set_font_size(cr,11);
	message = g_strdup_printf(_("Engine RPM:  %.1f"),ttm_data->rpm);
	cairo_text_extents (cr, message, &extents);
	space +=extents.height;
	cairo_move_to(cr,ttm_data->usable_begin+5,space+extents.height/8);
	cairo_show_text(cr,message);
	g_free(message);

	message = g_strdup_printf(_("Sample Time: %i ms."),ttm_data->sample_time);
	cairo_text_extents (cr, message, &extents);
	space +=extents.height;
	cairo_move_to(cr,ttm_data->usable_begin+5,space+extents.height/8);
	cairo_show_text(cr,message);
	g_free(message);

	cairo_stroke(cr);
	cairo_destroy(cr);

	/* Trigger redraw to main screen */
	if (!window) 
	{
		EXIT();
		return;
	}
	gdk_window_clear(window);
	EXIT();
	return;
}


/*!
 \brief ms2_ttm_update is a function called by a watch that was set looking
 for the state of a particular variable. When that var is set this function
 is fired off to take care of updating the MS2 TTM display
 \param watch is the pointer to the watch structure
 */
G_MODULE_EXPORT void ms2_ttm_update(RtvWatch *watch)
{
	ENTER();
	ms2_crunch_trigtooth_data();
	ms2_update_trigtooth_display();
	if (ttm_data->stop)
	{
		EXIT();
		return;
	}
	io_cmd_f((gchar *)OBJ_GET(ttm_data->darea,"io_cmd_function"),NULL);
	EXIT();
	return;
}



/*!
  \brief Postfunction wrapper for crunch trigtooth data
  */
G_MODULE_EXPORT void crunch_trigtooth_data_pf(void)
{
	ENTER();
	ms2_crunch_trigtooth_data();
	EXIT();
	return;
}


/*!
  \brief creates the watch that triggers the TTM update
  */
G_MODULE_EXPORT void ms2_ttm_watch(void)
{
	ENTER();
	create_rtv_single_bit_state_watch_f("status3",1,TRUE,TRUE,"ms2_ttm_update", (gpointer)ttm_data->darea);
	EXIT();
	return;
}


/*!
  \brief handler to deal with the zoom control which triggers a display update
  \param widget is the pointer to the slider the user moved
  \param data is unused
  */
G_MODULE_EXPORT gboolean ms2_ttm_zoom(GtkWidget *widget, gpointer data)
{
	gint page = 0;
	ENTER();
	if (ttm_data)
	{
		ttm_data->zoom = (gfloat)gtk_range_get_value(GTK_RANGE(widget));
		if (ttm_data->pixmap)
			ms2_update_trigtooth_display();
	}
	EXIT();
	return TRUE;
}


/*!
  \brief hte TTM expose event which handles redraw after being unobscured
  \param widget is hte pointer to the TTM drawingarea
  \param event is the pointers to the GdkEventExpose structure
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean logger_display_expose_event(GtkWidget * widget, GdkEventExpose *event , gpointer data)
{
	cairo_t *cr = NULL;
	GtkAllocation allocation;
	GdkWindow *window = gtk_widget_get_window(widget);
	gtk_widget_get_allocation(widget,&allocation);

	ENTER();
#if GTK_MINOR_VERSION >= 18
	if (gtk_widget_is_sensitive(GTK_WIDGET(widget)))
#else
	if (GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET(widget)))
#endif
	{
		cr = gdk_cairo_create(window);
		gdk_cairo_set_source_pixmap(cr,ttm_data->pixmap,0,0);
		cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
		cairo_fill(cr);
		cairo_destroy(cr);
	}
	else	/* INSENSITIVE display so grey it */
	{
		cr = gdk_cairo_create(window);
		gdk_cairo_set_source_pixmap(cr,ttm_data->pixmap,0,0);
		cairo_rectangle(cr,event->area.x,event->area.y,event->area.width, event->area.height);
		cairo_fill(cr);
		cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
		cairo_rectangle(cr,0,0,allocation.width, allocation.height);
		cairo_fill(cr);
		cairo_destroy(cr);
	}
	EXIT();
	return TRUE;
}


/*!
  \brief Resets the TTM buttons to defaults
  */
G_MODULE_EXPORT void reset_ttm_buttons(void)
{
	GtkWidget *widget = NULL;
	ENTER();
	widget = lookup_widget_f("toothlogger_disable_radio_button");
	if (GTK_IS_WIDGET(widget))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
	widget = lookup_widget_f("triggerlogger_disable_radio_button");
	if (GTK_IS_WIDGET(widget))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
	widget = lookup_widget_f("compositelogger_disable_radio_button");
	if (GTK_IS_WIDGET(widget))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
}
