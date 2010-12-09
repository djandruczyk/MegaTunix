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
#include <req_fuel.h>
#include <stdlib.h>
#include <user_outputs.h>


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


G_MODULE_EXPORT gboolean common_bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	static Firmware_Details *firmware = NULL;
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	gint bitshift = -1;
	gint bitval = -1;
	gint bitmask = -1;
	gint dload_val = -1;
	gint canID = 0;
	gint page = -1;
	gint tmp = 0;
	gint tmp32 = 0;
	gint offset = -1;
	DataSize size = 0;
	gint dl_type = -1;
	gint handler = 0;
	gint table_num = -1;
	Deferred_Data *d_data = NULL;
	gchar * swap_list = NULL;
	gchar * set_labels = NULL;
	gchar * table_2_update = NULL;
	gchar * group_2_update = NULL;
	GHashTable **interdep_vars = NULL;
	GHashTable *sources_hash = NULL;
	void (*check_limits)(gint) = NULL;

	sources_hash = DATA_GET(global_data,"sources_hash");
	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	interdep_vars = DATA_GET(global_data,"interdep_vars");


	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);

	canID = (GINT)OBJ_GET(widget,"canID");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	size = (DataSize)OBJ_GET(widget,"size");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	bitval = (GINT)OBJ_GET(widget,"bitval");
	bitmask = (GINT)OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift_f(bitmask);
	handler = (GINT)OBJ_GET(widget,"handler");
	swap_list = (gchar *)OBJ_GET(widget,"swap_labels");
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	group_2_update = (gchar *)OBJ_GET(widget,"group_2_update");
	table_2_update = (gchar *)OBJ_GET(widget,"table_2_update");


	/* If it's a check button then it's state is dependant on the button's state*/
	if (!GTK_IS_RADIO_BUTTON(widget))
		bitval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	switch ((MtxButton)handler)
	{
		case MULTI_EXPRESSION:
			/*printf("MULTI_EXPRESSION CHANGE\n");*/
			if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_value")))
			{
				/*              printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),(gchar *)OBJ_GET(widget,"source_value"));*/
				g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(OBJ_GET(widget,"source_value")));
			}
			/* FAll Through */
		case GENERIC:
			tmp = ms_get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;   /*clears bits */
			tmp = tmp | (bitval << bitshift);
			dload_val = tmp;
			if (dload_val == ms_get_ecu_data(canID,page,offset,size))
				return FALSE;
			break;
		case DEBUG_LEVEL:
			/* Debugging selection buttons */
			tmp32 = (GINT)DATA_GET(global_data,"dbg_lvl");
			tmp32 = tmp32 & ~bitmask;
			tmp32 = tmp32 | (bitval << bitshift);
			DATA_SET(global_data,"dbg_lvl",GINT_TO_POINTER(tmp32));
			break;

		case ALT_SIMUL:
			/* Alternate or simultaneous */
			if (firmware->capabilities & MSNS_E)
			{
				table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
				tmp = ms_get_ecu_data(canID,page,offset,size);
				tmp = tmp & ~bitmask;/* clears bits */
				tmp = tmp | (bitval << bitshift);
				dload_val = tmp;
				/*printf("ALT_SIMUL, MSnS-E, table num %i, dload_val %i, curr ecu val %i\n",table_num,dload_val, ms_get_ecu_data(canID,page,offset,size));*/
				if (dload_val == ms_get_ecu_data(canID,page,offset,size))
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				/*printf("last alt %i, cur alt %i\n",firmware->rf_params[table_num]->last_alternate,firmware->rf_params[table_num]->alternate);*/

				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);
				if (get_symbol_f("check_req_fuel_limits",(void *)&check_limits))
					check_limits(table_num);
			}
			else
			{
				table_num = (gint)strtol(OBJ_GET(widget,"table_num"),NULL,10);
				dload_val = bitval;
				if (dload_val == ms_get_ecu_data(canID,page,offset,size))
					return FALSE;
				firmware->rf_params[table_num]->last_alternate = firmware->rf_params[table_num]->alternate;
				firmware->rf_params[table_num]->alternate = bitval;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);
				if (get_symbol_f("check_req_fuel_limits",(void *)&check_limits))
					check_limits(table_num);
			}
			break;
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_bitmask_button_handler",(void *)&ecu_handler))
					return ecu_handler(widget,data);
				else
					dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": common_bitmask_button_handler()\n\tDefault case, but there is NO ecu_bitmask_button_handler available, unhandled case for widget %s, BUG!\n",glade_get_widget_name(widget)));
			}
			else
				return ecu_handler(widget,data);

			dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": bitmask_button_handler()\n\tbitmask button at page: %i, offset %i, NOT handled\n\tERROR!!, contact author\n",page,offset));
			return FALSE;
			break;

	}

	/* Swaps the label of another control based on widget state... */
	if ((set_labels) && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		set_widget_labels(set_labels);
	if (swap_list)
		swap_labels(swap_list,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	/* MUST use dispatcher, as the update functions run outside of the
	 * normal GTK+ context, so if we were to call it direct we'd get a 
	 * deadlock due to gtk_threads_enter/leave() calls,  so we use the
	 * dispatch queue to let it run in the correct "state"....
	 */
	if (table_2_update)
		gdk_threads_add_timeout(2000,force_update_table,table_2_update);

	/* Update controls that are dependant on a controls state...
	 * In this case, MAP sensor related ctrls */
	if (group_2_update)
	{
		gdk_threads_add_timeout(2000,force_view_recompute,NULL);
		gdk_threads_add_timeout(2000,trigger_group_update,group_2_update);
	}

	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download_f(widget,dload_val);
		ms_send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	return TRUE;
}


/*!
 * \brief slider_value_changed() handles controls based upon a slider
 * sort of like spinbutton controls
 */
G_MODULE_EXPORT gboolean common_slider_handler(GtkWidget *widget, gpointer data)
{
	gint page = 0;
	gint offset = 0;
	DataSize size = 0;
	gint canID = 0;
	gint dl_type = -1;
	gfloat value = 0.0;
	gint dload_val = 0;

	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	canID = (GINT)OBJ_GET(widget,"canID");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	size = (DataSize)OBJ_GET(widget,"size");

	value = gtk_range_get_value(GTK_RANGE(widget));
	dload_val = convert_before_download_f(widget,value);

	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != ms_get_ecu_data(canID,page,offset,size))
			ms_send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	return FALSE; /* Let other handlers run! */
}


