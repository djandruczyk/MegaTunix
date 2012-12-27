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
  \file src/plugins/mscommon/vex_support.c
  \ingroup MSCommonPlugin,Plugins
  \brief MS personality VE import/export functionality
  \author David Andruczyk
  */

#include <datamgmt.h>
#include <firmware.h>
#include <getfiles.h>
#include <mscommon_comms.h>
#include <mscommon_plugin.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vex_support.h>

static gchar *vex_comment = NULL;
extern gconstpointer *global_data;

/*!
 \brief import_handlers structure is used to hold the list of string 
 tags to search for and the function and function args to pass to the 
 appropriate handlers.
 */
static struct
{
	const gchar *import_tag;		/* string to find.. */
	ImportParserFunc function;	/* Function to call... */
	ImportParserArg parsetag;	/* Enum Tag fed to function... */

} import_handlers[] = 
{
	{ "EVEME", VEX_HEADER, VEX_EVEME},
	{ "UserRev:", VEX_HEADER, VEX_USER_REV}, 
	{ "UserComment:", VEX_HEADER, VEX_USER_COMMENT},
	{ "Date:", VEX_HEADER, VEX_DATE},
	{ "Time:", VEX_HEADER, VEX_TIME},
	{ "Page", VEX_PAGE, VEX_NONE},
	{ "VE Table RPM Range\0", VEX_RANGE, VEX_RPM_RANGE},
	{ "VE Table Load Range\0", VEX_RANGE, VEX_LOAD_RANGE},
	{ "VE Table\0", VEX_TABLE, VEX_NONE}
};


/*!
  \brief prompts the user for a filename to export all tables to in VEX format
  \param widget is the export button the user clicked on
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean select_vex_for_export(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
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
	fileio->default_path = g_strdup("MTX_VexFiles");
	fileio->title = g_strdup("Save your VEX file");
	fileio->parent = lookup_widget_f("main_window");
	fileio->on_top = TRUE;
	fileio->default_filename= g_strdup("VEX_Backup.vex");
	fileio->default_filename= g_strdup_printf("%s-%.4i%.2i%.2i%.2i%.2i.vex",g_strdelimit(firmware->name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension= g_strdup("vex");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar_f("tools_view","warning",_("NO FILE chosen for VEX export\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (!iochannel)
	{
		update_logbar_f("tools_view","warning",_("File open FAILURE! \n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}
	if (vex_comment == NULL)
		update_logbar_f("tools_view",NULL,_("VEX File Opened. VEX Comment undefined, exporting without one.\n"),FALSE,FALSE,FALSE);
	else
		update_logbar_f("tools_view",NULL,_("VEX File Opened. VEX Comment already stored.\n"),FALSE,FALSE,FALSE);
	all_table_export(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	EXIT();
	return TRUE;
}


/*!
  \brief prompts the user for a filename to export a single table to in 
  VEX format
  \param table_num is the table number to export
  */
G_MODULE_EXPORT void select_table_for_export(gint table_num)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	struct tm *tm = NULL;
	time_t *t = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return;
	}

	if ((table_num < 0) || (table_num >= firmware->total_tables))
	{
		EXIT();
		return;
	}
	t = (time_t *)g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("MTX_VexFiles");
	fileio->title = g_strdup("Save your VEX file");
	fileio->parent = lookup_widget_f("main_window");
	fileio->on_top = TRUE;
	fileio->default_filename= g_strdup_printf("%s-%.4i%.2i%.2i%.2i%.2i.vex",g_strdelimit(firmware->table_params[table_num]->table_name," ,",'_'),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min);
	fileio->default_extension= g_strdup("vex");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar_f("tools_view","warning",_("NO FILE chosen for VEX export\n"),FALSE,FALSE,FALSE);
		EXIT();
		return;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (!iochannel)
	{
		update_logbar_f("tools_view","warning",_("File open FAILURE! \n"),FALSE,FALSE,FALSE);
		EXIT();
		return;
	}
	if (vex_comment == NULL)
		update_logbar_f("tools_view",NULL,_("VEX File Opened. VEX Comment undefined, exporting without one.\n"),FALSE,FALSE,FALSE);
	else
		update_logbar_f("tools_view",NULL,_("VEX File Opened. VEX Comment already stored.\n"),FALSE,FALSE,FALSE);
	single_table_export(iochannel,table_num);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	EXIT();
	return;
}


/*!
  \brief Prompts the user with a filechooser so he/she an select a VEX file
  for import, This is for a VEX file containing MULTIPLE tables..
  \param widget is hte pointer to the import button the user clicked
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean select_vex_for_import(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;

	ENTER();
	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return FALSE;
	}

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("MTX_VexFiles");
	fileio->title = g_strdup("Select your VEX file to import");
	fileio->parent = lookup_widget_f("main_window");
	fileio->on_top = TRUE;
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar_f("tools_view","warning",_("NO FILE chosen for VEX import\n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "r+",NULL);
	if (!iochannel)
	{
		update_logbar_f("tools_view","warning",_("File open FAILURE! \n"),FALSE,FALSE,FALSE);
		EXIT();
		return FALSE;
	}
	update_logbar_f("tools_view",NULL,_("VEX File Closed\n"),FALSE,FALSE,FALSE);
	gtk_entry_set_text(GTK_ENTRY(lookup_widget_f("tools_vex_comment_entry")),"");

	all_table_import(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	EXIT();
	return TRUE;
}


/*!
  \brief prompts the user for a filename to import a single table to in 
  VEX format
  \param table_num is the table number to export
  */
