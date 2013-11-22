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
  \file include/xmlcomm.h
  \ingroup Headers
  \brief Header for the load/management of the per personality comm.xml data
  \author David Andruczyk
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __XMLCOMM_H__
#define __XMLCOMM_H__

#include <defines.h>
#include <enums.h>
#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


typedef struct _PotentialArg PotentialArg;
typedef struct _Command Command;
typedef struct _PostFunction PostFunction;
typedef struct _DBlock DBlock;

/*!
 * \brief _PotentialArgs struct holds information read from the 
 * communications XML, one PotentialArg per command
 */
struct _PotentialArg
{
	gchar *name;		/*!< Potential arg name */
	gchar *desc;		/*!< Description */
	gchar *internal_name;	/*!< Internal name used for linking */
	DataSize size;		/*!< Size of data */
	ArgType type;		/*!< Enumerated type */
	Action action;		/*!< Enum action */
	gint action_arg;	/*!< Action argument */
	gchar *static_string;	/*!< Static string */
	gint string_len;	/*!< Static String Length */
};


/*!
 * \brief _Command struct holds information read from the 
 * communications XML, describing each possible command. Their names
 * are used as internal keys to link firmware stuf to the XML definitions.
 */
struct _Command
{
	gboolean dynamic;	/*!< Hacky flag to know if this is dyn allocated*/
	gchar *name;		/*!< Command Name */
	gchar *desc;		/*!< Command Description */
	gchar *base;		/*!< Base command charactor(s) */
	CmdType type;		/*!< Command type enumeration */
	GArray *args;		/*!< Argument list array of PotentialArgs */
	GArray *post_functions;	/*!< Argument list array of PostFunctions */
	gchar *helper_func_name;/*!< Return data function name */
	gboolean defer_post_functions;	/*!< deferred post functions */
	gint helper_func_arg;	/*!< Return data arg (ENUM) */
	void  (*helper_function) (void *, gint);	/*!< Helper Function Pointer */
	gchar *func_call_name;	/*!< FUNC_CALL function name */
	gint func_call_arg;	/*!< Enum arg to function call */
	gboolean (*function) (void *, gint); /*!< Function call pointer */
};


/*!
 * \brief _PostFunction struct holds information regard post functions to be
 * fired.
 */
struct _PostFunction
{
	gchar *name;		/*!< Function name */
	void (*function) (void); /*!< Pointer to function */
	void (*function_w_arg) (void *); /*!< Pointer to function */
	gboolean w_arg;		/*!< Does it take an arg (void *) */
};

/*!
 * \brief _DBlock is a container for a data block
 */
struct _DBlock
{
	guint8 *data;		/*!< String of data */
	gint len;		/*!< length of data */
	ArgType type;		/*!< Enumerated type */
	Action action;		/*!< Enum action */
	gint arg;		/*!< Action argument */

};
/* Prototypes */
void load_arg_details(PotentialArg *, xmlNode *);
void load_cmd_args(Command *, xmlNode *);
void load_cmd_details(Command *, xmlNode *);
void load_cmd_post_functions(Command *, xmlNode *);
void load_comm_xml(gchar *);
void load_commands(GHashTable *, xmlNode *);
void load_potential_args(GHashTable *, xmlNode *);
void load_xmlcomm_elements(xmlNode *);
void parse_hex_string(gchar *, gchar *, gint *);
void xmlcomm_dump_commands(gpointer, gpointer, gpointer);
/* Prototypes */

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
