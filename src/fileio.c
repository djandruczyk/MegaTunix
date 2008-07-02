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

#include <api-versions.h>
#include <apicheck.h>
#include <config.h>
#include <configfile.h>
#include <datamgmt.h>
#include <datalogging_gui.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <firmware.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <helpers.h>
#include <keyparser.h>
#include <notifications.h>
#include <stdlib.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>
#include <vex_support.h>
#ifdef __WIN32__
 #include <windows.h>
#else
 #include <sys/stat.h>
#endif


extern gint dbg_lvl;
extern GObject *global_data;

EXPORT gboolean select_file_for_ecu_backup(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gboolean interrogated;
	extern GtkWidget *main_window;
	extern Firmware_Details *firmware;
	struct tm *tm = NULL;
	time_t *t = NULL;


	if (!interrogated)
		return FALSE;

	t = g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_ecu_snapshots");
	fileio->title = g_strdup("Save your ECU Settings to file");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->default_filename = g_strdup_printf("%s-%.4i_%.2i_%.2i-%.2i%.2i.ecu",g_strdelimit(firmware->name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	//fileio->default_filename = g_strdup("ECU_Backup.ecu");
	fileio->default_extension = g_strdup("ecu");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;
	fileio->shortcut_folders = g_strdup("MTX_ecu_snapshots");

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for ECU Backup\n"),FALSE,FALSE);
		return FALSE;
	}
	update_logbar("tools_view",NULL,g_strdup("Full Backup of ECU Initiated\n"),FALSE,FALSE);
	backup_all_ecu_settings(filename);
	update_logbar("tools_view",NULL,g_strdup("Full Backup File Closed\n"),FALSE,FALSE);
	g_free(filename);
	free_mtxfileio(fileio);
	return TRUE;
}



EXPORT gboolean select_file_for_ecu_restore(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gboolean interrogated;
	extern GtkWidget *main_window;

	if (!interrogated)
		return FALSE;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_ecu_snapshots");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Restore your ECU Settings from which file");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->shortcut_folders = g_strdup("ecu_snapshots,MTX_ecu_snapshots");

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for ECU restore\n"),FALSE,FALSE);
		return FALSE;
	}
	update_logbar("tools_view",NULL,g_strdup("Full Restore of ECU Initiated\n"),FALSE,FALSE);
	restore_all_ecu_settings(filename);
	g_free(filename);
	free_mtxfileio(fileio);
	return TRUE;

}

/*!
 \brief backup_all_ecu_settings() backs up the ECU to a filename passed
 \param filename (gchar *) filename to backup the ECU to
 */
void backup_all_ecu_settings(gchar *filename)
{
	extern Firmware_Details *firmware;
	ConfigFile *cfgfile;
	gchar * section = NULL;
	gint i = 0;
	gint x = 0;
	gint canID = 0;
	DataSize size = MTX_U08;	 /* <<<<< BAD BAD BAD >>>>> */
	GString *string = NULL;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		cfgfile = cfg_new();

	set_file_api(cfgfile,BACKUP_MAJOR_API,BACKUP_MINOR_API);

	update_logbar("tools_view",NULL,g_strdup_printf("Full Backup Commencing  to file:\n\t%s\n",filename),FALSE,FALSE);
	cfg_write_string(cfgfile,"Firmware","name",firmware->name);
	for(i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;
		string = g_string_sized_new(64);
		section = g_strdup_printf("page_%i",i);
		cfg_write_int(cfgfile,section,"num_variables",firmware->page_params[i]->length);
		for(x=0;x<firmware->page_params[i]->length;x++)
		{
			g_string_append_printf(string,"%i",get_ecu_data(canID,i,x,size));
			if (x < (firmware->page_params[i]->length-1))
				string = g_string_append(string,",");
		}
		cfg_write_string(cfgfile,section,"data",string->str);
		g_free(section);
		g_string_free(string,TRUE);
	}
	update_logbar("tools_view",NULL,g_strdup_printf("Full Backup Complete...\n"),FALSE,FALSE);
	cfg_write_file(cfgfile,filename);
	cfg_free(cfgfile);
	g_free(cfgfile);

}