G_MODULE_EXPORT void select_table_for_import(gint table_num)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!DATA_GET(global_data,"interrogated"))
	{
		EXIT();
		return;
	}

	if ((table_num < 0) || (table_num >= firmware->total_tables))
	{
		EXIT();
		return;
	}

	fileio = g_new0(MtxFileIO ,1);
	fileio->default_path = g_strdup("MTX_VexFiles");
	fileio->parent = lookup_widget_f("main_window");
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select your VEX file to import");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar_f("tools_view","warning",_("NO FILE chosen for VEX import\n"),FALSE,FALSE,FALSE);
		EXIT();
		return;
	}

	iochannel = g_io_channel_new_file(filename, "r+",NULL);
	if (!iochannel)
	{
		update_logbar_f("tools_view","warning",_("File open FAILURE! \n"),FALSE,FALSE,FALSE);
		EXIT();
		return;
	}
	update_logbar_f("tools_view",NULL,_("VEX File Closed\n"),FALSE,FALSE,FALSE);
	gtk_entry_set_text(GTK_ENTRY(lookup_widget_f("tools_vex_comment_entry")),"");

	single_table_import(iochannel,table_num);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	EXIT();
	return;
}


/*!
 \brief all_table_export() is the export function to dump all Tables to a "vex"
 file.  It has been modified to handle multiple tables per page.  
 There is a major problem with this in that the currect VEX 1.0 spec 
 doesn't allow for multiple tables per page, so import is likely to be 
 a problem on other tuning softwares.
 \param iochannel is the pointer to the output channel 
 to write the data to.
 \see all_table_import
 \returns TRUE on success, FALSE on failure
 */
G_MODULE_EXPORT gboolean all_table_export(GIOChannel *iochannel)
{
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint i = 0;
	gint j = 0;
	gint z = 0;
	gint table = -1;
	gsize count = 0;
	gint index = 0;
	gint x_page = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_base = 0;
	gint y_base = 0;
	gint z_base = 0;
	DataSize y_size = MTX_U08;
	DataSize z_size = MTX_U08;
	DataSize x_size = MTX_U08;
	gint x_mult = 0;
	gint y_mult = 0;
	gint z_mult = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	gint canID = 0;
	GIOStatus status;
	GString *output = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	canID = firmware->canID;


	/* For Page 0.... */
	output = g_string_sized_new(64); /*pre-allocate 64 chars */
	/*printf("total tables %i\n",firmware->total_tables);*/
	for (z=0;z<firmware->total_tables;z++)
	{
		table = z;
		x_page = firmware->table_params[table]->x_page;
		y_page = firmware->table_params[table]->y_page;
		z_page = firmware->table_params[table]->z_page;
		x_base = firmware->table_params[table]->x_base;
		z_base = firmware->table_params[table]->z_base;
		y_base = firmware->table_params[table]->y_base;
		x_size = firmware->table_params[table]->x_size;
		z_size = firmware->table_params[table]->z_size;
		y_size = firmware->table_params[table]->y_size;
		x_mult = get_multiplier_f(x_size);
		y_mult = get_multiplier_f(y_size);
		z_mult = get_multiplier_f(z_size);
		y_bincount = firmware->table_params[table]->y_bincount;
		x_bincount = firmware->table_params[table]->x_bincount;

		t = (time_t *)g_malloc(sizeof(time_t));
		time(t);
		tm = localtime(t);
		g_free(t);
		if (vex_comment == NULL)
			vex_comment = g_strdup("No comment given");

		g_string_append_printf(output, "EVEME 1.0\n");
		g_string_append_printf(output, "UserRev: 1.00\n");
		g_string_append_printf(output, "UserComment: Table %i; (%s) %s\n",table,firmware->table_params[table]->table_name,vex_comment);
		g_string_append_printf(output, "Date: %.2i-%.2i-%i\n",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year));

		g_string_append_printf(output, "Time: %.2i:%.2i\n",tm->tm_hour,tm->tm_min);
		g_string_append_printf(output, "Page %i\n",z_page);
		g_string_append_printf(output, "VE Table RPM Range              [%2i]\n",x_bincount);

		for (i=0;i<x_bincount;i++)
			g_string_append_printf(output,"   [%3d] = %3d\n",i,ms_get_ecu_data(canID,x_page,x_base+(i*x_mult),x_size));

		g_string_append_printf(output, "VE Table Load Range (MAP)       [%2i]\n",y_bincount);
		for (i=0;i<y_bincount;i++)
			g_string_append_printf(output,"   [%3d] = %3d\n",i,ms_get_ecu_data(canID,y_page,y_base+(i*y_mult),y_size));

		g_string_append_printf(output,"VE Table                        [%3i][%3i]\n",x_bincount,y_bincount);
		g_string_append_printf(output, "           ");
		for (i=0;i<x_bincount;i++)
		{
			g_string_append_printf(output,"[%3d]",i);
			if (i < (x_bincount-1))
				g_string_append_printf(output, " ");
		}
		g_string_append_printf(output, "\n");
		index = 0;
		for (i=0;i<x_bincount;i++)
		{
			g_string_append_printf(output,"   [%3d] =",i);
			for (j=0;j<y_bincount;j++)
			{
				if (j == 0)
					g_string_append_printf (output,"  %3d",ms_get_ecu_data(canID,z_page,z_base+(index*z_mult),z_size));
				else
					g_string_append_printf (output,"   %3d",ms_get_ecu_data(canID,z_page,z_base+(index*z_mult),z_size));
				index++;
			}
			g_string_append_printf(output,"\n");
		}
	}
	status = g_io_channel_write_chars(
			iochannel,output->str,output->len,&count,NULL);
	if (status != G_IO_STATUS_NORMAL)
		MTXDBG(CRITICAL,_("Error exporting VEX file\n"));
	g_string_free(output,TRUE);

	update_logbar_f("tools_view",NULL,_("VE-Table(s) Exported Successfully\n"),FALSE,FALSE,FALSE);

	if (vex_comment)
		g_free(vex_comment);
	vex_comment = NULL;
	EXIT();
	return TRUE; /* return TRUE on success, FALSE on failure */
}


