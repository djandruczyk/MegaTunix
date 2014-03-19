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
  \file src/plugins/libreems/libreems_gui_handlers.c
  \ingroup LibreEMSPlugin,Plugins
  \brief LibreEMS specific Gui handlers
  \author David Andruczyk
  */

#include <config.h>
#include <combo_loader.h>
#include <datamgmt.h>
#include <defines.h>
#include <firmware.h>
#include <libreems_benchtest.h>
#include <libreems_comms.h>
#include <libreems_helpers.h>
#include <libreems_gui_handlers.h>
#include <libreems_plugin.h>
#include <glade/glade.h>
#include <gtk/gtk.h>


extern gconstpointer *global_data;


/*!
  \brief ECU family plugin to do common (core) gui initialization, this will
  check and call the ecu specific gui init function if preset
  */
G_MODULE_EXPORT void common_gui_init(void)
{
	void (*ecu_gui_init_f)(void) = NULL;
	ENTER();
	/* This function is for doing any gui finalization on the CORE gui
	   for stuff specific to this firmware family.
	   */
	/* If ECU lib has a call run it */
	if (get_symbol_f("ecu_gui_init",(void **)&ecu_gui_init_f))
		ecu_gui_init_f();
}


/*!
  \brief ECU family plugin to handle toggle button handling.  If the handler
  assigned to the widget isn't found, call the ecu specific handler if it 
  exists and return it's result
  \param widget is the toggle button the user manipulated
  \param data is the data pointer attached to the widget
  \returns the result of the ECU handler or TRUE
  */
G_MODULE_EXPORT gboolean common_toggle_button_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	LibreEMSCommonToggleHandler handler;
	ENTER();

	handler = (LibreEMSCommonToggleHandler)(GINT)OBJ_GET(widget,"handler");

	switch (handler)
	{
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_toggle_button_handler",(void **)&ecu_handler))
				{
					EXIT();
					return ecu_handler(widget,data);
				}
				else
					MTXDBG(CRITICAL,_("Default case, but there is NO ecu_toggle_button_handler available, unhandled case for widget %s, BUG!\n"),glade_get_widget_name(widget));
			}
			else
			{
				EXIT();
				return ecu_handler(widget,data);
			}
			break;

	}
	EXIT();
	return TRUE;
 }


/*!
  \brief ECU family plugin to handle standard button handling.  If the handler
  assigned to the widget isn't found, call the ecu specific handler if it 
  exists and return it's result
  \param widget is the standard button the user manipulated
  \param data is the data pointer attached to the widget
  \returns the result of the ECU handler or TRUE
  */
G_MODULE_EXPORT gboolean common_std_button_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	LibreEMSCommonStdHandler handler;
	ENTER();

	handler = (LibreEMSCommonStdHandler)(GINT)OBJ_GET(widget,"handler");

	switch (handler)
	{
		case SOFT_BOOT_ECU:
			soft_boot_ecu();
			break;
		case HARD_BOOT_ECU:
			hard_boot_ecu();
			break;
		case BENCHTEST_START:
			benchtest_validate_and_run();
			break;
		case BENCHTEST_STOP:
			benchtest_stop();
			break;
		case BENCHTEST_BUMP:
			benchtest_bump();
			break;
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_std_button_handler",(void **)&ecu_handler))
				{
					EXIT();
					return ecu_handler(widget,data);
				}
				else
					MTXDBG(CRITICAL,_("Default case, but there is NO ecu_std_button_handler available, unhandled case for widget %s, BUG!\n"),glade_get_widget_name(widget));
			}
			else
			{
				EXIT();
				return ecu_handler(widget,data);
			}
			break;

	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU family plugin to handle radio button handling.  If the handler
  assigned to the widget isn't found, call the ecu specific handler if it 
  exists and return it's result
  \param widget is the radio button the user manipulated
  \param data is the data pointer attached to the widget
  \returns the result of the ECU handler or TRUE
  */