G_MODULE_EXPORT gboolean common_button_handler(GtkWidget *widget, gpointer data)
{
	gint handler = -1;
	gint tmpi = 0;
	gint tmp2 = 0;
	gint page = 0;
	gint offset = 0;
	gint canID = 0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	DataSize size = 0;
	gfloat tmpf = 0.0;
	gchar * tmpbuf = NULL;
	gchar * dest = NULL;
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;

	handler = (StdButton)OBJ_GET(widget,"handler");

	switch ((StdButton)handler)
	{
		case INCREMENT_VALUE:
		case DECREMENT_VALUE:
			dest = OBJ_GET(widget,"partner_widget");
			tmp2 = (GINT)OBJ_GET(widget,"amount");
			if (OBJ_GET(dest,"raw_lower"))
				raw_lower = (gint)strtol(OBJ_GET(dest,"raw_lower"),NULL,10);
			else
				raw_lower = get_extreme_from_size_f(size,LOWER);
			if (OBJ_GET(dest,"raw_upper"))
				raw_upper = (gint)strtol(OBJ_GET(dest,"raw_upper"),NULL,10);
			else
				raw_upper = get_extreme_from_size_f(size,UPPER);
			canID = (GINT)OBJ_GET(dest,"canID");
			page = (GINT)OBJ_GET(dest,"page");
			size = (DataSize)OBJ_GET(dest,"size");
			offset = (GINT)OBJ_GET(dest,"offset");
			tmpi = ms_get_ecu_data(canID,page,offset,size);
			if (handler == INCREMENT_VALUE)
				tmpi = tmpi+tmp2 > raw_upper? raw_upper:tmpi+tmp2;
			else
				tmpi = tmpi-tmp2 < raw_lower? raw_lower:tmpi-tmp2;
			ms_send_to_ecu(canID, page, offset, size, tmpi, TRUE);
			break;
		case GET_CURR_TPS:
			tmpbuf = OBJ_GET(widget,"source");
			lookup_current_value_f(tmpbuf,&tmpf);
			dest = OBJ_GET(widget,"dest_widget");
			tmpbuf = g_strdup_printf("%.0f",tmpf);
			gtk_entry_set_text(GTK_ENTRY(lookup_widget_f(dest)),tmpbuf);
			g_signal_emit_by_name(lookup_widget_f(dest),"activate",NULL);
			g_free(tmpbuf);
			break;
		case REQFUEL_RESCALE_TABLE:
			reqfuel_rescale_table(widget);
			break;
		case REQ_FUEL_POPUP:
			reqd_fuel_popup(widget);
			reqd_fuel_change(widget);
			break;
		default:
			break;
	}
	return TRUE;
}



