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
  \file src/vetable_gui.c
  \ingroup CoreMtx
  \brief VEtable scaler routine, which is semi-ECU specific and suited to be
  migrated to a plugin at some future point
  \author David Andruczyk
  */

#include <debugging.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <plugin.h>
#include <stdio.h>
#include <vetable_gui.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;

/*!
  \brief rescale_table() is called to rescale a subset of a Table (doesn't
  matter what kind of table). 
  \param widget is the pointer to widget containing the table number 
  we care about.
  */
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
	GList ***ecu_widgets = NULL;

	ENTER();
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	tmpbuf = (gchar *) OBJ_GET(widget,"data");
	vector = g_strsplit(tmpbuf,",",-1);

	scaler = lookup_widget(vector[0]);
	g_return_if_fail(GTK_IS_WIDGET(scaler));
	math_combo = lookup_widget(vector[1]);
	g_return_if_fail(GTK_IS_WIDGET(math_combo));
	g_strfreev(vector);

	tmpbuf = (gchar *)OBJ_GET(scaler,"table_num");
	table_num = (GINT)g_ascii_strtod(tmpbuf,NULL);

	z_base = firmware->table_params[table_num]->z_base;
	x_bins = firmware->table_params[table_num]->x_bincount;
	y_bins = firmware->table_params[table_num]->y_bincount;
	z_page = firmware->table_params[table_num]->z_page;

	tmpbuf = gtk_editable_get_chars(GTK_EDITABLE(scaler),0,-1);
	factor = (gfloat)g_ascii_strtod(g_strdelimit(tmpbuf,",.",'.'),NULL);
	g_free(tmpbuf);
	scaleop = (ScaleOp)gtk_combo_box_get_active(GTK_COMBO_BOX(math_combo));

	for (i=z_base;i<(z_base+(x_bins*y_bins));i++)
	{
		if (NULL != (list = ecu_widgets[z_page][i]))
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
	EXIT();
	return;
}


/*!
  \brief rescales the input based onthe factor and scale operation
  \param input is the input value
  \param scaleop is the enumeration of what we are doing math wise.
  \param factor is the other side of our math equation
  \returns the result of the math
  */
G_MODULE_EXPORT gfloat rescale(gfloat input, ScaleOp scaleop, gfloat factor)
{
	ENTER();
	switch (scaleop)
	{
		case ADD:
			EXIT();
			return input+factor;
			break;
		case SUBTRACT:
			EXIT();
			return input-factor;
			break;
		case MULTIPLY:
			EXIT();
			return input*factor;
			break;
		case DIVIDE:
			EXIT();
			return input/factor;
			break;
		case EQUAL:
			EXIT();
			return factor;
			break;
		default:
			printf(_("!!! ERROR !!!, rescaler passed invalid enum\n"));
			break;
	}
	EXIT();
	return 0;
}


/*!
  \brief Draws the VE marker on the table via calling the appropriate plugin
  */
G_MODULE_EXPORT gboolean draw_ve_marker(void)
{
	static void (*common_draw_ve_marker)(void) = NULL;

	/*printf("draw_ve_marker\n");*/
	ENTER();
	if (!common_draw_ve_marker)
		get_symbol("common_draw_ve_marker",(void **)&common_draw_ve_marker);
	g_return_val_if_fail(common_draw_ve_marker,FALSE);
	common_draw_ve_marker();
	EXIT();
	return FALSE;
}
