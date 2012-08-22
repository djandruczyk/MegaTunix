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
  \file src/table_builder.c
  \ingroup CoreMtx
  \brief Used to make dynamic sized tables (1 or 2 axis)
  \author David Andruczyk
  */

#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <gui_handlers.h>
#include <stdlib.h>
#include <table_builder.h>
#include <widgetmgmt.h>

extern gconstpointer *global_data;

G_MODULE_EXPORT void table_builder(GtkWidget *parent)
{
	GtkWidget *entry = NULL;
	GtkWidget *topframe = NULL;
	GtkWidget *top = NULL;
	gchar * tmpbuf = NULL;
	Firmware_Details * firmware = NULL;
	gint x = 0;
	gint y = 0;
	gint base = 0;
	gint mult = 0;
	gint rows = 0;
	gint cols = 0;
	gint lower = 0;
	gint upper = 0;
	gint page = 0;
	gint offset = 0;
	gint dl_type = IMMEDIATE;
	gint handler = GENERIC;
	gint precision = 0;
	gboolean use_color = FALSE;
	DataSize size = MTX_U08;
	GList *tab_widgets = NULL;
	GList ***ecu_widgets = NULL;
	Axis axis = (Axis)(GINT)OBJ_GET(parent,"axis");
	gint table_num = (GINT)strtol((gchar *)OBJ_GET(parent,"table_num"),NULL,10);

	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	top = gtk_widget_get_toplevel(parent);
	g_return_if_fail(firmware);
	g_return_if_fail(ecu_widgets);
	g_return_if_fail(top);
	g_return_if_fail((axis == _X_) || (axis == _Y_) || (axis == _Z_));
	g_return_if_fail((table_num >= 0) || (table_num < firmware->total_tables));
	tab_widgets = (GList *)OBJ_GET(top,"tab_widgets");
	//g_return_if_fail(tab_widgets);

	rows = firmware->table_params[table_num]->x_bincount;
	cols = firmware->table_params[table_num]->y_bincount;

	if (axis == _Z_)
	{
		mult = get_multiplier(firmware->table_params[table_num]->z_size);
		base = firmware->table_params[table_num]->z_base;
		page = firmware->table_params[table_num]->z_page;
		size = firmware->table_params[table_num]->z_size;
		lower = firmware->table_params[table_num]->z_raw_lower;
		upper = firmware->table_params[table_num]->z_raw_upper;
		precision = firmware->table_params[table_num]->z_precision;
		use_color = firmware->table_params[table_num]->z_use_color;
		gtk_table_resize(GTK_TABLE(parent),rows,cols);
		for(y=0;y<cols;y++)
		{
			for (x=0;x<rows;x++)
			{
				offset = (((y*rows)+x)*mult)+base;
				entry = gtk_entry_new();
				g_object_set(G_OBJECT(entry),"has-frame",FALSE,"max-length",6,"width-chars",3,"text","0",NULL);
				gtk_widget_add_events(entry,GDK_BUTTON_PRESS|GDK_KEY_PRESS|GDK_KEY_RELEASE|GDK_FOCUS_CHANGE);
				g_signal_connect(G_OBJECT(entry),"activate",G_CALLBACK(std_entry_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"focus_out_event",G_CALLBACK(focus_out_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"insert_text",G_CALLBACK(insert_text_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(entry_changed_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"button_press_event",G_CALLBACK(widget_grab),NULL);
				g_signal_connect(G_OBJECT(entry),"key_press_event",G_CALLBACK(key_event),NULL);
				g_signal_connect(G_OBJECT(entry),"key_release_event",G_CALLBACK(key_event),NULL);
				OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));
				OBJ_SET(entry,"page",GINT_TO_POINTER(page));
				OBJ_SET(entry,"dl_type",GINT_TO_POINTER(dl_type));
				OBJ_SET(entry,"handler",GINT_TO_POINTER(handler));
				OBJ_SET(entry,"size",GINT_TO_POINTER(size));
				OBJ_SET(entry,"precision",GINT_TO_POINTER(precision));
				OBJ_SET(entry,"use_color",GINT_TO_POINTER(use_color));
				OBJ_SET_FULL(entry,"fromecu_mult",g_memdup( firmware->table_params[table_num]->z_fromecu_mult,sizeof(gfloat)),g_free);
				OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",lower),g_free);
				OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",upper),g_free);
				OBJ_SET_FULL(entry,"table_num",g_strdup_printf("%i",table_num),g_free);
				gtk_table_attach(GTK_TABLE(parent),entry,x,x+1,cols-y-1,cols-y,(GtkAttachOptions)GTK_EXPAND|GTK_FILL|GTK_SHRINK,(GtkAttachOptions)0,0,0);
				tab_widgets = g_list_prepend(tab_widgets,entry);
				ecu_widgets[page][offset]= g_list_prepend(                     
						ecu_widgets[page][offset],
						(gpointer)entry);
			}
		}
	}
	OBJ_SET(top,"tab_widgets",tab_widgets);
	gtk_widget_show_all(parent);
}
