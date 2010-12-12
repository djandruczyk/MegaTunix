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
#include <combo_loader.h>
#include <datamgmt.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <firmware.h>
#include <glade/glade.h>
#include <mscommon_comms.h>
#include <mscommon_gui_handlers.h>
#include <req_fuel.h>
#include <serialio.h>
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

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	handler = (MtxButton)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	get_essentials(widget,&canID,&page,&offset,&size,&precision);

	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpi = (GINT)strtol(text,NULL,10);
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
			if (OBJ_GET(widget,"temp_dep"))
			{
				if ((GINT)DATA_GET(global_data,"temp_units") == CELSIUS)
					value = (tmpf*(9.0/5.0))+32;
				else
					value = tmpf;
			}
			else
				value = tmpf;
			dload_val = convert_before_download_f(widget,value);

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

			tmpbuf = g_strdup_printf("%1$.*2$f",real_value,precision);
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
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
	if (OBJ_GET(widget,"use_color"))
	{
		if (OBJ_GET(widget,"raw_lower"))
			raw_lower = (GINT)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
		else
			raw_lower = get_extreme_from_size_f(size,LOWER);
		if (OBJ_GET(widget,"raw_upper"))
			raw_upper = (GINT)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
		else
			raw_upper = get_extreme_from_size_f(size,UPPER);
		if (OBJ_GET(widget,"table_num"))
		{
			table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			if (firmware->table_params[table_num]->color_update == FALSE)
			{
				recalc_table_limits(canID,table_num);
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
	gchar * set_labels = NULL;
	Deferred_Data *d_data = NULL;
	GHashTable **interdep_vars = NULL;
	GHashTable *sources_hash = NULL;

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	interdep_vars = DATA_GET(global_data,"interdep_vars");


	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);

	get_essential_bits(widget,&canID,&page,&offset,&bitval,&bitmask,&bitshift);
	size = (DataSize)OBJ_GET(widget,"size");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	handler = (GINT)OBJ_GET(widget,"handler");


	/* If it's a check button then it's state is dependant on the button's state*/
	if (!GTK_IS_RADIO_BUTTON(widget))
		bitval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	switch ((MtxButton)handler)
	{
		case MULTI_EXPRESSION:
			/*printf("MULTI_EXPRESSION CHANGE\n");*/
			sources_hash = DATA_GET(global_data,"sources_hash");
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
				table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
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
				check_req_fuel_limits(table_num);
			}
			else
			{
				table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
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
				check_req_fuel_limits(table_num);
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
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	if ((set_labels) && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		set_widget_labels(set_labels);
	if (OBJ_GET(widget,"swap_labels"))
		swap_labels(widget,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	/* MUST use dispatcher, as the update functions run outside of the
	 * normal GTK+ context, so if we were to call it direct we'd get a 
	 * deadlock due to gtk_threads_enter/leave() calls,  so we use the
	 * dispatch queue to let it run in the correct "state"....
	 */
	if (OBJ_GET(widget,"table_2_update"))
		gdk_threads_add_timeout(2000,force_update_table,OBJ_GET(widget,"table_2_update"));

	/* Update controls that are dependant on a controls state...
	 * In this case, MAP sensor related ctrls */
	if (OBJ_GET(widget,"group_2_update"))
	{
		gdk_threads_add_timeout(2000,force_view_recompute,NULL);
		gdk_threads_add_timeout(2000,trigger_group_update,OBJ_GET(widget,"group_2_update"));
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

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	get_essentials(widget,&canID,&page,&offset,&size,NULL);

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

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	handler = (StdButton)OBJ_GET(widget,"handler");

	switch ((StdButton)handler)
	{
		case INCREMENT_VALUE:
		case DECREMENT_VALUE:
			dest = OBJ_GET(widget,"partner_widget");
			tmp2 = (GINT)OBJ_GET(widget,"amount");
			if (OBJ_GET(dest,"raw_lower"))
				raw_lower = (GINT)strtol(OBJ_GET(dest,"raw_lower"),NULL,10);
			else
				raw_lower = get_extreme_from_size_f(size,LOWER);
			if (OBJ_GET(dest,"raw_upper"))
				raw_upper = (GINT)strtol(OBJ_GET(dest,"raw_upper"),NULL,10);
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
 \brief common_combo_handler() handles all combo boxes
 \param widget (GtkWidget *) the widget being modified
 \param data (gpointer) not used
 \returns TRUE
 */
G_MODULE_EXPORT gboolean common_combo_handler(GtkWidget *widget, gpointer data)
{
	static Firmware_Details *firmware = NULL;
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	gboolean state = FALSE;
	gint handler = 0;
	gint total = 0;
	gchar * set_labels = NULL;
	gchar * tmpbuf = NULL;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gchar * dl_conv = NULL;
	gchar * ul_conv = NULL;
	gint precision = 0;
	gchar ** vector = NULL;
	guint i = 0;
	gint tmpi = 0;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint bitval = 0;
	gint bitmask = 0;
	gint bitshift = 0;
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

	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
		return TRUE;

	if (!GTK_IS_OBJECT(widget))
		return FALSE;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	interdep_vars = DATA_GET(global_data,"interdep_vars");
	get_essential_bits(widget, &canID, &page, &offset, &bitval, &bitmask, &bitshift);

	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	handler = (GINT) OBJ_GET(widget,"handler");
	size = (DataSize)OBJ_GET(widget,"size");
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");

	state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (!state)
	{
		/* Not selected by combo popdown button, thus is being edited. 
		 * Do a model scan to see if we actually hit the jackpot or 
		 * not, and get the iter for it...
		 */
		if (!search_model(model,widget,&iter))
		{
			printf("search model failure!\n");
			return FALSE;
		}
	}
	gtk_tree_model_get(model,&iter,CHOICE_COL,&choice, \
			BITVAL_COL,&bitval,-1);

	/*printf("choice %s, bitmask %i, bitshift %i bitval %i\n",choice,bitmask,bitshift, bitval );*/
	switch ((MtxButton)handler)
	{
		case MULTI_EXPRESSION:
			sources_hash = DATA_GET(global_data,"sources_hash");
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
		case ALT_SIMUL:
			/* Alternate or simultaneous */
			if (firmware->capabilities & MSNS_E)
			{
				table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
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
				check_req_fuel_limits(table_num);
			}
			else
			{
				table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
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
				check_req_fuel_limits(table_num);
			}
			break;
		case GENERIC:
			tmp = ms_get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;   /*clears bits */
			tmp = tmp | (bitval << bitshift);
			dload_val = tmp;
			if (dload_val == ms_get_ecu_data(canID,page,offset,size))
				return FALSE;
			break;
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_combo_handler",(void *)&ecu_handler))
					return ecu_handler(widget,data);
				else
				{
					dbg_func_f(CRITICAL,g_strdup(__FILE__": common_combo_handler()\n\tDefault case, ecu handler NOT found in plugins, BUG!\n"));
					return TRUE;
				}
			}
			else
				return ecu_handler(widget,data);
			break;
	}

	if (OBJ_GET(widget,"swap_labels"))
		swap_labels(widget,bitval);
	if (OBJ_GET(widget,"table_2_update"))
		gdk_threads_add_timeout(2000,force_update_table,OBJ_GET(widget,"table_2_update"));
	if (set_labels)
	{
		total = get_choice_count(model);
		tmpi = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
		vector = g_strsplit(set_labels,",",-1);
		if ((g_strv_length(vector)%(total+1)) != 0)
		{
			dbg_func_f(CRITICAL,g_strdup(__FILE__": common_combo_handler()\n\tProblem with set_widget_labels, counts don't match up\n"));
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
G_MODULE_EXPORT void swap_labels(GtkWidget *widget, gboolean state)
{
	GList *list = NULL;
	GtkWidget *tmpwidget = NULL;
	gchar **fields = NULL;
	gint i = 0;
	gint num_widgets = 0;

	fields = parse_keys_f(OBJ_GET(widget,"swap_labels"),&num_widgets,",");

	for (i=0;i<num_widgets;i++)
	{
		tmpwidget = NULL;
		tmpwidget = lookup_widget_f(fields[i]);
		if (GTK_IS_WIDGET(tmpwidget))
			switch_labels((gpointer)tmpwidget,GINT_TO_POINTER(state));
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
	table_num = (GINT)strtol((gchar *)data,NULL,10);
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
			g_list_foreach(ve_widgets[page][offset],update_widget,NULL);
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

	g_list_foreach(get_list_f((gchar *)data),update_widget,NULL);
	return FALSE;/* Make it cancel and not run again till called */
}


G_MODULE_EXPORT gboolean update_multi_expression(gpointer data)
{
	g_list_foreach(get_list_f("multi_expression"),update_widget,NULL);
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
			recalc_table_limits(0,i);
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
						update_widget,NULL);
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


G_MODULE_EXPORT gboolean common_spin_handler(GtkWidget *widget, gpointer data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	static Firmware_Details *firmware = NULL;
	gint dl_type = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint canID = 0;
	gint page = -1;
	DataSize size = -1;
	gint bitmask = -1;
	gint bitshift = -1;
	gint spconfig_offset = 0;
	gint oddfire_bit_offset = 0;
	gint tmpi = 0;
	gint tmp = 0;
	gint handler = -1;
	gint divider_offset = 0;
	gint table_num = -1;
	gint temp_units = 0;
	gint source = 0;
	gboolean temp_dep = FALSE;
	gfloat value = 0.0;
	GtkWidget * tmpwidget = NULL;
	Deferred_Data *d_data = NULL;
	GHashTable **interdep_vars = NULL;
	GdkColor black = {0,0,0,0};
	gboolean (*ecu_handler)(GtkWidget *, gpointer);


	if ((DATA_GET(global_data,"paused_handlers")) ||
			(!DATA_GET(global_data,"ready")))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		return TRUE;
	}

	if (!GTK_IS_WIDGET(widget))
	{
		dbg_func_f(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\twidget pointer is NOT valid\n"));
		return FALSE;
	}
	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	handler = (MtxButton)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	get_essential_bits(widget,&canID,&page,&offset,NULL,&bitmask,&bitshift);

	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);

	tmpi = (int)(value+.001);

	switch ((MtxButton)handler)
	{
		case REQ_FUEL_1:
		case REQ_FUEL_2:
			table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			firmware->rf_params[table_num]->last_req_fuel_total = firmware->rf_params[table_num]->req_fuel_total;
			firmware->rf_params[table_num]->req_fuel_total = value;
			check_req_fuel_limits(table_num);
			break;
		case LOCKED_REQ_FUEL:
			table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),firmware->rf_params[table_num]->req_fuel_total);
			break;
		case NUM_SQUIRTS_1:
		case NUM_SQUIRTS_2:
			/* This actually affects another variable */
			table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			divider_offset = firmware->table_params[table_num]->divider_offset;
			firmware->rf_params[table_num]->last_num_squirts = firmware->rf_params[table_num]->num_squirts;
			firmware->rf_params[table_num]->last_divider = ms_get_ecu_data(canID,page,divider_offset,size);

			firmware->rf_params[table_num]->num_squirts = tmpi;
			if (firmware->rf_params[table_num]->num_cyls % firmware->rf_params[table_num]->num_squirts)
				set_reqfuel_color_f(RED,table_num);
			else
			{
				dload_val = (GINT)(((float)firmware->rf_params[table_num]->num_cyls/(float)firmware->rf_params[table_num]->num_squirts)+0.001);

				firmware->rf_params[table_num]->divider = dload_val;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = divider_offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(divider_offset),
						d_data);
				set_reqfuel_color_f(BLACK,table_num);
				check_req_fuel_limits(table_num);
			}
			break;
		case NUM_CYLINDERS_1:
		case NUM_CYLINDERS_2:
			/* Updates a shared bitfield */
			table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			divider_offset = firmware->table_params[table_num]->divider_offset;
			firmware->rf_params[table_num]->last_divider = ms_get_ecu_data(canID,page,divider_offset,size);
			firmware->rf_params[table_num]->last_num_cyls = firmware->rf_params[table_num]->num_cyls;

			firmware->rf_params[table_num]->num_cyls = tmpi;
			if (firmware->rf_params[table_num]->num_cyls % firmware->rf_params[table_num]->num_squirts)
				set_reqfuel_color_f(RED,table_num);
			else
			{
				tmp = ms_get_ecu_data(canID,page,offset,size);
				tmp = tmp & ~bitmask;   /*clears bits */
				if (firmware->capabilities & MS2)
					tmp = tmp | ((tmpi) << bitshift);
				else
					tmp = tmp | ((tmpi-1) << bitshift);
				dload_val = tmp;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(offset),
						d_data);

				dload_val =
					(GINT)(((float)firmware->rf_params[table_num]->num_cyls/(float)firmware->rf_params[table_num]->num_squirts)+0.001);

				firmware->rf_params[table_num]->divider = dload_val;
				d_data = g_new0(Deferred_Data, 1);
				d_data->canID = canID;
				d_data->page = page;
				d_data->offset = divider_offset;
				d_data->value = dload_val;
				d_data->size = MTX_U08;
				g_hash_table_replace(interdep_vars[table_num],
						GINT_TO_POINTER(divider_offset),
						d_data);

				set_reqfuel_color_f(BLACK,table_num);
				check_req_fuel_limits(table_num);
			}
			break;
		case NUM_INJECTORS_1:
		case NUM_INJECTORS_2:
			/* Updates a shared bitfield */
			table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			firmware->rf_params[table_num]->last_num_inj = firmware->rf_params[table_num]->num_inj;
			firmware->rf_params[table_num]->num_inj = tmpi;

			tmp = ms_get_ecu_data(canID,page,offset,size);
			tmp = tmp & ~bitmask;   /*clears bits */
			if (firmware->capabilities & MS2)
				tmp = tmp | ((tmpi) << bitshift);
			else
				tmp = tmp | ((tmpi-1) << bitshift);
			dload_val = tmp;

			d_data = g_new0(Deferred_Data, 1);
			d_data->canID = canID;
			d_data->page = page;
			d_data->offset = offset;
			d_data->value = dload_val;
			d_data->size = MTX_U08;
			g_hash_table_replace(interdep_vars[table_num],
					GINT_TO_POINTER(offset),
					d_data);

			check_req_fuel_limits(table_num);
			break;
		case TRIGGER_ANGLE:
			spconfig_offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			if (spconfig_offset == 0)
			{
				dbg_func_f(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\tERROR Trigger Angle spinbutton call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;
				break;

			}
			if (value > 112.15)     /* Extra long trigger needed */
			{
				tmp = ms_get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);   /* Set xlong_trig */
				ms_send_to_ecu(canID, page, spconfig_offset, size, tmp,  TRUE);
				value -= 45.0;
				dload_val = convert_before_download_f(widget,value);
			}
			else if (value > 89.65) /* Long trigger needed */
			{
				tmp = ms_get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);   /* Set long_trig */
				ms_send_to_ecu(canID, page, spconfig_offset, size, tmp, TRUE);
				value -= 22.5;
				dload_val = convert_before_download_f(widget,value);
			}
			else    /* value <= 89.65 degrees, no long trigger*/
			{
				tmp = ms_get_ecu_data(canID,page,spconfig_offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				ms_send_to_ecu(canID, page, spconfig_offset, size, tmp, TRUE);
				dload_val = convert_before_download_f(widget,value);
			}

			break;

		case ODDFIRE_ANGLE:
			oddfire_bit_offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (oddfire_bit_offset == 0)
			{
				dbg_func_f(CRITICAL,g_strdup(__FILE__": spin_button_handler()\n\tERROR Offset Angle spinbutton call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;
				break;

			}
			if (value > 90) /*  */
			{
				tmp = ms_get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);   /* Set +90 */
				ms_send_to_ecu(canID, page, oddfire_bit_offset, size, tmp, TRUE);
				value -= 90.0;
				dload_val = convert_before_download_f(widget,value);
			}
			else if (value > 45) /* */
			{
				tmp = ms_get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);   /* Set +45 */
				ms_send_to_ecu(canID, page, oddfire_bit_offset, size, tmp, TRUE);
				value -= 45.0;
				dload_val = convert_before_download_f(widget,value);
			}
			else    /* value <= 45 degrees, */
			{
				tmp = ms_get_ecu_data(canID,page,oddfire_bit_offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				ms_send_to_ecu(canID, page, oddfire_bit_offset, size, tmp,  TRUE);
				dload_val = convert_before_download_f(widget,value);
			}

			break;

		case GENERIC:   /* Handles almost ALL other variables */
			if ((GBOOLEAN)OBJ_GET(widget,"temp_dep"))
			{
				if ((GINT)DATA_GET(global_data,"temp_units") == CELSIUS)
					value = (value*(9.0/5.0))+32;
			}
			dload_val = convert_before_download_f(widget,value);
			break;
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_spin_handler",(void *)&ecu_handler))
					return ecu_handler(widget,data);
				else
				{
					dbg_func_f(CRITICAL,g_strdup(__FILE__": common_spin_handler()\n\tDefault case, ecu handler NOT found in plugins, BUG!\n"));
					return TRUE;
				}
			}
			else
				return ecu_handler(widget,data);
			break;
	}
	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != ms_get_ecu_data(canID,page,offset,size))
			ms_send_to_ecu(canID, page, offset, size, dload_val, TRUE);
	}
	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	return TRUE;

}



G_MODULE_EXPORT void update_widget(gpointer object, gpointer user_data)
{
	static gint upd_count = 0;
	GtkWidget * widget = object;
	gdouble value = 0.0;

	if (DATA_GET(global_data,"leaving"))
		return;

	upd_count++;
	if ((upd_count%96) == 0)
	{
		while (gtk_events_pending())
		{
			if (DATA_GET(global_data,"leaving"))
			{
				return;
			}
			gtk_main_iteration();
		}
	}
	if (!GTK_IS_WIDGET(widget))
		return;

	/* If passed widget and user data are identical,  break out as
	 * we already updated the widget.
	 */
	if ((GTK_IS_WIDGET(user_data)) && (widget == user_data))
		return;

	/*printf("update_widget %s, page %i, offset %i bitval %i, mask %i, shift %i\n",(gchar *)glade_get_widget_name(widget), page,offset,bitval,bitmask,bitshift);*/
	/* update widget whether spin,radio or checkbutton  
	 * (checkbutton encompases radio)
	 */
	value = convert_after_upload_f(widget);
	
	if (GTK_IS_ENTRY(widget) || GTK_IS_SPIN_BUTTON(widget))
		update_entry(widget);
	else if (GTK_IS_COMBO_BOX(widget))
		update_combo(widget);
	else if (GTK_IS_CHECK_BUTTON(widget))
		update_checkbutton(widget);
	else if (GTK_IS_RANGE(widget))
		gtk_range_set_value(GTK_RANGE(widget),value);
	else if (GTK_IS_SCROLLED_WINDOW(widget))
	{
		/* This will looks really weird, but is used in the 
		 * special case of a treeview widget which is always
		 * packed into a scrolled window. Since the treeview
		 * depends on ECU variables, we call a handler here
		 * passing in a pointer to the treeview(the scrolled
		 * window's child widget)
		 */
		update_model_from_view(gtk_bin_get_child(GTK_BIN(widget)));
	}
	/* IF control has groups linked to it's state, adjust */

}


void update_checkbutton(GtkWidget *widget)
{
	gboolean cur_state = FALSE;
	gint tmpi = 0;
	gboolean new_state = FALSE;
	gint bitmask = 0;
	gint bitshift = 0;
	gint bitval = 0;
	gdouble value = 0.0;
	gchar * set_labels = NULL;

	get_essential_bits(widget, NULL, NULL, NULL, &bitval, &bitmask, &bitshift);

	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(widget)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget),FALSE);
	/* Swaps the label of another control based on widget state... */
	/* If value masked by bitmask, shifted right by bitshift = bitval
	 * then set button state to on...
	 */
	cur_state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	value = convert_after_upload_f(widget);
	tmpi = (GINT)value;
	/* Avoid unnecessary widget setting and signal propogation 
	 * First if.  If current bit is SET but button is NOT, set it
	 * Second if, If currrent bit is NOT set but button IS  then
	 * un-set it.
	 */
	if (((tmpi & bitmask) >> bitshift) == bitval)
		new_state = TRUE;
	else if (((tmpi & bitmask) >> bitshift) != bitval)
		new_state = FALSE;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),new_state);
	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");
	if ((set_labels) && (new_state))
		set_widget_labels(set_labels);
	if (OBJ_GET(widget,"swap_labels"))
		swap_labels(widget,new_state);
	if ((new_state) && (OBJ_GET(widget,"group_2_update")))
		handle_group_2_update(widget);
	if (new_state)
		handle_algorithm(widget);
	if (OBJ_GET(widget,"toggle_groups"))
		toggle_groups_linked(widget,new_state);
}


