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


#include <apicheck.h>
#include <api-versions.h>
#include <configfile.h>
#include <debugging.h>
#include <getfiles.h>
#include <glade/glade-xml.h>
#include <glib.h>
#include <runtime_sliders.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>
#include <widgetmgmt.h>

GHashTable *rt_sliders = NULL;
GHashTable *enr_sliders = NULL;
GHashTable *ww_sliders = NULL;
GHashTable **ve3d_sliders = NULL;
static GtkSizeGroup *size_group_left = NULL;
static GtkSizeGroup *size_group_right = NULL;
extern gint dbg_lvl;


/*!
 \brief load_sliders() is called to load up the runtime slider configurations
 from the file specified in the firmware's interrogation profile, and populate
 the gui wiht the newly created sliders.
 */
void load_sliders()
{
	ConfigFile *cfgfile = NULL;
	Rt_Slider *slider = NULL;
	extern Firmware_Details *firmware;
	extern volatile gboolean leaving;
	gchar *filename = NULL;
	gint count = 0;
	gint table = 0;
	gint row = 0;
	gint major = 0;
	gint minor = 0;
	gchar *ctrl_name = NULL;
	gchar *source = NULL;
	gchar *section = NULL;
	gint i = 0;
	extern gboolean tabs_loaded;
	extern gboolean rtvars_loaded;

	if ((!tabs_loaded) || (!firmware) || (leaving))
		return;
	if ((rtvars_loaded == FALSE) || (tabs_loaded == FALSE))
	{
		if (ww_sliders)
			ww_sliders = NULL;
		if (rt_sliders)
			rt_sliders = NULL;

		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": load_sliders()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}
	if (!rt_sliders)
		rt_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	if (!ww_sliders)
		ww_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);

	filename = get_file(g_strconcat(RTSLIDERS_DATA_DIR,PSEP,firmware->sliders_map_file,NULL),g_strdup("rts_conf"));
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		get_file_api(cfgfile,&major,&minor);
		if ((major != RT_SLIDERS_MAJOR_API) || (minor != RT_SLIDERS_MINOR_API))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\tRuntime Sliders profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n",major,minor,RT_SLIDERS_MAJOR_API,RT_SLIDERS_MINOR_API,filename));
			g_free(filename);
			return;
		}
		if(!cfg_read_int(cfgfile,"global","rt_total_sliders",&count))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t could NOT read \"rt_total_sliders\" value from\n\t file \"%s\"\n",filename));
			goto do_ww_sliders;
		}
		size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		for (i=0;i<count;i++)
		{
			slider = NULL;
			row = -1;
			table = -1;
			section = g_strdup_printf("rt_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"slider_name",&ctrl_name))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"slider_name\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if(!cfg_read_int(cfgfile,section,"table",&table))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_int(cfgfile,section,"row",&row))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_string(cfgfile,section,"source",&source))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename));
			}

			slider = add_slider(ctrl_name,table,0,row,source,RUNTIME_TAB);
			if (slider)
			{
				if (g_hash_table_lookup(rt_sliders,ctrl_name)==NULL)
					g_hash_table_insert(rt_sliders,g_strdup(ctrl_name),(gpointer)slider);
			}
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
		// Now warmup wizard.....
do_ww_sliders:
		if (!cfg_read_int(cfgfile,"global","ww_total_sliders",&count))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t could NOT read \"ww_total_sliders\" value from\n\t file \"%s\"\n",filename));
			goto finish_off;
		}
		for (i=0;i<count;i++)
		{
			slider = NULL;
			row = -1;
			table = -1;
			section = g_strdup_printf("ww_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"slider_name",&ctrl_name))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"slider_name\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if(!cfg_read_int(cfgfile,section,"table",&table))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_int(cfgfile,section,"row",&row))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_string(cfgfile,section,"source",&source))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename));
			}

			slider = add_slider(ctrl_name,table,0,row,source,WARMUP_WIZ_TAB);
			if (slider)
			{
				if (g_hash_table_lookup(ww_sliders,ctrl_name)==NULL)
					g_hash_table_insert(ww_sliders,g_strdup(ctrl_name),(gpointer)slider);
			}
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
finish_off:
		cfg_free(cfgfile);
		g_free(cfgfile);
	}
	else
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Filename \"%s\" NOT FOUND Critical error!!\n\n",filename));
	}
	g_free(filename);
}


