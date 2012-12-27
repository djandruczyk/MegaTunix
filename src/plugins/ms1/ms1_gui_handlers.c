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
  \file src/plugins/ms1/ms1_gui_handlers.c
  \ingroup MS1Plugin,Plugins
  \brief MS1 specific Gui handler functions
  \author David Andruczyk
  */

#include <config.h>
#include <defines.h>
#include <glade/glade.h>
#include <ms1_plugin.h>
#include <ms1_gui_handlers.h>
#include <ms1_tlogger.h>
#include <firmware.h>
#include <gtk/gtk.h>
#include <stdlib.h>


extern gconstpointer *global_data;


/*!
  \brief ECU specific handler for text entries
  \param widget is the widget the user interacted with
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean ecu_entry_handler(GtkWidget *widget, gpointer data)
{
	static Firmware_Details *firmware = NULL;
	GdkColor black = { 0, 0, 0, 0};
	gfloat tmpf = 0.0;
	gfloat scaler = 0.0;
	gint handler = -1;
	gint offset = 0;
	gint tmpi = 0;
	gint tmp = 0;
	gint dload_val = 0;
	gint canID = 0;
	gint page = 0;
	gint dl_type = 0;
	gint table_num = -1;
	gint precision = 0;
	gint raw_lower = 0;
	gint raw_upper = 0;
	gchar *text = NULL;
	gchar *tmpbuf = NULL;
	GdkColor color;
	gboolean use_color = FALSE;
	DataSize size = MTX_U08;

	ENTER();
	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");
	handler = (MS1StdHandler)(GINT)OBJ_GET(widget,"handler");

	get_essentials_f(widget,&canID,&page,&offset,&size,&precision);

	text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
	tmpi = (GINT)strtol(text,NULL,10);
	tmpf = (gfloat)g_ascii_strtod(g_strdelimit(text,",.",'.'),NULL);
	g_free(text);
	if ((tmpf != (gfloat)tmpi) && (precision == 0))
	{
		/* Pause signals while we change the value */
		/*              printf("resetting\n");*/
		g_signal_handlers_block_by_func (widget,*(void **)(&std_entry_handler_f), data);
		g_signal_handlers_block_by_func (widget,*(void **)(&entry_changed_handler_f), data);
		tmpbuf = g_strdup_printf("%i",tmpi);
		gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
		g_free(tmpbuf);
		g_signal_handlers_unblock_by_func (widget,*(void **)(&entry_changed_handler_f), data);
		g_signal_handlers_unblock_by_func (widget,*(void **)(&std_entry_handler_f), data);
	}
	switch ((MS1StdHandler)handler)
	{ 
		case TRIGGER_ANGLE:
			offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			if (offset == 0)
			{
				MTXDBG(CRITICAL,_("ERROR Trigger Angle entry call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;
				break;

			}
			if (tmpf > 112.15)      /* Extra long trigger needed */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);   /* Set xlong_trig */
				ms_send_to_ecu_f(canID, page, offset, size, tmp,  TRUE);
				tmpf -= 45.0;
				dload_val = convert_before_download_f(widget,tmpf);
			}
			else if (tmpf > 89.65) /* Long trigger needed */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);;
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);   /* Set long_trig */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				tmpf -= 22.5;
				dload_val = convert_before_download_f(widget,tmpf);
			}
			else    /* tmpf <= 89.65 degrees, no long trigger*/
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				dload_val = convert_before_download_f(widget,tmpf);
			}
			break;

		case ODDFIRE_ANGLE:
			offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (offset == 0)
			{
				MTXDBG(CRITICAL,_("ERROR Offset Angle entry changed call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;
				break;

			}
			if (tmpf > 90)  /*  */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);   /* Set +90 */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				tmpf -= 90.0;
				dload_val = convert_before_download_f(widget,tmpf);
			}
			else if (tmpf > 45) /* */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);   /* Set +45 */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				tmpf -= 45.0;
				dload_val = convert_before_download_f(widget,tmpf);
			}
			else    /* tmpf <= 45 degrees, */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				ms_send_to_ecu_f(canID, page, offset, size, tmp,  TRUE);
				dload_val = convert_before_download_f(widget,tmpf);
			}

			break;
		default:
			MTXDBG(CRITICAL,_("ERROR  handler (%i) NOT found for widget %s, command aborted! BUG!!!\n"),handler,glade_get_widget_name(widget));
			break;

	}
	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != ms_get_ecu_data_f(canID,page,offset,size))
		{
			/* special case for the ODD MS-1 variants and the very rare 16 bit variables */
			if ((firmware->capabilities & MS1) && ((size == MTX_U16) || (size == MTX_S16)))
			{
				ms_send_to_ecu_f(canID, page, offset, MTX_U08, (dload_val &0xff00) >> 8, TRUE);
				ms_send_to_ecu_f(canID, page, offset+1, MTX_U08, (dload_val &0x00ff), TRUE);
			}
			else
				ms_send_to_ecu_f(canID, page, offset, size, dload_val, TRUE);
		}
	}

	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	OBJ_SET(widget,"not_sent",NULL);
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific handler for spin buttons
  \param widget is the widget the user interacted with
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean ecu_spin_button_handler(GtkWidget *widget, gpointer data)
{
	static Firmware_Details *firmware = NULL;
	gint dl_type = -1;
	gint offset = -1;
	gint dload_val = -1;
	gint canID = 0;
	gint page = -1;
	DataSize size = MTX_U08;
	gint bitmask = -1;
	gint bitshift = -1;
	gint tmpi = 0;
	gint tmp = 0;
	gint handler = -1;
	gint divider_offset = 0;
	gint table_num = -1;
	gint source = 0;
	gfloat value = 0.0;
	GtkWidget * tmpwidget = NULL;
	Deferred_Data *d_data = NULL;
	GHashTable **interdep_vars = NULL;
	GdkColor black = {0,0,0,0};

	ENTER();

	if (!firmware)
		firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");

	handler = (MS1StdHandler)(GINT)OBJ_GET(widget,"handler");
	dl_type = (GINT) OBJ_GET(widget,"dl_type");

	get_essential_bits_f(widget,&canID,&page,&offset,NULL,&bitmask,&bitshift);

	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);

	tmpi = (int)(value+.001);

	switch ((MS1StdHandler)handler)
	{
		case TRIGGER_ANGLE:
			offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			if (offset == 0)
			{
				MTXDBG(CRITICAL,_("ERROR Trigger Angle spinbutton call, but spconfig_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;
				break;

			}
			if (value > 112.15)     /* Extra long trigger needed */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 1);   /* Set xlong_trig */
				ms_send_to_ecu_f(canID, page, offset, size, tmp,  TRUE);
				value -= 45.0;
				dload_val = convert_before_download_f(widget,value);
			}
			else if (value > 89.65) /* Long trigger needed */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				tmp = tmp | (1 << 0);   /* Set long_trig */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				value -= 22.5;
				dload_val = convert_before_download_f(widget,value);
			}
			else    /* value <= 89.65 degrees, no long trigger*/
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x3; /*clears lower 2 bits */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				dload_val = convert_before_download_f(widget,value);
			}

			break;
		case ODDFIRE_ANGLE:
			offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (offset == 0)
			{
				MTXDBG(CRITICAL,("ERROR Offset Angle spinbutton call, but oddfire_bit_offset variable is unset, Aborting handler!!!\n"));
				dl_type = 0;
				break;

			}
			if (value > 90) /*  */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 2);   /* Set +90 */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				value -= 90.0;
				dload_val = convert_before_download_f(widget,value);
			}
			else if (value > 45) /* */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				tmp = tmp | (1 << 1);   /* Set +45 */
				ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
				value -= 45.0;
				dload_val = convert_before_download_f(widget,value);
			}
			else    /* value <= 45 degrees, */
			{
				tmp = ms_get_ecu_data_f(canID,page,offset,size);
				tmp = tmp & ~0x7; /*clears lower 3 bits */
				ms_send_to_ecu_f(canID, page, offset, size, tmp,  TRUE);
				dload_val = convert_before_download_f(widget,value);
			}

			break;
		default:
			MTXDBG(CRITICAL,_("ERROR  handler (%i) NOT found for widget %s, command aborted! BUG!!!\n"),handler,glade_get_widget_name(widget));

			break;
	}
	if (dl_type == IMMEDIATE)
	{
		/* If data has NOT changed,  don't bother updating 
		 * and wasting time.
		 */
		if (dload_val != ms_get_ecu_data_f(canID,page,offset,size))
			ms_send_to_ecu_f(canID, page, offset, size, dload_val, TRUE);
	}
	gtk_widget_modify_text(widget,GTK_STATE_NORMAL,&black);
	EXIT();
	return TRUE;
}



