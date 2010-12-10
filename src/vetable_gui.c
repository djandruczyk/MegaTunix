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

#include <3d_vetable.h>
#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <gui_handlers.h>
#include <mtxmatheval.h>
#include <logviewer_gui.h>
#include <math.h>
#include <multi_expr_loader.h>
#include <plugin.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <vetable_gui.h>
#include <widgetmgmt.h>


/*!
 \brief rescale_table() is called to rescale a subset of a Table (doesn't
 matter what kind of table). 
 \param widget_name (gchar *) is the widget name of the scaler widget 
 that was used. From this widget we extract the table number and other 
 needed data to properly do the rescaling.
 */
static gboolean color_changed = FALSE;
extern gconstpointer *global_data;

G_MODULE_EXPORT void rescale_table(GtkWidget *widget)
{
	gint table_num = -1;
	gint z_base = -1;
	gint z_page = -1;
	gint x_bins = -1;
	gint y_bins = -1;
	GtkWidget *scaler = NULL;
	GtkWidget *math_combo = NULL;
	GtkWidget *tmpwidget = NULL;
	gchar ** vector = NULL;
	gchar * tmpbuf = NULL;
	GList *list = NULL;
	gint i = 0;
	guint j = 0;
	gint precision = 0;
	gfloat value = 0.0;
	gfloat factor = 0.0;
	gfloat retval = 0.0;
	ScaleOp scaleop = ADD;
	Firmware_Details *firmware = NULL;
	GList ***ve_widgets = NULL;

	ve_widgets = DATA_GET(global_data,"ve_widgets");
	firmware = DATA_GET(global_data,"firmware");

	tmpbuf = (gchar *) OBJ_GET(widget,"data");
	vector = g_strsplit(tmpbuf,",",-1);

	scaler = lookup_widget(vector[0]);
	g_return_if_fail(GTK_IS_WIDGET(scaler));
	math_combo = lookup_widget(vector[1]);
	g_return_if_fail(GTK_IS_WIDGET(math_combo));
	g_strfreev(vector);

	tmpbuf = (gchar *)OBJ_GET(scaler,"table_num");
	table_num = (gint)g_ascii_strtod(tmpbuf,NULL);

	z_base = firmware->table_params[table_num]->z_base;
	x_bins = firmware->table_params[table_num]->x_bincount;
	y_bins = firmware->table_params[table_num]->y_bincount;
	z_page = firmware->table_params[table_num]->z_page;

	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(scaler),0,-1);
	factor = (gfloat)g_ascii_strtod(g_strdelimit(tmpbuf,",.",'.'),NULL);
	g_free(tmpbuf);
	scaleop = gtk_combo_box_get_active(GTK_COMBO_BOX(math_combo));

	for (i=z_base;i<(z_base+(x_bins*y_bins));i++)
	{
		if (NULL != (list = ve_widgets[z_page][i]))
		{
			list = g_list_first(list);
			for (j=0;j<g_list_length(list);j++)
			{
				tmpwidget = (GtkWidget *)g_list_nth_data(list,j);
				if ((GBOOLEAN)OBJ_GET(tmpwidget,"marked"))
				{
					precision = (GINT)OBJ_GET(tmpwidget,"precision");
					tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(tmpwidget),0,-1);
					value = (gfloat)g_ascii_strtod(g_strdelimit(tmpbuf,",.",'.'),NULL);
					g_free(tmpbuf);
					retval = rescale(value,scaleop,factor);	
					value = retval;

					tmpbuf = g_strdup_printf("%1$.*2$f",retval,precision);
					gtk_entry_set_text(GTK_ENTRY(tmpwidget),tmpbuf);
					g_signal_emit_by_name(G_OBJECT(tmpwidget),"activate");

					g_free(tmpbuf);

					widget_grab(tmpwidget,NULL,GINT_TO_POINTER(TRUE));
					gtk_entry_set_text(GTK_ENTRY(scaler),"0");
				}
			}
		}
	}
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
}