/*!
 \brief load_ve3d_sliders() is called from 3d_vetable.c to load up the sliders
 specific to the 3D Table views. 
 \param table_num (gint) the table number passed to load sliders for
 */
void load_ve3d_sliders(gint table_num)
{
	ConfigFile *cfgfile = NULL;
	Rt_Slider *slider = NULL;
	extern Firmware_Details *firmware;
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

	if ((rtvars_loaded == FALSE) || (tabs_loaded == FALSE))
	{
		if (ve3d_sliders)
			ve3d_sliders[table_num]=NULL;
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": load_sliders()\n\tCRITICAL ERROR, Tabs not loaded OR Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}

	if (!ve3d_sliders)
		ve3d_sliders = g_new0(GHashTable *,firmware->total_tables);

	if (!ve3d_sliders[table_num])
		ve3d_sliders[table_num] = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);

	filename = get_file(g_strconcat(RTSLIDERS_DATA_DIR,PSEP,firmware->sliders_map_file,NULL),g_strdup("rts_conf"));
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		size_group_left = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		size_group_right = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

		if (!cfg_read_int(cfgfile,"global","ve3d_total_sliders",&count))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t could NOT read \"ve3d_total_sliders\" value from\n\t file \"%s\"\n",filename));
			goto finish_off;
		}
		for (i=0;i<count;i++)
		{
			slider = NULL;
			row = -1;
			table = -1;
			section = g_strdup_printf("ve3d_slider_%i",i);
			if(!cfg_read_string(cfgfile,section,"slider_name",&ctrl_name))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"slider_name\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if(!cfg_read_int(cfgfile,section,"table",&table))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"table\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_int(cfgfile,section,"row",&row))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"row\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_string(cfgfile,section,"source",&source))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_sliders()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename));
			}

			slider = add_slider(ctrl_name,table,table_num,row,source,VE3D_VIEWER_TAB);
			if (slider)
			{
				if (g_hash_table_lookup(ve3d_sliders[table_num],ctrl_name)==NULL)
					g_hash_table_insert(ve3d_sliders[table_num],g_strdup(ctrl_name),(gpointer)slider);
			}
					
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
finish_off:
		cfg_free(cfgfile);
		g_free(cfgfile);
	}
	else
		g_free(filename);
}


/*!
 \brief add_slider() creates the slider from the passed data, and attaches
 it the the gui.
 \param ctrl_name (gchar *) name of the slider as defined in the config file
 \param tbl (gint) table number to bind this slider to
 \param table_num (gint) the table_num from the firmware that this slider is
 bound to. (used for the sliders on the 3D view)
 \param row (gint) row of the table (tbl) that this slider goes on
 \param source (gchar *) data source for this slider 
 \param ident (TabIdent) enumeration of the page this slider goes on
 \returns a Struct Rt_Slider *
 */
