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

#include <config.h>
#include <comms.h>
#include <datamgmt.h>
#include <datalogging_gui.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <firmware.h>
#include <fileio.h>
#include <getfiles.h>
#include <gui_handlers.h>
#include <notifications.h>
#include <serialio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vex_support.h>
#include <xmlcomm.h>
#include <threads.h>

gchar *vex_comment;
extern gint dbg_lvl;
extern GObject *global_data;

/*!
 \brief import_handlers structure is used to hold the list of string 
 tags to search for and the function and function args to pass to the 
 appropriate handlers.
 */
static struct
{
	gchar *import_tag;		/* string to find.. */
	ImportParserFunc function;	/* Function to call... */
	ImportParserArg parsetag;	/* Enum Tag fed to function... */

} import_handlers[] = 
{
	{ "EVEME", HEADER, VEX_EVEME},
	{ "UserRev:", HEADER, VEX_USER_REV}, 
	{ "UserComment:", HEADER, VEX_USER_COMMENT},
	{ "Date:", HEADER, VEX_DATE},
	{ "Time:", HEADER, VEX_TIME},
	{ "Page", PAGE, VEX_NONE},
	{ "VE Table RPM Range\0", RANGE, VEX_RPM_RANGE},
	{ "VE Table Load Range\0", RANGE, VEX_LOAD_RANGE},
	{ "VE Table\0", TABLE, VEX_NONE}
};


EXPORT gboolean select_vex_for_export(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern gboolean interrogated;
	extern GtkWidget *main_window;

	if (!interrogated)
		return FALSE;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_VexFiles");
	fileio->title = g_strdup("Save your VEX file");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->default_filename= g_strdup("VEX_Backup.vex");
	fileio->default_extension= g_strdup("vex");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for VEX export\n"),FALSE,FALSE);
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (!iochannel)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("File open FAILURE! \n"),FALSE,FALSE);
		return FALSE;
	}
	if (vex_comment == NULL)
		update_logbar("tools_view",NULL,g_strdup("VEX File Opened. VEX Comment undefined, exporting without one.\n"),FALSE,FALSE);
	else
		update_logbar("tools_view",NULL,g_strdup("VEX File Opened. VEX Comment already stored.\n"),FALSE,FALSE);
	all_table_export(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	return TRUE;
}

void select_table_for_export(gint table_num)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern GtkWidget *main_window;
	extern gboolean interrogated;
	extern Firmware_Details *firmware;
	extern GtkWidget *main_window;

	if (!interrogated)
		return;

	if ((table_num < 0) || (table_num >= firmware->total_tables))
	{
		return;
	}

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_VexFiles");
	fileio->title = g_strdup("Save your VEX file");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->default_filename= g_strdup_printf("%s.vex",g_strdelimit(firmware->table_params[table_num]->table_name," ",'_'));
	fileio->default_extension= g_strdup("vex");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for VEX export\n"),FALSE,FALSE);
		return;
	}

	iochannel = g_io_channel_new_file(filename, "a+",NULL);
	if (!iochannel)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("File open FAILURE! \n"),FALSE,FALSE);
		return;
	}
	if (vex_comment == NULL)
		update_logbar("tools_view",NULL,g_strdup("VEX File Opened. VEX Comment undefined, exporting without one.\n"),FALSE,FALSE);
	else
		update_logbar("tools_view",NULL,g_strdup("VEX File Opened. VEX Comment already stored.\n"),FALSE,FALSE);
	single_table_export(iochannel,table_num);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	return;
}


EXPORT gboolean select_vex_for_import(GtkWidget *widget, gpointer data)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern GtkWidget *main_window;
	extern GHashTable *dynamic_widgets;
	extern gboolean interrogated;

	if (!interrogated)
		return FALSE;

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_VexFiles");
	fileio->title = g_strdup("Select your VEX file to import");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for VEX import\n"),FALSE,FALSE);
		return FALSE;
	}

	iochannel = g_io_channel_new_file(filename, "r+",NULL);
	if (!iochannel)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("File open FAILURE! \n"),FALSE,FALSE);
		return FALSE;
	}
	update_logbar("tools_view",NULL,g_strdup("VEX File Closed\n"),FALSE,FALSE);
	gtk_entry_set_text(GTK_ENTRY(g_hash_table_lookup(dynamic_widgets,"tools_vex_comment_entry")),"");

	all_table_import(iochannel);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	return TRUE;
}


