/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file include/combo_mask.h
  \ingroup Headers
  \brief Header for the Mtx Specific combobox regex mask loader/initializer
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __COMBO_MASK_H__
#define __COMBO_MASK_H__

#include <configfile.h>
#include <gtk/gtk.h>

/* A simple validating entry */

G_BEGIN_DECLS

#define TYPE_MASK_ENTRY             (mask_entry_get_type ())
#define MASK_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MASK_ENTRY, MaskEntry))
#define MASK_ENTRY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TYPE_MASK_ENTRY, MaskEntryClass))
#define IS_MASK_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MASK_ENTRY))
#define IS_MASK_ENTRY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TYPE_MASK_ENTRY))
#define MASK_ENTRY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TYPE_MASK_ENTRY, MaskEntryClass))


typedef struct _MaskEntry MaskEntry;
/*!
  \brief MaskEntry structure
  */
struct _MaskEntry
{
	GtkEntry entry;			/*!< GtkEntry this mask goes with */
	gchar *mask;			/*!< The actual mask */
};

typedef struct _MaskEntryClass MaskEntryClass;
/*!
  \brief MaskEntryClass structure
  */
struct _MaskEntryClass
{
	GtkEntryClass parent_class;	/*!< Parent Class */
};

/* Prototypes */
void mask_entry_editable_init (GtkEditableClass *iface);
void mask_entry_finalize(GObject *);
GtkWidget *mask_entry_new (void);
GtkWidget *mask_entry_new_with_mask (gchar *mask);
/* Prototypes */

G_END_DECLS

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