/*!
 \brief single_table_export() is the export function to dump one Tables
 to a "vex" file.  
 \param iochannel is the pointer to the output channel 
 \param table_num is the integer number of table to export
 \see single_table_import
 */
G_MODULE_EXPORT void single_table_export(GIOChannel *iochannel, gint table_num)
{
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint i = 0;
	gint j = 0;
	gint table = -1;
	gsize count = 0;
	gint index = 0;
	gint x_page = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_base = 0;
	gint y_base = 0;
	gint z_base = 0;
	DataSize x_size = MTX_U08;
	DataSize y_size = MTX_U08;
	DataSize z_size = MTX_U08;
	gint x_mult = 0;
	gint y_mult = 0;
	gint z_mult = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	GIOStatus status;
	GString *output = NULL;
	gint canID = 0;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	canID = firmware->canID;


	/* For Page 0.... */
	output = g_string_sized_new(64); /*pre-allocate 64 chars */
	/*printf("total tables %i\n",firmware->total_tables);*/
	table = table_num;
	x_page = firmware->table_params[table]->x_page;
	y_page = firmware->table_params[table]->y_page;
	z_page = firmware->table_params[table]->z_page;
	x_base = firmware->table_params[table]->x_base;
	y_base = firmware->table_params[table]->y_base;
	z_base = firmware->table_params[table]->z_base;
	x_size = firmware->table_params[table]->x_size;
	y_size = firmware->table_params[table]->y_size;
	z_size = firmware->table_params[table]->z_size;
	x_mult = get_multiplier_f(x_size);
	y_mult = get_multiplier_f(y_size);
	z_mult = get_multiplier_f(z_size);
	x_bincount = firmware->table_params[table]->x_bincount;
	y_bincount = firmware->table_params[table]->y_bincount;

	t = (time_t *)g_malloc(sizeof(time_t));
	time(t);
	tm = localtime(t);
	g_free(t);
	if (vex_comment == NULL)
		vex_comment = g_strdup("No comment given");

	g_string_append_printf(output, "EVEME 1.0\n");
	g_string_append_printf(output, "UserRev: 1.00\n");
	g_string_append_printf(output, "UserComment: Table %i; (%s) %s\n",table,firmware->table_params[table]->table_name,vex_comment);
	g_string_append_printf(output, "Date: %i-%.2i-%i\n",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year));

	g_string_append_printf(output, "Time: %.2i:%.2i\n",tm->tm_hour,tm->tm_min);
	g_string_append_printf(output, "Page %i\n",z_page);
	g_string_append_printf(output, "VE Table RPM Range              [%2i]\n",x_bincount);

	for (i=0;i<x_bincount;i++)
		g_string_append_printf(output,"   [%3d] = %3d\n",i,ms_get_ecu_data(canID,x_page,x_base+(i*x_mult),x_size));

	g_string_append_printf(output, "VE Table Load Range (MAP)       [%2i]\n",y_bincount);
	for (i=0;i<y_bincount;i++)
		g_string_append_printf(output, "   [%3d] = %3d\n",i,ms_get_ecu_data(canID,y_page,y_base+(i*y_mult),y_size));

	g_string_append_printf(output, "VE Table                        [%3i][%3i]\n",x_bincount,y_bincount);
	g_string_append_printf(output, "           ");
	for (i=0;i<x_bincount;i++)
	{
		g_string_append_printf(output, "[%3d]",i);
		if (i < (x_bincount-1))
			g_string_append_printf(output, " ");
	}
	g_string_append_printf(output, "\n");
	index = 0;
	for (i=0;i<x_bincount;i++)
	{
		g_string_append_printf(output, "   [%3d] =",i);
		for (j=0;j<y_bincount;j++)
		{
			if (j == 0)
				g_string_append_printf (output, "  %3d",ms_get_ecu_data(canID,z_page,z_base+(index*z_mult),z_size));
			else
				g_string_append_printf (output, "   %3d",ms_get_ecu_data(canID,z_page,z_base+(index*z_mult),z_size));
			index++;
		}
		g_string_append_printf(output,"\n");
	}
	status = g_io_channel_write_chars(
			iochannel,output->str,output->len,&count,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		MTXDBG(CRITICAL,_("Error exporting VEX file\n"));
	}
	g_string_free(output,TRUE);

	update_logbar_f("tools_view",NULL,_("VE-Table(s) Exported Successfully\n"),FALSE,FALSE,FALSE);

	if (vex_comment)
		g_free(vex_comment);
	vex_comment = NULL;
	EXIT();
	return; /* return TRUE on success, FALSE on failure */
}


/*!
 \brief all_table_import() is called to import Tables from a file.  
 There currently exists a big problem in that newer firmwares (msns-extra 
 and MS-II) have multiple tables per page and the VEX 1.0 spec does NOT 
 account for that, mtx can handle its own exported files but may have issues
 with file from other softwares. the best method is one table per file. 
 \param iochannel is the pointer to the output channel to read 
 \returns TRUE on success, FALSE otherwise
 */
G_MODULE_EXPORT gboolean all_table_import(GIOChannel *iochannel)
{
	gboolean go=TRUE;
	GIOStatus status = G_IO_STATUS_NORMAL;
	Vex_Import *vex = NULL;

	ENTER();
	if (!iochannel)
	{
		MTXDBG(CRITICAL,_("IOChannel undefined, returning!!\n"));
		EXIT();
		return FALSE;
	}
	vex = g_new0(Vex_Import, 1);
	vex->table = -1;

	/*reset_import_flags();*/
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
		MTXDBG(CRITICAL,_("Error seeking to beginning of the file\n"));
	/* process lines while we can */
	while (go)
	{
		status = process_vex_line(vex,iochannel);
		if ((status == G_IO_STATUS_EOF)||(status == G_IO_STATUS_ERROR))
		{
			go = FALSE;
			break;
		}
		/* This check means we got 1 whole ve/spark table, so we feed
		 * it to the ECU clear the data structure and start over...
		 */
		if ((status != G_IO_STATUS_EOF) 
				& (vex->got_page) 
				& (vex->got_load) 
				& (vex->got_rpm) 
				& (vex->got_ve))
		{
			feed_import_data_to_ecu(vex);
			dealloc_vex_struct(vex);
			vex = g_new0(Vex_Import, 1);
			vex->table = -1;
		}
	}
	dealloc_vex_struct(vex);

	gtk_widget_set_sensitive(lookup_widget_f("tools_undo_vex_button"),TRUE);

	if (status == G_IO_STATUS_ERROR)
	{
		MTXDBG(CRITICAL,_("Read was unsuccessful. %i %i %i %i \n"),vex->got_page, vex->got_load, vex->got_rpm, vex->got_ve);
		EXIT();
		return FALSE;
	}
	EXIT();
	return TRUE;
}