void select_table_for_import(gint table_num)
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	GIOChannel *iochannel = NULL;
	extern GtkWidget *main_window;
	extern GHashTable *dynamic_widgets;
	extern gboolean interrogated;
	extern Firmware_Details *firmware;

	if (!interrogated)
		return;

	if ((table_num < 0) || (table_num >= firmware->total_tables))
	{
		return;
	}

	fileio = g_new0(MtxFileIO ,1);
	fileio->external_path = g_strdup("MTX_VexFiles");
	fileio->parent = main_window;
	fileio->on_top = TRUE;
	fileio->title = g_strdup("Select your VEX file to import");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;

	filename = choose_file(fileio);
	if (filename == NULL)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("NO FILE chosen for VEX import\n"),FALSE,FALSE);
		return;
	}

	iochannel = g_io_channel_new_file(filename, "r+",NULL);
	if (!iochannel)
	{
		update_logbar("tools_view",g_strdup("warning"),g_strdup("File open FAILURE! \n"),FALSE,FALSE);
		return;
	}
	update_logbar("tools_view",NULL,g_strdup("VEX File Closed\n"),FALSE,FALSE);
	gtk_entry_set_text(GTK_ENTRY(g_hash_table_lookup(dynamic_widgets,"tools_vex_comment_entry")),"");

	single_table_import(iochannel,table_num);
	g_io_channel_shutdown(iochannel,TRUE,NULL);
	g_io_channel_unref(iochannel);
	free_mtxfileio(fileio);
	return;
}


/*!
 \brief all_table_export() is the export function to dump all Tables to a "vex"
 file.  It has been modified to handler multiple tables per page.  
 There is a major problem with this in that the currect VEX 1.0 spec 
 doesn't allow for multiple tables per page, so import is likely to be 
 a problem.
 \param iochannel a pointer to the output channel 
 to write the data to.
 \see all_table_import
 \returns TRUE on success, FALSE on failure
 */
gboolean all_table_export(GIOChannel *iochannel)
{
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint i = 0;
	gint j = 0;
	gint z = 0;
	gint table = -1;
	gsize count = 0;
	gint index = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_page = 0;
	gint y_base = 0;
	gint z_base = 0;
	gint x_base = 0;
	DataSize y_size = 0;
	DataSize z_size = 0;
	DataSize x_size = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	extern Firmware_Details *firmware;
	gint canID = firmware->canID;
	GIOStatus status;
	GString *output = NULL;


	/* For Page 0.... */
	output = g_string_sized_new(64); /*pre-allocate 64 chars */
	/*printf("total tables %i\n",firmware->total_tables);*/
	for (z=0;z<firmware->total_tables;z++)
	{
		table = z;
		z_page = firmware->table_params[table]->z_page;
		x_page = firmware->table_params[table]->x_page;
		y_page = firmware->table_params[table]->y_page;
		z_base = firmware->table_params[table]->z_base;
		y_base = firmware->table_params[table]->y_base;
		x_base = firmware->table_params[table]->x_base;
		z_size = firmware->table_params[table]->z_size;
		y_size = firmware->table_params[table]->y_size;
		x_size = firmware->table_params[table]->x_size;
		y_bincount = firmware->table_params[table]->y_bincount;
		x_bincount = firmware->table_params[table]->x_bincount;

		t = g_malloc(sizeof(time_t));
		time(t);
		tm = localtime(t);
		g_free(t);
		if (vex_comment == NULL)
			vex_comment = g_strdup("No comment given");

		g_string_append_printf(output, "EVEME 1.0\n");
		g_string_append_printf(output, "UserRev: 1.00\n");
		g_string_append_printf(output,"UserComment: Table %i; (%s) %s\n",table,firmware->table_params[table]->table_name,vex_comment);
		g_string_append_printf(output,"Date: %.2i-%.2i-%i\n",1+(tm->tm_mon),tm->tm_mday,1900+(tm->tm_year));

		g_string_append_printf(output,"Time: %.2i:%.2i\n",tm->tm_hour,tm->tm_min);
		g_string_append_printf(output,"Page %i\n",z_page);
		g_string_append_printf(output,"VE Table RPM Range              [%2i]\n",x_bincount);

		for (i=0;i<x_bincount;i++)
			g_string_append_printf(output,"   [%3d] = %3d\n",i,get_ecu_data(canID,x_page,x_base+i,x_size));

		g_string_append_printf(output, "VE Table Load Range (MAP)       [%2i]\n",y_bincount);
		for (i=0;i<y_bincount;i++)
			g_string_append_printf(output,"   [%3d] = %3d\n",i,get_ecu_data(canID,y_page,y_base+i,y_size));

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
					g_string_append_printf (output,"  %3d",get_ecu_data(canID,z_page,index+z_base,z_size));
				else
					g_string_append_printf (output,"   %3d",get_ecu_data(canID,z_page,index+z_base,z_size));
				index++;
			}
			g_string_append_printf(output,"\n");
		}
	}
	status = g_io_channel_write_chars(
			iochannel,output->str,output->len,&count,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": all_table_export()\n\tError exporting VEX file\n"));
	}
	g_string_free(output,TRUE);

	update_logbar("tools_view",NULL,g_strdup("VE-Table(s) Exported Successfully\n"),FALSE,FALSE);

	if (vex_comment)
		g_free(vex_comment);
	vex_comment = NULL;
	return TRUE; /* return TRUE on success, FALSE on failure */
}