G_MODULE_EXPORT gfloat rescale(gfloat input, ScaleOp scaleop, gfloat factor)
{
	switch (scaleop)
	{
		case ADD:
			return input+factor;
			break;
		case SUBTRACT:
			return input-factor;
			break;
		case MULTIPLY:
			return input*factor;
			break;
		case DIVIDE:
			return input/factor;
			break;
		case EQUAL:
			return factor;
			break;
		default:
			printf(_("!!! ERROR !!!, rescaler passed invalid enum\n"));
			break;
	}
	return 0;
}



G_MODULE_EXPORT void draw_ve_marker(void)
{
	static gint (*ms_get_ecu_data)(gint,gint,gint,DataSize) = NULL;
	static Firmware_Details *firmware = NULL;
	static GHashTable *sources_hash = NULL;
	static GList ***ve_widgets = NULL;
	static gfloat *prev_x_source;
	static gfloat *prev_y_source;
	static GtkWidget ***last_widgets = NULL;
	static gint **last = NULL;
	static GdkColor ** old_colors = NULL;
	static GdkColor color = { 0, 0,16384,16384};
	static void **y_eval;
	static void **x_eval;
	static gfloat last_z_weight[4] = {0,0,0,0};
	gfloat x_source = 0.0;
	gfloat y_source = 0.0;
	gfloat x_raw = 0.0;
	gfloat y_raw = 0.0;
	GtkWidget *widget = NULL;
	GtkRcStyle *style = NULL;
#ifndef __WIN32__
	GdkGC *gc = NULL;
#endif
	gint i = 0;
	gint table = 0;
	gint page = 0;
	gint base = 0;
	DataSize size = 0;
	gint z_bin[4] = {0,0,0,0};
	gint bin[4] = {0,0,0,0};
	gint mult = 0;
	gint z_mult = 0;
	gfloat left_w = 0.0;
	gfloat right_w = 0.0;
	gfloat top_w = 0.0;
	gfloat bottom_w = 0.0;
	gfloat z_weight[4] = {0,0,0,0};
	gfloat left = 0.0;
	gfloat right = 0.0;
	gfloat top = 0.0;
	gfloat bottom = 0.0;
	gfloat max = 0.0;
	gint heaviest = -1;
	gboolean focus = FALSE;
	gboolean *tracking_focus = NULL;
	GList *list = NULL;
	gint active_table;
	gchar *key = NULL;
	gint canID = 0;
	gint *algorithm = NULL;
	gchar *hash_key = NULL;
	GHashTable *hash = NULL;
	MultiSource *multi = NULL;

	if (!sources_hash)
		sources_hash = DATA_GET(global_data,"sources_hash");
	if (!ve_widgets)
		ve_widgets = DATA_GET(global_data,"ve_widgets");
	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	if (!ms_get_ecu_data)
		get_symbol("ms_get_ecu_data",(void *)&ms_get_ecu_data);
	canID = firmware->canID;
	algorithm = (gint *)DATA_GET(global_data,"algorithm");
	tracking_focus = (gboolean *)DATA_GET(global_data,"tracking_focus");

	active_table = (gint)DATA_GET(global_data,"active_table");

	if ((active_table < 0 ) || (active_table > (firmware->total_tables-1)))
		return;

	if (!x_eval)
		x_eval = g_new0(void *, firmware->total_tables);
	if (!y_eval)
		y_eval = g_new0(void *, firmware->total_tables);
	if (!prev_x_source)
		prev_x_source = g_new0(gfloat, firmware->total_tables);
	if (!prev_y_source)
		prev_y_source = g_new0(gfloat, firmware->total_tables);

	if (!last)
	{
		last = g_new0(gint *, firmware->total_tables);
		for (i=0;i<firmware->total_tables;i++)
			last[i] = g_new0(gint, 4);
	}
	if (!last_widgets)
	{
		last_widgets = g_new0(GtkWidget **, firmware->total_tables);
		for (i=0;i<firmware->total_tables;i++)
			last_widgets[i] = g_new0(GtkWidget *, firmware->table_params[i]->x_bincount * firmware->table_params[i]->y_bincount);
	}
	if (!old_colors)
	{
		old_colors = g_new0(GdkColor *, firmware->total_tables);
		for (i=0;i<firmware->total_tables;i++)
			old_colors[i] = g_new0(GdkColor , firmware->table_params[i]->x_bincount * firmware->table_params[i]->y_bincount);
	}

	table = active_table;

	if (firmware->table_params[table]->x_multi_source)
	{
		hash = firmware->table_params[table]->x_multi_hash;
		key = firmware->table_params[table]->x_source_key;
		hash_key = g_hash_table_lookup(sources_hash,key);
		if (algorithm[table] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = g_hash_table_lookup(hash,hash_key);
			else
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[table] == ALPHA_N)
			multi = g_hash_table_lookup(hash,"DEFAULT");
		else if (algorithm[table] == MAF)
		{
			multi = g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else
			multi = g_hash_table_lookup(hash,"DEFAULT");

		if (!multi)
			return;
		x_eval[table] = multi->dl_eval;
		lookup_current_value(multi->source,&x_source);
	}
	else
	{
		x_eval[table] = firmware->table_params[table]->x_dl_eval;
		lookup_current_value(firmware->table_params[table]->x_source,&x_source);
	}

	if (firmware->table_params[table]->y_multi_source)
	{
		hash = firmware->table_params[table]->y_multi_hash;
		key = firmware->table_params[table]->y_source_key;
		hash_key = g_hash_table_lookup(sources_hash,key);
		if (algorithm[table] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = g_hash_table_lookup(hash,hash_key);
			else
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[table] == ALPHA_N)
			multi = g_hash_table_lookup(hash,"DEFAULT");
		else if (algorithm[table] == MAF)
		{
			multi = g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else
			multi = g_hash_table_lookup(hash,"DEFAULT");

		if (!multi)
			return;

		y_eval[table] = multi->dl_eval;
		lookup_current_value(multi->source,&y_source);
	}
	else
	{
		y_eval[table] = firmware->table_params[table]->y_dl_eval;
		lookup_current_value(firmware->table_params[table]->y_source,&y_source);
	}
	if ((x_source == prev_x_source[table]) && (y_source == prev_y_source[table]))
	{
		/*printf("table marker,  values haven't changed\n"); */
		return;
	}
	else
	{
		/*printf("values have changed, continuing\n"); */
		prev_x_source[table] = x_source;
		prev_y_source[table] = y_source;
	}

	/* Find bin corresponding to current rpm  */
	page = firmware->table_params[table]->x_page;
	base = firmware->table_params[table]->x_base;
	size = firmware->table_params[table]->x_size;
	mult = get_multiplier(size);
	x_raw  = evaluator_evaluate_x(x_eval[table],x_source);
	for (i=0;i<firmware->table_params[table]->x_bincount-1;i++)
	{
		if (ms_get_ecu_data(canID,page,base,size) >= x_raw)
		{
			bin[0] = -1;
			bin[1] = 0;
			left_w = 0;
			right_w = 1;
			break;
		}
		left = ms_get_ecu_data(canID,page,base+(i*mult),size);
		right = ms_get_ecu_data(canID,page,base+((i+1)*mult),size);

		if ((x_raw > left) && (x_raw <= right))
		{
			bin[0] = i;
			bin[1] = i+1;

			right_w = (float)(x_raw-left)/(float)(right-left);
			left_w = 1.0-right_w;
			break;

		}
		if (x_raw > right)
		{
			bin[0] = i+1;
			bin[1] = -1;
			left_w = 1;
			right_w = 0;
		}
	}
	/* printf("left bin %i, right bin %i, left_weight %f, right_weight %f\n",bin[0],bin[1],left_w,right_w);*/

	page = firmware->table_params[table]->y_page;
	base = firmware->table_params[table]->y_base;
	size = firmware->table_params[table]->y_size;
	mult = get_multiplier(size);
	y_raw  = evaluator_evaluate_x(y_eval[table],y_source);
	for (i=0;i<firmware->table_params[table]->y_bincount-1;i++)
	{
		if (ms_get_ecu_data(canID,page,base,size) >= y_raw)
		{
			bin[2] = -1;
			bin[3] = 0;
			top_w = 1;
			bottom_w = 0;
			break;
		}
		bottom = ms_get_ecu_data(canID,page,base+(i*mult),size);
		top = ms_get_ecu_data(canID,page,base+((i+1)*mult),size);

		if ((y_raw > bottom) && (y_raw <= top))
		{
			bin[2] = i;
			bin[3] = i+1;

			top_w = (float)(y_raw-bottom)/(float)(top-bottom);
			bottom_w = 1.0-top_w;
			break;

		}
		if (y_raw > top)
		{
			bin[2] = i+1;
			bin[3] = -1;
			top_w = 0;
			bottom_w = 1;
		}
	}
	z_weight[0] = left_w*bottom_w;
	z_weight[1] = left_w*top_w;
	z_weight[2] = right_w*bottom_w;
	z_weight[3] = right_w*top_w;

	/* Cheap hack to see if things changed, if not don't 
	 * waste CPU time inside of pango changing the color unnecessarily
	 * */

	if (DATA_GET(global_data,"forced_update"))
		goto redraw;
	for (i=0;i<4;i++)
	{
		if ((fabs(z_weight[i]-last_z_weight[i]) > 0.01))
		{
			last_z_weight[i] = z_weight[i];
			goto redraw;
		}
	}
	/*
	   for (i=0;i<4;i++)
	   last_z_weight[i] = z_weight[i];
	 */
	return;

redraw:
	page = firmware->table_params[table]->z_page;
	base = firmware->table_params[table]->z_base;
	size = firmware->table_params[table]->z_size;
	z_mult = get_multiplier(size);
	/*printf("bottom bin %i, top bin %i, bottom_weight %f, top_weight %f\n",bin[2],bin[3],bottom_w,top_w);*/
	if (firmware->capabilities & PIS)
	{
		if ((bin[0] == -1) || (bin[2] == -1))
			z_bin[0] = -1;
		else
			z_bin[0] = bin[2]+(bin[0]*firmware->table_params[table]->y_bincount);
		if ((bin[0] == -1) || (bin[3] == -1))
			z_bin[1] = -1;
		else
			z_bin[1] = bin[3]+(bin[0]*firmware->table_params[table]->y_bincount);
		if ((bin[1] == -1) || (bin[2] == -1))
			z_bin[2] = -1;
		else
			z_bin[2] = bin[2]+(bin[1]*firmware->table_params[table]->y_bincount);
		if ((bin[1] == -1) || (bin[3] == -1))
			z_bin[3] = -1;
		else
			z_bin[3] = bin[3]+(bin[1]*firmware->table_params[table]->y_bincount);
	}
	else
	{
		if ((bin[0] == -1) || (bin[2] == -1))
			z_bin[0] = -1;
		else
			z_bin[0] = bin[0]+(bin[2]*firmware->table_params[table]->x_bincount);
		if ((bin[0] == -1) || (bin[3] == -1))
			z_bin[1] = -1;
		else
			z_bin[1] = bin[0]+(bin[3]*firmware->table_params[table]->x_bincount);
		if ((bin[1] == -1) || (bin[2] == -1))
			z_bin[2] = -1;
		else
			z_bin[2] = bin[1]+(bin[2]*firmware->table_params[table]->x_bincount);
		if ((bin[1] == -1) || (bin[3] == -1))
			z_bin[3] = -1;
		else
			z_bin[3] = bin[1]+(bin[3]*firmware->table_params[table]->x_bincount);
	}
	/* Take the PREVIOUS ones and reset them back to their DEFAULT color
	 */
	for (i=0;i<4;i++)
	{
		if (GTK_IS_WIDGET(last_widgets[table][last[table][i]]))
		{
			widget = last_widgets[table][last[table][i]];
#ifdef __WIN32__
			gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&old_colors[table][last[table][i]]);
#else
			if (GDK_IS_DRAWABLE(widget->window))
			{
				gc = OBJ_GET(widget, "old_gc");
				gdk_gc_set_rgb_fg_color(gc,&old_colors[table][last[table][i]]);
				/* Top */
				gdk_draw_rectangle(widget->window,gc,TRUE,0,0,widget->allocation.width,2);
				/* Bottom */
				gdk_draw_rectangle(widget->window,gc,TRUE,0,widget->allocation.height-3,widget->allocation.width,3);
				/* Left */
				gdk_draw_rectangle(widget->window,gc,TRUE,0,0,2,widget->allocation.height);
				/* Right */
				gdk_draw_rectangle(widget->window,gc,TRUE,widget->allocation.width-2,0,2,widget->allocation.height);
			}
#endif
		}
	}

	/*last_widgets[table][last[table][i]] = NULL; */
	color_changed = FALSE;

	max=0;
	for (i=0;i<4;i++)
	{
		if (z_weight[i] > max)
		{
			max = z_weight[i];
			heaviest = i;
		}
	}
	/*for (i=0;i<4;i++)
	  last_z_weight[i] = z_weight[i];
	 */

	/* Color the 4 vertexes according to their weight 
	 * Save the old colors as well
	 */
	for (i=0;i<4;i++)
	{
		if (z_bin[i] == -1)
			continue;
		/* HACK ALERT,  this assumes the list at 
		 * ve_widgets[page][offset], contains the VEtable widget at
		 * offset 0 of that list.  (assumptions are bad practice!)
		 */
		list = ve_widgets[firmware->table_params[table]->z_page][firmware->table_params[table]->z_base+(z_bin[i]*z_mult)];
		widget = g_list_nth_data(list,0);

		if (!GTK_IS_WIDGET(widget))
			return;
		if ((i == heaviest) && (tracking_focus[table]))
		{
			g_object_get(widget,"has_focus",&focus,NULL);
			if (!focus)
				gtk_widget_grab_focus(widget);
		}

		last_widgets[table][z_bin[i]] = widget;
		last[table][i] = z_bin[i];

		style = gtk_widget_get_modifier_style(widget);
		/* Save color of original widget */
		old_colors[table][z_bin[i]] = style->base[GTK_STATE_NORMAL];
		/*printf("table %i, zbin[%i] %i\n",table,i,z_bin[i]);*/
		color.red = z_weight[i]*32768 +32767;
		color.green = (1.0-z_weight[i])*65535 +0;
		color.blue = (1.0-z_weight[i])*32768 +0;

		/* modify_base is REALLY REALLY slow, as it triggers a size recalc all the
		 * way thru the widget tree, which is atrociously expensive!
		 gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
		 */
#ifdef __WIN32__
		gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
#else
		if (GDK_IS_DRAWABLE(widget->window))
		{
			if (OBJ_GET(widget,"old_gc"))
				gc = OBJ_GET(widget,"old_gc");
			else
			{
				gc = gdk_gc_new(widget->window);
				gdk_gc_set_subwindow(gc,GDK_INCLUDE_INFERIORS);
			}

			gdk_gc_set_rgb_fg_color(gc,&color);
			/* Top */
			gdk_draw_rectangle(widget->window,gc,TRUE,0,0,widget->allocation.width,2);
			/* Bottom */
			gdk_draw_rectangle(widget->window,gc,TRUE,0,widget->allocation.height-3,widget->allocation.width,3);
			/* Left */
			gdk_draw_rectangle(widget->window,gc,TRUE,0,0,2,widget->allocation.height);
			/* Right */
			gdk_draw_rectangle(widget->window,gc,TRUE,widget->allocation.width-2,0,2,widget->allocation.height);
			OBJ_SET(widget,"old_gc",(gpointer)gc);
		}
#endif
	}
}