/*!
 \brief single_table_import() is called to import a single table  from a file.  
 There currently exists a big problem in that newer firmwares (msns-extra 
 and MS-II) have multiple tables per page and the VEX 1.0 spec does NOT 
 account for that. 
 \param iochannel is the pointer to the output channel to read 
 \param table_num is the table number to import
 the data from.
 */
G_MODULE_EXPORT void single_table_import(GIOChannel *iochannel, gint table_num)
{
	gboolean go=TRUE;
	GIOStatus status = G_IO_STATUS_NORMAL;
	Vex_Import *vex = NULL;

	ENTER();
	if (!iochannel)
	{
		MTXDBG(CRITICAL,_("IOChannel undefined, returning!!\n"));
		EXIT();
		return;
	}
	vex = g_new0(Vex_Import, 1);
	vex->table = table_num;

	/*reset_import_flags();*/
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
		MTXDBG(CRITICAL,_("Error seeking to beginning of the file\n"));
	/* process lines while we can */
	while (go)
	{
		status = process_vex_line(vex,iochannel);
		if ((status == G_IO_STATUS_EOF)||(status == G_IO_STATUS_ERROR))
		{
			go = FALSE;
			break;
		}
		/* This check means we got 1 whole ve/spark table, so we feed
		 * it to the ECU and exit
		 */
		if ((status != G_IO_STATUS_EOF) 
				& (vex->got_page) 
				& (vex->got_load) 
				& (vex->got_rpm) 
				& (vex->got_ve))
		{
			feed_import_data_to_ecu(vex);
			go = FALSE;
		}
	}
	dealloc_vex_struct(vex);

	gtk_widget_set_sensitive(lookup_widget_f("tools_undo_vex_button"),TRUE);

	if (status == G_IO_STATUS_ERROR)
	{
		MTXDBG(CRITICAL,_("Read was unsuccessful. %i %i %i %i \n"),vex->got_page, vex->got_load, vex->got_rpm, vex->got_ve);
		EXIT();
		return;
	}
	EXIT();
	return;
}


/*!
 \brief process_vex_line() is called for to read the VEX file and
 dispatch handlers to process sections of the file.
 \param vex is the pointer to Vex_Import structure.
 \param iochannel is the iopchannel bytestream represented by the VEXfile
 \returns The status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL
 usually
 */
G_MODULE_EXPORT GIOStatus process_vex_line(Vex_Import * vex, GIOChannel *iochannel)
{
	GString *a_line = g_string_new("\0");
	GIOStatus status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	gint num_tests = sizeof(import_handlers)/sizeof(import_handlers[0]);

	ENTER();
	if (status == G_IO_STATUS_NORMAL) 
	{
		for (gint i=0;i<num_tests;i++)
		{
			if (g_strrstr(a_line->str,import_handlers[i].import_tag) != NULL)
			{
				status = handler_dispatch(vex, import_handlers[i].function, import_handlers[i].parsetag,a_line->str, iochannel);
				if (status != G_IO_STATUS_NORMAL)
				{
					MTXDBG(CRITICAL,_("VEX_line parsing ERROR\n"));
					EXIT();
					return status;
				}
				goto breakout;
			}
		}
	}
breakout:
	g_string_free(a_line, TRUE);
	EXIT();
	return status;
}


/*!
 \brief handler_dispatch() calls handlers based on ImportParserFunc passed. 
 It passes off the pointer to the Vex_Import struct, the function arg, 
 the string read, and the pointer to the IOchannel representing the 
 input bytestream.
 \param vex is the pointer to the Vex_Import datastructure.
 \param function is an enumeration used to determine which handler to call.
 \param arg is another enumeration passed to the functions 
 being dispatched from here
 \param string is The current line of the VEXfile just read.  Used for
 handlers than only need 1 lines's worth of data
 \param iochannel is the pointer to the input stream of the 
 vexfile for reading additional data (used by some dispatched functions)
 \see ImportParserFunc
 \see ImportParserArg
 \returns a GIOStatus of the dispatched function (usually G_IO_STATUS_NORMAL
 or G_IO_STATUS_ERROR)
 */
G_MODULE_EXPORT GIOStatus handler_dispatch(Vex_Import *vex, ImportParserFunc function, ImportParserArg arg, gchar * string, GIOChannel *iochannel)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	ENTER();
	switch (function)
	{
		case VEX_HEADER:
			status = process_header(vex, arg, string);
			break;
		case VEX_PAGE:
			status = process_page(vex, string);
			break;
		case VEX_RANGE:
			status = process_vex_range(vex, arg, string, iochannel);
			break;
		case VEX_TABLE:
			status = process_vex_table(vex, string, iochannel);
			break;
	}
	EXIT();
	return status;
}