/*!
 \brief single_table_export() is the export function to dump one Tables
 to a "vex" file.  
 \param iochannel a pointer to the output channel 
 \param table_num, integer number of table to export
 \see single_table_import
 \returns TRUE on success, FALSE on failure
 */
void single_table_export(GIOChannel *iochannel, gint table_num)
{
	struct tm *tm = NULL;
	time_t *t = NULL;
	gint i = 0;
	gint j = 0;
	gint table = -1;
	gsize count = 0;
	gint index = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_page = 0;
	gint y_base = 0;
	gint z_base = 0;
	gint x_base = 0;
	gint y_size = 0;
	gint z_size = 0;
	gint x_size = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	extern Firmware_Details *firmware;
	GIOStatus status;
	GString *output = NULL;
	gint canID = firmware->canID;


	/* For Page 0.... */
	output = g_string_sized_new(64); /*pre-allocate 64 chars */
	/*printf("total tables %i\n",firmware->total_tables);*/
	table = table_num;
	z_page = firmware->table_params[table]->z_page;
	x_page = firmware->table_params[table]->x_page;
	y_page = firmware->table_params[table]->y_page;
	z_base = firmware->table_params[table]->z_base;
	y_base = firmware->table_params[table]->y_base;
	x_base = firmware->table_params[table]->x_base;
	z_size = firmware->table_params[table]->z_size;
	y_size = firmware->table_params[table]->y_size;
	x_size = firmware->table_params[table]->x_size;
	y_bincount = firmware->table_params[table]->y_bincount;
	x_bincount = firmware->table_params[table]->x_bincount;

	t = g_malloc(sizeof(time_t));
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
		g_string_append_printf(output,"   [%3d] = %3d\n",i,get_ecu_data(canID,x_page,x_base+i,x_size));

	g_string_append_printf(output, "VE Table Load Range (MAP)       [%2i]\n",y_bincount);
	for (i=0;i<y_bincount;i++)
		g_string_append_printf(output, "   [%3d] = %3d\n",i,get_ecu_data(canID,y_page,y_base+i,y_size));

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
				g_string_append_printf (output, "  %3d",get_ecu_data(canID,z_page,index+z_base,z_size));
			else
				g_string_append_printf (output, "   %3d",get_ecu_data(canID,z_page,index+z_base,z_size));
			index++;
		}
		g_string_append_printf(output,"\n");
	}
	status = g_io_channel_write_chars(
			iochannel,output->str,output->len,&count,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": all_table_export()\n\tError exporting VEX file\n"));
	}
	g_string_free(output,TRUE);

	update_logbar("tools_view",NULL,g_strdup("VE-Table(s) Exported Successfully\n"),FALSE,FALSE);

	if (vex_comment)
		g_free(vex_comment);
	vex_comment = NULL;
	return; /* return TRUE on success, FALSE on failure */
}


/*!
 \brief all_table_import() is called to import Tables from a file.  
 There currently exists a big problem in that newer firmwares (msns-extra 
 and MS-II) have multiple tables per page and the VEX 1.0 spec does NOT 
 account for that. 
 \param iochannel pointer to the output channel to read 
 the data from.
 */
gboolean all_table_import(GIOChannel *iochannel)
{
	gboolean go=TRUE;
	GIOStatus status = G_IO_STATUS_NORMAL;
	Vex_Import *vex = NULL;
	GModule *module = NULL;
	PostFunction *pf = NULL;
	GArray *pfuncs = NULL;
	extern GHashTable *dynamic_widgets;

	if (!iochannel)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": all_table_import()\n\tIOChannel undefined, returning!!\n"));
		return FALSE;
	}
	vex = g_new0(Vex_Import, 1);
	vex->table = -1;

	/*reset_import_flags();*/
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": all_table_import()\n\tError seeking to beginning of the file\n"));
	}
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

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_undo_vex_button"),TRUE);

	if (status == G_IO_STATUS_ERROR)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": all_table_import()\n\tRead was unsuccessful. %i %i %i %i \n",vex->got_page, vex->got_load, vex->got_rpm, vex->got_ve));
		return FALSE;
	}
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("update_ve_const");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	g_module_close(module);
	io_cmd(NULL,pfuncs);
	return TRUE;
}