/*!
 \brief restore_all_ecu_settings() reads the filename passed and if all checks
 pass the file will be loaded and any values that differ from the values
 currently in the ECU will be replaced.
 \param filename (filename to read for ecu restoration
WARNING:  This function is not yet capable of handling CAN devices, and will
always restore to can ID ZERO (which can be BAD!!), backup/restore needs to
be rewritten..
 */
void restore_all_ecu_settings(gchar *filename)
{
	extern Firmware_Details *firmware;
	ConfigFile *cfgfile;
	GArray *pfuncs = NULL;
	PostFunction *pf = NULL;
	GModule *module = NULL;
	gchar * section = NULL;
	gint canID = firmware->canID;
	DataSize size = MTX_U08;
	gint page = 0;
	gint offset = 0;
	gint tmpi = 0;
	gint major = 0;
	gint minor = 0;
	gchar *tmpbuf = NULL;
	guchar *data = NULL;
	gchar **keys = NULL;
	gint num_keys = 0;
	gint dload_val = 0;

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		get_file_api(cfgfile,&major,&minor);
		if (major != BACKUP_MAJOR_API) 
		{
			update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tAPI MAJOR version mismatch: \"%i\" != \"%i\",\ncannot load this file for restoration\n",major, BACKUP_MAJOR_API),FALSE,FALSE);
			return;
		}
		if (minor != BACKUP_MINOR_API) 
			update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tAPI MINOR version mismatch: \"%i\" != \"%i\",\nLoading this file,  though there is a version mismatch,  EXPECT ERRORS!\n",minor, BACKUP_MINOR_API),FALSE,FALSE);

		cfg_read_string(cfgfile,"Firmware","name",&tmpbuf);
		if (g_strcasecmp(g_strdelimit(tmpbuf," ,",'_'),g_strdelimit(firmware->name," ,",'_')) != 0)
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\nFirmware name mismatch:\n\"%s\" != \"%s\",\ncannot load this file for restoration\n",tmpbuf,firmware->name));
			update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\nFirmware name mismatch:\n\"%s\" != \"%s\"\ncannot load this file for restoration\n",tmpbuf,firmware->name),FALSE,FALSE);
			if (tmpbuf)
				g_free(tmpbuf);
			cfg_free(cfgfile);
			g_free(cfgfile);
			return;
		}
		set_title(g_strdup("Restoring ECU settings from File"));
		for (page=0;page<firmware->total_pages;page++)
		{
			if (!(firmware->page_params[page]->dl_by_default))
				continue;

			section = g_strdup_printf("page_%i",page);
			if(cfg_read_int(cfgfile,section,"num_variables",&tmpi))
				if (tmpi != firmware->page_params[page]->length)
				{
					update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n",tmpi,firmware->page_params[page]->length),FALSE,FALSE);
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n",tmpi,firmware->page_params[page]->length));
				}
			if (cfg_read_string(cfgfile,section,"data",&tmpbuf))
			{
				keys = parse_keys(tmpbuf,&num_keys,",");
				if (num_keys != firmware->page_params[page]->length)
				{
					update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n",num_keys,firmware->page_params[page]->length),FALSE,FALSE);
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n",num_keys,firmware->page_params[page]->length));
				}
				if (firmware->chunk_support)
				{
					data = g_new0(guchar, firmware->page_params[page]->length);
					for (offset=0;offset<num_keys;offset++)
						data[offset]=(guchar)atoi(keys[offset]);
					chunk_write(canID,page,0,num_keys,data);

				}
				else
				{
					for (offset=0;offset<num_keys;offset++)
					{
						dload_val = atoi(keys[offset]);
						if (dload_val != get_ecu_data_last(canID,page,offset,size))
						{
							/*printf("writing data for page %i, offset %i\n",page,offset);*/
							send_to_ecu(canID,page,offset,size,dload_val, FALSE);
						}
					}
				}

				g_strfreev(keys);
				g_free(tmpbuf);
			}
		}
		start_restore_monitor();
	}
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("update_ve_const_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("set_store_black_pf");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	g_module_close(module);

	io_cmd(NULL,pfuncs);

}