/*!
 \brief process_header() processes portions of the header of the VEX file 
 and populates the Vex_Import datastructure for this Table
 \param vex is the pointer to the Vex_Import structure
 \param arg is the enumeration of header portion to process
 \param string is the text of the header line to process
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
G_MODULE_EXPORT GIOStatus process_header(Vex_Import *vex, ImportParserArg arg, gchar * string)
{
	gchar ** str_array = NULL;
	gchar *result = NULL;
	gchar *tmpbuf = NULL;

	ENTER();
	if (!string)
	{
		MTXDBG(CRITICAL,_("String passed was NULL\n"));
		EXIT();
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, " ", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	switch (arg)
	{
		case VEX_EVEME:
			vex->version = g_strdup(result);
			tmpbuf = g_strdup_printf(_("VEX Header: EVEME %s"),result);
			update_logbar_f("tools_view", NULL, tmpbuf,FALSE,FALSE,FALSE);
			g_free(tmpbuf);
			break;
		case VEX_USER_REV:	
			vex->revision = g_strdup(result);
			tmpbuf = g_strdup_printf(_("VEX Header: Revision %s"),result);
			update_logbar_f("tools_view", NULL, tmpbuf,FALSE,FALSE,FALSE);
			g_free(tmpbuf);
			break;
		case VEX_USER_COMMENT:	
			vex->comment = g_strdup(result);
			tmpbuf = g_strdup_printf(_("VEX Header: UserComment: %s"),result);
			update_logbar_f("tools_view", NULL,tmpbuf,FALSE,FALSE,FALSE);
			g_free(tmpbuf);
			break;
		case VEX_DATE:	
			vex->date = g_strdup(result);
			tmpbuf = g_strdup_printf(_("VEX Header: Date %s"),result);
			update_logbar_f("tools_view", NULL,tmpbuf,FALSE,FALSE,FALSE);
			g_free(tmpbuf);
			break;
		case VEX_TIME:	
			vex->time = g_strdup(result);
			tmpbuf =  g_strdup_printf(_("VEX Header: Time %s"),result);
			update_logbar_f("tools_view", NULL, tmpbuf,FALSE,FALSE,FALSE);
			g_free(tmpbuf);
			break;
		default:
			break;

	}
	g_free(result);
	EXIT();
	return G_IO_STATUS_NORMAL;

}

/*!
 \brief process_page() extracts the page variable from the VEX file and 
 stores it in the Vex_Import Structure for this table.
 \param vex is the Pointer to the Vex_Import structure
 \param string is the line of VEX file in which to extract the page out of
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
G_MODULE_EXPORT GIOStatus process_page(Vex_Import *vex, gchar *string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar *tmpbuf = NULL;
	gchar ** str_array = NULL;
	gint page = -1;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	if (!string)
	{
		MTXDBG(CRITICAL,_("String passed was NULL\n"));
		EXIT();
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, " ", 2);	
	page = atoi(str_array[1]);
	g_strfreev(str_array);
	if ((page < 0 ) || (page > firmware->total_pages))
	{
		status =  G_IO_STATUS_ERROR;
		tmpbuf = g_strdup_printf(_("VEX Import: Page %i out of range <---ERROR\n"),page);
		update_logbar_f("tools_view","warning",tmpbuf,FALSE,FALSE,FALSE);
		g_free(tmpbuf);
		EXIT();
		return status;
	}
	else
	{
		status = G_IO_STATUS_NORMAL;
		vex->page = page;
		vex->got_page = TRUE;
		tmpbuf = g_strdup_printf(_("VEX Import: Page %i\n"),page);
		update_logbar_f("tools_view",NULL,tmpbuf,FALSE,FALSE,FALSE);
		g_free(tmpbuf);

	}

	status = process_table(vex);
	EXIT();
	return status;
}


/*!
 \brief process_table() extracts the table_number out of the comment field
 stores it in the Vex_Import Structure for this table.
 \param vex is the Pointer to the Vex_Import structure
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
G_MODULE_EXPORT GIOStatus process_table(Vex_Import *vex)
{
	gchar **string = NULL;
	gchar *tmpbuf = NULL;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	/* Search for out magic semicolon, if missing AND multi-table/page
	 * firmware, ABORT */
	if ((vex->table < 0) && (vex->table <= firmware->total_tables))
	{
		if ((!g_strrstr(vex->comment,";")) && (firmware->total_tables == firmware->total_tables))
		{
			update_logbar_f("tools_view","warning",_("VEX Import: Multi Table per page firmware,\nbut Table number is not defined in comment field, load aborted!!!\n"),FALSE,FALSE,FALSE);
			EXIT();
			return G_IO_STATUS_ERROR;
		}

		string = g_strsplit(vex->comment,";",-1);
		if ((string[0] == NULL) || (!g_strrstr(string[0],"Table")))
		{
			update_logbar_f("tools_view","warning",_("VEX Import: Multi Table per page firmware,\n\tbut Table number is not defined in comment field, load aborted!!!\n"),FALSE,FALSE,FALSE);
			vex->table = -1;
			g_strfreev(string);
			EXIT();
			return G_IO_STATUS_ERROR;
		}
		else
		{
			/* the +5 gets us past theword "Table" */
			vex->table = (gint)g_ascii_strtod(string[0]+5,NULL);
			g_strfreev(string);
		}
	}

	tmpbuf = g_strdup_printf(_("VEX Import: Table %i\n"),vex->table);
	update_logbar_f("tools_view",NULL,tmpbuf,FALSE,FALSE,FALSE);
	g_free(tmpbuf);
	EXIT();
	return G_IO_STATUS_NORMAL;
}


