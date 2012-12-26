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
  \file src/wrappers.c
  \ingroup CoreMtx
  \brief wrapper functions to fix lib/symbol issues on Windows OS's
  \author David Andruczyk
  */

#include <combo_mask.h>
#include <debugging.h>
#include <gauge.h>
#include <mtxmatheval.h>
#include <multi_expr_loader.h>

/*!
  \brief wrapper for evaluator_create()
  \param expr is the math expression to create an evaluator for
  \returns pointer to opaque pointer representing the evaluator
  */
G_MODULE_EXPORT void * evaluator_create_w(char *expr)
{
	ENTER();
	EXIT();
	return evaluator_create(expr);
}


/*!
  \brief wrapper for evaluator_destroy()
  \param eval is the evaluator to destroy....
  */
G_MODULE_EXPORT void evaluator_destroy_w( void *eval)
{
	evaluator_destroy(eval);
}


/*!
  \brief wrapper for evaluator_evaluator_x()
  \param eval is the evaluator to evaluate
  \param x is the input to the math
  \returns result of the math
  */
G_MODULE_EXPORT double evaluator_evaluate_x_w(void * eval, double x)
{
	ENTER();
	EXIT();
	return evaluator_evaluate_x(eval,x);
}


/*!
  \brief wrapper for mask_entry_new_with_mask()
  \param mask is the regex mask for this new entry
  \returns pointer to the new widget
  */
G_MODULE_EXPORT GtkWidget *mask_entry_new_with_mask_w(gchar *mask)
{
	ENTER();
	EXIT();
	return mask_entry_new_with_mask(mask);
}