/*!
 \brief single_table_import() is called to import a single from a file.  
 There currently exists a big problem in that newer firmwares (msns-extra 
 and MS-II) have multiple tables per page and the VEX 1.0 spec does NOT 
 account for that. 
 \param iochannel pointer to the output channel to read 
 \param table_num,  table number to import
 the data from.
 */
void single_table_import(GIOChannel *iochannel, gint table_num)
{
	gboolean go=TRUE;
	GIOStatus status = G_IO_STATUS_NORMAL;
	Vex_Import *vex = NULL;
	GModule *module = NULL;
	PostFunction *pf = NULL;
	GArray *pfuncs = NULL;
	extern GHashTable *dynamic_widgets;

	if (!iochannel)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": single_table_import()\n\tIOChannel undefined, returning!!\n"));
		return;
	}
	vex = g_new0(Vex_Import, 1);
	vex->table = table_num;

	/*reset_import_flags();*/
	status = g_io_channel_seek_position(iochannel,0,G_SEEK_SET,NULL);
	if (status != G_IO_STATUS_NORMAL)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": single_table_import()\n\tError seeking to beginning of the file\n"));
	}
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

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_undo_vex_button"),TRUE);

	if (status == G_IO_STATUS_ERROR)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": single_table_import()\n\tRead was unsuccessful. %i %i %i %i \n",vex->got_page, vex->got_load, vex->got_rpm, vex->got_ve));
		return;
	}
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("update_ve_const");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	g_module_close(module);
	io_cmd(NULL,pfuncs);
	return;
}


/*!
 \brief process_vex_line() is called for to read the VEX file and
 dispatch handlers to process sections of the file.
 \param vex (Vex_Import *) pointer to Vex_Import structure.
 \param iochannel (GIOChannel *) the bytestream represented by the VEXfile
 \returns The status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL
 usually
 */
GIOStatus process_vex_line(Vex_Import * vex, GIOChannel *iochannel)
{
	GString *a_line = g_string_new("\0");
	GIOStatus status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	gint i = 0;
	gint num_tests = sizeof(import_handlers)/sizeof(import_handlers[0]);

	if (status == G_IO_STATUS_NORMAL) 
	{
		for (i=0;i<num_tests;i++)
		{
			if (g_strrstr(a_line->str,import_handlers[i].import_tag) != NULL)
			{
				status = handler_dispatch(vex, import_handlers[i].function, import_handlers[i].parsetag,a_line->str, iochannel);
				if (status != G_IO_STATUS_NORMAL)
				{
					if (dbg_lvl & CRITICAL)
						dbg_func(g_strdup(__FILE__": process_vex_line()\n\tVEX_line parsing ERROR\n"));
					return status;
				}
				goto breakout;
			}
		}
	}
breakout:
	g_string_free(a_line, TRUE);
	return status;
}


/*!
 \brief handler_dispatch() calls handlers based on ImportParserFunc passed. 
 It passes off the pointer to the Vex_Import struct, the function arg, 
 the string read, and the pointer to the IOchannel representing the 
 input bytestream.
 \param vex (Vex_Import *) The pointer to the Vex_Import datastructure.
 \param function (ImportParserFunc) an enumeration used to determine 
 which handler to call.
 \param arg (ImportParserArg) another enumeration passed to the functiosn 
 being dispatched from here
 \param string (gchar *) The current line of the VEXfile just read.  Used for
 handlers than only need 1 lines's worth of data
 \param iochannel (GIOChannel *) the pointer to the input stream of the 
 vexfile for reading additional data (used by some dispatched functions)
 \see ImportParserFunc
 \see ImportParserArg
 \returns a GIOStatus of the dispatched function (usually G_IO_STATUS_NORMAL
 or G_IO_STATUS_ERROR)
 */
GIOStatus handler_dispatch(Vex_Import *vex, ImportParserFunc function, ImportParserArg arg, gchar * string, GIOChannel *iochannel)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	switch (function)
	{
		case HEADER:
			status = process_header(vex, arg, string);
			break;
		case PAGE:
			status = process_page(vex, string);
			break;
		case RANGE:
			status = process_vex_range(vex, arg, string, iochannel);
			break;
		case TABLE:
			status = process_vex_table(vex, string, iochannel);
			break;
	}
	return status;
}