Rt_Slider *  add_slider(gchar *ctrl_name, gint tbl, gint table_num, gint row, gchar *source, TabIdent ident)
{
	Rt_Slider *slider = NULL;
	GtkWidget *label = NULL;
	GtkWidget *pbar = NULL;
	GtkWidget *table = NULL;
	GtkWidget *hbox = NULL;
	gchar * name = NULL;
	extern GHashTable *dynamic_widgets;
	extern Rtv_Map *rtv_map;
	GObject *object = NULL;

	slider = g_malloc0(sizeof(Rt_Slider));

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": add_slider()\n\tBad things man, object doesn't exist for %s\n",source));
		return NULL;
	}

	slider->ctrl_name = g_strdup(ctrl_name);
	slider->tbl = tbl;
	slider->table_num = table_num;
	slider->row = row;
	slider->class = MTX_PROGRESS;
	slider->friendly_name = (gchar *) g_object_get_data(object,"dlog_gui_name");
	slider->lower = (gint)g_object_get_data(object,"lower_limit");

	slider->upper = (gint)g_object_get_data(object,"upper_limit");
	slider->history = (GArray *) g_object_get_data(object,"history");
	slider->object = object;

	if (ident == RUNTIME_TAB)
		name = g_strdup_printf("runtime_rt_table%i",slider->tbl);
	else if (ident == WARMUP_WIZ_TAB)
		name = g_strdup_printf("ww_rt_table%i",slider->tbl);
	else if (ident == VE3D_VIEWER_TAB)
		name = g_strdup_printf("ve3d_rt_table%i_%i",slider->tbl,slider->table_num);
	else
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": add_slider()\n\tpage ident passed is not handled, ERROR, widget add aborted\n"));
		return NULL;
	}
	table = g_hash_table_lookup(dynamic_widgets,name);
	if (!table)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": add_slider()\n\t table \"%s\" was not found, RuntimeSlider map or runtime datamap has a typo\n",name));
		g_free(name);
		return NULL;
	}
	g_free(name);
	hbox = gtk_hbox_new(FALSE,5);

	label = gtk_label_new(NULL);
	slider->label = label;
	gtk_label_set_markup(GTK_LABEL(label),slider->friendly_name);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	label = gtk_entry_new();
	gtk_entry_set_has_frame(GTK_ENTRY(label),FALSE);
	gtk_entry_set_width_chars(GTK_ENTRY(label),5);
	gtk_entry_set_alignment(GTK_ENTRY(label),1);
	gtk_entry_set_editable(GTK_ENTRY(label),FALSE);
	gtk_widget_modify_base(GTK_WIDGET(label),GTK_STATE_NORMAL,&table->style->bg[GTK_STATE_NORMAL]);

	slider->textval = label;
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	if ((ident == RUNTIME_TAB) || (ident == VE3D_VIEWER_TAB))
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

	return slider;
}


/*!
 \brief register_rt_Range() creates the slider from the passed data, 
 and attaches it the the gui. This is called during gui tab loading to embed
 sliders into regular tabs.
 \param widget (GtkWidget *) name of widget defined in Gui datamap file. Used
 to load al lthe necessary attributes to stick the control in the right place.
 */
EXPORT void register_rt_range(GtkWidget * widget)
{
	GObject * object = NULL;
	extern Rtv_Map *rtv_map;
	Rt_Slider *slider = g_malloc0(sizeof(Rt_Slider));
	gchar * source = (gchar *)g_object_get_data(G_OBJECT(widget),"source");
	TabIdent ident = (TabIdent)g_object_get_data(G_OBJECT(widget),"tab_ident");
		
	if (!rtv_map)
		return;
	object = g_hash_table_lookup(rtv_map->rtv_hash,source);

	if (!rt_sliders)
		rt_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	if (!ww_sliders)
		ww_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	if (!enr_sliders)
		enr_sliders = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	
	if  (!G_IS_OBJECT(object))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": register_rt_range()\n\tBad things man, object doesn't exist for %s\n",source));
		return;
	}
	slider->ctrl_name = g_strdup(glade_get_widget_name(widget));
	slider->tbl = -1;
	slider->table_num = -1;
	slider->row = -1;
	slider->class = MTX_RANGE;
	slider->history = (GArray *) g_object_get_data(object,"history");
	slider->object = object;
	slider->textval = NULL;
	slider->pbar = widget;

	switch (ident)
	{
		case RUNTIME_TAB:
			g_hash_table_insert(rt_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		case ENRICHMENTS_TAB:
			g_hash_table_insert(enr_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		case WARMUP_WIZ_TAB:
			g_hash_table_insert(ww_sliders,g_strdup(slider->ctrl_name),(gpointer)slider);
			break;
		default:
			break;
	}


}


/*!
 \brief free_ve3d_sliders() frees the sliders associated with the table_num
 passed to it.
 \param table_num (gint) the table_number to free the sliders for
 \returns FALSE
 */
gboolean free_ve3d_sliders(gint table_num)
{
	extern GHashTable **ve3d_sliders;
	gchar * widget = NULL;
	if (ve3d_sliders)
	{
		if (ve3d_sliders[table_num])
		{
			g_hash_table_destroy(ve3d_sliders[table_num]);
			ve3d_sliders[table_num] = NULL;
		}
	}

	widget = g_strdup_printf("ve3d_rt_table0_%i",table_num);
	deregister_widget(widget);
	g_free(widget);

	widget = g_strdup_printf("ve3d_rt_table1_%i",table_num);
	deregister_widget(widget);
	g_free(widget);
	return FALSE;
}
