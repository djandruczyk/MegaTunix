/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * Most of this file contributed by Perry Harrington
 * slight changes applied (naming, addition ofbspot 1-3 vars)
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <configfile.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <errno.h>
#include <interrogate.h>
#include <mode_select.h>
#include <notifications.h>
#include <serialio.h>
#include <stdlib.h>
#include <structures.h>
#include <string.h>
#include <stringmatch.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>

extern unsigned int ecu_caps;
extern gboolean connected;
extern GtkWidget *ms_ecu_revision_entry;
extern GtkTextBuffer *textbuffer;
extern GtkWidget *interr_view;
extern struct Serial_Params *serial_params;
extern struct DynamicEntries entries;
gboolean interrogated = FALSE;

void interrogate_ecu()
{
	/* As of 10/26/2003 several MegaSquirt variants have arrived, the 
	 * major ones being the DualTable Code, MegaSquirtnSpark, and
	 * MegaSquirtnEDIS.  MegaTunix attempts to talk to all of them
	 * and it is this functions job to try and determine which unit 
	 * we are talking to.  The Std Version number query "Q" cannot be 
	 * relied upon as severl of the forks use the same damn number.
	 * So we use the approach of querying which commands respond to 
	 * try and get as close as possible.
	 */
	struct pollfd ufds;
	struct Command *cmd = NULL;
	struct Canidate *canidate = NULL;
	gint size = 1024;
	gint res = 0;
	gint count = 0;
	gint i = 0;
	gint total = 0;
	gint last_page = 0;
	gint tests_to_run = 0;
	gchar *string = NULL;
	GArray *cmd_array = NULL;
	unsigned char buf[size];
	unsigned char *ptr = buf;
	static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

	/* prevent multiple runs of interrogator simultaneously */
	g_static_mutex_lock(&mutex);

	if (!connected)
	{
		gdk_threads_enter();
		no_ms_connection();
		gdk_threads_leave();
		g_static_mutex_unlock(&mutex);
		return;
	}
	
	/* Allocate memory to store interrogation results */
	canidate = g_malloc0(sizeof(struct Canidate));
	canidate->bytecounts = g_hash_table_new(g_str_hash,g_str_equal);

	/* Build table of strings to enum values */
	build_string_2_enum_table();
	/* Load tests from config files */
	cmd_array = validate_and_load_tests();
	/* how many tests.... */
	tests_to_run = cmd_array->len;

	/* Configure port for polled IO and flush I/O buffer */
	ufds.fd = serial_params->fd;
	ufds.events = POLLIN;
	tcflush(serial_params->fd, TCIOFLUSH);

	for (i=0;i<tests_to_run;i++)
	{
		count = 0;
		total = 0;
		cmd = g_array_index(cmd_array,struct Command *, i);

		/* flush buffer to known state.. */
		memset (buf,0,size);

		ptr = buf;
		/* set page */
		if (last_page != cmd->page)
			set_ms_page(cmd->page);
		last_page = cmd->page;

		string = g_strdup(cmd->string);
		res = write(serial_params->fd,string,cmd->len);
		dbg_func(g_strdup_printf(__FILE__": interrogate_ecu() sent command \"%s\"\n",string),INTERROGATOR);
		g_free(string);
		if (res != cmd->len)
			dbg_func(__FILE__": Error writing data to the ECU\n",CRITICAL);
		res = poll (&ufds,1,25);
		if (res)
		{	
			while (poll(&ufds,1,25))
			{
				total += count = read(serial_params->fd,ptr+total,64);
			}
			dbg_func(g_strdup_printf(__FILE__": interrogate_ecu() received %i bytes\n",total),INTERROGATOR);
			ptr = buf;

			/* copy data from tmp buffer to struct pointer */
			switch (cmd->store_type)
			{
				case SIG:
					canidate->sig_str = g_strndup(buf,total);
					break;
				case EXTVER:
					canidate->quest_str = g_strndup(buf,total);
					break;
				case VNUM:
					memcpy(&(canidate->ver_num),buf,total);
					break;
				default:
					break;
			}
		}

		/* store number of bytes received in counter variable */
		g_hash_table_insert(canidate->bytecounts,cmd->handle,
				GINT_TO_POINTER(total));
	}
	/* Reset page to 0 just to be 100% sure... */
	set_ms_page(0);
	/* flush serial port */
	tcflush(serial_params->fd, TCIOFLUSH);

	interrogated = TRUE;

	determine_ecu(canidate,cmd_array);	

	if (canidate)
		g_free(canidate);

	free_test_commands(cmd_array);

	g_static_mutex_unlock(&mutex);

	return;
}