/*!
 \brief process_header() processes portions of the header of the VEX file 
 and populates the Vex_Import datastructure for this Table
 \param vex (Vex_Import *) pointer to the Vex_Import structure
 \param arg (ImportPArserArg) enumeration of header portion to process
 \param string (gchar *) text of the header line to process
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_header(Vex_Import *vex, ImportParserArg arg, gchar * string)
{
	gchar ** str_array = NULL;
	gchar *result = NULL;

	if (!string)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": process_header()\n\t String passed was NULL\n"));
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, " ", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	switch (arg)
	{
		case VEX_EVEME:
			vex->version = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: EVEME %s",result),FALSE,FALSE);
			break;
		case VEX_USER_REV:	
			vex->revision = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Revision %s",result),FALSE,FALSE);
			break;
		case VEX_USER_COMMENT:	
			vex->comment = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: UserComment: %s",result),FALSE,FALSE);
			break;
		case VEX_DATE:	
			vex->date = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Date %s",result),FALSE,FALSE);
			break;
		case VEX_TIME:	
			vex->time = g_strdup(result);
			update_logbar("tools_view", NULL, g_strdup_printf("VEX Header: Time %s",result),FALSE,FALSE);
			break;
		default:
			break;

	}
	g_free(result);
	return G_IO_STATUS_NORMAL;

}

/*!
 \brief process_page() extracts the page variable from the VEX file and 
 stores it in the Vex_Import Structure for this table.
 \param vex (Vex_Import *) Pointer to the Vex_Import structure
 \param string (gchar *) line of VEX file in which to extract the page out of
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_page(Vex_Import *vex, gchar *string)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gchar ** str_array = NULL;
	gint page = -1;
	extern Firmware_Details *firmware;

	if (!string)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": process_page()\n\t String passed was NULL\n"));
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, " ", 2);	
	page = atoi(str_array[1]);
	g_strfreev(str_array);
	if ((page < 0 ) || (page > firmware->total_pages))
	{
		status =  G_IO_STATUS_ERROR;
		update_logbar("tools_view","warning",g_strdup_printf("VEX Import: Page %i out of range <---ERROR\n",page),FALSE,FALSE);
		return status;
	}
	else
	{
		status = G_IO_STATUS_NORMAL;
		vex->page = page;
		vex->got_page = TRUE;
		update_logbar("tools_view",NULL,g_strdup_printf("VEX Import: Page %i\n",page),FALSE,FALSE);

	}

	status = process_table(vex);
	return status;
}


/*!
 \brief process_table() extracts the table_number out of the comment field
 stores it in the Vex_Import Structure for this table.
 \param vex (Vex_Import *) Pointer to the Vex_Import structure
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_table(Vex_Import *vex)
{
	gchar **string = NULL;
	extern Firmware_Details *firmware;

	/* Search for out magic semicolon, if missing AND multi-table/page
	 * firmware, ABORT */
	if ((vex->table < 0) && (vex->table <= firmware->total_tables))
	{
		if ((!g_strrstr(vex->comment,";")) && (firmware->total_tables == firmware->total_tables))
		{
			update_logbar("tools_view","warning",g_strdup("VEX Import: Multi Table per page firmware,\nbut Table number is not defined in comment field, load aborted!!!\n"),FALSE,FALSE);
			return G_IO_STATUS_ERROR;
		}

		string = g_strsplit(vex->comment,";",-1);
		if ((string[0] == NULL) || (!g_strrstr(string[0],"Table")))
		{
			update_logbar("tools_view","warning",g_strdup("VEX Import: Multi Table per page firmware,\n\tbut Table number is not defined in comment field, load aborted!!!\n"),FALSE,FALSE);
			vex->table = -1;
			g_strfreev(string);
			return G_IO_STATUS_ERROR;
		}
		else
		{
			/* the +5 gets us past theword "Table" */
			vex->table = (gint)g_ascii_strtod(string[0]+5,NULL);
			g_strfreev(string);
		}
	}

	update_logbar("tools_view",NULL,g_strdup_printf("VEX Import: Table %i\n",vex->table),FALSE,FALSE);
	return G_IO_STATUS_NORMAL;
}