/*!
 \brief read_number_from_line() is used to read the RPM/LOAD values from the
 VEX file and store them in the Vex_Import structure
 \param dest is the pointer to array within the Vex_Import struct
 \param iochannel is the pointer to the iochannel representing the
 VEX file
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
G_MODULE_EXPORT GIOStatus read_number_from_line(gint *dest, GIOChannel *iochannel)
{
	GIOStatus status;
	gchar ** str_array = NULL;
	gchar * result = NULL;
	GString *a_line = g_string_new("\0");
	ENTER();
	status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		EXIT();
		return status;
	}

	/* Make sure the line contains an "=" sign, otherwise we'll segfault*/
	if (strstr(a_line->str, "=") != NULL)
	{
		str_array = g_strsplit(a_line->str, "=", 2);
		result = g_strdup(str_array[1]);	
		g_strfreev(str_array);
		*dest = atoi(result);
		g_free(result);
	}
	else
		status = G_IO_STATUS_ERROR;

	g_string_free(a_line, TRUE);
	EXIT();
	return status;
}

/*!
 \brief process_vex_range() is used to read the vex_ranges for RPM/LOAD 
 allocate the memory for storing the data and call the needed functions to
 read the values into the arrays.
 \see read_number_from_line
 \param vex is the pointer to the Vex_Import structure
 \param arg is the enumeration to decide which range we are going to
 read
 \param string is the Line of text passed to parse.
 \param iochannel is the Pointer to iochannel representing VEXfile for
 retrieving additional data.
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
G_MODULE_EXPORT GIOStatus process_vex_range(Vex_Import *vex, ImportParserArg arg, gchar * string, GIOChannel *iochannel)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gint i = 0;
	gint value = 0;
	gint num_bins = 0;
	gchar ** str_array = NULL;
	gchar * result = NULL;

	ENTER();
	if (!string)
	{
		MTXDBG(CRITICAL,_("String passed was NULL\n"));
		EXIT();
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, "[", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	str_array = g_strsplit(result, "]", 2);
	g_free(result);
	result = g_strdup(str_array[0]);	

	num_bins = atoi(result);
	g_free(result);
	/* Allocate memory for array of values... :) */
	switch(arg)
	{
		case VEX_RPM_RANGE:
			vex->total_x_bins = num_bins;
			vex->x_bins = g_new0(gint, num_bins);
			vex->got_rpm = TRUE;
			break;
		case VEX_LOAD_RANGE:
			vex->total_y_bins = num_bins;
			vex->y_bins = g_new0(gint, num_bins);
			vex->got_load = TRUE;
			break;
		default:
			break;
	}

	for (i=0; i<num_bins; i++) 
	{
		status = read_number_from_line(&value,iochannel);
		if (status != G_IO_STATUS_NORMAL) 
		{
			update_logbar_f("tools_view","warning",_("VEX Import: File I/O Read problem, file may be incomplete <---ERROR\n"),FALSE,FALSE,FALSE);
			break;
		}
		switch (arg)
		{
			case VEX_RPM_RANGE:
				vex->x_bins[i] = value;
				break;
			case VEX_LOAD_RANGE:
				vex->y_bins[i] = value;
				break;
			default:
				break;
		}
	}
	if (status == G_IO_STATUS_NORMAL)
	{
		if (arg == VEX_RPM_RANGE)
			update_logbar_f("tools_view",NULL,_("VEX Import: RPM bins loaded successfully \n"),FALSE,FALSE,FALSE);
		if (arg == VEX_LOAD_RANGE)
			update_logbar_f("tools_view",NULL,_("VEX Import: LOAD bins loaded successfully \n"),FALSE,FALSE,FALSE);
	}
	EXIT();
	return status;
}


/*!
 \brief process_vex_table() reads, processes and stores the Table data into
 the Vex_Import structure in preparation for import to the ECU.
 \param vex is the *) pointer to the Vex_Import structure
 \param string is the pointer to the current line of the VEXfile
 \param iochannel is the Pointer to the iochannel representing the
 VEX file for reading more data
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
G_MODULE_EXPORT GIOStatus process_vex_table(Vex_Import *vex, gchar * string, GIOChannel *iochannel)
{
	gint i = 0, j = 0;
	GString *a_line;
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array = NULL;
	gchar ** str_array2 = NULL;
	gchar * result = NULL;
	gchar *pos = NULL;
	gchar *numbers = NULL;
	gint value = 0;
	gint x_bins = 0;
	gint y_bins = 0;

	ENTER();
	if (!string)
	{
		MTXDBG(CRITICAL,_("String passed was NULL\n"));
		EXIT();
		return G_IO_STATUS_ERROR;
	}
	/* Get first number of [  x][  y] in the string line */
	str_array = g_strsplit(string, "[", 3);
	result = g_strdup(str_array[1]);	
	str_array2 = g_strsplit(result, "]", 2);
	g_free(result);
	result = g_strdup(str_array2[0]);	
	g_strfreev(str_array2);
	x_bins = atoi(result);
	g_free(result);

	/* Get first number of [  x][  y] in the string line */
	result = g_strdup(str_array[2]);
	g_strfreev(str_array);
	str_array2 = g_strsplit(result, "]", 2);
	g_free(result);
	result = g_strdup(str_array2[0]);
	g_strfreev(str_array2);
	y_bins = atoi(result);	
	g_free(result);

	vex->total_tbl_bins = x_bins*y_bins;
	vex->tbl_bins = g_new0(gint,(x_bins * y_bins));

	/* Need to skip down one line to the actual data.... */
	a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		g_string_free(a_line, TRUE);
		update_logbar_f("tools_view","warning",_("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n"),FALSE,FALSE,FALSE);
		EXIT();
		return status;
	}
	g_string_free(a_line, TRUE);

	/* iterate over table */
	for (i=0; i<y_bins; i++) {
		a_line = g_string_new("\0");
		status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
		if (status != G_IO_STATUS_NORMAL) 
		{
			g_string_free(a_line, TRUE);
			update_logbar_f("tools_view","warning", _("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n"),FALSE,FALSE,FALSE);
			break;
		}
		pos = g_strrstr(a_line->str,"=\0");
		numbers = g_strdup(pos+sizeof(char));

		/* process a row */
		for (j=0; j<x_bins; j++) 
		{
			value = (int)strtol(numbers,&numbers,10);
			vex->tbl_bins[j+(i*x_bins)] = value;
		}		
		g_string_free(a_line, TRUE);
	}
	if (status == G_IO_STATUS_NORMAL)
	{
		vex->got_ve = TRUE;
		update_logbar_f("tools_view",NULL,_("VEX Import: VE-Table loaded successfully\n"),FALSE,FALSE,FALSE);
	}
	EXIT();
	return status;
}


