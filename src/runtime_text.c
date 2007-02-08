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


/*!
 \brief load_rt_text() is called to load up the runtime text configurations
 from the file specified in the firmware's interrogation profile, and populate
 a new window with the runtiem vars text value box.
 */
void load_rt_text()
{
	ConfigFile *cfgfile = NULL;
	struct Rt_Text *rt_text = NULL;
	GtkWidget *window = NULL;
	GtkWidget *vbox = NULL;
	gint count = 0;
	gchar *filename = NULL;
	gchar *ctrl_name = NULL;
	gchar *source = NULL;
	gchar *section = NULL;
	gint i = 0;
	extern volatile gboolean leaving;
	extern gboolean tabs_loaded;
	extern gboolean rtvars_loaded;
	extern struct Firmware_Details *firmware;

	if ((!tabs_loaded) || (!firmware) || (leaving))
		return;
	if ((rtvars_loaded == FALSE) || (tabs_loaded == FALSE))
	{
		if (rtt_hash)
			rtt_hash = NULL;
		dbg_func(g_strdup(__FILE__": load_rt_text()\n\tCRITICAL ERROR, Realtime Variable definitions NOT LOADED!!!\n\n"),CRITICAL);
		return;
	}
	if (!rtt_hash)
		rtt_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);

	filename = get_file(g_strconcat(RTTEXT_DATA_DIR,PSEP,firmware->rtt_map_file,NULL),g_strdup("rtt_conf"));
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		if(!cfg_read_int(cfgfile,"global","rtt_total",&count))
		{
			dbg_func(g_strdup_printf(__FILE__": load_rtt()\n\t could NOT read \"rtt_total\" value from\n\t file \"%s\"\n",filename),CRITICAL);
			return;
		}
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window),"RT Text");
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
				dbg_func(g_strdup_printf(__FILE__": load_rt_text()\n\t Failed reading \"int_name\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);
			if (!cfg_read_string(cfgfile,section,"source",&source))
				dbg_func(g_strdup_printf(__FILE__": load_rt_text()\n\t Failed reading \"source\" from section \"%s\" in file\n\t%s\n",section,filename),CRITICAL);

			rt_text = add_rtt(vbox,ctrl_name,source);
			if (rt_text)
			{
				if (g_hash_table_lookup(rtt_hash,ctrl_name)==NULL)
					g_hash_table_insert(rtt_hash,g_strdup(ctrl_name),(gpointer)rt_text);
			}
			g_free(section);
			g_free(ctrl_name);
			g_free(source);
		}
		gtk_widget_show_all(window);
		cfg_free(cfgfile);
		g_free(cfgfile);
	}
	else
		dbg_func(g_strdup_printf(__FILE__": load_rt_text()\n\t Filename \"%s\" NOT FOUND Critical error!!\n\n",filename),CRITICAL);
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
struct Rt_Text * add_rtt(GtkWidget *parent, gchar *ctrl_name, gchar *source)
{
	struct Rt_Text *rtt = NULL;
	GtkWidget *label = NULL;
	GtkWidget *hbox = NULL;
	extern struct Rtv_Map *rtv_map;
	GObject *object = NULL;

	rtt = g_malloc0(sizeof(struct Rt_Text));

	object = g_hash_table_lookup(rtv_map->rtv_hash,source);
	if (!G_IS_OBJECT(object))
	{
		dbg_func(g_strdup_printf(__FILE__": add_rtt()\n\tBad things man, object doesn't exist for %s\n",source),CRITICAL);
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
#if GTK_MINOR_VERSION >= 6
	if (gtk_minor_version >= 6)
		gtk_label_set_width_chars(GTK_LABEL(label),6);
#endif
	rtt->textval = label;
	gtk_misc_set_alignment(GTK_MISC(label),1,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

	gtk_box_pack_start(GTK_BOX(parent),hbox,TRUE,TRUE,0);

	rtt->parent = hbox;
	gtk_widget_show_all(rtt->parent);

	return rtt;
}