/*!
 \brief read_number_from_line() is used to read the RPM/LOAD values from the
 VEX file and store them in the Vex_Import structure
 \param dest (gint *) pointer to array within the Vex_Import struct
 \param iochannel (GIOChannel *) pointer to the iochannel representing the
 VEX file
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus read_number_from_line(gint *dest, GIOChannel *iochannel)
{
	GIOStatus status;
	gchar ** str_array = NULL;
	gchar * result = NULL;
	GString *a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		return status;
	}

	/* Make sure the line contains an "=" sign, otherwise we'll segfault*/
	if (strstr(a_line->str, "=") != NULL)
	{
		str_array = g_strsplit(a_line->str, "=", 2);
		result = g_strdup(str_array[1]);	
		g_strfreev(str_array);
		*dest = atoi(result);
	}
	else
		status = G_IO_STATUS_ERROR;

	g_string_free(a_line, TRUE);
	return status;
}

/*!
 \brief process_vex_range() is used to read the vex_ranges for RPM/LOAD 
 allocate the memory for storing the data and call the needed functions to
 read the values into the arrays.
 \see read_number_from_line
 \param vex (Vex_Import *) pointer to the Vex_Import structure
 \param arg (ImportParserArg) enumeration to decide which range we are going to
 read
 \param string (gchar *) Line of text passed to parse.
 \param iochannel (GIOChannel *) Pointer to iochannel representing VEXfile for
 retrieving additional data.
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_vex_range(Vex_Import *vex, ImportParserArg arg, gchar * string, GIOChannel *iochannel)
{
	GIOStatus status = G_IO_STATUS_ERROR;
	gint i = 0;
	gint value = 0;
	gint num_bins = 0;
	gchar ** str_array = NULL;
	gchar * result = NULL;

	if (!string)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": process_vex_range()\n\t String passed was NULL\n"));
		return G_IO_STATUS_ERROR;
	}
	str_array = g_strsplit(string, "[", 2);
	result = g_strdup(str_array[1]);	
	g_strfreev(str_array);
	str_array = g_strsplit(result, "]", 2);
	result = g_strdup(str_array[0]);	

	num_bins = atoi(result);
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
			update_logbar("tools_view","warning",g_strdup("VEX Import: File I/O Read problem, file may be incomplete <---ERROR\n"),FALSE,FALSE);
			break;
		}
		if ((value < 0) || (value > 255))
		{
			status = G_IO_STATUS_ERROR;
			update_logbar("tools_view","warning",g_strdup_printf("VEX Import: RPM/Load bin %i value %i out of bounds <---ERROR\n",i,value),FALSE,FALSE);
			break;
		}
		else
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
			update_logbar("tools_view",NULL,g_strdup("VEX Import: RPM bins loaded successfully \n"),FALSE,FALSE);
		if (arg == VEX_LOAD_RANGE)
			update_logbar("tools_view",NULL,g_strdup("VEX Import: LOAD bins loaded successfully \n"),FALSE,FALSE);
	}
	return status;
}


/*!
 \brief process_vex_table() reads, processes and stores the Table data into
 the Vex_Import structure in preparation for import to the ECU.
 \param vex (stuct Vex_Import *) pointer to the Vex_Import structure
 \param string (gchar *) pointer to the current line of the VEXfile
 \param iochannel (GIOChannel *), Pointer to the iochannel representing the
 VEX file for reading more data
 \returns status of the operation (G_IO_STATUS_ERROR/G_IO_STATUS_NORMAL)
 */
GIOStatus process_vex_table(Vex_Import *vex, gchar * string, GIOChannel *iochannel)
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

	if (!string)
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup(__FILE__": process_vex_table()\n\t String passed was NULL\n"));
		return G_IO_STATUS_ERROR;
	}
	/* Get first number of [  x][  y] in the string line */
	str_array = g_strsplit(string, "[", 3);
	result = g_strdup(str_array[1]);	
	str_array2 = g_strsplit(result, "]", 2);
	result = g_strdup(str_array2[0]);	
	g_strfreev(str_array2);
	x_bins = atoi(result);

	/* Get first number of [  x][  y] in the string line */
	result = g_strdup(str_array[2]);
	g_strfreev(str_array);
	str_array2 = g_strsplit(result, "]", 2);
	result = g_strdup(str_array2[0]);
	g_strfreev(str_array2);
	y_bins = atoi(result);	

	vex->total_tbl_bins = x_bins*y_bins;
	vex->tbl_bins = g_new0(gint,(x_bins * y_bins));

	/* Need to skip down one line to the actual data.... */
	a_line = g_string_new("\0");
	status = g_io_channel_read_line_string(iochannel, a_line, NULL, NULL);
	if (status != G_IO_STATUS_NORMAL) 
	{
		g_string_free(a_line, TRUE);
		update_logbar("tools_view","warning",g_strdup("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n"),FALSE,FALSE);
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
			update_logbar("tools_view","warning", g_strdup("VEX Import: VE-Table I/O Read problem, file may be incomplete <---ERROR\n"),FALSE,FALSE);
			break;
		}
		pos = g_strrstr(a_line->str,"=\0");
		numbers = g_strdup(pos+sizeof(char));

		/* process a row */
		for (j=0; j<x_bins; j++) 
		{
			value = (int)strtol(numbers,&numbers,10);
			if ((value < 0) || (value > 255))
			{
				status = G_IO_STATUS_ERROR;
				update_logbar("tools_view","warning",g_strdup_printf("VEX Import: VE-Table value %i at row %i column %i  is out of range. <---ERROR\n",value,i,j),FALSE,FALSE);
				goto breakout;
			}
			else
				vex->tbl_bins[j+(i*x_bins)] = value;
		}		
		g_string_free(a_line, TRUE);
	}
