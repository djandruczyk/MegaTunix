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


#include <configfile.h>
#include <debugging.h>
#include <getfiles.h>
#include <runtime_controls.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>

GHashTable *rt_controls = NULL;
GHashTable *ww_controls = NULL;

void load_controls()
{
	ConfigFile *cfgfile = NULL;
	extern struct Firmware_Details *firmware;
	gchar *filename = NULL;
	gint count = 0;
	gint table = 0;
	gint row = 0;
	gchar *ctrl_name = NULL;
	gchar *source = NULL;
	gchar *section = NULL;
	gint i = 0;

	filename = get_file(g_strconcat(RTSLIDERS_DIR,"/",firmware->controls_map_file,".rts_conf",NULL));
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		cfg_read_int(cfgfile,"global","rt_total_sliders",&count);
		for (i=0;i<count;i++)
		{
			row = -1;
			table = -1;
			section = g_strdup_printf("rt_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"control_name",&ctrl_name))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"control_name\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"table",&table))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_int(cfgfile,section,"row",&row))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_string(cfgfile,section,"source",&source))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);

			add_control(ctrl_name,table,row,source,RUNTIME_PAGE);
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
		// Now warmup wizard.....
		cfg_read_int(cfgfile,"global","ww_total_sliders",&count);
		for (i=0;i<count;i++)
		{
			row = -1;
			table = -1;
			section = g_strdup_printf("ww_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"control_name",&ctrl_name))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"control_name\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"table",&table))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_int(cfgfile,section,"row",&row))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_string(cfgfile,section,"source",&source))
				dbg_func(g_strdup_printf(__FILE__": load_controls()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);

			add_control(ctrl_name,table,row,source,WARMUP_WIZ_PAGE);
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
	}
	cfg_free(cfgfile);
	g_free(filename);
}

void add_control(gchar *ctrl_name, gint tbl, gint row, gchar *source, PageIdent ident)
{
	struct Rt_Control *control = NULL;
	GtkWidget *label = NULL;
	GtkWidget *pbar = NULL;
	GtkWidget *table = NULL;
	gchar * name = NULL;
	extern GHashTable *dynamic_widgets;
	extern struct RtvMap *rtv_map;
	GObject *object = NULL;

	control = g_malloc0(sizeof(struct Rt_Control));

	if (!rt_controls)
		rt_controls = g_hash_table_new(NULL,NULL);
	if (!ww_controls)
		ww_controls = g_hash_table_new(NULL,NULL);

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		dbg_func(g_strdup_printf(__FILE__": add_control()\n\tBad things man, object doesn't exist for %s\n",source),CRITICAL);
		return;
	}

	control->ctrl_name = g_strdup(ctrl_name);
	control->tbl = tbl;
	control->row = row;
	control->friendly_name = (gchar *) g_object_get_data(object,"dlog_gui_name");
	control->lower = (gint)g_object_get_data(object,"lower_limit");

	control->upper = (gint)g_object_get_data(object,"upper_limit");
	control->history = (gfloat *) g_object_get_data(object,"history");
	control->object = object;

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),g_strdup(control->friendly_name));
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	if (ident == RUNTIME_PAGE)
		name = g_strdup_printf("runtime_rt_table%i",control->tbl);
	else if (ident == WARMUP_WIZ_PAGE)
		name = g_strdup_printf("ww_rt_table%i",control->tbl);
	else
	{
		dbg_func(g_strdup_printf(__FILE__": add_control()\n\tpage ident passed is not handled, ERROR, widget add aborted\n"),CRITICAL);
		return;
	}
	table = g_hash_table_lookup(dynamic_widgets,name);
	if (!table)
	{
		dbg_func(g_strdup_printf(__FILE__": add_control()\n\t table \"%s\" was not found, RuntimeSlider map or runtime datamap has a typo\n",name),CRITICAL);
		g_free(name);
		return;
	}
	g_free(name);
	gtk_table_attach (GTK_TABLE (table),label,
			0,1,control->row,(control->row)+1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);
	control->label = label;

	label = gtk_label_new(NULL);
	gtk_widget_set_size_request(label,55,-1);
	gtk_table_attach (GTK_TABLE (table),label,
			1,2,control->row,(control->row)+1,
			(GtkAttachOptions) (GTK_SHRINK),
			(GtkAttachOptions) (GTK_FILL|GTK_SHRINK), 0, 0);
	control->textval = label;

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_table_attach (GTK_TABLE (table),pbar,
			2,3,control->row,(control->row)+1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 0, 0);
	control->pbar = pbar;

	control->parent = table;
	gtk_widget_show_all(control->parent);

	if (ident == RUNTIME_PAGE)
	{

		if (g_hash_table_lookup(rt_controls,g_strdup(ctrl_name))==NULL)
			g_hash_table_insert(rt_controls,g_strdup(ctrl_name),
					(gpointer)control);
	}
	else if (ident == WARMUP_WIZ_PAGE)
	{
		if (g_hash_table_lookup(ww_controls,g_strdup(ctrl_name))==NULL)
			g_hash_table_insert(ww_controls,g_strdup(ctrl_name),
					(gpointer)control);
	}
	return;
}