G_MODULE_EXPORT gboolean common_bitmask_button_handler(GtkWidget *widget, gpointer data)
{
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	gint handler = 0;
	ENTER();

	handler = (GINT)OBJ_GET(widget,"handler");

	/* No handlers yet, try ecu specific plugin */
	switch (handler)
	{
		default:
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_bitmask_button_handler",(void **)&ecu_handler))
				{
					EXIT();
					return ecu_handler(widget,data);
				}
				else
					MTXDBG(CRITICAL,_("Default case, but there is NO ecu_bitmask_button_handler available, unhandled case for widget %s, BUG!\n"),glade_get_widget_name(widget));
			}
			else
			{
				EXIT();
				return ecu_handler(widget,data);
			}
			break;
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU family plugin to handle text entry handling.  If the handler
  assigned to the widget isn't found, call the ecu specific handler if it 
  exists and return it's result
  \param widget is the text entry the user manipulated
  \param data is the data pointer attached to the widget
  \returns the result of the ECU handler or TRUE
  */
G_MODULE_EXPORT gboolean common_entry_handler(GtkWidget *widget, gpointer data)
{
	static Firmware_Details *firmware = NULL;
	static gboolean (*ecu_handler)(GtkWidget *, gpointer) = NULL;
	GdkColor black = {0,0,0,0};
	LibreEMSCommonStdHandler handler;
	gchar *text = NULL;
	gchar *tmpbuf = NULL;
	gfloat tmpf = -1;
	gfloat value = -1;
	gint table_num = -1;
	gint tmpi = -1;
	gint tmp = -1;
	gint base = -1;
	gint old = -1;
	gint locID = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint dl_type = -1;
	gint precision = -1;
	gint spconfig_offset = -1;
	gint oddfire_bit_offset = -1;
	gint mtx_temp_units = 0;
	gfloat scaler = 0.0;
	gboolean temp_dep = FALSE;
	gfloat real_value = 0.0;
	DataSize size = MTX_U08;
	gint raw_lower = 0;
	gint raw_upper = 0;
	GdkColor color;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_val_if_fail(firmware,FALSE);

	handler = (LibreEMSCommonStdHandler)(GINT)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	precision = (GINT) OBJ_GET(widget,"precision");
	get_essentials(widget,&locID,&offset,&size,&precision);

	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpi = (GINT)strtol(text,NULL,10);
	tmpf = (gfloat)g_ascii_strtod(g_strdelimit(text,",.",'.'),NULL);
	/*printf("text \"%s\" int val \"%i\", float val \"%f\" precision %i \n",text,tmpi,tmpf,precision);*/

	g_free(text);

	if ((tmpf != (gfloat)tmpi) && (precision == 0))
	{
		/* Pause signals while we change the value */
		g_signal_handlers_block_by_func(widget,*(void **)&insert_text_handler_f,data);
		g_signal_handlers_block_by_func(widget,*(void **)&std_entry_handler_f, data);
		g_signal_handlers_block_by_func(widget,*(void **)&entry_changed_handler_f, data);
		tmpbuf = g_strdup_printf("%i",tmpi);
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		g_free(tmpbuf);
		g_signal_handlers_unblock_by_func(widget,*(void **)&entry_changed_handler_f, data);
		g_signal_handlers_unblock_by_func(widget,*(void **)&std_entry_handler_f, data);
		g_signal_handlers_unblock_by_func(widget,*(void **)&insert_text_handler_f,data);
	}
	switch (handler)
	{
		case NOOP: /* Used for special widgets that do NOT refer to
			      direct ECU locations (LibreEMS BenchTest tool) */
			if (OBJ_GET(widget,"temp_dep"))
				value = temp_to_ecu_f(tmpf);
			else
				value = tmpf;
			dload_val = convert_before_download_f(widget,value);
			g_signal_handlers_block_by_func(widget,*(void **)&insert_text_handler_f,data);
			g_signal_handlers_block_by_func(widget,*(void **)& std_entry_handler_f, data);
			g_signal_handlers_block_by_func(widget,*(void **)& entry_changed_handler_f, data);
			tmpbuf = g_strdup_printf("%1$.*2$f",(gfloat)dload_val,precision);
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
			g_signal_handlers_unblock_by_func(widget,*(void **)& entry_changed_handler_f, data);
			g_signal_handlers_unblock_by_func(widget,*(void **)& std_entry_handler_f, data);
			g_signal_handlers_unblock_by_func(widget,*(void **)&insert_text_handler_f,data);
			break;

		case GENERIC:
			if (OBJ_GET(widget,"temp_dep"))
				value = temp_to_ecu_f(tmpf);
			else
				value = tmpf;
			dload_val = convert_before_download_f(widget,value);

			/* What we are doing is doing the forward/reverse 
			 * conversion which will give us an exact value 
			 * if the user inputs something in between,  thus 
			 * we can reset the display to a sane value...
			 */
			g_signal_handlers_block_by_func(widget,*(void **)&insert_text_handler_f,data);
			g_signal_handlers_block_by_func(widget,*(void **)&std_entry_handler_f, data);
			g_signal_handlers_block_by_func(widget,*(void **)&entry_changed_handler_f, data);
			old = get_ecu_data(widget);
			set_ecu_data(widget,&dload_val);

			real_value = convert_after_upload_f(widget);
			set_ecu_data(widget,&old);

			if (OBJ_GET(widget,"temp_dep"))
				value = temp_to_host_f(real_value);
			else
				value = real_value;
			tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
			gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
			g_free(tmpbuf);
			g_signal_handlers_unblock_by_func(widget,*(void **)&entry_changed_handler_f, data);
			g_signal_handlers_unblock_by_func(widget,*(void **)&std_entry_handler_f, data);
			g_signal_handlers_unblock_by_func(widget,*(void **)&insert_text_handler_f,data);
			break;
		default:
			/* We need to fall to ECU SPECIFIC entry handler for 
			   anything specific there */
			if (!ecu_handler)
			{
				if (get_symbol_f("ecu_entry_handler",(void **)&ecu_handler))
				{
					EXIT();
					return ecu_handler(widget,data);
				}
				else
					MTXDBG(CRITICAL,_("Default case, but there is NO ecu_entry_handler available, unhandled case for widget %s, BUG!\n"),glade_get_widget_name(widget));
			}
			else
			{
				EXIT();
				return ecu_handler(widget,data);
			}
			break;
	}
	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != get_ecu_data(widget))
		{
			/*printf("Value changed (%i != %i), update it\n",dload_val,get_ecu_data(widget));*/
			send_to_ecu(widget, dload_val, TRUE);
		}
		else
		{
			/*printf("not sent, returning!\n");*/
			OBJ_SET(widget,"not_sent",NULL);
			EXIT();
			return TRUE;
		}
	}
	OBJ_SET(widget,"not_sent",NULL);
	OBJ_SET(widget,"last_value",GINT_TO_POINTER(tmpi*1000));
	EXIT();
	return TRUE;
}