/*!
 \brief std_combo_handler() handles all combo boxes
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
G_MODULE_EXPORT gboolean std_combo_handler(GtkWidget *widget, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	gboolean state = FALSE;
	gint handler = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint total = 0;
	guchar bitval = 0;
	gchar * set_labels = NULL;
	gchar * swap_list = NULL;
	gchar * tmpbuf = NULL;
	gchar * table_2_update = NULL;
	gchar * group_2_update = NULL;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * dl_conv = NULL;
	gchar * ul_conv = NULL;
	gint precision = 0;
	gchar ** vector = NULL;
	guint i = 0;
	gint tmpi = 0;
	gint page = 0;
	gint offset = 0;
	gint canID = 0;
	gint table_num = 0;
	gchar * range = NULL;
	DataSize size = MTX_U08;
	gchar * choice = NULL;
	guint8 tmp = 0;
	gint dload_val = 0;
	gint dl_type = 0;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	Deferred_Data *d_data = NULL;
	GtkWidget *tmpwidget = NULL;
	void *eval = NULL;
	GHashTable **interdep_vars = NULL;
	GHashTable *sources_hash = NULL;
	Firmware_Details *firmware = NULL;
	void (*check_limits)(gint) = NULL;

	sources_hash = DATA_GET(global_data,"sources_hash");
	firmware = DATA_GET(global_data,"firmware");
	interdep_vars = DATA_GET(global_data,"interdep_vars");

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	page = (GINT) OBJ_GET(widget,"page");
	offset = (GINT) OBJ_GET(widget,"offset");
	bitmask = (GINT) OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift_f(bitmask);
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	handler = (GINT) OBJ_GET(widget,"handler");
	canID = (GINT)OBJ_GET(widget,"canID");
	size = (DataSize)OBJ_GET(widget,"size");
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	swap_list = (gchar *)OBJ_GET(widget,"swap_labels");
	table_2_update = (gchar *)OBJ_GET(widget,"table_2_update");
	group_2_update = (gchar *)OBJ_GET(widget,"group_2_update");

	state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (state == 0)
	{
		/* Not selected by combo popdown button, thus is being edited. 
		 * Do a model scan to see if we actually hit the jackpot or 
		 * not, and get the iter for it...
		 */
		if (!search_model_f(model,widget,&iter))
			return FALSE;
	}

	/*printf("choice %s, bitmask %i, bitshift %i bitval %i\n",choice,bitmask,bitshift, bitval );*/
	switch ((MtxButton)handler)
	{
		case MULTI_EXPRESSION:
			/*printf("combo MULTI EXPRESSION\n");*/
			if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_values")))
			{
				tmpbuf = OBJ_GET(widget,"source_values");
				vector = g_strsplit(tmpbuf,",",-1);
				if ((guint)gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) >= g_strv_length(vector))
				{
					printf("combo size doesn't match source_values for multi_expression\n");
					return FALSE;
				}
				/*printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]);*/
				g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]));
				gdk_threads_add_timeout(2000,update_multi_expression,NULL);
			}
		case GENERIC:
			tmp = ms_get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;   /*clears bits */
			tmp = tmp | (bitval << bitshift);
			dload_val = tmp;
			if (dload_val == ms_get_ecu_data(canID,page,offset,size))
				return FALSE;
			break;
		default:
			printf(_("std_combo_handler, default case!!! wrong wrong wrong!!\n"));
			break;
	}

	if (swap_list)
		swap_labels(swap_list,bitval);
	if (table_2_update)
		gdk_threads_add_timeout(2000,force_update_table,table_2_update);
	if (set_labels)
	{
		total = get_choice_count_f(model);
		tmpi = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
		vector = g_strsplit(set_labels,",",-1);
		if ((g_strv_length(vector)%(total+1)) != 0)
		{
			dbg_func_f(CRITICAL,g_strdup(__FILE__": std_combo_handler()\n\tProblem with set_widget_labels, counts don't match up\n"));
			goto combo_download;
		}
		for (i=0;i<(g_strv_length(vector)/(total+1));i++)
		{
			tmpbuf = g_strconcat(vector[i*(total+1)],",",vector[(i*(total+1))+1+tmpi],NULL);
			set_widget_labels(tmpbuf);
			g_free(tmpbuf);
		}
		g_strfreev(vector);
	}