breakout:
	if (status == G_IO_STATUS_NORMAL)
	{
		vex->got_ve = TRUE;
		update_logbar("tools_view",NULL,g_strdup_printf("VEX Import: VE-Table loaded successfully\n"),FALSE,FALSE);
	}
	return status;
}


/*!
 \brief vex_comment_parse() stores the comment field  as entered by the
 user on the Megatunix GUI for VEX export.
 \param widget (GtkWidget *) pointer to textentry widget where user enters
 the comment
 \param data (gpointer) unused
 \returns TRUE
 */
EXPORT gboolean vex_comment_parse(GtkWidget *widget, gpointer data)
{
	/* Gets data from VEX comment field in tools gui and stores it 
	 * so that it gets written to the vex file 
	 */
	vex_comment = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	update_logbar("tools_view",NULL,g_strdup("VEX Comment Stored\n"),FALSE,FALSE);
	return TRUE;
}


/*!
 \brief dealloc_vex_struct() deallocates the memory used by the Vex_Import
 datastructure
 \param vex (Vex_Import *) Pointer to the Vex_Import structure 
 to deallocate.
 */
void dealloc_vex_struct(Vex_Import *vex)
{
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
}


/*!
 \brief feed_import_data_to_ecu() Forwards the data in the VEX file to the
 ECU.  NOTE this may have problems with firmware using multiple tables in
 a page, as the VEX format 1.0 does NOT handle that condition.
 \param vex (Vex_Import *) pointer to the Vex_Impot datastructure.
 */