/*!
 \brief This ECU family specific function is called after a read of 
 the block of data from the ECU.  It takes care of updating evey 
 control that relates to an ECU variable on screen
 */
G_MODULE_EXPORT void update_ecu_controls_pf(void)
{
	static Firmware_Details *firmware = NULL;
	static GList ***ecu_widgets = NULL;
	gint page = 0;
	gint offset = 0;
	gint i = 0;

	ENTER();
	if (!ecu_widgets)
		ecu_widgets = (GList ***)DATA_GET(global_data,"ecu_widgets");
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	g_return_if_fail(firmware);
	g_return_if_fail(ecu_widgets);

	set_title_f(g_strdup(_("Updating Controls...")));

	for (i=0;i<firmware->total_tables;i++)
		recalc_table_limits_f(0,i);

	thread_update_widget_f("info_label",MTX_LABEL,g_strdup_printf(_("<b>Ready...</b>")));
    update_current_notebook_page_f();
	set_title_f(g_strdup(_("Ready...")));
	EXIT();
	return;
}


/*!
  \brief This function handles caling the appropriate update function
  based on the widget type.
  \param object is the pointer to the widget in question
  \param data is a pointer to verify we don't do duplicate updates and
  allows us to break out of a recursive loop
  */
G_MODULE_EXPORT void update_widget(gpointer object, gpointer data)
{
	static void (*insert_text_handler)(GtkEntry *, const gchar *, gint, gint *, gpointer);
	GtkWidget *widget = (GtkWidget *)object;
	gint tmpi = 0;
	gint last = 0;
	gdouble value = 0.0;

	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return;
	}
	if (!GTK_IS_WIDGET(widget))
	{
		EXIT();
		return;
	}
	if (!insert_text_handler)
		get_symbol_f("insert_text_handler",(void **)&insert_text_handler);

	g_return_if_fail(insert_text_handler);
	/* If passed widget and user data are identical,  break out as
	 * we already updated the widget.
	 */
	if ((GTK_IS_WIDGET(data)) && (widget == data))
	{
		EXIT();
		return;
	}

	/*printf("update_widget %s\n",(gchar *)glade_get_widget_name(widget));*/
	/* update widget whether spin,radio or checkbutton  
	 * (checkbutton encompases radio)
	 */
	value = convert_after_upload_f(widget);
	tmpi = value*1000;
	last = (GINT)OBJ_GET(widget,"last_value");
	/*printf("Old %i, new %i\n",last, tmpi);*/
	if ((tmpi == last) && (!DATA_GET(global_data,"force_update")) && (!OBJ_GET(widget,"temp_dep")))
	{
		/*printf("new and old match, exiting early....\n");*/
		EXIT();
		return;
	}
	else
	{
		/*printf("Values changed, updating..\n");*/
		OBJ_SET(widget,"last_value",GINT_TO_POINTER(tmpi));
	}

	if (GTK_IS_ENTRY(widget) || GTK_IS_SPIN_BUTTON(widget))
	{
		g_signal_handlers_block_by_func(widget,*(void **)&insert_text_handler_f,NULL);
		g_signal_handlers_block_by_func(widget,*(void **)&std_entry_handler_f,NULL);
		g_signal_handlers_block_by_func(widget,*(void **)&entry_changed_handler_f,NULL);
		update_entry(widget);
		g_signal_handlers_unblock_by_func(widget,*(void **)&entry_changed_handler_f,NULL);
		g_signal_handlers_unblock_by_func(widget,*(void **)&std_entry_handler_f,NULL);
		g_signal_handlers_unblock_by_func(widget,*(void **)&insert_text_handler_f,NULL);
	}
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
		 update_model_from_view(gtk_bin_get_child(GTK_BIN(widget)));
		 */
	}
	/* IF control has groups linked to it's state, adjust */
	EXIT();
	return;
}