/*!
  \brief ECU specific handler for std buttons
  \param widget is the widget the user interacted with
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean ecu_std_button_handler(GtkWidget *widget, gpointer data)
{
	gint handler = -1;
	gboolean restart = FALSE;

	ENTER();
	handler = (GINT)OBJ_GET(widget,"handler");

	switch ((MS1StdHandler)handler)
	{
		case REBOOT_GETERR:
			if (DATA_GET(global_data,"offline"))
				break;
			if (DATA_GET(global_data,"realtime_id"))
			{
				stop_tickler_f(RTV_TICKLER);
				restart = TRUE;
			}
			gtk_widget_set_sensitive(widget,FALSE);
			io_cmd_f("ms1_extra_reboot_get_error",NULL);
			if (restart)
				start_tickler_f(RTV_TICKLER);
			break;
		default:
			MTXDBG(CRITICAL,_("ERROR  handler (%i) NOT found for widget %s, command aborted! BUG!!!\n"),handler,glade_get_widget_name(widget));
			break;
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific handler for toggle buttons
  \param widget is the widget the user interacted with
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean ecu_toggle_button_handler(GtkWidget *widget, gpointer data)
{
	GtkWidget *tmpwidget = NULL;
	gint handler = (GINT)OBJ_GET(widget, "handler");

	ENTER();
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{       /* It's pressed (or checked) */
		switch ((MS1ToggleHandler)handler)
		{

			case START_TOOTHMON_LOGGER:
				tmpwidget = lookup_widget_f("triggerlogger_buttons_table");
				if (GTK_IS_WIDGET(tmpwidget))
					gtk_widget_set_sensitive(GTK_WIDGET(tmpwidget),FALSE);
				bind_ttm_to_page((GINT)OBJ_GET(widget,"page"));
				start(TOOTHMON_TICKLER);
				break;
			case START_TRIGMON_LOGGER:
				tmpwidget = lookup_widget_f("toothlogger_buttons_table");
				if (GTK_IS_WIDGET(tmpwidget))
					gtk_widget_set_sensitive(GTK_WIDGET(tmpwidget),FALSE);
				bind_ttm_to_page((GINT)OBJ_GET(widget,"page"));
				start(TRIGMON_TICKLER);
				break;
			case STOP_TOOTHMON_LOGGER:
				stop(TOOTHMON_TICKLER);
				tmpwidget = lookup_widget_f("triggerlogger_buttons_table");
				if (GTK_IS_WIDGET(tmpwidget))
					alter_widget_state_f(tmpwidget,NULL);
				break;
			case STOP_TRIGMON_LOGGER:
				stop(TRIGMON_TICKLER);
				tmpwidget = lookup_widget_f("toothlogger_buttons_table");
				if (GTK_IS_WIDGET(tmpwidget))
					alter_widget_state_f(tmpwidget,NULL);
				break;
			default:
				MTXDBG(CRITICAL,_("ERROR  handler (%i) NOT found for widget %s, command aborted! BUG!!!\n"),handler,glade_get_widget_name(widget));
				break;
		}
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific handler for combo boxes
  \param widget is the widget the user interacted with
  \param data is unused
  \returns TRUE on success, FALSE otherwise
  */