void update_entry(GtkWidget *widget)
{
	static Firmware_Details *firmware = NULL;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint precision = 0;
	gboolean changed = FALSE;
	gboolean use_color = FALSE;
	DataSize size = 0;
	gchar * widget_text = NULL;
	gchar * tmpbuf = NULL;
	gfloat scaler = 0.0;
	gdouble value = 0.0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	gint table_num = -1;
	gfloat spin_value = 0.0;
	gboolean force_color_update = FALSE;
	GdkColor color;
	GdkColor black = {0,0,0,0};

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");
	get_essentials(widget, &canID, &page, &offset, &size, &precision);
	value = convert_after_upload_f(widget);

	if ((GINT)OBJ_GET(widget,"handler") == ODDFIRE_ANGLE)
	{
		offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
		if (offset == 0)
			return;
		switch (ms_get_ecu_data(canID,page,offset,size))
		{
			case 4:
				value += 90;
				break;
			case 2:
				value += 45;
				break;
			case 0:
				value += 0;
				break;
			default:
				dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t ODDFIRE_ANGLE_UPDATE invalid value for oddfire_bit_offset at ecu_data[%i][%i], ERROR\n",page,offset));

		}
		if (GTK_IS_ENTRY(widget))
		{
			tmpbuf = g_strdup_printf("%1$.*2$f",value+90,precision);
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
		}
		else
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
	}
	else if ((GINT)OBJ_GET(widget,"handler") == TRIGGER_ANGLE)
	{
		offset = (GINT)OBJ_GET(widget,"spconfig_offset");
		switch ((ms_get_ecu_data(canID,page,offset,size) & 0x03))
		{
			case 2:
				value += 45.0;
				tmpbuf = g_strdup_printf("%1$.*2$f",value+45,precision);
				break;
			case 1:
				value += 22.5;
				tmpbuf = g_strdup_printf("%1$.*2$f",value+22.5,precision);
				break;
			case 0:
				value += 0;
				tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
				break;
			default:
				dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t TRIGGER_ANGLE_UPDATE invalid value for spconfig_offset at ecu_data[%i][%i], ERROR\n",page,offset));

		}
		if (GTK_IS_ENTRY(widget))
		{
			tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
		}
		else
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
	}
	else
	{
		if ((GBOOLEAN)OBJ_GET(widget,"temp_dep"))
		{
			if ((GINT)DATA_GET(global_data,"temp_units") == CELSIUS)
				value = (value-32)*(5.0/9.0);
		}
		if (GTK_IS_ENTRY(widget))
		{

			widget_text = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
			tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
			/* If different, update it */
			if (g_ascii_strcasecmp(widget_text,tmpbuf) != 0)
			{
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				changed = TRUE;
			}
			g_free(tmpbuf);
		}
		else
		{
			spin_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			if (value != spin_value)
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
		}

		if (OBJ_GET(widget,"use_color"))
		{
			force_color_update = (GBOOLEAN)OBJ_GET(widget,"force_color_update");
			if (OBJ_GET(widget,"table_num"))
				table_num = (GINT)strtol(OBJ_GET(widget,"table_num"),NULL,10);
			if (OBJ_GET(widget,"raw_lower"))
				raw_lower = (GINT)strtol(OBJ_GET(widget,"raw_lower"),NULL,10);
			else
				raw_lower = get_extreme_from_size_f(size,LOWER);
			if (OBJ_GET(widget,"raw_upper"))
				raw_upper = (GINT)strtol(OBJ_GET(widget,"raw_upper"),NULL,10);
			else
				raw_upper = get_extreme_from_size_f(size,UPPER);

			if ((table_num >= 0) && (firmware->table_params[table_num]->color_update))
			{
				scaler = 256.0/((firmware->table_params[table_num]->z_maxval - firmware->table_params[table_num]->z_minval)*1.05);
				color = get_colors_from_hue_f(256.0 - (ms_get_ecu_data(canID,page,offset,size)-firmware->table_params[table_num]->z_minval)*scaler, 0.50, 1.0);
				gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
			}
			else
			{
				if ((changed) || (value == 0) || (force_color_update))
				{
					color = get_colors_from_hue_f(((gfloat)(ms_get_ecu_data(canID,page,offset,size)-raw_lower)/raw_upper)*-300.0+180, 0.50, 1.0);
					gtk_widget_modify_base(GTK_WIDGET(widget),GTK_STATE_NORMAL,&color);
				}
			}
		}
	}
	if (OBJ_GET(widget,"not_sent"))
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
}


