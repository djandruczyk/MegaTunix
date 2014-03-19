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
  \file src/plugins/libreems/fileio.c
  \ingroup LibreEMSPlugin,Plugins
  \brief ECU Specific file save/restore functions
  \author David Andruczyk
  */

#include <api-versions.h>
#include <datamgmt.h>
#include <defines.h>
#include <fileio.h>
#include <firmware.h>
#include <getfiles.h>
#include <libreems_comms.h>
#include <libreems_plugin.h>
#include <stdio.h>
#include <time.h>
#ifdef __WIN32__
 #include <windows.h>
#else
 #include <sys/stat.h>
#endif

/*!
  \brief Pops up a filechooser so the User can select a file to save the 
  current ECU state to.
  \param widget is unused
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean select_file_for_ecu_backup(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	extern gconstpointer *global_data;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return FALSE;
	}

	t = (time_t *)g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup(BACKUP_DATA_DIR);
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->title = g_strdup("Save your ECU Settings to file");
	fileio->parent = lookup_widget_f("main_window");
	fileio->on_top = TRUE;
	fileio->default_filename = g_strdup_printf("%s-%.4i_%.2i_%.2i-%.2i%.2i.ecu",g_strdelimit(firmware->name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension = g_strdup("ecu");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;
	fileio->shortcut_folders = g_strdup("MTX_ecu_snapshots");

	filename = choose_file(fileio);
	free_mtxfileio(fileio);
	if (filename == NULL)
	{
		update_logbar_f("tools_view","warning",_("NO FILE chosen for ECU Backup\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}
	update_logbar_f("tools_view",NULL,_("Full Backup of ECU Initiated\n"),FALSE,FALSE,FALSE);
	backup_all_ecu_settings(filename);
	update_logbar_f("tools_view",NULL,_("Full Backup File Closed\n"),FALSE,FALSE,FALSE);
	g_free(filename);
	EXIT();
	return TRUE;
}


/*!
  \brief Pops up a filechooser so the User can select a file to restore the 
  current ECU state to.
  \param widget is unused
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean select_file_for_ecu_restore(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gconstpointer *global_data;

	ENTER();
	printf("select for restore!\n");
	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return FALSE;
	}

	fileio = g_new0(MtxFileIO ,1);
	fileio->project = (const gchar *)DATA_GET(global_data,"project_name");
	fileio->default_path = g_strdup(BACKUP_DATA_DIR);
	fileio->parent = lookup_widget_f("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Restore your ECU Settings from which file");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->shortcut_folders = g_strdup(BACKUP_DATA_DIR);

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar_f("tools_view","warning",_("NO FILE chosen for ECU restore\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}
	update_logbar_f("tools_view",NULL,_("Full Restore of ECU Initiated\n"),FALSE,FALSE,FALSE);
	restore_all_ecu_settings(filename);
	g_free(filename);
	free_mtxfileio(fileio);
	EXIT();
	return TRUE;

}

/*!
 \brief Firmware specific function that backs up the ECU to a filename passed
 \param filename is the filename to backup the ECU to
 */
G_MODULE_EXPORT void backup_all_ecu_settings(gchar *filename)
{
	ConfigFile *cfgfile;
	gchar * section = NULL;
	gchar * tmpbuf = NULL;
	gint i = 0;
	gint locID = 0;
	gint x = 0;
	gint canID = 0;
	DataSize size = MTX_U08;	 /* <<<<< BAD BAD BAD >>>>> */
	GString *string = NULL;
	extern gconstpointer *global_data;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(filename);
	g_return_if_fail(firmware);

	cfgfile = cfg_open_file(filename);
	if(!cfgfile)
		cfgfile = cfg_new();

	set_file_api_f(cfgfile,BACKUP_MAJOR_API,BACKUP_MINOR_API);

	update_logbar_f("tools_view",NULL,g_strdup_printf("%s %s\n",_("Full Backup Commencing to file:\n\t"),filename),FALSE,FALSE,TRUE);
	cfg_write_string(cfgfile,"Firmware","name",firmware->name);
	for(i=0;i<firmware->total_pages;i++)
	{
		if (firmware->page_params[i]->read_only)
			continue;
		string = g_string_sized_new(64);
		section = g_strdup_printf("page_%i",i);
		cfg_write_int(cfgfile,section,"num_variables",firmware->page_params[i]->length);
		for(x=0;x<firmware->page_params[i]->length;x++)
		{
			locID = firmware->page_params[i]->phys_ecu_page;
			g_string_append_printf(string,"%i",libreems_get_ecu_data(canID,locID,x,size));
			if (x < (firmware->page_params[i]->length-1))
				string = g_string_append(string,",");
		}
		cfg_write_string(cfgfile,section,"data",string->str);
		g_free(section);
		g_string_free(string,TRUE);
	}
	update_logbar_f("tools_view",NULL,_("Full Backup Complete...\n"),FALSE,FALSE,FALSE);
	cfg_write_file(cfgfile,filename);
	cfg_free(cfgfile);
}


/*!
 \brief Firmware/ECU specific function that reads the filename passed and 
 if all checks pass the file will be loaded and any values that differ 
 from the values currently in the ECU will be replaced.
 \param filename (filename to read for ecu restoration
WARNING:  This function is not yet capable of handling CAN devices, and will
always restore to can ID ZERO (which can be BAD!!), backup/restore needs to
be rewritten..
 */
