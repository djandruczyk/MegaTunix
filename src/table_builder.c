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

#include <debugging.h>
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
	gint max = 0;
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

	ENTER();
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

	rows = firmware->table_params[table_num]->y_bincount;
	cols = firmware->table_params[table_num]->x_bincount;
	max = rows*cols;

	if (axis == _X_)
	{
		mult = get_multiplier(firmware->table_params[table_num]->x_size);
		base = firmware->table_params[table_num]->x_base;
		page = firmware->table_params[table_num]->x_page;
		size = firmware->table_params[table_num]->x_size;
		lower = firmware->table_params[table_num]->x_raw_lower;
		upper = firmware->table_params[table_num]->x_raw_upper;
		if (lower == upper)
		{
			lower = get_extreme_from_size(size,LOWER);
			upper = get_extreme_from_size(size,UPPER);
		}
		precision = firmware->table_params[table_num]->x_precision;
		use_color = firmware->table_params[table_num]->x_use_color;
		gtk_table_resize(GTK_TABLE(parent),1,cols);
		for (x=0;x<cols;x++)
		{
			offset = (x*mult)+base;
			entry = gtk_entry_new();
			tmpbuf = g_strdup_printf("XTable%i_entry_%i_of_%i",table_num,x,cols);
			OBJ_SET_FULL(entry,"fullname",tmpbuf,g_free);
			register_widget(tmpbuf,entry);
			OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));

			g_object_set(G_OBJECT(entry),"has-frame",FALSE,"max-length",6,"width-chars",5,NULL);
			gtk_widget_add_events(entry,GDK_BUTTON_PRESS_MASK|GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_FOCUS_CHANGE_MASK);
			g_signal_connect(G_OBJECT(entry),"activate",G_CALLBACK(std_entry_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"focus_out_event",G_CALLBACK(focus_out_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"insert_text",G_CALLBACK(insert_text_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"button_press_event",G_CALLBACK(widget_grab),NULL);
			g_signal_connect(G_OBJECT(entry),"key_press_event",G_CALLBACK(key_event),NULL);
			g_signal_connect(G_OBJECT(entry),"key_release_event",G_CALLBACK(key_event),NULL);
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));
			OBJ_SET(entry,"page",GINT_TO_POINTER(page));
			OBJ_SET(entry,"canID",GINT_TO_POINTER(firmware->canID));
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(dl_type));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(handler));
			OBJ_SET(entry,"size",GINT_TO_POINTER(size));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(precision));
			OBJ_SET(entry,"use_color",GINT_TO_POINTER(use_color));
			OBJ_SET(entry,"fromecu_mult",firmware->table_params[table_num]->x_fromecu_mult);
			OBJ_SET(entry,"fromecu_add",firmware->table_params[table_num]->x_fromecu_add);
			OBJ_SET_FULL(entry,"source",g_strdup(firmware->table_params[table_num]->x_source),g_free);
			OBJ_SET_FULL(entry,"suffix",g_strdup(firmware->table_params[table_num]->x_suffix),g_free);
			OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",lower),g_free);
			OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",upper),g_free);
			OBJ_SET_FULL(entry,"table_num",g_strdup_printf("%i",table_num),g_free);
			if (firmware->table_params[table_num]->x_multi_source)
			{
				OBJ_SET(entry,"ignore_algorithm",GINT_TO_POINTER(firmware->table_params[table_num]->x_ignore_algorithm));
				OBJ_SET(entry,"multi_source",GINT_TO_POINTER(firmware->table_params[table_num]->x_multi_source));
				OBJ_SET_FULL(entry,"source_key",g_strdup(firmware->table_params[table_num]->x_source_key),g_free);
				OBJ_SET_FULL(entry,"multi_expr_keys",g_strdup(firmware->table_params[table_num]->x_multi_expr_keys),g_free);
				OBJ_SET_FULL(entry,"sources",g_strdup(firmware->table_params[table_num]->x_sources),g_free);
				OBJ_SET_FULL(entry,"suffixes",g_strdup(firmware->table_params[table_num]->x_suffixes),g_free);
				OBJ_SET_FULL(entry,"fromecu_mults",g_strdup(firmware->table_params[table_num]->x_fromecu_mults),g_free);
				OBJ_SET_FULL(entry,"fromecu_adds",g_strdup(firmware->table_params[table_num]->x_fromecu_adds),g_free);
				OBJ_SET_FULL(entry,"lookuptables",g_strdup(firmware->table_params[table_num]->x_lookuptables),g_free);
			}
			/*
			   if (firmware->table_params[table_num]->x_depend_on)
			   {
			   OBJ_SET(entry,"lookuptable",OBJ_GET(firmware->table_params[table_num]->x_object,"lookuptable"));
			   OBJ_SET(entry,"alt_lookuptable",OBJ_GET(firmware->table_params[table_num]->x_object,"alt_lookuptable"));
			   OBJ_SET(entry,"dep_object",OBJ_GET(firmware->table_params[table_num]->x_object,"dep_object"));
			   }*/
			gtk_table_attach(GTK_TABLE(parent),entry,x,x+1,0,1,(GtkAttachOptions)GTK_EXPAND|GTK_FILL|GTK_SHRINK,(GtkAttachOptions)0,0,0);
			tab_widgets = g_list_prepend(tab_widgets,entry);
			ecu_widgets[page][offset]= g_list_prepend(                     
					ecu_widgets[page][offset],
					(gpointer)entry);
		}
	}
	if (axis == _Y_)
	{
		mult = get_multiplier(firmware->table_params[table_num]->y_size);
		base = firmware->table_params[table_num]->y_base;
		page = firmware->table_params[table_num]->y_page;
		size = firmware->table_params[table_num]->y_size;
		lower = firmware->table_params[table_num]->y_raw_lower;
		upper = firmware->table_params[table_num]->y_raw_upper;
		if (lower == upper)
		{
			lower = get_extreme_from_size(size,LOWER);
			upper = get_extreme_from_size(size,UPPER);
		}
		precision = firmware->table_params[table_num]->y_precision;
		use_color = firmware->table_params[table_num]->y_use_color;
		gtk_table_resize(GTK_TABLE(parent),1,cols);
		for (y=0;y<rows;y++)
		{
			offset = (y*mult)+base;
			entry = gtk_entry_new();
			tmpbuf = g_strdup_printf("YTable%i_entry_%i_of_%i",table_num,y,cols);
			OBJ_SET_FULL(entry,"fullname",tmpbuf,g_free);
			register_widget(tmpbuf,entry);
			OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));
			g_object_set(G_OBJECT(entry),"has-frame",FALSE,"max-length",6,"width-chars",5,NULL);
			gtk_widget_add_events(entry,GDK_BUTTON_PRESS_MASK|GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_FOCUS_CHANGE_MASK);
			g_signal_connect(G_OBJECT(entry),"activate",G_CALLBACK(std_entry_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"focus_out_event",G_CALLBACK(focus_out_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"insert_text",G_CALLBACK(insert_text_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(entry_changed_handler),NULL);
			g_signal_connect(G_OBJECT(entry),"button_press_event",G_CALLBACK(widget_grab),NULL);
			g_signal_connect(G_OBJECT(entry),"key_press_event",G_CALLBACK(key_event),NULL);
			g_signal_connect(G_OBJECT(entry),"key_release_event",G_CALLBACK(key_event),NULL);
			OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));
			OBJ_SET(entry,"page",GINT_TO_POINTER(page));
			OBJ_SET(entry,"canID",GINT_TO_POINTER(firmware->canID));
			OBJ_SET(entry,"dl_type",GINT_TO_POINTER(dl_type));
			OBJ_SET(entry,"handler",GINT_TO_POINTER(handler));
			OBJ_SET(entry,"size",GINT_TO_POINTER(size));
			OBJ_SET(entry,"precision",GINT_TO_POINTER(precision));
			OBJ_SET(entry,"use_color",GINT_TO_POINTER(use_color));
			OBJ_SET(entry,"fromecu_mult",firmware->table_params[table_num]->y_fromecu_mult);
			OBJ_SET(entry,"fromecu_add",firmware->table_params[table_num]->y_fromecu_add);
			OBJ_SET_FULL(entry,"source",g_strdup(firmware->table_params[table_num]->y_source),g_free);
			OBJ_SET_FULL(entry,"suffix",g_strdup(firmware->table_params[table_num]->y_suffix),g_free);
			OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",lower),g_free);
			OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",upper),g_free);
			OBJ_SET_FULL(entry,"table_num",g_strdup_printf("%i",table_num),g_free);
			if (firmware->table_params[table_num]->y_multi_source)
			{
				OBJ_SET(entry,"ignore_algorithm",GINT_TO_POINTER(firmware->table_params[table_num]->y_ignore_algorithm));
				OBJ_SET(entry,"multi_source",GINT_TO_POINTER(firmware->table_params[table_num]->y_multi_source));
				OBJ_SET_FULL(entry,"source_key",g_strdup(firmware->table_params[table_num]->y_source_key),g_free);
				OBJ_SET_FULL(entry,"multi_expr_keys",g_strdup(firmware->table_params[table_num]->y_multi_expr_keys),g_free);
				OBJ_SET_FULL(entry,"sources",g_strdup(firmware->table_params[table_num]->y_sources),g_free);
				OBJ_SET_FULL(entry,"suffixes",g_strdup(firmware->table_params[table_num]->y_suffixes),g_free);
				OBJ_SET_FULL(entry,"fromecu_mults",g_strdup(firmware->table_params[table_num]->y_fromecu_mults),g_free);
				OBJ_SET_FULL(entry,"fromecu_adds",g_strdup(firmware->table_params[table_num]->y_fromecu_adds),g_free);
				OBJ_SET_FULL(entry,"lookuptables",g_strdup(firmware->table_params[table_num]->y_lookuptables),g_free);
			}
			/*
			   if (firmware->table_params[table_num]->y_depend_on)
			   {
			   OBJ_SET(entry,"lookuptable",OBJ_GET(firmware->table_params[table_num]->y_object,"lookuptable"));
			   OBJ_SET(entry,"alt_lookuptable",OBJ_GET(firmware->table_params[table_num]->y_object,"alt_lookuptable"));
			   OBJ_SET(entry,"dep_object",OBJ_GET(firmware->table_params[table_num]->y_object,"dep_object"));
			   }*/
			gtk_table_attach(GTK_TABLE(parent),entry,0,1,rows-y-1,rows-y,(GtkAttachOptions)GTK_EXPAND|GTK_SHRINK,(GtkAttachOptions)0,0,0);
			tab_widgets = g_list_prepend(tab_widgets,entry);
			ecu_widgets[page][offset]= g_list_prepend(                     
					ecu_widgets[page][offset],
					(gpointer)entry);
		}
	}
	if (axis == _Z_)
	{
		mult = get_multiplier(firmware->table_params[table_num]->z_size);
		base = firmware->table_params[table_num]->z_base;
		page = firmware->table_params[table_num]->z_page;
		size = firmware->table_params[table_num]->z_size;
		lower = firmware->table_params[table_num]->z_raw_lower;
		upper = firmware->table_params[table_num]->z_raw_upper;
		if (lower == upper)
		{
			lower = get_extreme_from_size(size,LOWER);
			upper = get_extreme_from_size(size,UPPER);
		}
		precision = firmware->table_params[table_num]->z_precision;
		use_color = firmware->table_params[table_num]->z_use_color;
		gtk_table_resize(GTK_TABLE(parent),rows,cols);
		for(y=0;y<rows;y++)
		{
			for (x=0;x<cols;x++)
			{
				offset = (((y*cols)+x)*mult)+base;
				entry = gtk_entry_new();
				tmpbuf = g_strdup_printf("Table%i_entry_%i_of_%i",table_num,((y*cols)+x),max);
				OBJ_SET_FULL(entry,"fullname",tmpbuf,g_free);
				register_widget(tmpbuf,entry);
				OBJ_SET(entry,"last_value",GINT_TO_POINTER(-G_MAXINT));
				g_object_set(G_OBJECT(entry),"has-frame",FALSE,"max-length",6,"width-chars",5,NULL);
				gtk_widget_add_events(entry,GDK_BUTTON_PRESS_MASK|GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_FOCUS_CHANGE_MASK);
				g_signal_connect(G_OBJECT(entry),"activate",G_CALLBACK(std_entry_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"focus_out_event",G_CALLBACK(focus_out_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"insert_text",G_CALLBACK(insert_text_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(entry_changed_handler),NULL);
				g_signal_connect(G_OBJECT(entry),"button_press_event",G_CALLBACK(widget_grab),NULL);
				g_signal_connect(G_OBJECT(entry),"key_press_event",G_CALLBACK(key_event),NULL);
				g_signal_connect(G_OBJECT(entry),"key_release_event",G_CALLBACK(key_event),NULL);
				OBJ_SET(entry,"offset",GINT_TO_POINTER(offset));
				OBJ_SET(entry,"page",GINT_TO_POINTER(page));
				OBJ_SET(entry,"canID",GINT_TO_POINTER(firmware->canID));
				OBJ_SET(entry,"dl_type",GINT_TO_POINTER(dl_type));
				OBJ_SET(entry,"handler",GINT_TO_POINTER(handler));
				OBJ_SET(entry,"size",GINT_TO_POINTER(size));
				OBJ_SET(entry,"precision",GINT_TO_POINTER(precision));
				OBJ_SET(entry,"use_color",GINT_TO_POINTER(use_color));
				OBJ_SET(entry,"fromecu_mult",firmware->table_params[table_num]->z_fromecu_mult);
				OBJ_SET(entry,"fromecu_add",firmware->table_params[table_num]->z_fromecu_add);
				OBJ_SET_FULL(entry,"source",g_strdup(firmware->table_params[table_num]->z_source),g_free);
				OBJ_SET_FULL(entry,"suffix",g_strdup(firmware->table_params[table_num]->z_suffix),g_free);
				OBJ_SET_FULL(entry,"raw_lower",g_strdup_printf("%i",lower),g_free);
				OBJ_SET_FULL(entry,"raw_upper",g_strdup_printf("%i",upper),g_free);
				OBJ_SET_FULL(entry,"table_num",g_strdup_printf("%i",table_num),g_free);
				if (firmware->table_params[table_num]->z_multi_source)
				{
					OBJ_SET(entry,"ignore_algorithm",GINT_TO_POINTER(firmware->table_params[table_num]->z_ignore_algorithm));
					OBJ_SET(entry,"multi_source",GINT_TO_POINTER(firmware->table_params[table_num]->z_multi_source));
					OBJ_SET_FULL(entry,"source_key",g_strdup(firmware->table_params[table_num]->z_source_key),g_free);
					OBJ_SET_FULL(entry,"multi_expr_keys",g_strdup(firmware->table_params[table_num]->z_multi_expr_keys),g_free);
					OBJ_SET_FULL(entry,"sources",g_strdup(firmware->table_params[table_num]->z_sources),g_free);
					OBJ_SET_FULL(entry,"suffixes",g_strdup(firmware->table_params[table_num]->z_suffixes),g_free);
					OBJ_SET_FULL(entry,"fromecu_mults",g_strdup(firmware->table_params[table_num]->z_fromecu_mults),g_free);
					OBJ_SET_FULL(entry,"fromecu_adds",g_strdup(firmware->table_params[table_num]->z_fromecu_adds),g_free);
					OBJ_SET_FULL(entry,"lookuptables",g_strdup(firmware->table_params[table_num]->z_lookuptables),g_free);
				}
				if (firmware->table_params[table_num]->z_depend_on)
				{
					OBJ_SET(entry,"lookuptable",OBJ_GET(firmware->table_params[table_num]->z_object,"lookuptable"));
					OBJ_SET(entry,"alt_lookuptable",OBJ_GET(firmware->table_params[table_num]->z_object,"alt_lookuptable"));
					OBJ_SET(entry,"dep_object",OBJ_GET(firmware->table_params[table_num]->z_object,"dep_object"));
				}
				gtk_table_attach(GTK_TABLE(parent),entry,x,x+1,rows-y-1,rows-y,(GtkAttachOptions)GTK_EXPAND|GTK_FILL|GTK_SHRINK,(GtkAttachOptions)0,0,0);
				tab_widgets = g_list_prepend(tab_widgets,entry);
				ecu_widgets[page][offset]= g_list_prepend(                     
						ecu_widgets[page][offset],
						(gpointer)entry);
			}
		}
	}
	OBJ_SET(top,"tab_widgets",tab_widgets);
	gtk_widget_show_all(parent);
	EXIT();
	return;
}
