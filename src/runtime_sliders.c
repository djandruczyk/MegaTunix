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
#include <runtime_sliders.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>

GHashTable *rt_sliders = NULL;
GHashTable *ww_sliders = NULL;
static GtkSizeGroup *size_group_left = NULL;
static GtkSizeGroup *size_group_right = NULL;

void load_sliders()
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
	extern gboolean tabs_loaded;
	extern gboolean rtvars_loaded;

	if (!tabs_loaded)
		return;
	if (!rtvars_loaded)
	{
		dbg_func(__FILE__": load_sliders()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n",CRITICAL);
		return;
	}

	filename = get_file(g_strconcat(RTSLIDERS_DIR,"/",firmware->sliders_map_file,".rts_conf",NULL));
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(!cfg_read_int(cfgfile,"global","rt_total_sliders",&count))
			dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t could NOT read \"rt_total_sliders\" value from\n\t file \"%s\"\n",filename),CRITICAL);
		size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		for (i=0;i<count;i++)
		{
			row = -1;
			table = -1;
			section = g_strdup_printf("rt_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"slider_name",&ctrl_name))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"slider_name\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"table",&table))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_int(cfgfile,section,"row",&row))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_string(cfgfile,section,"source",&source))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);

			add_slider(ctrl_name,table,row,source,RUNTIME_PAGE);
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
		// Now warmup wizard.....
		if (!cfg_read_int(cfgfile,"global","ww_total_sliders",&count))
			dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t could NOT read \"ww_total_sliders\" value from\n\t file \"%s\"\n",filename),CRITICAL);
		for (i=0;i<count;i++)
		{
			row = -1;
			table = -1;
			section = g_strdup_printf("ww_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"slider_name",&ctrl_name))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"slider_name\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if(!cfg_read_int(cfgfile,section,"table",&table))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_int(cfgfile,section,"row",&row))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_string(cfgfile,section,"source",&source))
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);

			add_slider(ctrl_name,table,row,source,WARMUP_WIZ_PAGE);
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
		cfg_free(cfgfile);
	}
	else
		dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Filename \"%s\" NOT FOUND Critical error!!\n\n",filename),CRITICAL);
	g_free(filename);
}

void add_slider(gchar *ctrl_name, gint tbl, gint row, gchar *source, PageIdent ident)
{
	struct Rt_Slider *slider = NULL;
	GtkWidget *label = NULL;
	GtkWidget *pbar = NULL;
	GtkWidget *table = NULL;
	GtkWidget *hbox = NULL;
	gchar * name = NULL;
	extern GHashTable *dynamic_widgets;
	extern struct RtvMap *rtv_map;
	GObject *object = NULL;

	slider = g_malloc0(sizeof(struct Rt_Slider));

	if (!rt_sliders)
		rt_sliders = g_hash_table_new(NULL,NULL);
	if (!ww_sliders)
		ww_sliders = g_hash_table_new(NULL,NULL);

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		dbg_func(g_strdup_printf(__FILE__": add_slider()\n\tBad things man, object doesn't exist for %s\n",source),CRITICAL);
		return;
	}

	slider->ctrl_name = g_strdup(ctrl_name);
	slider->tbl = tbl;
	slider->row = row;
	slider->friendly_name = (gchar *) g_object_get_data(object,"dlog_gui_name");
	slider->lower = (gint)g_object_get_data(object,"lower_limit");

	slider->upper = (gint)g_object_get_data(object,"upper_limit");
	slider->history = (gfloat *) g_object_get_data(object,"history");
	slider->object = object;

	if (ident == RUNTIME_PAGE)
		name = g_strdup_printf("runtime_rt_table%i",slider->tbl);
	else if (ident == WARMUP_WIZ_PAGE)
		name = g_strdup_printf("ww_rt_table%i",slider->tbl);
	else
	{
		dbg_func(g_strdup_printf(__FILE__": add_slider()\n\tpage ident passed is not handled, ERROR, widget add aborted\n"),CRITICAL);
		return;
	}
	table = g_hash_table_lookup(dynamic_widgets,name);
	if (!table)
	{
		dbg_func(g_strdup_printf(__FILE__": add_slider()\n\t table \"%s\" was not found, RuntimeSlider map or runtime datamap has a typo\n",name),CRITICAL);
		g_free(name);
		return;
	}
	g_free(name);
	hbox  = gtk_hbox_new(FALSE,10);

	label = gtk_label_new(NULL);
	slider->label = label;
	gtk_label_set_markup(GTK_LABEL(label),g_strdup(slider->friendly_name));
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	//gtk_table_attach (GTK_TABLE (table),label,
	//		0,1,slider->row,(slider->row)+1,
	//		(GtkAttachOptions) (GTK_FILL),
	//		(GtkAttachOptions) (GTK_FILL), 0, 0);

	label = gtk_label_new(NULL);
	slider->textval = label;
	gtk_misc_set_alignment(GTK_MISC(label),1,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	if (ident == RUNTIME_PAGE)
	{
		if (((tbl+1) % 2) == 0)
			gtk_size_group_add_widget(size_group_right,hbox);
		else
			gtk_size_group_add_widget(size_group_left,hbox);
	}

	gtk_table_attach (GTK_TABLE (table),hbox,
			0,2,slider->row,(slider->row)+1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 0, 0);

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			GTK_PROGRESS_LEFT_TO_RIGHT);
	
	gtk_table_attach (GTK_TABLE (table),pbar,
			2,3,slider->row,(slider->row)+1,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 0, 0);
	slider->pbar = pbar;

	slider->parent = table;
	gtk_widget_show_all(slider->parent);

	if (ident == RUNTIME_PAGE)
	{

		if (g_hash_table_lookup(rt_sliders,g_strdup(ctrl_name))==NULL)
			g_hash_table_insert(rt_sliders,g_strdup(ctrl_name),
					(gpointer)slider);
	}
	else if (ident == WARMUP_WIZ_PAGE)
	{
		if (g_hash_table_lookup(ww_sliders,g_strdup(ctrl_name))==NULL)
			g_hash_table_insert(ww_sliders,g_strdup(ctrl_name),
					(gpointer)slider);
	}
	return;
}
