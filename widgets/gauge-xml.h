/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * 
 * Megasquirt gauge widget XML I/O header
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 * This is a the PRIVATE implementation header file for INTERNAL functions
 * of the widget.  Public functions as well the the gauge structure are 
 * defined in the gauge.h header file
 *
 */

#ifndef __GAUGE_XML_H__
#define  __GAUGE_XML_H__

#include <gtk/gtk.h>
#include <gauge.h>
#include <libxml/parser.h>
#include <libxml/tree.h>



/* Prototypes */
void testload(GtkWidget *);
void output_xml(GtkWidget * );
void mtx_gauge_color_range_export(MtxDispatchHelper *);
void mtx_gauge_color_export(MtxDispatchHelper *);
void mtx_gauge_text_block_export(MtxDispatchHelper *);
void mtx_gauge_tick_group_export(MtxDispatchHelper *);
void mtx_gauge_gfloat_export(MtxDispatchHelper *);
void mtx_gauge_gint_export(MtxDispatchHelper *);
void mtx_gauge_gchar_export(MtxDispatchHelper *);
void mtx_gauge_color_range_import(MtxGaugeFace *, xmlNode *, gpointer);
void mtx_gauge_text_block_import(MtxGaugeFace *, xmlNode *, gpointer);
void mtx_gauge_tick_group_import(MtxGaugeFace *, xmlNode *, gpointer);
void mtx_gauge_color_import(MtxGaugeFace *, xmlNode *, gpointer);
void mtx_gauge_gfloat_import(MtxGaugeFace *, xmlNode *, gpointer);
void mtx_gauge_gint_import(MtxGaugeFace *, xmlNode *, gpointer);
void mtx_gauge_gchar_import(MtxGaugeFace *, xmlNode *, gpointer);

/** Structure used to streamline the xml import/export */
static const struct
{
	void (*import_func) (MtxGaugeFace *, xmlNode *, gpointer);
	void (*export_func) (MtxDispatchHelper *);
	gchar * varname;
} xml_functions[] = {
	{ mtx_gauge_color_import, mtx_gauge_color_export,"bg_color"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"needle_color"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_tail"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"needle_width"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"majtick_color"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"mintick_color"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"value_font_color"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"major_tick_text_color"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"gradient_begin_color"},
	{ mtx_gauge_color_import, mtx_gauge_color_export,"gradient_end_color"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"precision"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"width"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"height"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"start_deg"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"stop_deg"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"start_radian"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"stop_radian"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"lbound"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"ubound"},
	{ mtx_gauge_gchar_import, mtx_gauge_gchar_export,"value_font"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"value_font_scale"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"value_str_xpos"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"value_str_ypos"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"antialias"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"show_value"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"tick_inset"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"major_ticks"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"major_tick_len"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"major_tick_width"},
	{ mtx_gauge_gchar_import, mtx_gauge_gchar_export,"major_tick_text_font"},
	{ mtx_gauge_gchar_import, mtx_gauge_gchar_export,"major_tick_text"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"major_tick_text_inset"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"major_tick_text_scale"},
	{ mtx_gauge_gint_import, mtx_gauge_gint_export,"minor_ticks"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"minor_tick_len"},
	{ mtx_gauge_gfloat_import, mtx_gauge_gfloat_export,"minor_tick_width"},
	{ mtx_gauge_text_block_import, mtx_gauge_text_block_export,"text_block"},
	{ mtx_gauge_tick_group_import, mtx_gauge_tick_group_export,"tick_group"},
	{ mtx_gauge_color_range_import, mtx_gauge_color_range_export,"color_range"},
};


#endif
