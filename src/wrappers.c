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
#include <crx.h>
#include <combo_mask.h>
#include <debugging.h>
#include <defines.h>
#include <enums.h>
#include <gauge.h>
#include <gtk/gtk.h>
#include <wrappers.h>
#include <mtxmatheval.h>


G_MODULE_EXPORT void * eval_create(char *expr)
{
	return evaluator_create(expr);
}

G_MODULE_EXPORT void eval_destroy( void *eval)
{
	evaluator_destroy(eval);
}

G_MODULE_EXPORT double eval_x(void * eval, double x)
{
	return evaluator_evaluate_x(eval,x);
}

//G_MODULE_EXPORT char * regex_wrapper(char *pattern, char *string, int *len)
//{
//	return regex(pattern,string,len);
//}

G_MODULE_EXPORT GtkWidget *mask_entry_new_with_mask_wrapper(gchar *mask)
{
	return mask_entry_new_with_mask(mask);
}

G_MODULE_EXPORT GtkWidget *mtx_gauge_face_new_wrapper(void)
{
	return mtx_gauge_face_new();
}

G_MODULE_EXPORT void mtx_gauge_face_import_xml_wrapper(GtkWidget *gauge, gchar *xml)
{
	mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),xml);
	return;
}

G_MODULE_EXPORT void mtx_gauge_face_set_value_wrapper(GtkWidget *gauge, gfloat value)
{
	mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),value);
	return;
}

