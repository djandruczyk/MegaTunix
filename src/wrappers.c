/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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


G_MODULE_EXPORT void * evaluator_create_w(char *expr)
{
	return evaluator_create(expr);
}

G_MODULE_EXPORT void evaluator_destroy_w( void *eval)
{
	evaluator_destroy(eval);
}

G_MODULE_EXPORT double evaluator_evaluate_x_w(void * eval, double x)
{
	return evaluator_evaluate_x(eval,x);
}

G_MODULE_EXPORT GtkWidget *mask_entry_new_with_mask_w(gchar *mask)
{
	return mask_entry_new_with_mask(mask);
}