void update_combo(GtkWidget *widget)
{
	static void (*update_ms2_user_outputs)(GtkWidget *) = NULL;
	gint tmpi = -1;
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	gint bitval = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint t_bitval = 0;
	gdouble value = 0;
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	gint valid = 0;
	gint i = 0;
	gchar * tmpbuf = NULL; 
	GdkColor red = {0,65535,0,0};
	GdkColor white = {0,65535,65535,65535};


	get_essential_bits(widget,&canID, &page, &offset, &bitval, &bitmask, &bitshift);
	/*printf("Combo at page %i, offset %i, bitmask %i, bitshift %i, value %i\n",page,offset,bitmask,bitshift,(GINT)value);*/

	if ((GINT)OBJ_GET(widget,"handler") == MS2_USER_OUTPUTS)
	{
		if (!update_ms2_user_outputs)
			get_symbol_f("update_ms2_user_outputs",(void *)&update_ms2_user_outputs);
		if (update_ms2_user_outputs)
			update_ms2_user_outputs(widget);
		else
			dbg_func_f(CRITICAL,g_strdup(__FILE__": update_combo()\n\tCould NOT locate fucntion pointer for \"update_ms2_user_outputs\" BUG! \n"));
	}

	value = convert_after_upload_f(widget);
	tmpi = ((GINT)value & bitmask) >> bitshift;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (!GTK_IS_TREE_MODEL(model))
		printf(_("ERROR no model for Combo at page %i, offset %i, bitmask %i, bitshift %i, value %i\n"),page,offset,bitmask,bitshift,(GINT)value);
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model),&iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,BITVAL_COL,&t_bitval,-1);
		if (tmpi == t_bitval)
		{
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
			gtk_widget_modify_base(GTK_BIN (widget)->child,GTK_STATE_NORMAL,&white);
			if (OBJ_GET(widget,"group_2_update"))
				combo_handle_group_2_update(widget);
			if (OBJ_GET(widget,"algorithms"))
				combo_handle_algorithms(widget);
			goto combo_toggle;
		}
		valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &iter);
		i++;

	}
	/*printf("COULD NOT FIND MATCH for data for combo %p, data %i!!\n",widget,tmpi);*/
	gtk_widget_modify_base(GTK_BIN(widget)->child,GTK_STATE_NORMAL,&red);
	return;

