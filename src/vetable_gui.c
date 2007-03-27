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
#include <gui_handlers.h>
#include <gui_handlers.h>
#include "../mtxmatheval/mtxmatheval.h"
#include <logviewer_gui.h>
#include <math.h>
#include <req_fuel.h>
#include <rtv_processor.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>
#include <threads.h>
#include <vetable_gui.h>


/*!
 \brief rescale_table() is called to rescale a subset of a Table (doesn't
 matter what kind of table). 
 \param widget_name (gchar *) is the widget name of the scaler widget 
 that was used. From this widget we extract the table number and other 
 needed data to properly do the rescaling.
 */
static gboolean color_changed = FALSE;

void rescale_table(GtkWidget *widget)
{
	extern Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern GHashTable *dynamic_widgets;
	gint table_num = -1;
	gint z_base = -1;
	gint z_page = -1;
	gint x_bins = -1;
	gint y_bins = -1;
	gint old = 0;
	gint page = 0;
	gint offset = 0;
	gboolean ign_parm = FALSE;
	extern gint **ms_data;
	GtkWidget *scaler = NULL;
	GtkWidget *tmpwidget = NULL;
	gchar * tmpbuf = NULL;
	GList *list = NULL;
	gfloat percentage = 0.0;
	gint i = 0;
	gint j = 0;
	gint raw_lower = 0;
	gint raw_upper = 255;
	gfloat value = 0.0;
	gfloat real_value = 0.0;
	GdkColor color;
	gchar *widget_name = NULL;
	extern GdkColor black;
	gboolean use_color = FALSE;
	extern gboolean forced_update;

	widget_name = (gchar *) g_object_get_data(G_OBJECT(widget),"data");

	scaler = g_hash_table_lookup(dynamic_widgets,widget_name);
	g_return_if_fail(GTK_IS_WIDGET(scaler));
	tmpbuf = (gchar *)g_object_get_data(G_OBJECT(scaler),"table_num");
	table_num = (gint)g_ascii_strtod(tmpbuf,NULL);

	z_base = firmware->table_params[table_num]->z_base;
	x_bins = firmware->table_params[table_num]->x_bincount;
	y_bins = firmware->table_params[table_num]->y_bincount;
	z_page = firmware->table_params[table_num]->z_page;

	percentage = gtk_spin_button_get_value(GTK_SPIN_BUTTON(scaler));
	for (i=z_base;i<(z_base+(x_bins*y_bins));i++)
	{
		if (NULL != (list = ve_widgets[z_page][i]))
		{
			list = g_list_first(list);
			for (j=0;j<g_list_length(list);j++)
			{
				tmpwidget = (GtkWidget *)g_list_nth_data(list,j);
				if ((gboolean)g_object_get_data(G_OBJECT(tmpwidget),"marked"))
				{
					ign_parm = (gboolean)g_object_get_data(G_OBJECT(tmpwidget),"ign_parm");
					page = (gint)g_object_get_data(G_OBJECT(tmpwidget),"page");
					offset = (gint)g_object_get_data(G_OBJECT(tmpwidget),"offset");
					use_color = (gint)g_object_get_data(G_OBJECT(tmpwidget),"use_color");
					if (g_object_get_data(G_OBJECT(tmpwidget),"raw_upper") != NULL)
						raw_upper = (gint)g_object_get_data(G_OBJECT(tmpwidget),"raw_upper");
					if (g_object_get_data(G_OBJECT(tmpwidget),"raw_lower") != NULL)
						raw_lower = (gint)g_object_get_data(G_OBJECT(tmpwidget),"raw_lower");
					value = ms_data[page][offset];
					value = (value*percentage)/100.0;
					if (value < raw_lower)
						value = raw_lower;
					if (value > raw_upper)
						value = raw_upper;

					/* What we are doing is doing the 
					 * forware/reverse conversion which
					 * will give us an exact value if the 
					 * user inputs something in
					 * between,  thus we can reset the 
					 * display to a sane value...
					 */
					old = ms_data[page][offset];
					ms_data[page][offset] = value;

					real_value = convert_after_upload(tmpwidget);
					ms_data[page][offset] = old;

					tmpbuf = g_strdup_printf("%i",(gint)real_value);
					g_signal_handlers_block_by_func (G_OBJECT(tmpwidget),
							G_CALLBACK (entry_changed_handler),
							NULL);
					gtk_entry_set_text(GTK_ENTRY(tmpwidget),tmpbuf);
					g_signal_handlers_unblock_by_func (G_OBJECT(tmpwidget),
							G_CALLBACK (entry_changed_handler),
							NULL);

					g_free(tmpbuf);

					write_ve_const(tmpwidget, page, offset, (gint)value, ign_parm, TRUE);
					gtk_widget_modify_text(tmpwidget,GTK_STATE_NORMAL,&black);
					widget_grab(tmpwidget,NULL,GINT_TO_POINTER(TRUE));
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(scaler),100.0);
					if (use_color)
					{
						color = get_colors_from_hue(((gfloat)value/256.0)*360.0,0.33, 1.0);
						gtk_widget_modify_base(GTK_WIDGET(tmpwidget),GTK_STATE_NORMAL,&color);
					}


				}
			}
		}
	}
	forced_update = TRUE;
}

