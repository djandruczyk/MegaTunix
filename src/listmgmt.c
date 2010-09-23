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
#include <defines.h>
#include <init.h>
#include <listmgmt.h>

static GHashTable *lists_hash = NULL;
extern GData *global_data;

/*!
 \brief get_list returns the list referenced by name
 \param key Text name of list to return a pointer to
 \returns pointer to GList
 \see store_list
 */
GList * get_list(gchar * key)
{
	if (!lists_hash)
	{
		lists_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		DATA_SET_FULL(&global_data,"lists_hash",lists_hash,dealloc_lists_hash);
	}
	return (GList *)g_hash_table_lookup(lists_hash,key);
}


/*!
 \brief store_list stores a list by a textual name
 \param key Text name of list to store
 \param list pointer to list to store
 \see get_list
 */
void store_list(gchar * key, GList * list)
{
	if (!lists_hash)
	{
		lists_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		DATA_SET_FULL(&global_data,"lists_hash",lists_hash,dealloc_lists_hash);
	}
	g_hash_table_replace(lists_hash,g_strdup(key),(gpointer)list);
	return;
}


/*!
 \brief remove_list removes a list from the hashtable
 \param key Text name of list to store
 \see get_list
 */
void remove_list(gchar *key)
{
	if (!lists_hash)
		return;
	g_hash_table_remove(lists_hash,key);
}


gint list_sort(gconstpointer a, gconstpointer b)
{
	ListElement *a1 = (ListElement *)a;
	ListElement *b1 = (ListElement *)b;
	return g_ascii_strcasecmp(a1->name,b1->name);
}


gint list_object_sort(gconstpointer a, gconstpointer b, gpointer data)
{
	const gchar *key = (const gchar *)data;
	return g_ascii_strcasecmp((gchar *)DATA_GET(a,key),(gchar *)DATA_GET(b,key));
}


void free_element(gpointer data, gpointer user_data)
{
	ListElement *a = (ListElement *)data;
	g_free(a->filename);
	g_free(a->name);
	g_free(a);
}

void simple_free_element(gpointer data, gpointer user_data)
{
	g_free((gchar *)data);
}