/*!
 \brief vex_comment_parse() stores the comment field  as entered by the
 user on the Megatunix GUI for VEX export.
 \param widget is the pointer to textentry widget where user enters
 the comment
 \param data is unused
 \returns TRUE
 */
G_MODULE_EXPORT gboolean vex_comment_parse(GtkWidget *widget, gpointer data)
{
	/* Gets data from VEX comment field in tools gui and stores it 
	 * so that it gets written to the vex file 
	 */
	ENTER();
	vex_comment = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	update_logbar_f("tools_view",NULL,_("VEX Comment Stored\n"),FALSE,FALSE,FALSE);
	EXIT();
	return TRUE;
}


/*!
 \brief dealloc_vex_struct() deallocates the memory used by the Vex_Import
 datastructure
 \param vex is the Pointer to the Vex_Import structure 
 to deallocate.
 */
G_MODULE_EXPORT void dealloc_vex_struct(Vex_Import *vex)
{
	ENTER();
	if (vex->version)
		g_free(vex->version);
	if (vex->revision)
		g_free(vex->revision);
	if (vex->comment)
		g_free(vex->comment);
	if (vex->date)
		g_free(vex->date);
	if (vex->time)
		g_free(vex->time);
	if (vex->x_bins)
		g_free(vex->x_bins);
	if (vex->y_bins)
		g_free(vex->y_bins);
	if (vex->tbl_bins)
		g_free(vex->tbl_bins);
	if (vex)
		g_free(vex);
	EXIT();
	return;
}


/*!
 \brief feed_import_data_to_ecu() Forwards the data in the VEX file to the
 ECU.  NOTE this may have problems with firmware using multiple tables in
 a page, as the VEX format 1.0 does NOT handle that condition.
 \param vex is the pointer to the Vex_Impot datastructure.
 */