/*!
 \brief reqfuel_rescale_table() is called to rescale a VEtable based on a
 newly sent reqfuel variable.
 \param widget_name (gchar *) is the widget name of the scaler widget 
 that was used. From this widget we extract the table number and other 
 needed data to properly do the rescaling.
 */
void reqfuel_rescale_table(GtkWidget *widget)
{
	extern Firmware_Details *firmware;
	extern GList ***ve_widgets;
	extern GHashTable *dynamic_widgets;
	gint table_num = -1;
	gint z_base = -1;
	gint z_page = -1;
	gint x_bins = -1;
	gint y_bins = -1;
	gint old = 0;
	gint page = 0;
	gint offset = 0;
	gboolean ign_parm = FALSE;
	extern gint **ms_data;
	GtkWidget *tmpwidget = NULL;
	gchar * tmpbuf = NULL;
	GList *list = NULL;
	gfloat percentage = 0.0;
	gint i = 0;
	gint j = 0;
	gint x = 0;
	gchar **vector = NULL;
	guchar *data = NULL;
	gint raw_lower = 0;
	gint raw_upper = 255;
	gfloat value = 0.0;
	gfloat real_value = 0.0;
	gfloat new_reqfuel = 0.0;
	GdkColor color;
	extern GdkColor black;
	gboolean use_color = FALSE;
	extern gboolean forced_update;

	g_return_if_fail(GTK_IS_WIDGET(widget));
	if (!g_object_get_data(G_OBJECT(widget),"applicable_tables"))
	{
		printf("applicable tables not defined!!!\n");
		return;
	}
	if (!g_object_get_data(G_OBJECT(widget),"table_num"))
	{
		printf("table_num not defined!!!\n");
		return;
	}
	tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"applicable_tables");
	table_num = (gint)g_ascii_strtod(tmpbuf,NULL);

	tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"data");
	tmpwidget = g_hash_table_lookup(dynamic_widgets,tmpbuf);
	g_return_if_fail(GTK_IS_WIDGET(tmpwidget));
	//	new_reqfuel = gtk_spin_button_get_value(GTK_SPIN_BUTTON(tmpwidget));
	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(tmpwidget),0,-1);
	new_reqfuel = (gfloat)g_ascii_strtod(tmpbuf,NULL);
	percentage = firmware->rf_params[table_num]->req_fuel_total/new_reqfuel;

	firmware->rf_params[table_num]->last_req_fuel_total = firmware->rf_params[table_num]->req_fuel_total;
	firmware->rf_params[table_num]->req_fuel_total = new_reqfuel;
	check_req_fuel_limits(table_num);


	tmpbuf = (gchar *)g_object_get_data(G_OBJECT(widget),"applicable_tables");
	vector = g_strsplit(tmpbuf,",",-1);
	if (!vector)
		return;

	for (x=0;x<g_strv_length(vector);x++)
	{
		table_num = (gint)strtol(vector[x],NULL,10);

		z_base = firmware->table_params[table_num]->z_base;
		x_bins = firmware->table_params[table_num]->x_bincount;
		y_bins = firmware->table_params[table_num]->y_bincount;
		z_page = firmware->table_params[table_num]->z_page;
		data = g_new0(guchar, x_bins*y_bins);

		for (i=z_base;i<(z_base+(x_bins*y_bins));i++)
		{
			if (NULL != (list = ve_widgets[z_page][i]))
			{
				list = g_list_first(list);
				for (j=0;j<g_list_length(list);j++)
				{
					tmpwidget = (GtkWidget *)g_list_nth_data(list,j);
					if (GTK_IS_ENTRY(tmpwidget))
					{
						ign_parm = (gboolean)g_object_get_data(G_OBJECT(tmpwidget),"ign_parm");
						page = (gint)g_object_get_data(G_OBJECT(tmpwidget),"page");
						offset = (gint)g_object_get_data(G_OBJECT(tmpwidget),"offset");
						use_color = (gint)g_object_get_data(G_OBJECT(tmpwidget),"use_color");
						if (g_object_get_data(G_OBJECT(tmpwidget),"raw_upper") != NULL)
							raw_upper = (gint)g_object_get_data(G_OBJECT(tmpwidget),"raw_upper");
						if (g_object_get_data(G_OBJECT(tmpwidget),"raw_lower") != NULL)
							raw_lower = (gint)g_object_get_data(G_OBJECT(tmpwidget),"raw_lower");
						value = ms_data[page][offset];
						value *= percentage;
						if (value < raw_lower)
							value = raw_lower;
						if (value > raw_upper)
							value = raw_upper;

						/* What we are doing is doing the 
						 * forware/reverse conversion which
						 * will give us an exact value if the 
						 * user inputs something in
						 * between,  thus we can reset the 
						 * display to a sane value...
						 */
						old = ms_data[page][offset];
						ms_data[page][offset] = value;

						real_value = convert_after_upload(tmpwidget);
						ms_data[page][offset] = old;

						tmpbuf = g_strdup_printf("%i",(gint)real_value);
						g_signal_handlers_block_by_func (G_OBJECT(tmpwidget),
								G_CALLBACK (entry_changed_handler),
								NULL);
						gtk_entry_set_text(GTK_ENTRY(tmpwidget),tmpbuf);
						g_signal_handlers_unblock_by_func (G_OBJECT(tmpwidget),
								G_CALLBACK (entry_changed_handler),
								NULL);
						g_free(tmpbuf);

						if (!firmware->chunk_support)
							write_ve_const(tmpwidget, page, offset, (gint)value, ign_parm, TRUE);
						data[i] = (guchar)value;
						gtk_widget_modify_text(tmpwidget,GTK_STATE_NORMAL,&black);
						if (use_color)
						{
							color = get_colors_from_hue(((gfloat)value/256.0)*360.0,0.33, 1.0);
							gtk_widget_modify_base(GTK_WIDGET(tmpwidget),GTK_STATE_NORMAL,&color);
						}


					}
				}
			}
		}
		if (firmware->chunk_support)
			chunk_write(z_page,z_base,x_bins*y_bins,data);
	}
	//	io_cmd(IO_UPDATE_VE_CONST,NULL);
	g_strfreev(vector);
	color_changed = TRUE;
	forced_update = TRUE;
}