combo_download:
	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download_f(widget,dload_val);
		ms_send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	return TRUE;
}


/*!
 \brief swap_labels() is a utility function that extracts the list passed to 
 it, and kicks off a subfunction to do the swapping on each widget in the list
 \param input (gchar *) name of list to enumeration and run the subfunction on
 \param state (gboolean) passed on to subfunction
 the default label
 */
G_MODULE_EXPORT void swap_labels(const gchar * input, gboolean state)
{
	GList *list = NULL;
	GtkWidget *widget = NULL;
	gchar **fields = NULL;
	gint i = 0;
	gint num_widgets = 0;

	fields = parse_keys_f(input,&num_widgets,",");

	for (i=0;i<num_widgets;i++)
	{
		widget = NULL;
		widget = lookup_widget_f(fields[i]);
		if (GTK_IS_WIDGET(widget))
			switch_labels((gpointer)widget,GINT_TO_POINTER(state));
		else if ((list = get_list_f(fields[i])) != NULL)
			g_list_foreach(list,switch_labels,GINT_TO_POINTER(state));
	}
	g_strfreev(fields);
}



/*!
 \brief switch_labels() swaps labels that depend on the state of another 
 control. Handles temp dependant labels as well..
 \param key (gpointer) gpointer encapsulation of the widget
 \param data (gpointer)  gpointer encapsulation of the target state if TRUE 
 we use the alternate label, if FALSE we use
 the default label
 */
G_MODULE_EXPORT void switch_labels(gpointer key, gpointer data)
{
	GtkWidget * widget = (GtkWidget *) key;
	gboolean state = (GBOOLEAN) data;
	gint temp_units;

	temp_units = (GINT)DATA_GET(global_data,"temp_units");
	if (GTK_IS_WIDGET(widget))
	{
		if ((GBOOLEAN)OBJ_GET(widget,"temp_dep") == TRUE)
		{
			if (state)
			{
				if (temp_units == FAHRENHEIT)
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"alt_f_label"));
				else
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"alt_c_label"));
			}
			else
			{
				if (temp_units == FAHRENHEIT)
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"f_label"));
				else
					gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"c_label"));
			}
		}
		else
		{
			if (state)
				gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"alt_label"));
			else
				gtk_label_set_text(GTK_LABEL(widget),OBJ_GET(widget,"label"));
		}
	}
}


/*!
 \brief set_widget_labels takes a passed string which is a comma 
 separated string of name/value pairs, first being the widget name 
 (global name) and the second being the string to set on this widget
 */
