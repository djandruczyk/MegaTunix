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
  \file src/plugins/mscommon/vetable_gui.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS personality functions for VEtable cell highlighting
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <firmware.h>
#include <mscommon_plugin.h>
#include <multi_expr_loader.h>
#include <math.h>
#include <stdio.h>


extern gconstpointer *global_data;


/*!
  \brief Handles drawing the border around the VEtable cells that are 
  currently influencing the engine. The more "red" the box is the more 
  inflential that vertex is,  vertex with the same color are equally 
  inflential.
  */
G_MODULE_EXPORT void common_draw_ve_marker(void)
{
	static Firmware_Details *firmware = NULL;
	static GHashTable *sources_hash = NULL;
	static GList ***ecu_widgets = NULL;
	static gfloat *prev_x_source;
	static gfloat *prev_y_source;
	static GtkWidget ***last_widgets = NULL;
	static gint **last = NULL;
	static GdkColor ** old_colors = NULL;
	static GdkColor color = { 0, 0,16384,16384};
	static gfloat **x_add;
	static gfloat **y_add;
	static gfloat **x_mult;
	static gfloat **y_mult;
	static gfloat last_z_weight[4] = {0,0,0,0};
	static GdkColormap *colormap = NULL;
	gfloat x_source = 0.0;
	gfloat y_source = 0.0;
	gfloat x_raw = 0.0;
	gfloat y_raw = 0.0;
	GtkWidget *widget = NULL;
	GtkRcStyle *style = NULL;
	GdkWindow *window = NULL;
	cairo_t *cr = NULL;
	gint i = 0;
	guint j = 0;
	gint table = 0;
	gint page = 0;
	gint base = 0;
	DataSize size = MTX_U08;
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
	GtkAllocation allocation;

	ENTER();
	if (!sources_hash)
		sources_hash = (GHashTable *)DATA_GET(global_data,"sources_hash");
	if (!ecu_widgets)
		ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	if (!colormap)
		colormap = gdk_colormap_get_system();

	g_return_if_fail(ecu_widgets);
	g_return_if_fail(firmware);
	g_return_if_fail(colormap);

	canID = firmware->canID;
	algorithm = (gint *)DATA_GET(global_data,"algorithm");
	tracking_focus = (gboolean *)DATA_GET(global_data,"tracking_focus");

	active_table = (GINT)DATA_GET(global_data,"active_table");

	if ((active_table < 0 ) || (active_table > (firmware->total_tables-1)))
	{
		EXIT();
		return;
	}

	if (!x_mult)
		x_mult = g_new0(gfloat *, firmware->total_tables);
	if (!y_mult)
		y_mult = g_new0(gfloat *, firmware->total_tables);
	if (!x_add)
		x_add = g_new0(gfloat *, firmware->total_tables);
	if (!y_add)
		y_add = g_new0(gfloat *, firmware->total_tables);
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
		hash_key = (gchar *)g_hash_table_lookup(sources_hash,key);
		if (algorithm[table] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = (MultiSource *)g_hash_table_lookup(hash,hash_key);
			else
				multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[table] == ALPHA_N)
			multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");
		else if (algorithm[table] == MAF)
		{
			multi = (MultiSource *)g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");
		}
		else
			multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");

		if (!multi)
		{
			/*printf("X multi source set, but undefined, BUG!\n");*/
			EXIT();
			return;
		}
		x_mult[table] = multi->multiplier;
		x_add[table] = multi->adder;
		lookup_current_value_f(multi->source,&x_source);
	}
	else
	{
		x_mult[table] = firmware->table_params[table]->x_fromecu_mult;
		x_add[table] = firmware->table_params[table]->x_fromecu_add;
		lookup_current_value_f(firmware->table_params[table]->x_source,&x_source);
	}

	if (firmware->table_params[table]->y_multi_source)
	{
		hash = firmware->table_params[table]->y_multi_hash;
		key = firmware->table_params[table]->y_source_key;
		hash_key = (gchar *)g_hash_table_lookup(sources_hash,key);
		if (algorithm[table] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = (MultiSource *)g_hash_table_lookup(hash,hash_key);
			else
				multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[table] == ALPHA_N)
			multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");
		else if (algorithm[table] == MAF)
		{
			multi = (MultiSource *)g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");
		}
		else
			multi = (MultiSource *)g_hash_table_lookup(hash,"DEFAULT");

		if (!multi)
		{
			/*printf("Y multi source set, but undefined, BUG!\n");*/
			EXIT();
			return;
		}

		y_mult[table] = multi->multiplier;
		y_add[table] = multi->adder;
		lookup_current_value_f(multi->source,&y_source);
	}
	else
	{
		y_mult[table] = firmware->table_params[table]->y_fromecu_mult;
		y_add[table] = firmware->table_params[table]->y_fromecu_add;
		lookup_current_value_f(firmware->table_params[table]->y_source,&y_source);
	}
	if ((x_source == prev_x_source[table]) && (y_source == prev_y_source[table]))
	{
		EXIT();
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
	mult = get_multiplier_f(size);
	/* Backwards calc (To->ECU) */
	if ((x_mult[table]) && (x_add[table]))
		x_raw = (x_source - (*x_add[table]))/(*x_mult[table]);
	else if (x_mult[table])
		x_raw = x_source/(*x_mult[table]);
	else
		x_raw = x_source;
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
	mult = get_multiplier_f(size);
	/* Backwards calc (To->ECU) */
	if ((y_mult[table]) && (y_add[table]))
		y_raw = (y_source - (*y_add[table]))/(*y_mult[table]);
	else if (y_mult[table])
		y_raw = y_source/(*y_mult[table]);
	else
		y_raw = y_source;

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
	EXIT();
	return;

redraw:
	page = firmware->table_params[table]->z_page;
	base = firmware->table_params[table]->z_base;
	size = firmware->table_params[table]->z_size;
	z_mult = get_multiplier_f(size);
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
			gtk_widget_get_allocation(widget,&allocation);
			window = gtk_entry_get_text_window(GTK_ENTRY(widget));
			if (GDK_IS_DRAWABLE(window))
			{
				cr = gdk_cairo_create(window);
				gdk_cairo_set_source_color(cr,&old_colors[table][last[table][i]]);
				cairo_set_line_width(cr,2);
				cairo_rectangle(cr,1,1,allocation.width-2,allocation.height-2);
				cairo_stroke(cr);
				cairo_destroy(cr);
			}
		}
	}

	/*last_widgets[table][last[table][i]] = NULL; */

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
		 * ecu_widgets[page][offset], contains the VEtable widget at
		 * offset 0 of that list.  (assumptions are bad practice!)
		 */
		list = ecu_widgets[firmware->table_params[table]->z_page][firmware->table_params[table]->z_base+(z_bin[i]*z_mult)];

		j = 0;
		while (j<g_list_length(list))
		{
			widget = (GtkWidget *)g_list_nth_data(list,j);
			if (GTK_IS_ENTRY(widget))
				break;
			else
				j++;
		}
		if (!GTK_IS_WIDGET(widget))
		{
			EXIT();
			return;
		}
		if ((i == heaviest) && (tracking_focus[table]))
		{
			g_object_get(widget,"has_focus",&focus,NULL);
			if (!focus)
				gtk_widget_grab_focus(widget);
		}

		last_widgets[table][z_bin[i]] = widget;
		last[table][i] = z_bin[i];

		style = gtk_widget_get_modifier_style(widget);
		gtk_widget_get_allocation(widget,&allocation);
		/* Save color of original widget */
		old_colors[table][z_bin[i]] = style->base[GTK_STATE_NORMAL];
		/*printf("table %i, zbin[%i] %i\n",table,i,z_bin[i]);*/
		color.red = z_weight[i]*32768 +32767;
		color.green = (1.0-z_weight[i])*65535 +0;
		color.blue = (1.0-z_weight[i])*32768 +0;
		color.pixel=65535;

		/* modify_base is REALLY REALLY slow, as it triggers a size 
		 * recalc all the way thru the widget tree, which is 
		 * atrociously expensive!
		 */
		window = gtk_entry_get_text_window(GTK_ENTRY(widget));
		if (GDK_IS_DRAWABLE(window))
		{
			cr = gdk_cairo_create(window);
			gdk_cairo_set_source_color(cr,&color);
			cairo_set_line_width(cr,2);
			cairo_rectangle(cr,1,1,allocation.width-2,allocation.height-2);
			cairo_stroke(cr);
			cairo_destroy(cr);
		}
	}
	EXIT();
	return;
}
