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


#include <args.h>
#include <apicheck.h>
#include <api-versions.h>
#include <configfile.h>
#include <debugging.h>
#include <getfiles.h>
#include <firmware.h>
#include <glib.h>
#include <gui_handlers.h>
#include <keybinder.h>
#include <keyparser.h>
#include <listmgmt.h>
#include <notifications.h>
#include <runtime_status.h>
#include <widgetmgmt.h>



extern GObject *global_data;

/*!
 \brief load_status_pf() is called to create the ECU status window, load the 
 settings from the StatusMapFile.
 */
EXPORT void load_status_pf(void)
{
	ConfigFile *cfgfile = NULL;
	extern Firmware_Details *firmware;
	gchar *filename = NULL;
	gchar *section = NULL;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint tmpi = 0;
	gint count = 0;
	gint row = 0;
	gint col = 0;
	gchar * tmpbuf = NULL;
	gchar ** tmpvector = NULL;
	gchar ** keys = NULL;
	gint num_keys = 0;
	gint major = 0;
	gint minor = 0;
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * frame;
	GtkWidget * table;
	CmdLineArgs *args = OBJ_GET(global_data,"args");
	GdkColor color;
	extern gboolean interrogated;


	if (!(interrogated))
		return;
	if (!firmware->status_map_file)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": firmware->status_map_file is UNDEFINED,\n\texiting status window creation routine!!!!\n"));
		return;
	}

	gdk_threads_enter();
	set_title(g_strdup(_("Loading RT Status...")));
	filename = get_file(g_strconcat(RTSTATUS_DATA_DIR,PSEP,firmware->status_map_file,NULL),g_strdup("status_conf"));
	if (!filename)
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_runtime_status()\n\t File \"%s.status_conf\" not found!!, exiting function\n",firmware->status_map_file));
		set_title(g_strdup(_("ERROR RT Statusfile DOES NOT EXIST!!!")));
		gdk_threads_leave();
		return;
	}
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		get_file_api(cfgfile,&major,&minor);
		if ((major != RT_STATUS_MAJOR_API) || (minor != RT_STATUS_MINOR_API))
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\tRuntime Status profile API mismatch (%i.%i != %i.%i):\n\tFile %s will be skipped\n",major,minor,RT_STATUS_MAJOR_API,RT_STATUS_MINOR_API,filename));
			g_free(filename);
			set_title(g_strdup(_("ERROR RT Status API MISMATCH!!!")));
			gdk_threads_leave();
			return;
		}

		if(!cfg_read_int(cfgfile,"global","total_status",&count))
		{
			dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\t could NOT read \"total_status\" value from\n\t file \"%s\"\n",filename));
			set_title(g_strdup(_("ERROR RT Status cfgfile problem!!!")));
			gdk_threads_leave();
			return;
		}

		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_focus_on_map((GtkWindow *)window,FALSE);
		gtk_window_set_title(GTK_WINDOW(window),_("ECU Status"));
		x = (GINT)OBJ_GET(global_data,"status_x_origin");
		y = (GINT)OBJ_GET(global_data,"status_y_origin");
		gtk_window_move(GTK_WINDOW(window),x,y);
		w = (GINT)OBJ_GET(global_data,"status_width");
		h = (GINT)OBJ_GET(global_data,"status_height");
		gtk_window_set_default_size(GTK_WINDOW(window),w,h);
		gtk_window_resize(GTK_WINDOW(window),w,h);
		g_signal_connect(G_OBJECT(window),"delete_event",
				G_CALLBACK(prevent_close),NULL);
		register_widget("status_window",window);
		gtk_widget_realize(window);

		frame = gtk_frame_new("ECU Status");
		gtk_container_add(GTK_CONTAINER(window),frame);
		gtk_container_set_border_width(GTK_CONTAINER(frame),5);
		table = gtk_table_new(1,count,FALSE);
		gtk_container_add(GTK_CONTAINER(frame),table);
		gtk_container_set_border_width(GTK_CONTAINER(table),5);
		for (i=0;i<count;i++)
		{
			row = -1;
			col = -1;
			section = g_strdup_printf("status_%i",i);
			if (!cfg_read_string(cfgfile,section,"create_label",&tmpbuf))
			{
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\t Failed reading \"create_label\" from section \"%s\" in file\n\t%s\n",section,filename));
				break;
			}
			if (!cfg_read_int(cfgfile,section,"row",&row))
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\t Failed reading \"row\" number from section \"%s\" in file\n\t%s\n",section,filename));
			if (!cfg_read_int(cfgfile,section,"col",&col))
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\t Failed reading \"col\" number from section \"%s\" in file\n\t%s\n",section,filename));
			frame = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_ETCHED_IN);
			gtk_table_attach(GTK_TABLE(table),frame,
					col,col+1,row,row+1,
					(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
					(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
					0,0);

			label = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(label),_(tmpbuf));
			gtk_widget_set_sensitive(GTK_WIDGET(label),FALSE);
			g_free(tmpbuf);
			if (cfg_read_string(cfgfile,section,"active_fg",&tmpbuf))
			{
				gdk_color_parse(tmpbuf,&color);
				gtk_widget_modify_fg(label,GTK_STATE_NORMAL,&color);
				g_free(tmpbuf);
			}
			if (cfg_read_string(cfgfile,section,"inactive_fg",&tmpbuf))
			{
				gdk_color_parse(tmpbuf,&color);
				gtk_widget_modify_fg(label,GTK_STATE_INSENSITIVE,&color);
				g_free(tmpbuf);
			}

			gtk_container_add(GTK_CONTAINER(frame),label);
			if (!cfg_read_string(cfgfile,section,"keys",&tmpbuf))
				dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\t Failed reading \"keys\" from section \"%s\" in file\n\t%s\n",section,filename));
			else
			{
				keys = parse_keys(tmpbuf,&num_keys,",");
				g_free(tmpbuf);
			}

			bind_keys(G_OBJECT(label),cfgfile,section,keys,num_keys);
			g_strfreev(keys);
			/* Bind widgets to lists if thy have the bind_to_list flag set...
			 *         */
			if (cfg_read_string(cfgfile,section,"bind_to_list",&tmpbuf))
			{
				tmpvector = parse_keys(tmpbuf,&tmpi,",");
				g_free(tmpbuf);
				/* This looks convoluted,  but it allows for an arbritrary 
				 * number of lists, that are indexed by a keyword.
				 * The get_list function looks the list up in a hashtable, if
				 * it isn't found (i.e. new list) it returns NULL which is OK
				 * as g_list_prepend() uses that to create a new list,  that
				 * returned list is used to store back into the hashtable so
				 * that the list is always stored and up to date...
				 */
				for (x=0;x<tmpi;x++)
					store_list(tmpvector[x],g_list_prepend(get_list(tmpvector[x]),(gpointer)label));
				g_strfreev(tmpvector);
			}
			g_free(section);
		}
		if (!args->hide_status)
			gtk_widget_show_all(window);
		cfg_free(cfgfile);
	}
	else
	{
		dbg_func(CRITICAL,g_strdup_printf(__FILE__": load_status_pf()\n\tCould not load %s\n",filename));
	}

	g_free(filename);
	set_title(g_strdup(_("RT Status Loaded...")));
	gdk_threads_leave();
	return;
}