G_MODULE_EXPORT void set_widget_labels(const gchar *input)
{
	gchar ** vector = NULL;
	gint count = 0;
	gint i = 0;
	GtkWidget * widget = NULL;

	if (!input)
		return;

	vector = parse_keys_f(input,&count,",");
	if (count%2 != 0)
	{
		dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": set_widget_labels()\n\tstring passed was not properly formatted, should be an even number of elements, Total elements %i, string itself \"%s\"",count,input));

		return;
	}
	for(i=0;i<count;i+=2)
	{
		widget = lookup_widget_f(vector[i]);
		if ((widget) && (GTK_IS_LABEL(widget)))
			gtk_label_set_text(GTK_LABEL(widget),vector[i+1]);
		else
			dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": set_widget_labels()\n\t Widget \"%s\" could NOT be located or is NOT a label\n",vector[i]));

	}
	g_strfreev(vector);

}


/*!
   \brief force_update_table() updates a subset of widgets (specifically ONLY
    the Z axis widgets) of a table on screen.
     \param table_num, integer number of the table in question
      */
G_MODULE_EXPORT gboolean force_update_table(gpointer data)
{
	gint offset = -1;
	gint page = -1;
	gint table_num = -1;
	gint base = 0;
	gint length = 0;
	Firmware_Details *firmware = NULL;
	GList ***ve_widgets = NULL;

	ve_widgets = DATA_GET(global_data,"ve_widgets");

	firmware = DATA_GET(global_data,"firmware");

	if (DATA_GET(global_data,"leaving"))
		return FALSE;
	if (page > firmware->total_pages)
		return FALSE;
	table_num = (gint)strtol((gchar *)data,NULL,10);
	if ((table_num < 0) || (table_num > (firmware->total_tables-1)))
		return FALSE;
	base = firmware->table_params[table_num]->z_base;
	length = firmware->table_params[table_num]->x_bincount *
		firmware->table_params[table_num]->y_bincount;
	page =  firmware->table_params[table_num]->z_page;
	for (offset=base;offset<base+length;offset++)
	{
		if ((DATA_GET(global_data,"leaving")) || (!firmware))
			return FALSE;
		if (ve_widgets[page][offset] != NULL)
			g_list_foreach(ve_widgets[page][offset],update_widget_f,NULL);
	}
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	return FALSE;
}


/*!
 \brief trigger_group_update() updates a subset of widgets (any widgets in
 the group name passed. This runs as a timeout delayed asynchronously from
 when the ctrl is modified, to prevent a deadlock.
 \param data, string name of list of controls
 */
G_MODULE_EXPORT gboolean trigger_group_update(gpointer data)
{
	if (DATA_GET(global_data,"leaving"))
		return FALSE;

	g_list_foreach(get_list_f((gchar *)data),update_widget_f,NULL);
	return FALSE;/* Make it cancel and not run again till called */
}


G_MODULE_EXPORT gboolean update_multi_expression(gpointer data)
{
	g_list_foreach(get_list_f("multi_expression"),update_widget_f,NULL);
	return FALSE;
}



/*!
 \brief update_ve_const_pf() is called after a read of the VE/Const block of 
 data from the ECU.  It takes care of updating evey control that relates to
 an ECU variable on screen
 */