combo_toggle:
	if (OBJ_GET(widget,"toggle_labels"))
		combo_toggle_labels_linked(widget,i);
	if (OBJ_GET(widget,"toggle_groups"))
		combo_toggle_groups_linked(widget,i);
	if (OBJ_GET(widget,"swap_labels"))
		swap_labels(widget,tmpi);
	if (OBJ_GET(widget,"set_widgets_label"))
		combo_set_labels(widget,model);
}


void combo_handle_group_2_update(GtkWidget *widget)
{
	static GHashTable *sources_hash = NULL;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	
	if (!sources_hash)
		sources_hash = DATA_GET(global_data,"sources_hash");
	

	if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_values")))
	{
		tmpbuf = OBJ_GET(widget,"source_values");
		vector = g_strsplit(tmpbuf,",",-1);
		if ((guint)gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) >= g_strv_length(vector))
		{
			dbg_func_f(CRITICAL,g_strdup(__FILE__": update_widget()\n\tCOMBOBOX Problem with source_values,  length mismatch, check datamap\n"));
			return ;
		}
		/*printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]);*/
		g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]));
		g_list_foreach(get_list_f("multi_expression"),update_widget,NULL);
		g_strfreev(vector);
	}
}


void combo_handle_algorithms(GtkWidget *widget)
{
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	gint algo = 0;
	gint i = 0;
	gint *algorithm = NULL;

	if (!algorithm)
		algorithm = DATA_GET(global_data,"algorithm");

	tmpbuf = (gchar *)OBJ_GET(widget,"algorithms");
	if (tmpbuf)
	{
		vector = g_strsplit(tmpbuf,",",-1);
		if ((guint)gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) >= g_strv_length(vector))
		{
			dbg_func_f(CRITICAL,g_strdup(__FILE__": update_widget()\n\tCOMBOBOX Problem with algorithms, length mismatch, check datamap\n"));
			return ;
		}
		algo = translate_string_f(vector[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))]);
		g_strfreev(vector);

		tmpbuf = (gchar *)OBJ_GET(widget,"applicable_tables");
		if (!tmpbuf)
		{
			dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t Check/Radio button  %s has algorithm defines but no applicable tables, BUG!\n",(gchar *)glade_get_widget_name(widget)));
			return;
		}

		vector = g_strsplit(tmpbuf,",",-1);
		i = 0;
		while (vector[i])
		{
			algorithm[(GINT)strtol(vector[i],NULL,10)]=(Algorithm)algo;
			i++;
		}
		g_strfreev(vector);
	}
}

