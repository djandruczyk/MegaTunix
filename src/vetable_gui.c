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
#include <rtv_processor.h>
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
void rescale_table(gchar * widget_name)
{
	extern struct Firmware_Details *firmware;
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
	GtkWidget *widget = NULL;
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
	extern GdkColor black;
	gboolean use_color = FALSE;

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
				widget = (GtkWidget *)g_list_nth_data(list,j);
				if ((gboolean)g_object_get_data(G_OBJECT(widget),"marked"))
				{
					ign_parm = (gboolean)g_object_get_data(G_OBJECT(widget),"ign_parm");
					page = (gint)g_object_get_data(G_OBJECT(widget),"page");
					offset = (gint)g_object_get_data(G_OBJECT(widget),"offset");
					use_color = (gint)g_object_get_data(G_OBJECT(widget),"use_color");
					if (g_object_get_data(G_OBJECT(widget),"raw_upper") != NULL)
						raw_upper = (gint)g_object_get_data(G_OBJECT(widget),"raw_upper");
					if (g_object_get_data(G_OBJECT(widget),"raw_lower") != NULL)
						raw_lower = (gint)g_object_get_data(G_OBJECT(widget),"raw_lower");
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

					real_value = convert_after_upload(widget);
					ms_data[page][offset] = old;

					tmpbuf = g_strdup_printf("%i",(gint)real_value);
					gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
					g_free(tmpbuf);

					write_ve_const(widget, page, offset, (gint)value, ign_parm, TRUE);
					gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
					widget_grab(widget,NULL,GINT_TO_POINTER(TRUE));
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(scaler),100.0);
					if (use_color)
					{
						color = get_colors_from_hue(((gfloat)value/256.0)*360.0,0.33, 1.0);
						gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
					}


				}
			}
		}
	}
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
	GList *list = NULL;
	extern struct Firmware_Details *firmware;
	static void ***eval;
	extern gint ** ms_data;
	extern GList ***ve_widgets;
	extern gint *algorithm;
	extern gint active_table;


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
	if ((algorithm[table] == ALPHA_N) && (firmware->table_params[table]->an_x_source))
		lookup_current_value(firmware->table_params[table]->an_x_source,&x_source);
	else
		lookup_current_value(firmware->table_params[table]->x_source,&x_source);

	if ((!eval[table][0]) && (firmware->table_params[table]->x_conv_expr))
	{
		eval[table][0] = evaluator_create(firmware->table_params[table]->x_conv_expr);
	}
	if ((!eval[table][1]) && (firmware->table_params[table]->y_conv_expr))
	{
		eval[table][1] = evaluator_create(firmware->table_params[table]->y_conv_expr);
	}

	/* Find bin corresponding to current rpm  */
	for (i=0;i<firmware->table_params[table]->x_bincount-1;i++)
	{
		page = firmware->table_params[table]->x_page;
		base = firmware->table_params[table]->x_base;
		if (evaluator_evaluate_x(eval[table][0],ms_data[page][base]) >= x_source)
		{
			bin[0] = -1;
			bin[1] = 0;
			left_w = 1;
			right_w = 1;
			break;
		}
		left = evaluator_evaluate_x(eval[table][0],ms_data[page][base+i]);
		right = evaluator_evaluate_x(eval[table][0],ms_data[page][base+i+1]);

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
			right_w = 1;
		}
	}
//	printf("left bin %i, right bin %i, left_weight %f, right_weight %f\n",bin[0],bin[1],left_w,right_w);

	if ((algorithm[table] == ALPHA_N) && (firmware->table_params[table]->an_y_source))
		lookup_current_value(firmware->table_params[table]->an_y_source,&y_source);
	else
		lookup_current_value(firmware->table_params[table]->y_source,&y_source);
	for (i=0;i<firmware->table_params[table]->y_bincount-1;i++)
	{
		page = firmware->table_params[table]->y_page;
		base = firmware->table_params[table]->y_base;
		if (evaluator_evaluate_x(eval[table][1],ms_data[page][base]) >= y_source)
		{
			bin[2] = -1;
			bin[3] = 0;
			top_w = 1;
			bottom_w = 1;
			break;
		}
		bottom = evaluator_evaluate_x(eval[table][1],ms_data[page][base+i]);
		top = evaluator_evaluate_x(eval[table][1],ms_data[page][base+i+1]);

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
			top_w = 1;
			bottom_w = 1;
		}
	}
	z_weight[0] = left_w*bottom_w;
	z_weight[1] = left_w*top_w;
	z_weight[2] = right_w*bottom_w;
	z_weight[3] = right_w*top_w;

	/* Cheap hack to see if things changed, if not don't waste CPU time inside
	 * of pango changing the color unnecessarily
	 * */
	for (i=0;i<4;i++)
	{
		if (fabs(z_weight[i]-last_z_weight[i]) > 0.03)
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
		z_bin[0] = firmware->table_params[table]->z_base+bin[0]+(bin[2]*firmware->table_params[table]->x_bincount);
	if ((bin[0] == -1) || (bin[3] == -1))
		z_bin[1] = -1;
	else
		z_bin[1] = firmware->table_params[table]->z_base+bin[0]+(bin[3]*firmware->table_params[table]->x_bincount);
	if ((bin[1] == -1) || (bin[2] == -1))
		z_bin[2] = -1;
	else
		z_bin[2]= firmware->table_params[table]->z_base+bin[1]+(bin[2]*firmware->table_params[table]->x_bincount);
	if ((bin[1] == -1) || (bin[3] == -1))
		z_bin[3] = -1;
	else
		z_bin[3]= firmware->table_params[table]->z_base+bin[1]+(bin[3]*firmware->table_params[table]->x_bincount);
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
				gtk_widget_modify_base(GTK_WIDGET(last_widgets[table][last[table][i]]),GTK_STATE_NORMAL,&old_colors[table][last[table][i]]);
				last_widgets[table][last[table][i]] = NULL;
				//	}
				//}
			}
		}
	}

	for (i=0;i<4;i++)
	{
		if (z_bin[i] == -1)
			continue;
		list = ve_widgets[firmware->table_params[table]->z_page][z_bin[i]];
		widget = g_list_nth_data(list,0);


		last_widgets[table][z_bin[i]] = widget;
		last[table][i] = z_bin[i];

		style = gtk_widget_get_modifier_style(widget);

		old_colors[table][z_bin[i]] = style->base[GTK_STATE_NORMAL];
		color.red = z_weight[i]*32768 +32767;
		color.green = z_weight[i]*32768 +32767;
		color.blue = z_weight[i]*32768 +32767;
		gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
		last_z_weight[i] = z_weight[i];
	}

}