G_MODULE_EXPORT void restore_all_ecu_settings(gchar *filename)
{
	ConfigFile *cfgfile;
	gchar * section = NULL;
	gchar * msgbuf = NULL;
	gint canID = 0;
	DataSize size = MTX_U08;
	gint tmpi = 0;
	gint major = 0;
	gint minor = 0;
	gchar *tmpbuf = NULL;
	guint8 *data = NULL;
	gchar **keys = NULL;
	gint num_keys = 0;
	PostFunction *pf = NULL;
	GArray *pfuncs = NULL;
	extern gconstpointer *global_data;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(filename);
	g_return_if_fail(firmware);
	canID = firmware->canID;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
	{
		update_logbar_f("tools_view","warning",g_strdup_printf(_(":restore_all_ecu_settings()\n\t Unable to open this file (%s)\n"),filename),FALSE,FALSE,TRUE);
		EXIT();
		return;
	}
	if (cfgfile)
	{
		gint page = 0;
		get_file_api_f(cfgfile,&major,&minor);
		if (major != BACKUP_MAJOR_API) 
		{
			update_logbar_f("tools_view","warning",g_strdup_printf(_(":restore_all_ecu_settings()\n\tAPI MAJOR version mismatch: \"%i\" != \"%i\"\n can not load this file for restoration\n"),major,BACKUP_MAJOR_API),FALSE,FALSE,TRUE);
			cfg_free(cfgfile);
			EXIT();
			return;
		}
		if (minor != BACKUP_MINOR_API) 
			update_logbar_f("tools_view","warning",g_strdup_printf(_(": restore_all_ecu_settings()\n\tAPI MINOR version mismatch: \"%i\" != \"%i\"\n Will try to load this file for restoration, expect issues\n"),minor,BACKUP_MINOR_API),FALSE,FALSE,TRUE);

		cfg_read_string(cfgfile,"Firmware","name",&tmpbuf);
		if (g_ascii_strcasecmp(g_strdelimit(tmpbuf," ,",'_'),g_strdelimit(firmware->name," ,",'_')) != 0)
		{
			MTXDBG(CRITICAL,_("Firmware name mismatch:\"%s\" != \"%s\",cannot load this file for restoration\n"),tmpbuf,firmware->name);

			update_logbar_f("tools_view","warning",g_strdup_printf(_(": restore_all_ecu_settings()\nFirmware name mismatch: \"%s\" != \"%s\"\ncan NOT load this file for restoration!\n"),tmpbuf,firmware->name),FALSE,FALSE,TRUE);
			if (tmpbuf)
				g_free(tmpbuf);
			cfg_free(cfgfile);
			EXIT();
			return;
		}
		g_free(tmpbuf);
		set_title_f(g_strdup(_("Restoring ECU settings from File")));
		for (page=0;page<firmware->total_pages;page++)
		{
			if (firmware->page_params[page]->read_only)
				continue;
			gint locID = firmware->page_params[page]->phys_ecu_page;

			section = g_strdup_printf("page_%i",page);
			if(cfg_read_int(cfgfile,section,"num_variables",&tmpi))
				if (tmpi != firmware->page_params[page]->length)
				{
					update_logbar_f("tools_view","warning",g_strdup_printf(_(": restore_all_ecu_settings()\n\tNumber of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n"),tmpi,firmware->page_params[page]->length),FALSE,FALSE,TRUE);
					MTXDBG(CRITICAL,_("Number of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n"),tmpi,firmware->page_params[page]->length);
				}
			if (cfg_read_string(cfgfile,section,"data",&tmpbuf))
			{
				gint offset = 0;
				keys = parse_keys_f(tmpbuf,&num_keys,",");
				if (num_keys != firmware->page_params[page]->length)
				{
					update_logbar_f("tools_view","warning",g_strdup_printf(_(": restore_all_ecu_settings()\n\tNumber of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n"),num_keys,firmware->page_params[page]->length),FALSE,FALSE,TRUE);
					MTXDBG(CRITICAL,_("Number of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n"),num_keys,firmware->page_params[page]->length);
				}
				if (firmware->chunk_support)
				{
					data = g_new0(guint8, firmware->page_params[page]->length);
					for (offset=0;offset<num_keys;offset++)
						data[offset]=(guint8)atoi(keys[offset]);
					if (DATA_GET(global_data,"offline"))
						libreems_store_new_block(canID,locID,0,data,num_keys);
					else
						libreems_chunk_write(canID,locID,0,num_keys,data);
				}
				else
				{
					gint dload_val = 0;
					if (DATA_GET(global_data,"offline"))
					{
						for (offset=0;offset<num_keys;offset++)
						{
							dload_val = atoi(keys[offset]);
							libreems_set_ecu_data(canID,locID,offset,size,dload_val);
						}
					}
					else
					{
						for (offset=0;offset<num_keys;offset++)
						{
							dload_val = atoi(keys[offset]);
							if (dload_val != libreems_get_ecu_data_last(canID,locID,offset,size))
							{
								/*printf("writing data for page %i, offset %i\n",page,offset);*/
								libreems_send_to_ecu(canID,locID,offset,size,dload_val, FALSE);
							}
						}
					}
				}
				g_strfreev(keys);
				g_free(tmpbuf);
			}
			g_free(section);
		}
		/*queue_burn_ecu_flash(page);
		start_restore_monitor_f();
		*/
		cfg_free(cfgfile);
	}
	if (DATA_GET(global_data,"offline"))
	{
		pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));
		pf = g_new0(PostFunction,1);
		pf->name = g_strdup("update_ecu_controls_pf");
		get_symbol_f(pf->name,(void **)&pf->function);
		pf->w_arg = FALSE;
		pfuncs = g_array_append_val(pfuncs,pf);

		pf = g_new0(PostFunction,1);
		pf->name = g_strdup("set_store_black_pf");
		get_symbol_f(pf->name,(void **)&pf->function);
		pf->w_arg = FALSE;
		pfuncs = g_array_append_val(pfuncs,pf);

		io_cmd_f(NULL,pfuncs);
	}
}