/*!
  \brief updates a check/radio button
  \param widget is the pointer to the widget to update
  */
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

	ENTER();
	get_essential_bits(widget, NULL, NULL, &bitval, &bitmask, &bitshift);

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
		set_widget_labels_f(set_labels);
	if (OBJ_GET(widget,"swap_labels"))
		swap_labels_f(widget,new_state);
	/* Most likely MS specific 
	if ((new_state) && (OBJ_GET(widget,"group_2_update")))
		handle_group_2_update(widget);
	if (new_state)
		handle_algorithm(widget);
		*/
	if (OBJ_GET(widget,"toggle_groups"))
		combo_toggle_groups_linked_f(widget,new_state);
	EXIT();
	return;
}


/*!
  \brief updates a text entry
  \param widget is the pointer to the widget to update
  */
void update_entry(GtkWidget *widget)
{
	static void (*update_handler)(GtkWidget *) = NULL;
	static Firmware_Details *firmware = NULL;
	gboolean changed = FALSE;
	gboolean use_color = FALSE;
	DataSize size = MTX_U08;
	gint handler = -1;
	gchar * widget_text = NULL;
	gchar * tmpbuf = NULL;
	gfloat scaler = 0.0;
	gdouble value = 0.0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	gint precision = 0;
	GdkColor color;
	GdkColor black = {0,0,0,0};

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	g_return_if_fail(firmware);

	value = convert_after_upload_f(widget);
	handler = (GINT)OBJ_GET(widget,"handler");
	precision = (GINT)OBJ_GET(widget,"precision");

	/* Fringe case for module specific handlers */
	if (OBJ_GET(widget,"modspecific"))
	{
		if (!update_handler)
		{
			if (get_symbol_f("ecu_update_entry",(void **)&update_handler))
				update_handler(widget);
			else
				MTXDBG(CRITICAL,_("Default case, but there is NO ecu_update_entry function available, unhandled case for widget %s, BUG!\n"),glade_get_widget_name(widget));
		}
		else
			update_handler(widget);

	}
	if ((GBOOLEAN)OBJ_GET(widget,"temp_dep"))
		value = temp_to_host_f(value);
	if (GTK_IS_SPIN_BUTTON(widget))
	{
		gfloat spin_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
		if (value != spin_value)
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
	}
	else
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

	if (OBJ_GET(widget,"use_color"))
	{
		gint table_num = -1;
		if (OBJ_GET(widget,"table_num"))
			table_num = (GINT)strtol((gchar *)OBJ_GET(widget,"table_num"),NULL,10);

		if (table_num >= 0)
			update_entry_color_f(widget,table_num,TRUE,FALSE);
		else
			update_entry_color_f(widget,0,FALSE,((changed) || (value == 0)));
	}
	if (OBJ_GET(widget,"not_sent"))
	{
		gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
		OBJ_SET(widget,"not_sent",NULL);
	}
	EXIT();
	return;
}


/*!
  \brief updates a combo box/combo box entry
  \param widget is the pointer to the widget to update
  */