void determine_ecu(void *ptr, GArray *cmd_array)
{
	struct Canidate *canidate = (struct Canidate *)ptr;
	struct Canidate *potential = NULL;
	struct Command *cmd = NULL;
	gint i = 0;
	gint j = 0;
	gint cbytes = 0;
	gint pbytes = 0;
	gint num_tests = cmd_array->len;
	gint passcount = 0;
	gint match = -1;
	gchar * tmpbuf = NULL;
	GDir *dir;
	GError *error;
	gchar * path;
	gchar * filename = NULL;

	path = g_strconcat(DATA_DIR,"/",INTERROGATOR_DIR,"/Profiles/",NULL);
	dir = g_dir_open(path,0,&error);	
	filename = (gchar *)g_dir_read_name(dir);
	while (filename != NULL)
	{
		potential = load_profile(cmd_array,filename);

		passcount = 0;
		for (j=0;j<num_tests;j++)
		{
			cmd = g_array_index(cmd_array,struct Command *, j);
			cbytes = (gint)g_hash_table_lookup(
					canidate->bytecounts,
					cmd->handle);
			pbytes = (gint)g_hash_table_lookup(
					potential->bytecounts,
					cmd->handle);
			dbg_func(g_strdup_printf(__FILE__": determine_ecu() Test %s, canidate bytes %i, potential %i\n",cmd->string,cbytes,pbytes),INTERROGATOR);
			if (cbytes != pbytes)
			{
				// Mismatch, abort test and move on to the 
				// next one...
				dbg_func(g_strdup_printf(__FILE__": determine_ecu(), counts don't match, loading next profile\n"),INTERROGATOR);
				close_profile(potential);
				goto end_of_loop; 
			}
		}
		dbg_func(g_strdup_printf(__FILE__": determine_ecu() all bytecount tests passed for firmware %s\n",potential->firmware_name),INTERROGATOR);
		/* If all test pass, now check the Extended version
		 * If it matches,  jump out...
		 */
		if ((potential->quest_str != NULL) && (canidate->quest_str != NULL))
		{
			dbg_func(g_strdup_printf(__FILE__": determine_ecu() testing ext version, canidate %s, potential %s\n",canidate->quest_str,potential->quest_str),INTERROGATOR);
			if (strstr(canidate->quest_str,potential->quest_str) != NULL)
			{
				dbg_func(__FILE__": determine_ecu() found match on ext version\n",INTERROGATOR);
				match = i;
				break;
			}
		}
		else if ((potential->sig_str != NULL) && (canidate->sig_str != NULL))
		{
			dbg_func(g_strdup_printf(__FILE__": determine_ecu() testing signature, canidate %s, potential %s\n",canidate->sig_str,potential->sig_str),INTERROGATOR);
			if (strstr(canidate->sig_str,potential->sig_str) != NULL)
			{
				dbg_func(__FILE__": determine_ecu() found match on signature\n",INTERROGATOR);
				match = i;
				break;
			}
		}
		else
		{
			dbg_func(g_strdup_printf(__FILE__": determine_ecu() found match on bytecounts alone...\n"),INTERROGATOR);
			match = i;
			break;
		}
end_of_loop:
		i++;
		filename =(gchar *) g_dir_read_name(dir);
	}
	/* Update the screen with the data... */
	for (i=0;i<num_tests;i++)
	{
		cmd = g_array_index(cmd_array,struct Command *,i);
		tmpbuf = g_strdup_printf("Command \"%s\" (%s), returned %i bytes\n",
				cmd->string, 
				cmd->desc, 
				(gint) g_hash_table_lookup(
							   canidate->bytecounts, 
							   cmd->handle));
		// Store counts for VE/realtime readback... 
		dbg_func(tmpbuf,INTERROGATOR);
		gdk_threads_enter();
		update_logbar(interr_view,NULL,tmpbuf,FALSE,FALSE);
		gdk_threads_leave();
		g_free(tmpbuf);
		if (cmd->store_type == VNUM)
		{
			if (canidate->ver_num == 0)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strdup_printf("%.1f",
						((float)canidate->ver_num/10.0));
			gdk_threads_enter();
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_revision_entry)
					,tmpbuf);
			gdk_threads_leave();
			g_free(tmpbuf);
		}
		if (cmd->store_type == SIG)
		{
			if (canidate->sig_str == NULL)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strndup(
						canidate->sig_str,
						(gint)g_hash_table_lookup(canidate->bytecounts, cmd->handle));
			gdk_threads_enter();
			gtk_entry_set_text(GTK_ENTRY(entries.ecu_signature_entry),tmpbuf);
			gdk_threads_leave();
			g_free(tmpbuf);
		}
		if (cmd->store_type == EXTVER)
		{
			if (canidate->quest_str == NULL)
				tmpbuf = g_strdup("");
			else
				tmpbuf = g_strndup(
						canidate->quest_str,
						(gint)g_hash_table_lookup(canidate->bytecounts, cmd->handle));
			gdk_threads_enter();
			gtk_entry_set_text(GTK_ENTRY(entries.extended_revision_entry),tmpbuf);
			gdk_threads_leave();
			g_free(tmpbuf);
		}

	}
	if (match == -1) // (we DID NOT find one)
	{
		tmpbuf = g_strdup_printf("Firmware NOT DETECTED properly, Expect MegaTunix to NOT behave properly \nContact the author with the contents of this window\n");
		dbg_func(g_strdup_printf(__FILE__":\n\tdetermine_ecu() Firmware NOT DETECTED, send contents of the\ninterrogation window and the firmware details to the MegaTunix author\n"),CRITICAL);
		gdk_threads_enter();
		update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
		gdk_threads_leave();
		g_free(tmpbuf);
		goto cleanup;
	}

	/* Set flags */
	ecu_caps = potential->capabilities;
	/* Enable/Disable Controls */
	parse_ecu_capabilities(ecu_caps);

	/* Set expected sizes for commands */
	serial_params->table0_size = (gint)g_hash_table_lookup(
			potential->bytecounts,"_CMD_V0_");
	serial_params->table1_size = (gint)g_hash_table_lookup(
			potential->bytecounts,"_CMD_V1_");
	serial_params->rtvars_size = (gint)g_hash_table_lookup(
			potential->bytecounts,"_CMD_A_");
	serial_params->ignvars_size = (gint)g_hash_table_lookup(
			potential->bytecounts,"_CMD_I_");
	serial_params->memblock_size = (gint)g_hash_table_lookup(
			potential->bytecounts,"_CMD_F0_");

	/* Display firmware version in the window... */
	tmpbuf = g_strdup_printf("Detected Firmware: %s\n",potential->firmware_name);

	dbg_func(g_strdup_printf(__FILE__": determine_ecu() Detected Firmware: %s\n",potential->firmware_name),INTERROGATOR);
	gdk_threads_enter();
	update_logbar(interr_view,"warning",tmpbuf,FALSE,FALSE);
	gdk_threads_leave();
	g_free(tmpbuf);
	goto freeup;