void handle_algorithm(GtkWidget *widget)
{
	static gint * algorithm = NULL;
	gint algo = 0;
	gint i = 0;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;

	if (!algorithm)
		algorithm = DATA_GET(global_data,"algorithm");

	algo = (Algorithm)OBJ_GET(widget,"algorithm");
	if (algo > 0)
	{
		tmpbuf = (gchar *)OBJ_GET(widget,"applicable_tables");
		if (!tmpbuf)
		{
			dbg_func_f(CRITICAL,g_strdup_printf(__FILE__": update_widget()\n\t Check/Radio button  %s has algorithm defines but no applicable tables, BUG!\n",(gchar *)glade_get_widget_name(widget)));
			return;
		}

		vector = g_strsplit(tmpbuf,",",-1);
		i = 0;
		while (vector[i])
		{
			algorithm[(GINT)strtol(vector[i],NULL,10)]=(Algorithm)algo;
			i++;
		}
		g_strfreev(vector);
	}
}

void handle_group_2_update(GtkWidget *widget)
{
	static GHashTable *sources_hash = NULL;

	if (!sources_hash)
		sources_hash = DATA_GET(global_data,"sources_hash");
	if ((OBJ_GET(widget,"source_key")) && (OBJ_GET(widget,"source_value")))
	{
		/*      printf("key %s value %s\n",(gchar *)OBJ_GET(widget,"source_key"),(gchar *)OBJ_GET(widget,"source_value"));*/
		g_hash_table_replace(sources_hash,g_strdup(OBJ_GET(widget,"source_key")),g_strdup(OBJ_GET(widget,"source_value")));

	}
	gdk_threads_add_timeout(2000,force_view_recompute,NULL);
	gdk_threads_add_timeout(2000,trigger_group_update,OBJ_GET(widget,"group_2_update"));

}