G_MODULE_EXPORT void update_ve_const_pf(void)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gfloat tmpf = 0.0;
	gint reqfuel = 0;
	gint i = 0;
	guint mask = 0;
	guint shift = 0;
	guint tmpi = 0;
	guint8 addon = 0;
	gint mult = 0;
	Firmware_Details *firmware = NULL;
	GList ***ve_widgets = NULL;

	ve_widgets = DATA_GET(global_data,"ve_widgets");
	firmware = DATA_GET(global_data,"firmware");
	canID = firmware->canID;

	if (DATA_GET(global_data,"leaving"))
		return;
	if (!((DATA_GET(global_data,"connected")) ||
				(DATA_GET(global_data,"offline"))))
		return;

	gdk_threads_enter();
	set_title_f(g_strdup(_("Updating Controls...")));
	gdk_threads_leave();
	DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(TRUE));

	/* DualTable Fuel Calculations
	 * DT code no longer uses the "alternate" firing mode as each table
	 * is pretty much independant from the other,  so the calcs are a 
	 * little simpler...
	 *
	 *                                        /        num_inj_1      \
	 *                 req_fuel_per_squirt * (-------------------------)
	 *                                        \         divider       /
	 * req_fuel_total = --------------------------------------------------
	 *                              10
	 *
	 * where divider = num_cyls/num_squirts;
	 *
	 *
	 *  B&G, MSnS, MSnEDIS req Fuel calc *
	 * req-fuel 
	 *                                /        num_inj        \
	 *         req_fuel_per_squirt * (-------------------------)
	 *                                \ divider*(alternate+1) /
	 * req_fuel_total = ------------------------------------------
	 *                              10
	 *
	 * where divider = num_cyls/num_squirts;
	 *
	 * The req_fuel_per_squirt is the part stored in the MS ECU as 
	 * the req_fuel variable.  Take note when doing conversions.  
	 * On screen the value is divided by ten from what is 
	 * in the MS.  
	 * 
	 */


	/* All Tables */
	if (firmware->capabilities & MS2)
	{
		addon = 0;
		mult = 100;
	}
	else
	{
		addon = 1;
		mult = 1;
	}

	for (i=0;i<firmware->total_tables;i++)
	{
		if (firmware->table_params[i]->color_update == FALSE)
		{
			recalc_table_limits_f(0,i);
			if ((firmware->table_params[i]->last_z_maxval != firmware->table_params[i]->z_maxval) || (firmware->table_params[i]->last_z_minval != firmware->table_params[i]->z_minval))
				firmware->table_params[i]->color_update = TRUE;
			else
				firmware->table_params[i]->color_update = FALSE;
		}

		if (firmware->table_params[i]->reqfuel_offset < 0)
			continue;

		tmpi = ms_get_ecu_data(canID,firmware->table_params[i]->num_cyl_page,firmware->table_params[i]->num_cyl_offset,size);
		mask = firmware->table_params[i]->num_cyl_mask;
		shift = get_bitshift_f(firmware->table_params[i]->num_cyl_mask);
		firmware->rf_params[i]->num_cyls = ((tmpi & mask) >> shift)+addon;
		firmware->rf_params[i]->last_num_cyls = ((tmpi & mask) >> shift)+addon;
		/*printf("num_cyls for table %i in the firmware is %i\n",i,firmware->rf_params[i]->num_cyls);*/

		tmpi = ms_get_ecu_data(canID,firmware->table_params[i]->num_inj_page,firmware->table_params[i]->num_inj_offset,size);
		mask = firmware->table_params[i]->num_cyl_mask;
		shift = get_bitshift_f(firmware->table_params[i]->num_cyl_mask);

		firmware->rf_params[i]->num_inj = ((tmpi & mask) >> shift)+addon;
		firmware->rf_params[i]->last_num_inj = ((tmpi & mask) >> shift)+addon;
		/*printf("num_inj for table %i in the firmware is %i\n",i,firmware->rf_params[i]->num_inj);*/

		firmware->rf_params[i]->divider = ms_get_ecu_data(canID,firmware->table_params[i]->divider_page,firmware->table_params[i]->divider_offset,size);
		firmware->rf_params[i]->last_divider = firmware->rf_params[i]->divider;
		firmware->rf_params[i]->alternate = ms_get_ecu_data(canID,firmware->table_params[i]->alternate_page,firmware->table_params[i]->alternate_offset,size);
		firmware->rf_params[i]->last_alternate = firmware->rf_params[i]->alternate;
		/*printf("alternate for table %i in the firmware is %i\n",i,firmware->rf_params[i]->alternate);*/
		reqfuel = ms_get_ecu_data(canID,firmware->table_params[i]->reqfuel_page,firmware->table_params[i]->reqfuel_offset,firmware->table_params[i]->reqfuel_size);
		/*printf("reqfuel for table %i in the firmware is %i\n",i,reqfuel);*/

		/*
		   printf("reqfuel_page %i, reqfuel_offset %i\n",firmware->table_params[i]->reqfuel_page,firmware->table_params[i]->reqfuel_offset);

		   printf("num_inj %i, divider %i\n",firmware->rf_params[i]->num_inj,firmware->rf_params[i]->divider);
		   printf("num_cyls %i, alternate %i\n",firmware->rf_params[i]->num_cyls,firmware->rf_params[i]->alternate);
		   printf("req_fuel_per_1_squirt is %i\n",reqfuel);
		 */

		/* Calcs vary based on firmware. 
		 * DT uses num_inj/divider
		 * MSnS-E uses the SAME in DT mode only
		 * MSnS-E uses B&G form in single table mode
		 */
		if (firmware->capabilities & MS1_DT)
		{
			/*
			 * printf("DT\n");
			 */
			tmpf = (float)(firmware->rf_params[i]->num_inj)/(float)(firmware->rf_params[i]->divider);
		}
		else if (firmware->capabilities & MSNS_E)
		{
			shift = get_bitshift_f(firmware->table_params[i]->dtmode_mask);
			if ((ms_get_ecu_data(canID,firmware->table_params[i]->dtmode_page,firmware->table_params[i]->dtmode_offset,size) & firmware->table_params[i]->dtmode_mask) >> shift)
			{
				/*
				 * printf("MSnS-E DT\n"); 
				 */
				tmpf = (float)(firmware->rf_params[i]->num_inj)/(float)(firmware->rf_params[i]->divider);
			}
			else
			{
				/*
				 * printf("MSnS-E non-DT\n"); 
				 */
				tmpf = (float)(firmware->rf_params[i]->num_inj)/((float)(firmware->rf_params[i]->divider)*((float)(firmware->rf_params[i]->alternate)+1.0));
			}
		}
		else
		{
			/*
			 * printf("B&G\n"); 
			 */
			tmpf = (float)(firmware->rf_params[i]->num_inj)/((float)(firmware->rf_params[i]->divider)*((float)(firmware->rf_params[i]->alternate)+1.0));
		}

		/* ReqFuel Total */
		/*
		 * printf("intermediate tmpf is %f\n",tmpf);
		 */
		tmpf *= (float)reqfuel;
		tmpf /= (10.0*mult);
		firmware->rf_params[i]->req_fuel_total = tmpf;
		firmware->rf_params[i]->last_req_fuel_total = tmpf;
		/*
		 * printf("req_fuel_total for table number %i is %f\n",i,tmpf);
		 */

		/* Injections per cycle */
		firmware->rf_params[i]->num_squirts = (float)(firmware->rf_params[i]->num_cyls)/(float)(firmware->rf_params[i]->divider);
		/*
		 * printf("num_squirts for table number %i is %i\n",i,firmware->rf_params[i]->num_squirts);
		 */
		if (firmware->rf_params[i]->num_squirts < 1 )
			firmware->rf_params[i]->num_squirts = 1;
		firmware->rf_params[i]->last_num_squirts = firmware->rf_params[i]->num_squirts;

		gdk_threads_enter();
		set_reqfuel_color_f(BLACK,i);
		gdk_threads_leave();
	}


	/* Update all on screen controls (except bitfields (done above)*/
	gdk_threads_enter();
	for (page=0;page<firmware->total_pages;page++)
	{
		if ((DATA_GET(global_data,"leaving")) || (!firmware))
			return;
		if (!firmware->page_params[page]->dl_by_default)
			continue;
		thread_update_widget_f("info_label",MTX_LABEL,g_strdup_printf(_("<big><b>Updating Controls on Page %i</b></big>"),page));
		for (offset=0;offset<firmware->page_params[page]->length;offset++)
		{
			if ((DATA_GET(global_data,"leaving")) || (!firmware))
				return;
			if (ve_widgets[page][offset] != NULL)
				g_list_foreach(ve_widgets[page][offset],
						update_widget_f,NULL);
		}
	}
	for (i=0;i<firmware->total_tables;i++)
		firmware->table_params[i]->color_update = FALSE;

	DATA_SET(global_data,"paused_handlers",GINT_TO_POINTER(FALSE));
	thread_update_widget_f("info_label",MTX_LABEL,g_strdup_printf(_("Ready...")));
	set_title_f(g_strdup(_("Ready...")));
	gdk_threads_leave();
	return;
}