void draw_ve_marker()
{
	gfloat x_source = 0.0;
	gfloat y_source = 0.0;
	GtkWidget *widget = NULL;
	static GtkWidget ***last_widgets = NULL;
	static gint **last = NULL;
	static GdkColor ** old_colors = NULL;
	static GdkColor color= { 0, 0,16384,16384};
	//extern GdkColor white;
	GdkColor newcolor;
	gfloat value = 0.0;
	extern gint **ms_data;
	GtkRcStyle *style = NULL;
	gint i = 0;
	gint j = 0;
	gint table = 0;
	gint page = 0;
	gint base = 0;
	gint z_bin[4] = {0,0,0,0};
	gint bin[4] = {0,0,0,0};
	gfloat left_w = 0.0;
	gfloat right_w = 0.0;
	gfloat top_w = 0.0;
	gfloat bottom_w = 0.0;
	static gfloat last_z_weight[4] = {0,0,0,0};
	gfloat z_weight[4] = {0,0,0,0};
	gfloat left = 0.0;
	gfloat right = 0.0;
	gfloat top = 0.0;
	gfloat bottom = 0.0;
	gfloat max = 0.0;
	gint heaviest = -1;
	GList *list = NULL;
	static void ***eval;
	extern Firmware_Details *firmware;
	extern gint ** ms_data;
	extern GList ***ve_widgets;
	extern gint *algorithm;
	extern gint active_table;
	extern gboolean forced_update;
	extern gboolean *tracking_focus;
	extern GHashTable *sources_hash;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	GHashTable *hash = NULL;
	MultiSource *multi = NULL;
	enum
	{
		_X_=0,
		_Y_
	};

	if ((active_table < 0 )||(active_table > (firmware->total_tables-1)))
		return;

	if (!eval)
		eval = g_new0(void **, firmware->total_tables);


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
	if (!eval[table])
		eval[table] = g_new0(void *, 2);

	if (firmware->table_params[table]->x_multi_source)
	{
		printf("x_multi_source for table %i\n",table);
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

		eval[table][_X_] = multi->evaluator;
		lookup_current_value(multi->source,&x_source);
	}
	else
	{
		eval[table][_X_] = firmware->table_params[table]->x_eval;
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
			printf("multi is null!!\n");

		eval[table][_Y_] = multi->evaluator;
		lookup_current_value(multi->source,&y_source);
	}
	else
	{
		eval[table][_Y_] = firmware->table_params[table]->y_eval;
		lookup_current_value(firmware->table_params[table]->y_source,&y_source);
	}
	/* Find bin corresponding to current rpm  */
	for (i=0;i<firmware->table_params[table]->x_bincount-1;i++)
	{
		page = firmware->table_params[table]->x_page;
		base = firmware->table_params[table]->x_base;
		if (evaluator_evaluate_x(eval[table][_X_],ms_data[page][base]) >= x_source)
		{
			bin[0] = -1;
			bin[1] = 0;
			left_w = 0;
			right_w = 1;
			break;
		}
		left = evaluator_evaluate_x(eval[table][_X_],ms_data[page][base+i]);
		right = evaluator_evaluate_x(eval[table][_X_],ms_data[page][base+i+1]);

		if ((x_source > left) && (x_source <= right))
		{
			bin[0] = i;
			bin[1] = i+1;

			right_w = (float)(x_source-left)/(float)(right-left);
			left_w = 1.0-right_w;
			break;

		}
		if (x_source > right)
		{
			bin[0] = i+1;
			bin[1] = -1;
			left_w = 1;
			right_w = 0;
		}
	}
	//	printf("left bin %i, right bin %i, left_weight %f, right_weight %f\n",bin[0],bin[1],left_w,right_w);

	for (i=0;i<firmware->table_params[table]->y_bincount-1;i++)
	{
		page = firmware->table_params[table]->y_page;
		base = firmware->table_params[table]->y_base;
		if (evaluator_evaluate_x(eval[table][_Y_],ms_data[page][base]) >= y_source)
		{
			bin[2] = -1;
			bin[3] = 0;
			top_w = 1;
			bottom_w = 0;
			break;
		}
		bottom = evaluator_evaluate_x(eval[table][_Y_],ms_data[page][base+i]);
		top = evaluator_evaluate_x(eval[table][_Y_],ms_data[page][base+i+1]);

		if ((y_source > bottom) && (y_source <= top))
		{
			bin[2] = i;
			bin[3] = i+1;

			top_w = (float)(y_source-bottom)/(float)(top-bottom);
			bottom_w = 1.0-top_w;
			break;

		}
		if (y_source > top)
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

	if (forced_update)
		goto redraw;
	for (i=0;i<4;i++)
	{
		if ((fabs(z_weight[i]-last_z_weight[i]) > 0.03))
			goto redraw;
	}
	for (i=0;i<4;i++)
		last_z_weight[i] = z_weight[i];
	return;

redraw:
	//	printf("bottom bin %i, top bin %i, bottom_weight %f, top_weight %f\n",bin[2],bin[3],bottom_w,top_w);

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
	for (i=0;i<4;i++)
	{
		for (j=0;j<4;j++)
		{
			if ((last[table][i] != z_bin[j] ) && (last_widgets[table][last[table][i]]))
			{
				//for (k=0;i<4;k++)
				//	{
				//			if (z_bin[k] != last[table][i])
				//			{
				//				printf("setting to normal coord %i\n",last[table][i]);
				if (color_changed)
				{
					value = ms_data[firmware->table_params[table]->z_page][firmware->table_params[table]->z_base+z_bin[i]];
					newcolor = get_colors_from_hue(((gfloat)value/256.0)*360.0,0.33, 1.0);
					gtk_widget_modify_base(GTK_WIDGET(last_widgets[table][last[table][i]]),GTK_STATE_NORMAL,&newcolor);
				}
				else
				{
					gtk_widget_modify_base(GTK_WIDGET(last_widgets[table][last[table][i]]),GTK_STATE_NORMAL,&old_colors[table][last[table][i]]);
				}

				last_widgets[table][last[table][i]] = NULL;
			}
		}
	}
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
	for (i=0;i<4;i++)
		last_z_weight[i] = z_weight[i];

	for (i=0;i<4;i++)
	{
		if (z_bin[i] == -1)
			continue;
		list = ve_widgets[firmware->table_params[table]->z_page][firmware->table_params[table]->z_base+z_bin[i]];
		widget = g_list_nth_data(list,0);

		if ((i == heaviest) && (tracking_focus[table]) && (widget != last_widgets[table][z_bin[i]]))
			gtk_widget_grab_focus(widget);

		last_widgets[table][z_bin[i]] = widget;
		last[table][i] = z_bin[i];

		style = gtk_widget_get_modifier_style(widget);

		old_colors[table][z_bin[i]] = style->base[GTK_STATE_NORMAL];
		//	printf("table %i, zbin[%i] %i\n",table,i,z_bin[i]);
		color.red = z_weight[i]*32768 +32767;
		color.green = (1.0-z_weight[i])*65535 +0;
		color.blue = (1.0-z_weight[i])*32768 +0;

		gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
	}

}