cleanup:
	/* Set flags */
	ecu_caps = 0;				// no flags 

	/* Enable/Disable Controls */
	parse_ecu_capabilities(ecu_caps);

	serial_params->table0_size = 125;	/* assumptions!!!! */
	serial_params->table1_size = 0;		/* assumptions!!!! */
	serial_params->rtvars_size = 22;	/* assumptions!!!! */

freeup:
	if (canidate->sig_str)
		g_free(canidate->sig_str);
	if (canidate->quest_str)
		g_free(canidate->quest_str);
	return;

}

GArray * validate_and_load_tests()
{
	ConfigFile *cfgfile;
	GArray * cmd_array = NULL;
	gchar * filename;
	gchar *section = NULL;
	gchar * tmpbuf;
	gint total_tests = 0;
	gint i = 0;
	gboolean tmpi = FALSE;
	struct Command *cmd = NULL;

	filename = g_strconcat(DATA_DIR,"/",INTERROGATOR_DIR,"/","tests",NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		dbg_func(g_strdup_printf(__FILE__":\n\tfile %s,\n\topened successfully\n",filename),INTERROGATOR);
		cmd_array = g_array_new(FALSE,FALSE,sizeof(struct Command *));
		cfg_read_int(cfgfile,"interrogation_tests","total_tests",&total_tests);
		for (i=0;i<total_tests;i++)
		{
			cmd = (struct Command *)new_cmd_struct();
			section = g_strdup_printf("test_%.2i",i);
			cfg_read_string(cfgfile,section,"raw_cmd",
					&cmd->string);
			cfg_read_int(cfgfile,section,"cmd_length",
					&cmd->len);
			cfg_read_string(cfgfile,section,"cmd_handle",
					&cmd->handle);
			cfg_read_string(cfgfile,section,"cmd_desc",
					&cmd->desc);
			cfg_read_int(cfgfile,section,"page",
					&cmd->page);
			cfg_read_boolean(cfgfile,section,"store_data",
					&tmpi);
			if (tmpi)
			{
				cfg_read_string(cfgfile,section,"store_type",
						&tmpbuf);
				cmd->store_type = translate_string(tmpbuf);
				g_free(tmpbuf);
			}

			g_free(section);
			g_array_insert_val(cmd_array,i,cmd);
		}
		cfg_free(cfgfile);
		
		
	}
	else
	{
		dbg_func(g_strdup_printf(__FILE__":\n\tfailure opening \"%s\" file\nMegaTunix was NOT installed!!!! \nPlease run \"make install\" from the top level MegaTunix dir\n",filename),CRITICAL);
		exit(-1);
	}

	g_free(filename);
	return cmd_array;
}

