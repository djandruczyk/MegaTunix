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
#include <datamgmt.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <glade/glade.h>
#include <mscommon_comms.h>
#include <mscommon_gui_handlers.h>
#include <stdlib.h>


extern gconstpointer *global_data;


G_MODULE_EXPORT gboolean common_entry_handler(GtkWidget *widget, gpointer data)
{
	static Firmware_Details *firmware = NULL;
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	GdkColor black = {0,0,0,0};
	gint handler = -1;
	gchar *text = NULL;
	gchar *tmpbuf = NULL;
	gfloat tmpf = -1;
	gfloat value = -1;
	gint table_num = -1;
	gint tmpi = -1;
	gint tmp = -1;
	gint page = -1;
	gint canID = 0;
	gint base = -1;
	gint old = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint dl_type = -1;
	gint precision = -1;
	gint spconfig_offset = -1;
	gint oddfire_bit_offset = -1;
	gint temp_units = 0;
	gfloat scaler = 0.0;
	gboolean temp_dep = FALSE;
	gfloat real_value = 0.0;
	gboolean use_color = FALSE;
	DataSize size = 0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	GdkColor color;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	/* If paused or not ready yet, just reset ctrl to black and return */
	if ((DATA_GET(global_data,"paused_handlers")) || 
			(!DATA_GET(global_data,"ready")))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	temp_units = (GINT)DATA_GET(global_data,"temp_units");
	temp_dep = (GBOOLEAN)OBJ_GET(widget,"temp_dep");
	handler = (MtxButton)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	canID = (GINT)OBJ_GET(widget,"canID");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	if (!OBJ_GET(widget,"size"))
		size = MTX_U08 ;        /* default! */
	else
		size = (DataSize)OBJ_GET(widget,"size");
	if (OBJ_GET(widget,"raw_lower"))
		raw_lower = (gint)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
	else
		raw_lower = get_extreme_from_size_f(size,LOWER);
	if (OBJ_GET(widget,"raw_upper"))
		raw_upper = (gint)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
	else
		raw_upper = get_extreme_from_size_f(size,UPPER);
	if (!OBJ_GET(widget,"base"))
		base = 10;
	else
		base = (GINT)OBJ_GET(widget,"base");
	precision = (GINT)OBJ_GET(widget,"precision");
	use_color = (GBOOLEAN)OBJ_GET(widget,"use_color");
	if (use_color)
		if (OBJ_GET(widget,"table_num"))
			table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);

	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpi = (gint)strtol(text,NULL,base);
	tmpf = (gfloat)g_ascii_strtod(g_strdelimit(text,",.",'.'),NULL);
	/*
	 * printf("base \"%i\", text \"%s\" int val \"%i\", float val \"%f\" precision %i \n",base,text,tmpi,tmpf,precision);
	 */

	g_free(text);
	/* This isn't quite correct, as the base can either be base10 
	 * or base16, the problem is the limits are in base10
	 */

	if ((tmpf != (gfloat)tmpi) && (precision == 0))
	{
		/* Pause signals while we change the value */
		/*              printf("resetting\n");*/
		g_signal_handlers_block_by_func (widget,(gpointer)std_entry_handler_f, data);
		g_signal_handlers_block_by_func (widget,(gpointer)entry_changed_handler_f, data);
		tmpbuf = g_strdup_printf("%i",tmpi);
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		g_free(tmpbuf);
		g_signal_handlers_unblock_by_func (widget,(gpointer)entry_changed_handler_f, data);
		g_signal_handlers_unblock_by_func (widget,(gpointer)std_entry_handler_f, data);
	}
	switch (handler)
	{
		case GENERIC:
			if (temp_dep)
			{
				if (temp_units == CELSIUS)
					value = (tmpf*(9.0/5.0))+32;
				else
					value = tmpf;
			}
			else
				value = tmpf;
			if (base == 10)
			{
				dload_val = convert_before_download_f(widget,value);
			}
			else if (base == 16)
				dload_val = convert_before_download_f(widget,tmpi);
			else
			{
				dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": std_entry_handler()\n\tBase of textentry \"%i\" is invalid!!!\n",base));
				return TRUE;
			}
			/* What we are doing is doing the forward/reverse 
			 * conversion which will give us an exact value 
			 * if the user inputs something in between,  thus 
			 * we can reset the display to a sane value...
			 */
			old = ms_get_ecu_data(canID,page,offset,size);
			ms_set_ecu_data(canID,page,offset,size,dload_val);

			real_value = convert_after_upload_f(widget);
			ms_set_ecu_data(canID,page,offset,size,old);

			g_signal_handlers_block_by_func (widget,(gpointer) std_entry_handler_f, data);
			g_signal_handlers_block_by_func (widget,(gpointer) entry_changed_handler_f, data);

			if (base == 10)
			{
				tmpbuf = g_strdup_printf("%1$.*2$f",real_value,precision);
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				g_free(tmpbuf);
			}
			else
			{       tmpbuf = g_strdup_printf("%.2X",(gint)real_value);
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				g_free(tmpbuf);
			}
			g_signal_handlers_unblock_by_func (widget,(gpointer) entry_changed_handler_f, data);
			g_signal_handlers_unblock_by_func (widget,(gpointer) std_entry_handler_f, data);
			break;
		default:
			/* We need to fall to ECU SPECIFIC entry handler for 
			   anything specific there */
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_entry_handler",(void *)&ecu_handler))
					return ecu_handler(widget,data);
				else
					dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": common_entry_handler()\n\tDefault case, but there is NO ecu_entry_handler available, unhandled case for widget %s, BUG!\n",glade_get_widget_name(widget)));
			}

			/* We don't care about anything else for now... */
			break;
	}
	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != ms_get_ecu_data(canID,page,offset,size))
		{
			/* special case for the ODD MS-1 variants and the very rare 167 bit variables */
			if ((firmware->capabilities & MS1) && ((size == MTX_U16) || (size == MTX_S16)))
			{
				ms_send_to_ecu(canID, page, offset, MTX_U08, (dload_val &0xff00) >> 8, TRUE);
				ms_send_to_ecu(canID, page, offset+1, MTX_U08, (dload_val &0x00ff), TRUE);
			}
			else
				ms_send_to_ecu(canID, page, offset, size, dload_val, TRUE);
		}
	}
	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	if (use_color)
	{
		if (table_num >= 0)
		{
			if (firmware->table_params[table_num]->color_update == FALSE)
			{
				recalc_table_limits_f(canID,table_num);
				if ((firmware->table_params[table_num]->last_z_maxval != firmware->table_params[table_num]->z_maxval) || (firmware->table_params[table_num]->last_z_minval != firmware->table_params[table_num]->z_minval))
					firmware->table_params[table_num]->color_update = TRUE;
				else
					firmware->table_params[table_num]->color_update = FALSE;
			}

			scaler = 256.0/((firmware->table_params[table_num]->z_maxval - firmware->table_params[table_num]->z_minval)*1.05);
			color = get_colors_from_hue_f(256 - (dload_val - firmware->table_params[table_num]->z_minval)*scaler, 0.50, 1.0);
		}
		else
		{
			color = get_colors_from_hue_f(((gfloat)(dload_val-raw_lower)/raw_upper)*-300.0+180, 0.50, 1.0);
		}
		gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
	}
	OBJ_SET(widget,"not_sent",GINT_TO_POINTER(FALSE));
	return TRUE;
}