void combo_set_labels(GtkWidget *widget, GtkTreeModel *model)
{
	gint total = 0;
	gint tmpi = 0;
	gint i = 0;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	gchar * set_labels = NULL;

	set_labels = (gchar *)OBJ_GET(widget,"set_widgets_label");

	total = get_choice_count(model);
	tmpi = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	vector = g_strsplit(set_labels,",",-1);
	if ((g_strv_length(vector)%(total+1)) != 0)
	{
		dbg_func_f(CRITICAL,g_strdup(__FILE__": combo_set_labels()\n\tProblem wiht set_labels, counts don't match up\n"));
		return;
	}
	for (i=0;i<(g_strv_length(vector)/(total+1));i++)
	{
		tmpbuf = g_strconcat(vector[i*(total+1)],",",vector[(i*(total+1))+1+tmpi],NULL);
		set_widget_labels(tmpbuf);
		g_free(tmpbuf);
	}
	g_strfreev(vector);
}


G_MODULE_EXPORT gint get_choice_count(GtkTreeModel *model)
{
	gchar *choice = NULL;
	gboolean valid = TRUE;
	GtkTreeIter iter;
	gint i = 0;

	valid = gtk_tree_model_get_iter_first(model,&iter);
	while (valid)
	{
		gtk_tree_model_get(model,&iter,-1);
		valid = gtk_tree_model_iter_next (model, &iter);
		i++;
	}
	return i;
}


/*!
 * \brief toggle_groups_linked is used to change the state of controls that
 * are "linked" to various other controls for the purpose of making the 
 * UI more intuitive.  i.e. if u uncheck a feature, this can be used to 
 * grey out a group of related controls.
 * \param toggle_groups, comms sep list of group names
 * \param new_state,  new state of the button linking to these groups
 */