void update_combo(GtkWidget *widget)
{
	static void (*update_ms2_user_outputs)(GtkWidget *) = NULL;
	gint tmpi = -1;
	gint locID = 0;
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

	ENTER();
	get_essential_bits(widget,&locID, &offset, &bitval, &bitmask, &bitshift);
	/*printf("Combo at locID %i, offset %i, bitmask %i, bitshift %i, value %i\n",locID,offset,bitmask,bitshift,(GINT)value);*/
	value = convert_after_upload_f(widget);
	tmpi = ((GINT)value & bitmask) >> bitshift;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (!GTK_IS_TREE_MODEL(model))
		printf(_("ERROR no model for Combo at locID %i, offset %i, bitmask %i, bitshift %i, value %i\n"),locID,offset,bitmask,bitshift,(GINT)value);
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model),&iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model),&iter,BITVAL_COL,&t_bitval,-1);
		if (tmpi == t_bitval)
		{
			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
			gtk_widget_modify_base(gtk_bin_get_child(GTK_BIN(widget)),GTK_STATE_NORMAL,&white);
			/* Most likely MS specific 
			if (OBJ_GET(widget,"group_2_update"))
				combo_handle_group_2_update(widget);
			if (OBJ_GET(widget,"algorithms"))
				combo_handle_algorithms(widget);
				*/
			goto combo_toggle;
		}
		valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &iter);
		i++;

	}
	/*printf("COULD NOT FIND MATCH for data for combo %p, data %i!!\n",widget,tmpi);*/
	gtk_widget_modify_base(gtk_bin_get_child(GTK_BIN(widget)),GTK_STATE_NORMAL,&red);
	EXIT();
	return;

combo_toggle:
	if (OBJ_GET(widget,"toggle_labels"))
		combo_toggle_labels_linked_f(widget,i);
	if (OBJ_GET(widget,"toggle_groups"))
		combo_toggle_groups_linked_f(widget,i);
	if (OBJ_GET(widget,"swap_labels"))
		swap_labels_f(widget,tmpi);
	if (OBJ_GET(widget,"set_widgets_label"))
		combo_set_labels_f(widget,model);
	EXIT();
	return;
}


/*!
  \brief extracts theessential bits from a a widget pointer, This function 
  verifies the passed pointers are valid, so only specific values can be 
  requested if wanted
  \param widget is the pointer to the widget to get the essentions from
  \param locID is a pointer to where to store the Location ID or NULL
  \param offset is a pointer to where to store the Offset or NULL
  \param bitval is a pointer to where to store the bitval or NULL
  \param bitmask is a pointer to where to store the bitmask or NULL
  \param bitshift is a pointer to where to store the bitshift or NULL
  */
G_MODULE_EXPORT void get_essential_bits(GtkWidget *widget, gint *locID, gint *offset, gint *bitval, gint *bitmask, gint *bitshift)
{
	ENTER();
	if (!GTK_IS_WIDGET(widget))
	{
		EXIT();
		return;
	}
	if (locID)
		*locID = (GINT)OBJ_GET(widget,"location_id");
	if (offset)
		*offset = (GINT)OBJ_GET(widget,"offset");
	if (bitval)
		*bitval = (GINT)OBJ_GET(widget,"bitval");
	if (bitmask)
		*bitmask = (GINT)OBJ_GET(widget,"bitmask");
	if (bitshift)
		*bitshift = get_bitshift_f((GINT)OBJ_GET(widget,"bitmask"));
	EXIT();
	return;
}


/*!
  \brief extracts the essential bits from a a widget pointer, This function 
  verifies the passed pointers are valid, so only specific values can be 
  requested if wanted
  \param widget is the pointer to the widget to get the essentions from
  \param locID is a pointer to where to store the Location ID or NULL
  \param offset is a pointer to where to store the Offset or NULL
  \param size is a pointer to where to store the size or NULL
  \param precision is a pointer to where to store the precision or NULL
  */
G_MODULE_EXPORT void get_essentials(GtkWidget *widget, gint *locID, gint *offset, DataSize *size, gint *precision)
{
	ENTER();
	if (!GTK_IS_WIDGET(widget))
	{
		EXIT();
		return;
	}
	if (locID)
		*locID = (GINT)OBJ_GET(widget,"location_id");
	if (offset)
		*offset = (GINT)OBJ_GET(widget,"offset");
	if (size)
	{
		if (!OBJ_GET(widget,"size"))
			*size = MTX_U08 ;        /* default! */
		else
			*size = (DataSize)(GINT)OBJ_GET(widget,"size");
	}
	if (precision)
		*precision = (DataSize)(GINT)OBJ_GET(widget,"precision");
	EXIT();
	return;
}