void * new_cmd_struct()
{
	struct Command *cmd = NULL;
	cmd = g_malloc0(sizeof(struct Command));
	return cmd;
}

void free_test_commands(GArray * cmd_array)
{
	struct Command *cmd = NULL;
	gint i = 0;
	for (i=0;i<cmd_array->len;i++)
	{
		cmd = g_array_index(cmd_array,struct Command *,i);
		if (cmd->string)
			g_free(cmd->string);
		if (cmd->desc)
			g_free(cmd->desc);
		g_free(cmd);
	}
	g_array_free(cmd_array,TRUE);
}

void close_profile(void *ptr)
{
	struct Canidate *canidate = (struct Canidate *) ptr;
	
	dbg_func(__FILE__": close_profile(),\n\tdeallocating memory for potential canidate match\n",INTERROGATOR);
	if (canidate->sig_str)
		g_free(canidate->sig_str);
	if (canidate->quest_str)
		g_free(canidate->quest_str);
	if (canidate->firmware_name)
		g_free(canidate->firmware_name);
	if (canidate->bytecounts)
		g_hash_table_destroy(canidate->bytecounts);
	g_free(canidate);
	dbg_func(__FILE__": close_profile(),\n\tdeallocation of memory for potential canidate complete\n",INTERROGATOR);
	
}

		
void * load_profile(GArray * cmd_array, gchar * file)
{
	ConfigFile *cfgfile;
	gchar * tmpbuf;
	gchar * filename;
	struct Canidate *canidate = NULL;

	filename = g_strconcat(DATA_DIR,"/",INTERROGATOR_DIR,"/Profiles/",file,NULL);
	cfgfile = cfg_open_file(filename);
	if (cfgfile)
	{	
		canidate = g_malloc0(sizeof(struct Canidate));
		dbg_func(g_strdup_printf(__FILE__": load_profile() file:\n\t%s\n\topened successfully\n",filename),INTERROGATOR);
		canidate->bytecounts = g_hash_table_new(g_str_hash,g_str_equal);
		cfg_read_string(cfgfile,"interrogation_profile","name",&canidate->firmware_name);
		cfg_read_string(cfgfile,"parameters","bytecounts",
				&tmpbuf);
		parse_bytecounts(cmd_array, canidate->bytecounts, tmpbuf);
		g_free(tmpbuf);
		cfg_read_string(cfgfile,"parameters","SignatureQueryString",
				&canidate->sig_str);
		cfg_read_string(cfgfile,"parameters","ExtVerQueryString",
				&canidate->quest_str);
		cfg_read_int(cfgfile,"parameters","VerNumber",
				&canidate->ver_num);
		cfg_read_string(cfgfile,"parameters","Capabilities",
				&tmpbuf);
		canidate->capabilities = translate_capabilities(tmpbuf);
		g_free(tmpbuf);

		cfg_free(cfgfile);
		g_free(filename);
		
	}
	else
		dbg_func(g_strdup_printf(__FILE__": load_profile() failure opening file:\n\t%s\n",filename),CRITICAL);
	return canidate;
}

void parse_bytecounts(GArray *cmd_array, GHashTable *hash, gchar * input)
{
	gchar **vector;
	struct Command *cmd = NULL;
	gint i = 0;
	gint count = 0;

	vector = g_strsplit(input,",",0);
	
	while (vector[i++] != NULL)
		count++;
	if (cmd_array->len != count)
		dbg_func(__FILE__": parse_bytecounts(), ERROR, vector length does not coincide with commands list, invalid profile\n",CRITICAL);

	for (i=0;i<count;i++)
	{
		cmd = g_array_index(cmd_array,struct Command *, i);
		g_hash_table_insert(hash, cmd->handle,GINT_TO_POINTER(atoi(vector[i])));
		dbg_func(g_strdup_printf(__FILE__": parse_bytecounts() inserting key %s, val %i\n",cmd->handle,atoi(vector[i])),INTERROGATOR);
	}	
	g_strfreev(vector);

}

gint translate_capabilities(gchar *string)
{
	gchar **vector;
	gint i = 0;
	gint value = 0;
	

	vector = g_strsplit(string,",",0);
	dbg_func(g_strdup_printf(__FILE__": translate_capabilities() string fed is %s\n",string),INTERROGATOR);
	while (vector[i] != NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": translate_capabilities() trying to translate %s\n",vector[i]),INTERROGATOR);
		value += translate_string(vector[i]);
		i++;
	}

	g_strfreev(vector);
	return value;	
		
}