G_MODULE_EXPORT void toggle_groups_linked(GtkWidget *widget,gboolean new_state)
{
	gint num_choices = 0;
	gint num_groups = 0;
	gint i = 0;
	gboolean state = FALSE;
	gchar **choices = NULL;
	gchar **groups = NULL;
	gchar * toggle_groups = NULL;
	gchar * tmpbuf = NULL;
	GHashTable *widget_group_states = NULL;

	if (!DATA_GET(global_data,"ready"))
		return;
	widget_group_states = DATA_GET(global_data,"widget_group_states");
	toggle_groups = (gchar *)OBJ_GET(widget,"toggle_groups");
	/*      printf("groups to toggle %s to state %i\n",toggle_groups,new_state);*/

	choices = parse_keys_f(toggle_groups,&num_choices,",");
	if (num_choices != 2)
		printf(_("toggle_groups_linked, numeber of choices is out of range, it should be 2, it is %i"),num_choices);

	/*printf("toggle groups defined for widget %p at page %i, offset %i\n",widget,page,offset);*/

	/* Turn off */
	groups = parse_keys_f(choices[0],&num_groups,":");
	/*      printf("Choice %i, has %i groups\n",0,num_groups);*/
	state = FALSE;
	for (i=0;i<num_groups;i++)
	{
		/*              printf("setting all widgets in group %s to state %i\n\n",groups[i],state);*/
		tmpbuf = g_strdup_printf("!%s",groups[i]);
		g_hash_table_replace(widget_group_states,g_strdup(tmpbuf),GINT_TO_POINTER(!state));
		g_list_foreach(get_list_f(tmpbuf),alter_widget_state_f,NULL);
		g_free(tmpbuf);

		g_hash_table_replace(widget_group_states,g_strdup(groups[i]),GINT_TO_POINTER(state));
		g_list_foreach(get_list_f(groups[i]),alter_widget_state_f,NULL);
	}
	g_strfreev(groups);
	/* Turn on */
	groups = parse_keys_f(choices[1],&num_groups,":");
	/*      printf("Choice %i, has %i groups\n",1,num_groups);*/
	state = new_state;
	for (i=0;i<num_groups;i++)
	{
		/*              printf("setting all widgets in group %s to state %i\n\n",groups[i],state);*/
		tmpbuf = g_strdup_printf("!%s",groups[i]);
		g_hash_table_replace(widget_group_states,g_strdup(tmpbuf),GINT_TO_POINTER(!state));
		g_list_foreach(get_list_f(tmpbuf),alter_widget_state_f,NULL);
		g_free(tmpbuf);
		g_hash_table_replace(widget_group_states,g_strdup(groups[i]),GINT_TO_POINTER(state));
		g_list_foreach(get_list_f(groups[i]),alter_widget_state_f,NULL);
	}
	g_strfreev(groups);
	g_strfreev(choices);
}


/*!
 * \brief combo_toggle_groups_linked is used to change the state of controls that
 * are "linked" to various other controls for the purpose of making the 
 * UI more intuitive.  i.e. if u uncheck a feature, this can be used to 
 * grey out a group of related controls.
 * \param widget, combo button
 * \param active, which entry in list was selected
 */
G_MODULE_EXPORT void combo_toggle_groups_linked(GtkWidget *widget,gint active)
{
	gint num_groups = 0;
	gint num_choices = 0;
	gint i = 0;
	gint j = 0;
	gboolean state = FALSE;
	gint page = 0;
	gint offset = 0;

	gchar **choices = NULL;
	gchar **groups = NULL;
	gchar * toggle_groups = NULL;
	gchar * tmpbuf = NULL;
	GHashTable *widget_group_states = NULL;

	if (!DATA_GET(global_data,"ready"))
		return;
	widget_group_states = DATA_GET(global_data,"widget_group_states");
	toggle_groups = (gchar *)OBJ_GET(widget,"toggle_groups");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");

	/*printf("toggling combobox groups\n");*/
	choices = parse_keys_f(toggle_groups,&num_choices,",");
	if (active >= num_choices)
	{
		printf(_("active is %i, num_choices is %i\n"),active,num_choices);
		printf(_("active is out of bounds for widget %s\n"),glade_get_widget_name(widget));
	}
	/*printf("toggle groups defined for combo box %p at page %i, offset %i\n",widget,page,offset);*/

	/* First TURN OFF all non active groups */
	for (i=0;i<num_choices;i++)
	{
		groups = parse_keys_f(choices[i],&num_groups,":");
		/*printf("Choice %i, has %i groups\n",i,num_groups);*/
		if (i == active)
			continue;
		state = FALSE;
		for (j=0;j<num_groups;j++)
		{
			/*printf("setting all widgets in group %s to state %i\n\n",groups[j],state);*/
			tmpbuf = g_strdup_printf("!%s",groups[j]);
			g_hash_table_replace(widget_group_states,g_strdup(tmpbuf),GINT_TO_POINTER(!state));
			g_list_foreach(get_list_f(tmpbuf),alter_widget_state_f,NULL);
			g_free(tmpbuf);
			g_hash_table_replace(widget_group_states,g_strdup(groups[j]),GINT_TO_POINTER(state));
			g_list_foreach(get_list_f(groups[j]),alter_widget_state_f,NULL);
		}
		g_strfreev(groups);
	}

	/* Then TURN ON all active groups */
	groups = parse_keys_f(choices[active],&num_groups,":");
	state = TRUE;
	for (j=0;j<num_groups;j++)
	{
		/*printf("setting all widgets for %s in group %s to state %i\n\n",glade_get_widget_name(widget),groups[j],state);*/
		tmpbuf = g_strdup_printf("!%s",groups[j]);
		g_hash_table_replace(widget_group_states,g_strdup(tmpbuf),GINT_TO_POINTER(!state));
		g_list_foreach(get_list_f(tmpbuf),alter_widget_state_f,NULL);
		g_free(tmpbuf);
		g_hash_table_replace(widget_group_states,g_strdup(groups[j]),GINT_TO_POINTER(state));
		g_list_foreach(get_list_f(groups[j]),alter_widget_state_f,NULL);
	}
	g_strfreev(groups);

	/*printf ("DONE!\n\n\n");*/
	g_strfreev(choices);
}


/*!
 * \brief combo_toggle_labels_linked is used to change the state of controls that
 * are "linked" to various other controls for the purpose of making the 
 * UI more intuitive.  i.e. if u uncheck a feature, this can be used to 
 * grey out a group of related controls.
 * \param widget, combo button
 * \param active, which entry in list was selected
 */     