G_MODULE_EXPORT void feed_import_data_to_ecu(Vex_Import *vex)
{
	gint i = 0;
	gchar *tmpbuf = NULL;
	guint8 **ecu_data = NULL;
	guint8 **ecu_data_backup = NULL;
	guint8 *data = NULL;
	gchar * msgbuf = NULL;
	guchar *ptr = NULL;
	guint16 *ptr16 = NULL;
	guint32 *ptr32 = NULL;
	gint total = 0;
	gint canID = 0;
	gint page = -1;
	gint base = 0;
	DataSize size = MTX_U08;
	gint mult = 0;
	gint table = -1;
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	ecu_data = firmware->ecu_data;
	ecu_data_backup = firmware->ecu_data_backup;

	/* Since we assume the page is where the table is this can cause
	 * major problems with some firmwares that use two tables inside
	 * of one page....
	 */
	page = vex->page;
	table = vex->table;
	if ((table < 0) || (table >= firmware->total_tables))
	{
		MTXDBG(CRITICAL,_("Table passed (%i) is out of range(%i)\n"),table,firmware->total_tables);
		EXIT();
		return;
	}

	/* If dimensions do NOT match, ABORT!!! */
	if (firmware->table_params[table]->x_bincount != vex->total_x_bins)
	{
		msgbuf = g_strdup_printf(_("VEX Import: number of RPM bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n"),firmware->table_params[table]->x_bincount,vex->total_x_bins);
		update_logbar_f("tools_view","warning",msgbuf,FALSE,FALSE,FALSE);
		MTXDBG(CRITICAL,msgbuf);
		g_free(msgbuf);
		EXIT();
		return;
	}
	if (firmware->table_params[table]->y_bincount != vex->total_y_bins)
	{
		msgbuf = g_strdup_printf(_("VEX Import: number of LOAD bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n"),firmware->table_params[table]->y_bincount,vex->total_y_bins);
		update_logbar_f("tools_view","warning",msgbuf,FALSE,FALSE,FALSE);
		MTXDBG(CRITICAL,msgbuf);
		g_free(msgbuf);
		EXIT();
		return;
	}

	/* Backup the ALL pages of data first... */
	for (i=0;i<firmware->total_pages;i++)
	{
		if (!firmware->page_params[i]->dl_by_default)
			continue;
		memset((void *)ecu_data_backup[i], 0, firmware->page_params[i]->length);
		memcpy(ecu_data_backup[i], ecu_data[i],firmware->page_params[i]->length);
	}

	canID = firmware->canID;
	page = firmware->table_params[table]->x_page;
	base = firmware->table_params[table]->x_base;
	size = firmware->table_params[table]->x_size;
	mult = get_multiplier_f(size);
	if (firmware->chunk_support)
	{
		total = vex->total_x_bins;
		data = (guint8 *)g_malloc0(mult*total);;
		if (mult == 1)
		{
			ptr = (guchar *)data;
			for (i=0;i<total;i++)
				ptr[i]=vex->x_bins[i];
		}
		if (mult == 2)
		{
			ptr16 = (guint16 *)data;
			for (i=0;i<total;i++)
			{
				if (firmware->bigendian)
					ptr16[i]=GINT16_TO_BE(vex->x_bins[i]);
				else
					ptr16[i]=GINT16_TO_LE(vex->x_bins[i]);
			}
		}
		if (mult == 4)
		{
			ptr32 = (guint32 *)data;
			for (i=0;i<total;i++)
			{
				if (firmware->bigendian)
					ptr32[i]=GINT_TO_BE(vex->x_bins[i]);
				else
					ptr32[i]=GINT_TO_LE(vex->x_bins[i]);
			}
		}
		ms_chunk_write(canID,page,base,(total*mult),data);
	}
	else
	{
		for (i=0;i<vex->total_x_bins;i++)
		{
			if (vex->x_bins[i] != ms_get_ecu_data_last(canID,page,base+(i*mult),size))
				ms_send_to_ecu(canID,page,base+i,size, vex->x_bins[i], TRUE);
		}
	}

	canID = firmware->canID;
	page = firmware->table_params[table]->y_page;
	base = firmware->table_params[table]->y_base;
	size = firmware->table_params[table]->y_size;
	mult = get_multiplier_f(size);
	if (firmware->chunk_support)
	{
		total = vex->total_y_bins;
		data = (guint8 *)g_malloc0(mult*total);
		if (mult == 1)
		{
			ptr = (guchar *)data;
			for (i=0;i<total;i++)
				ptr[i]=vex->y_bins[i];
		}
		if (mult == 2)
		{
			ptr16 = (guint16 *)data;
			for (i=0;i<total;i++)
			{
				if (firmware->bigendian)
					ptr16[i]=GINT16_TO_BE(vex->y_bins[i]);
				else
					ptr16[i]=GINT16_TO_LE(vex->y_bins[i]);
			}
		}
		if (mult == 4)
		{
			ptr32 = (guint32 *)data;
			for (i=0;i<total;i++)
			{
				if (firmware->bigendian)
					ptr32[i]=GINT_TO_BE(vex->y_bins[i]);
				else
					ptr32[i]=GINT_TO_LE(vex->y_bins[i]);
			}
		}
		ms_chunk_write(canID,page,base,(total*mult),data);
	}
	else
	{
		for (i=0;i<vex->total_y_bins;i++)
		{
			if (vex->y_bins[i] != ms_get_ecu_data_last(canID,page,base+i,size))
				ms_send_to_ecu(canID,page,base+(i*mult),size,vex->y_bins[i], TRUE);
		}
	}

	canID = firmware->canID;
	page = firmware->table_params[table]->z_page;
	base = firmware->table_params[table]->z_base;
	size = firmware->table_params[table]->z_size;
	mult = get_multiplier_f(size);
	if (firmware->chunk_support)
	{
		total = (vex->total_y_bins)*(vex->total_x_bins);
		data = (guint8 *)g_malloc0(mult*total);
		if (mult == 1)
		{
			ptr = (guchar *)data;
			for (i=0;i<total;i++)
				ptr[i]=vex->tbl_bins[i];
		}
		if (mult == 2)
		{
			ptr16 = (guint16 *)data;
			for (i=0;i<total;i++)
			{
				if (firmware->bigendian)
					ptr16[i]=GINT16_TO_BE(vex->tbl_bins[i]);
				else
					ptr16[i]=GINT16_TO_LE(vex->tbl_bins[i]);
			}
		}
		if (mult == 4)
		{
			ptr32 = (guint32 *)data;
			for (i=0;i<total;i++)
			{
				if (firmware->bigendian)
					ptr32[i]=GINT_TO_BE(vex->tbl_bins[i]);
				else
					ptr32[i]=GINT_TO_LE(vex->tbl_bins[i]);
			}
		}
		ms_chunk_write(canID,page,base,(total*mult),data);
	}
	else
	{
		for (i=0;i<((vex->total_y_bins)*(vex->total_x_bins));i++)
		{
			if (vex->tbl_bins[i] != ms_get_ecu_data_last(canID,page,base+i,size))
				ms_send_to_ecu(canID,page,base+(i*mult),size,vex->tbl_bins[i], TRUE);
		}
	}
	io_cmd_f(firmware->burn_all_command,NULL);

	tmpbuf = g_strdup_printf(_("VEX Import: VEtable on page %i updated with data from the VEX file\n"),vex->page);
	update_logbar_f("tools_view",NULL,tmpbuf,FALSE,FALSE,FALSE);
	g_free(tmpbuf);
	EXIT();
	return;
}


/*!
 \brief revert_to_previous_data() reverts the VEX import by using the backup
 of the internal datastructures.
 */
G_MODULE_EXPORT void revert_to_previous_data(void)
{
	gint canID=0;
	gint page=0;
	gint offset=0;
	gint total = 0;
	guchar *data = NULL;
	GModule *module = NULL;
	PostFunction *pf = NULL;
	GArray *pfuncs = NULL;
	guint8 **ecu_data_backup = NULL;
	/* Called to back out a load of a VEtable from VEX import */
	Firmware_Details *firmware = NULL;

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	ecu_data_backup = firmware->ecu_data_backup;

	for (page=0;page<firmware->total_pages;page++)
	{
		if (!firmware->page_params[page]->dl_by_default)
			continue;
		if (firmware->chunk_support)
		{
			total = firmware->page_params[page]->length;
			data = g_new0(guchar,total);
			for (offset=0;offset<total;offset++)
				data[offset]=ms_get_ecu_data_backup(canID,page,offset,MTX_U08);
			ms_chunk_write(canID,page,0,total,data);

		}
		else
		{
			for (offset = 0;offset<firmware->page_params[page]->length;offset++)
			{
				if (ms_get_ecu_data_backup(canID,page,offset,MTX_U08) != ms_get_ecu_data(canID,page,offset,MTX_U08))
				{
					ms_send_to_ecu(canID,page,offset,MTX_U08,ecu_data_backup[page][offset], FALSE);
				}
			}
		}
	}
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("update_ecu_controls_pf");
	if (module)
		g_module_symbol(module,pf->name,(void **)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	g_module_close(module);
	io_cmd_f(NULL,pfuncs);

	gtk_widget_set_sensitive(lookup_widget_f("tools_undo_vex_button"),FALSE);
	update_logbar_f("tools_view","warning",_("Reverting to previous settings....\n"),FALSE,FALSE,FALSE);
	io_cmd_f(firmware->burn_all_command,NULL);
	EXIT();
	return;
}