G_MODULE_EXPORT gboolean ecu_combo_handler(GtkWidget *widget, gpointer data)
{
	gint handler = -1;
	gint dl_type = 0;
	gint tmp = -1;
	gint table_num = -1;
	gint offset = -1;
	gint page = -1;
	DataSize size = MTX_U08;
	gint canID = 0;
	gint bitmask = -1;
	gint bitshift = -1;
	gint bitval = 0;
	gint dload_val = 0;
	Deferred_Data *d_data = NULL;
	GHashTable **interdep_vars = NULL;
	Firmware_Details *firmware = NULL;
	void (*check_limits)(gint);

	ENTER();
	firmware = (Firmware_Details *)DATA_GET(global_data,"firmware");
	handler = (GINT)OBJ_GET(widget,"handler");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	if (!OBJ_GET(widget,"size"))
		size = MTX_U08 ;        /* default! */
	else
		size = (DataSize)(GINT)OBJ_GET(widget,"size");

	get_essential_bits_f(widget,&canID,&page,&offset,&bitval,&bitmask,&bitshift);

	switch (handler)
	{
		default:
			MTXDBG(CRITICAL,("ERROR  handler (%i) NOT found for widget %s, command aborted! BUG!!!\n"),handler,glade_get_widget_name(widget));
			EXIT();
			return TRUE;
			break;
	}
	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download_f(widget,dload_val);
		ms_send_to_ecu_f(canID, page, offset, size, dload_val, TRUE);
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific handler to update a text entry
  \param widget is the widget the user interacted with
  */
G_MODULE_EXPORT void ecu_update_entry(GtkWidget *widget)
{
	gint canID = 0;
	gint page = 0;
	gint offset = 0;
	DataSize size = MTX_U08;
	gint handler = -1;
	gfloat value = 0.0;
	gint precision = 0;
	gchar *tmpbuf = NULL;

	ENTER();
	handler = (GINT)OBJ_GET(widget,"handler");
	value = convert_after_upload_f(widget);
	get_essentials_f(widget, &canID, &page, &offset, &size, &precision);

	switch ((MS1StdHandler)handler)
	{
		case TRIGGER_ANGLE:
			offset = (GINT)OBJ_GET(widget,"spconfig_offset");
			switch ((ms_get_ecu_data_f(canID,page,offset,size) & 0x03))
			{
				case 2:
					value += 45.0;
					break;
				case 1:
					value += 22.5;
					break;
				case 0:
					value += 0;
					break;
				default:
					MTXDBG(CRITICAL,_("TRIGGER_ANGLE_UPDATE invalid value for spconfig_offset at ecu_data[%i][%i], ERROR\n"),(GINT)OBJ_GET(widget,"page"),(GINT)OBJ_GET(widget,"offset"));

			}
			if (GTK_IS_ENTRY(widget))
			{
				tmpbuf = g_strdup_printf("%1$.*2$f",value,precision);
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				g_free(tmpbuf);
			}
			else
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);
			break;
		case ODDFIRE_ANGLE:
			offset = (GINT)OBJ_GET(widget,"oddfire_bit_offset");
			if (offset == 0)
			{
				EXIT();
				return;
			}
			switch ((GINT)ms_get_ecu_data_f(canID,page,offset,size))
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
					MTXDBG(CRITICAL,_("ODDFIRE_ANGLE_UPDATE invalid value for oddfire_offset at ecu_data[%i][%i], ERROR\n"),(GINT)OBJ_GET(widget,"page"),(GINT)OBJ_GET(widget,"offset"));

			}
			if (GTK_IS_ENTRY(widget))
			{
				tmpbuf = g_strdup_printf("%1$.*2$f",value+90,precision);
				gtk_entry_set_text(GTK_ENTRY(widget),tmpbuf);
				g_free(tmpbuf);
			}
			else
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),value);

			break;

		default:
			MTXDBG(CRITICAL,_("ERROR handler (%i) NOT found for widget %s, command aborted! BUG!!!\n"),handler,glade_get_widget_name(widget));
	}
	EXIT();
	return;
}


/*!
  \brief ECU specific combo update routine.  This is called from the common
  routine to handle any firmware specific stuff related to a ComboBoxEntry
  \param widget is the pointer to the GtkComboBox widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_update_combo(GtkWidget * widget)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific handler to initialize any part of the core gui
  that is specific to this ECU firmware
  */
G_MODULE_EXPORT void ecu_gui_init(void)
{
	ENTER();
	EXIT();
	return;
	/* We don't need anything specific to this ecu initialized */
}


