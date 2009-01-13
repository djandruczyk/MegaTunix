/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#ifndef __MTXLOADER_H__
#define __MTXLOADER_H__

#include <gtk/gtk.h>

/* Prototypes */
/**********************************************************************
 * samples holds the three temperatures and corresponding three
 * resistances for the user's input. temps are lowest to highest
 * resistances are corresponding values and will be highest to
 * lowest due to the nature of thermistors.					
 **********************************************************************/
typedef struct _samples samples;
typedef struct _inc_entry inc_entry;

struct _samples {
  double t1, t2, t3, r1, r2, r3;
};

 struct _inc_entry {
  int ms_val;
  int adc;
  int temp_f;
  int temp_c;
  int ohms;
};

/* Prototypes */
EXPORT gboolean load_firmware (GtkButton*);
EXPORT gboolean get_signature (GtkButton*);
EXPORT gboolean leave (GtkWidget *, gpointer);
EXPORT gboolean about_popup (GtkWidget *, gpointer);
EXPORT gboolean use_sensor (GtkWidget *, gpointer);
EXPORT gboolean get_sensor_info (GtkWidget *, gpointer);
void load_defaults(void);
void save_defaults(void);
void init_controls(void);
void output(gchar *);
void boot_jumper_prompt(void);


#endif
