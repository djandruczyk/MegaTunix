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
#include <datalogging_gui.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <fileio.h>
#include <firmware.h>
#include <getfiles.h>
#include <gtk/gtk.h>
#include <gui_handlers.h>
#include <keyparser.h>
#include <notifications.h>
#include <stdlib.h>
#include <structures.h>
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

EXPORT gboolean select_file_for_ecu_backup(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gboolean interrogated;
	extern GtkWidget *main_window;

	if (!interrogated)
		return FALSE;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_ecu_snapshots");
	fileio->title = g_strdup("Save your ECU Settings to file");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->default_filename = g_strdup("ECU_Backup.ecu");
	fileio->default_extension = g_strdup("ecu");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;
	fileio->shortcut_folders = g_strdup("MTX_ecu_snapshots");

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for ECU Backup\n"),TRUE,FALSE);
		return FALSE;
	}
	update_logbar("tools_view",NULL,g_strdup("Full Backup of ECU Initiated\n"),TRUE,FALSE);
	backup_all_ecu_settings(filename);
	update_logbar("tools_view",NULL,g_strdup("Full Backup File Closed\n"),TRUE,FALSE);
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
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for ECU restore\n"),TRUE,FALSE);
		return FALSE;
	}
	update_logbar("tools_view",NULL,g_strdup("Full Restore of ECU Initiated\n"),TRUE,FALSE);
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
	extern gint **ms_data;
	GString *string = NULL;

	cfgfile = cfg_open_file(filename);
	if (!cfgfile)
		cfgfile = cfg_new();

	set_file_api(cfgfile,BACKUP_MAJOR_API,BACKUP_MINOR_API);

	update_logbar("tools_view",NULL,g_strdup_printf("Full Backup Commencing  to file:\n\t%s\n",filename),TRUE,FALSE);
	cfg_write_string(cfgfile,"Firmware","name",firmware->name);
	for(i=0;i<firmware->total_pages;i++)
	{
		string = g_string_sized_new(64);
		section = g_strdup_printf("page_%i",i);
		cfg_write_int(cfgfile,section,"num_variables",firmware->page_params[i]->length);
		for(x=0;x<firmware->page_params[i]->length;x++)
		{
			string = g_string_append(string,g_strdup_printf("%i",ms_data[i][x]));
			if (x < (firmware->page_params[i]->length-1))
				string = g_string_append(string,",");
		}
		cfg_write_string(cfgfile,section,"data",string->str);
		g_string_free(string,TRUE);
	}
	update_logbar("tools_view",NULL,g_strdup_printf("Full Backup Complete...\n"),TRUE,FALSE);
	cfg_write_file(cfgfile,filename);
	cfg_free(cfgfile);
	g_free(cfgfile);

	if (section)
		g_free(section);
}


/*!
 \brief restore_all_ecu_settings() reads the filename passed and if all checks
 pass the file will be loaded and any values that differ from the values
 currently in the ECU will be replaced.
 \param filename (filename to read for ecu restoration
 */
void restore_all_ecu_settings(gchar *filename)
{
	extern Firmware_Details *firmware;
	ConfigFile *cfgfile;
	gchar * section = NULL;
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
	extern gint **ms_data_last;

	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{
		get_file_api(cfgfile,&major,&minor);
		if (major != BACKUP_MAJOR_API) 
		{
			update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tAPI MAJOR version mismatch: \"%i\" != \"%i\",\ncannot load this file for restoration\n",major, BACKUP_MAJOR_API),TRUE,FALSE);
			return;
		}
		if (minor != BACKUP_MINOR_API) 
			update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tAPI MINOR version mismatch: \"%i\" != \"%i\",\nLoading this file,  though there is a version mismatch,  EXPECT ERRORS!\n",minor, BACKUP_MINOR_API),TRUE,FALSE);
			
		cfg_read_string(cfgfile,"Firmware","name",&tmpbuf);
		if (g_strcasecmp(tmpbuf,firmware->name) != 0)
		{
			if (dbg_lvl & CRITICAL)
				dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tFirmware name mismatch: \"%s\" != \"%s\",\ncannot load this file for restoration\n",tmpbuf,firmware->name));
			update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tFirmware name mismatch: \"%s\" != \"%s\",\ncannot load this file for restoration\n",tmpbuf,firmware->name),TRUE,FALSE);
			if (tmpbuf)
				g_free(tmpbuf);
			cfg_free(cfgfile);
			g_free(cfgfile);
			return;
		}
		set_title(g_strdup("Restoring ECU settings from File"));
		for (page=0;page<firmware->total_pages;page++)
		{
			if ((firmware->ro_above > 0 ) && (page > firmware->ro_above))
				break;

			section = g_strdup_printf("page_%i",page);
			if(cfg_read_int(cfgfile,section,"num_variables",&tmpi))
				if (tmpi != firmware->page_params[page]->length)
				{
					update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n",tmpi,firmware->page_params[page]->length),TRUE,FALSE);
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in backup \"%i\" and firmware specification \"%i\" do NOT match,\n\tcorruption SHOULD be expected\n",tmpi,firmware->page_params[page]->length));
				}
			if (cfg_read_string(cfgfile,section,"data",&tmpbuf))
			{
				keys = parse_keys(tmpbuf,&num_keys,",");
				if (num_keys != firmware->page_params[page]->length)
				{
					update_logbar("tools_view","warning",g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n",num_keys,firmware->page_params[page]->length),TRUE,FALSE);
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup_printf(__FILE__": restore_all_ecu_settings()\n\tNumber of variables in this backup \"%i\" does NOT match the length of the table \"%i\", expect a crash!!!\n",num_keys,firmware->page_params[page]->length));
				}
				if (firmware->chunk_support)
				{
					data = g_new0(guchar, firmware->page_params[page]->length);
					for (offset=0;offset<num_keys;offset++)
						data[offset]=(guchar)atoi(keys[offset]);
					chunk_write(page,0,num_keys,data);

				}
				else
				{
					for (offset=0;offset<num_keys;offset++)
					{
						dload_val = atoi(keys[offset]);
						if (dload_val != ms_data_last[page][offset])
						{
							//					printf("writing data for page %i, offset %i\n",page,offset);
							write_ve_const(NULL,page,offset,dload_val,firmware->page_params[page]->is_spark, FALSE);
						}
					}
				}

				g_strfreev(keys);
				g_free(tmpbuf);
			}
		}
		start_restore_monitor();
	}
	io_cmd(IO_UPDATE_VE_CONST,NULL);
}