void feed_import_data_to_ecu(Vex_Import *vex)
{
	gint i = 0;
	extern Firmware_Details *firmware;
	guint8 **ecu_data = firmware->ecu_data;
	guint8 **ecu_data_backup = firmware->ecu_data_backup;
	guint8 *data = NULL;
	gint total = 0;
	gint canID = 0;
	gint page = -1;
	gint base = 0;
	DataSize size = 0;
	gint table = -1;

	printf("feed data\n");
	/* Since we assume the page is where the table is this can cause
	 * major problems with some firmwares that use two tables inside
	 * of one page....
	 */
	page = vex->page;
	table = vex->table;
	if ((table < 0) || (table >= firmware->total_tables))
	{
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": feed_import_data_to_ecu()\n\ttable passed (%i) is out of range(%i)\n",table,firmware->total_tables));
		return;
	}

	/* If dimensions do NOT match, ABORT!!! */
	if (firmware->table_params[table]->x_bincount != vex->total_x_bins)
	{
		update_logbar("tools_view","warning",g_strdup_printf("VEX Import: number of RPM bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->table_params[table]->x_bincount,vex->total_x_bins),FALSE,FALSE);
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": VEX Import: number of RPM bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->table_params[table]->x_bincount,vex->total_x_bins));

		return;
	}
	if (firmware->table_params[table]->y_bincount != vex->total_y_bins)
	{
		update_logbar("tools_view","warning",g_strdup_printf("VEX Import: number of LOAD bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->table_params[table]->y_bincount,vex->total_y_bins),FALSE,FALSE);
		if (dbg_lvl & CRITICAL)
			dbg_func(g_strdup_printf(__FILE__": VEX Import: number of LOAD bins inside VEXfile and FIRMWARE DO NOT MATCH (%i!=%i), aborting!!!\n",firmware->table_params[table]->y_bincount,vex->total_y_bins));
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
	if (firmware->chunk_support)
	{
		printf("chunk write X vars\n");

		total = (vex->total_x_bins);
		data = g_new0(guint8, total);
		for (i=0;i<total;i++)
			data[i]=(guchar)vex->x_bins[i];
		chunk_write(canID,page,base,total,data);
	}
	else
	{
		printf("std write X vars\n");

		for (i=0;i<vex->total_x_bins;i++)
		{
			if (vex->x_bins[i] != get_ecu_data_last(canID,page,base+i,size))
				send_to_ecu(canID,page,base+i,size, vex->x_bins[i], TRUE);
		}
	}

	canID = firmware->canID;
	page = firmware->table_params[table]->y_page;
	base = firmware->table_params[table]->y_base;
	size = firmware->table_params[table]->y_size;
	if (firmware->chunk_support)
	{
		printf("chunk write Y vars\n");
		total = (vex->total_y_bins);
		data = g_new0(guchar, total);
		for (i=0;i<total;i++)
			data[i]=(guchar)vex->y_bins[i];
		chunk_write(canID,page,base,total,data);
	}
	else
	{
		printf("std write Y vars\n");
		for (i=0;i<vex->total_y_bins;i++)
		{
			if (vex->y_bins[i] != get_ecu_data_last(canID,page,base+i,size))
				send_to_ecu(canID,page,base+i,size,vex->y_bins[i], TRUE);
		}
	}

	canID = firmware->canID;
	page = firmware->table_params[table]->z_page;
	base = firmware->table_params[table]->z_base;
	size = firmware->table_params[table]->z_size;
	if (firmware->chunk_support)
	{
		printf("chunk write Z vars\n");
		total = (vex->total_y_bins)*(vex->total_x_bins);
		data = g_new0(guchar, total);
		for (i=0;i<total;i++)
			data[i]=(guchar)vex->tbl_bins[i];
		chunk_write(canID,page,base,total,data);
	}
	else
	{
		printf("std write Z vars\n");
		for (i=0;i<((vex->total_y_bins)*(vex->total_x_bins));i++)
		{
			if (vex->tbl_bins[i] != get_ecu_data_last(canID,page,base+i,size))
				send_to_ecu(canID,page,base+i,size,vex->tbl_bins[i], TRUE);
		}
	}
	io_cmd(firmware->burn_all_command,NULL);

	update_logbar("tools_view",NULL,g_strdup_printf("VEX Import: VEtable on page %i updated with data from the VEX file\n",vex->page),FALSE,FALSE);
}


/*!
 \brief revert_to_previous_data() reverts the VEX import by using the backup
 of the internal datastructures.
 */
void revert_to_previous_data()
{
	gint canID=0;
	gint page=0;
	gint offset=0;
	gint total = 0;
	guchar *data = NULL;
	GModule *module = NULL;
	PostFunction *pf = NULL;
	GArray *pfuncs = NULL;
	/* Called to back out a load of a VEtable from VEX import */
	extern Firmware_Details *firmware;
	guint8 **ecu_data = firmware->ecu_data;
	guint8 **ecu_data_backup = firmware->ecu_data_backup;
	extern GHashTable *dynamic_widgets;

	for (page=0;page<firmware->total_pages;page++)
	{
		if (!firmware->page_params[page]->dl_by_default)
			continue;
		if (firmware->chunk_support)
		{
			total = firmware->page_params[page]->length;
			data = g_new0(guchar,total);
			for (offset=0;offset<total;offset++)
				data[offset]=get_ecu_data_backup(canID,page,offset,MTX_U08);
			chunk_write(canID,page,0,total,data);

		}
		else
		{
			for (offset = 0;offset<firmware->page_params[page]->length;offset++)
			{
				if (get_ecu_data_backup(canID,page,offset,MTX_U08) != get_ecu_data(canID,page,offset,MTX_U08))
				{
					set_ecu_data(canID,page,offset,MTX_U08,get_ecu_data_backup(canID,page,offset,MTX_U08));
					send_to_ecu(canID,page,offset,MTX_U08,ecu_data_backup[page][offset], FALSE);
				}
			}
		}
		memcpy(ecu_data[page], ecu_data_backup[page],firmware->page_params[page]->length);
	}
	module = g_module_open(NULL,G_MODULE_BIND_LAZY);
	pfuncs = g_array_new(FALSE,TRUE,sizeof(PostFunction *));

	pf = g_new0(PostFunction,1);
	pf->name = g_strdup("update_ve_const");
	if (module)
		g_module_symbol(module,pf->name,(void *)&pf->function);
	pf->w_arg = FALSE;
	pfuncs = g_array_append_val(pfuncs,pf);
	g_module_close(module);
	io_cmd(NULL,pfuncs);

	gtk_widget_set_sensitive(g_hash_table_lookup(dynamic_widgets,"tools_undo_vex_button"),FALSE);
	update_logbar("tools_view","warning",g_strdup("Reverting to previous settings....\n"),FALSE,FALSE);
	io_cmd(firmware->burn_all_command,NULL);
}
