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
#include <gui_handlers.h>
#include <runtime_status.h>
#include <runtime_text.h>
#include <stdio.h>
#include <stdlib.h>
#include <structures.h>
#include <widgetmgmt.h>

GHashTable *rtt_hash = NULL;
GtkWidget *rtt_window = NULL;
extern gint dbg_lvl;

/*!
 \brief load_rt_text() is called to load up the runtime text configurations
 from the file specified in the firmware's interrogation profile, and populate
 a new window with the runtiem vars text value box.
 */
void load_rt_text()
{
	ConfigFile *cfgfile = NULL;
	Rt_Text *rt_text = NULL;
	GtkWidget *window = NULL;
	GtkWidget *vbox = NULL;
	gint count = 0;
	gchar *filename = NULL;
	gchar *ctrl_name = NULL;
	gchar *source = NULL;
	gchar *section = NULL;
	gint i = 0;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint major = 0;
	gint minor = 0;
	extern GObject *global_data;
	extern CmdLineArgs *args;
	extern volatile gboolean leaving;
	extern gboolean tabs_loaded;
	extern gboolean rtvars_loaded;
	extern Firmware_Details *firmware;

	if ((!tabs_loaded) || (!firmware) || (leaving))
		return;
	if ((rtvars_loaded == FALSE) || (tabs_loaded == FALSE))
	{
		if (rtt_hash)
			rtt_hash = NULL;
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": load_rt_text()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"));
		return;
	}
	if (!rtt_hash)
		rtt_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);

	filename = get_file(g_strconcat(RTTEXT_DATA_DIR,PSEP,firmware->rtt_map_file,NULL),g_strdup("rtt_conf"));
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		get_file_api(cfgfile,&major,&minor);
		if ((major != RT_TEXT_MAJOR_API) || (minor != RT_TEXT_MINOR_API))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_rtt()\n\tRuntime Text profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n",major,minor,RT_TEXT_MAJOR_API,RT_TEXT_MINOR_API,filename));
			g_free(filename);
			return;
		}

		if(!cfg_read_int(cfgfile,"global","rtt_total",&count))
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": load_rtt()\n\t could NOT read \"rtt_total\" value from\n\t file \"%s\"\n",filename));
			return;
		}
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_focus_on_map((GtkWindow *)window,FALSE);
		gtk_window_set_title(GTK_WINDOW(window),"Runtime Vars");
		x = (gint)g_object_get_data(global_data,"rtt_x_origin");
		y = (gint)g_object_get_data(global_data,"rtt_y_origin");
		gtk_window_move(GTK_WINDOW(window),x,y);
		w = (gint)g_object_get_data(global_data,"rtt_width");
		h = (gint)g_object_get_data(global_data,"rtt_height");
		gtk_window_set_default_size(GTK_WINDOW(window),w,h);
		gtk_window_resize(GTK_WINDOW(window),w,h);

		register_widget("rtt_window",window);
		g_signal_connect(G_OBJECT(window),"destroy_event",
				G_CALLBACK(prevent_close),NULL);
		g_signal_connect(G_OBJECT(window),"delete_event",
				G_CALLBACK(prevent_close),NULL);
		vbox = gtk_vbox_new(TRUE,3);
		gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
		gtk_container_add(GTK_CONTAINER(window),vbox);

		for (i=0;i<count;i++)
		{
			rt_text = NULL;
			section = g_strdup_printf("rt_text_%i",i);
			if(!cfg_read_string(cfgfile,section,"int_name",&ctrl_name))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_rt_text()\n\t Failed reading \"int_name\" from section \"%s\" in file\n\t%s\n",section,filename));
			}
			if (!cfg_read_string(cfgfile,section,"source",&source))
			{
				if (dbg_lvl & CRITICAL)
					dbg_func(g_strdup_printf(__FILE__": load_rt_text()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename));
			}

			rt_text = add_rtt(vbox,ctrl_name,source);
			if (rt_text)
			{
				if (!g_hash_table_lookup(rtt_hash,ctrl_name))
					g_hash_table_insert(rtt_hash,
							g_strdup(ctrl_name),
							(gpointer)rt_text);
			}
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
		rtt_window = window;
		if (!args->hide_rttext)
			gtk_widget_show_all(window);
		cfg_free(cfgfile);
		g_free(cfgfile);
	}
	else
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": load_rt_text()\n\t Filename \"%s\" NOT FOUND Critical error!!\n\n",filename));
	}
	g_free(filename);
}


/*!
 \brief add_rtt() creates the rt_text from the passed data, and attaches
 it the the gui.
 \param parent (GtkWidget *) parent widget
 \param ctrl_name (gchar *) name of the rt_text as defined in the config file
 \param source (gchar *) data source for this rt_text 
 \returns a Struct Rt_Text *
 */
Rt_Text * add_rtt(GtkWidget *parent, gchar *ctrl_name, gchar *source)
{
	Rt_Text *rtt = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	extern Rtv_Map *rtv_map;
	GObject *object = NULL;

	rtt = g_malloc0(sizeof(Rt_Text));

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, object doesn't exist for %s\n",source));
		return NULL;
	}

	rtt->ctrl_name = g_strdup(ctrl_name);
	rtt->friendly_name = (gchar *) g_object_get_data(object,"dlog_gui_name");
	rtt->history = (GArray *) g_object_get_data(object,"history");
	rtt->object = object;

	hbox = gtk_hbox_new(FALSE,5);

	label = gtk_label_new(NULL);
	rtt->name_label = label;
	gtk_label_set_markup(GTK_LABEL(label),rtt->friendly_name);
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	label = gtk_label_new(NULL);
	set_fixed_size(label,6);
	rtt->textval = label;
	gtk_misc_set_alignment(GTK_MISC(label),1,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	gtk_box_pack_start(GTK_BOX(parent),hbox,TRUE,TRUE,0);

	rtt->parent = hbox;
	gtk_widget_show_all(rtt->parent);

	return rtt;
}