G_MODULE_EXPORT void combo_toggle_labels_linked(GtkWidget *widget,gint active)
{
	gint num_groups = 0;
	gint i = 0;
	gchar **groups = NULL;
	gchar * toggle_labels = NULL;

	if (!DATA_GET(global_data,"ready"))
		return;
	toggle_labels = (gchar *)OBJ_GET(widget,"toggle_labels");

	groups = parse_keys_f(toggle_labels,&num_groups,",");
	/* toggle_labels is a list of groups of widgets that need to have their 
	 * label reset to a specific one from an array bound to that widget.
	 * So we get the names of those groups, and call "set_widget_label_from_array"
	 * passing in the index of the one in the array we want
	 */
	for (i=0;i<num_groups;i++)
		g_list_foreach(get_list_f(groups[i]),set_widget_label_from_array,GINT_TO_POINTER(active));   

	g_strfreev(groups);
}



G_MODULE_EXPORT void set_widget_label_from_array(gpointer key, gpointer data)
{       
	gchar *labels = NULL;
	gchar **vector = NULL;
	GtkWidget *label = (GtkWidget *)key;
	gint index = (GINT)data;

	if (!GTK_IS_LABEL(label))
		return; 
	labels = OBJ_GET(label,"labels");
	if (!labels)
		return; 
	vector = g_strsplit(labels,",",-1);
	if (index > g_strv_length(vector))
		return; 
	gtk_label_set_text(GTK_LABEL(label),vector[index]);
	g_strfreev(vector);
	return;         
}      


G_MODULE_EXPORT gboolean search_model(GtkTreeModel *model, GtkWidget *box, GtkTreeIter *iter)
{
	gchar *choice = NULL;
	gboolean valid = TRUE;
	gchar * cur_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(box));
	valid = gtk_tree_model_get_iter_first(model,iter);
	while (valid)
	{
		gtk_tree_model_get(model,iter,0, &choice, -1);
		if (g_ascii_strcasecmp(cur_text,choice) == 0)
		{
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(box),iter);
			return TRUE;
		}
		valid = gtk_tree_model_iter_next (model, iter);
	}
	return FALSE;
}

void get_essential_bits(GtkWidget *widget, gint *canID, gint *page, gint *offset, gint *bitval, gint *bitmask, gint *bitshift)
{
	if (!GTK_IS_WIDGET(widget))
		return;
	if (canID)
		*canID = (GINT)OBJ_GET(widget,"canID");
	if (page)
		*page = (GINT)OBJ_GET(widget,"page");
	if (offset)
		*offset = (GINT)OBJ_GET(widget,"offset");
	if (bitval)
		*bitval = (GINT)OBJ_GET(widget,"bitval");
	if (bitmask)
		*bitmask = (GINT)OBJ_GET(widget,"bitmask");
	if (bitshift)
		*bitshift = get_bitshift_f((GINT)OBJ_GET(widget,"bitmask"));
}


void get_essentials(GtkWidget *widget, gint *canID, gint *page, gint *offset, DataSize *size, gint *precision)
{
	if (!GTK_IS_WIDGET(widget))
		return;
	if (canID)
		*canID = (GINT)OBJ_GET(widget,"canID");
	if (page)
		*page = (GINT)OBJ_GET(widget,"page");
	if (offset)
		*offset = (GINT)OBJ_GET(widget,"offset");
	if (size)
	{
		if (!OBJ_GET(widget,"size"))
			*size = MTX_U08 ;        /* default! */
		else
			*size = (DataSize)OBJ_GET(widget,"size");
	}
	if (precision)
		*precision = (DataSize)OBJ_GET(widget,"precision");
}


/*!
   \brief recalc_table_limits() Finds the minimum and maximum values for a 
    2D table (this will be deprecated when thevetables are a custom widget)
     */
G_MODULE_EXPORT void recalc_table_limits(gint canID, gint table_num)
{
	static Firmware_Details *firmware = NULL;
	gint i = 0;
	gint x_count = 0;
	gint y_count = 0;
	gint z_base = 0;
	gint z_page = 0;
	gint z_size = 0;
	gint z_mult = 0;
	gint tmpi = 0;
	gint max = 0;
	gint min = 0;

	if (!firmware)
		firmware = DATA_GET(global_data,"firmware");

	/* Limit check */
	if ((table_num < 0 ) || (table_num > firmware->total_tables-1))
		return;
	firmware->table_params[table_num]->last_z_maxval = firmware->table_params[table_num]->z_maxval;
	firmware->table_params[table_num]->last_z_minval = firmware->table_params[table_num]->z_minval;
	x_count = firmware->table_params[table_num]->x_bincount;
	y_count = firmware->table_params[table_num]->y_bincount;
	z_base = firmware->table_params[table_num]->z_base;
	z_page = firmware->table_params[table_num]->z_page;
	z_size = firmware->table_params[table_num]->z_size;
	z_mult = get_multiplier_f(z_size);
	min = get_extreme_from_size_f(z_size,UPPER);
	max = get_extreme_from_size_f(z_size,LOWER);

	for (i=0;i<x_count*y_count;i++)
	{
		tmpi = ms_get_ecu_data(canID,z_page,z_base+(i*z_mult),z_size);
		if (tmpi > max)
			max = tmpi;
		if (tmpi < min)
			min = tmpi;
	}
	if (min == max) /* FLAT table, causes black screen */
	{
		min -= 10;
		max += 10;
	}
	firmware->table_params[table_num]->z_maxval = max;
	firmware->table_params[table_num]->z_minval = min;
	return;
}


G_MODULE_EXPORT void thread_refresh_widgets_at_offset(gint page, gint offset)
{
	guint i = 0;
	Firmware_Details *firmware = NULL;
	GList ***ve_widgets = NULL;

	ve_widgets = DATA_GET(global_data,"ve_widgets");

	firmware = DATA_GET(global_data,"firmware");

	for (i=0;i<g_list_length(ve_widgets[page][offset]);i++)
		thread_refresh_widget_f(g_list_nth_data(ve_widgets[page][offset],i));
	/*update_ve3d_if_necessary_f(page,offset);*/
}
