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
 *
 * Just about all of this was written by Richard Barrington....
 * 
 * Large portions of this code are based on the examples provided with 
 * the GtkGlExt libraries.
 *
 */

#include <3d_vetable.h>
#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <enums.h>
#include <globals.h>
#include <gui_handlers.h>
#include <serialio.h>
#include <structures.h>
#include <time.h>
#include <tuning_gui.h>


#define DEFAULT_WIDTH  400
#define DEFAULT_HEIGHT 320                                                                                                                         



int build_tuning(GtkWidget *parent_frame)
{
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *drawing_area;
	GdkGLConfig *gl_config;
	GtkWidget *table;
	extern struct DynamicButtons buttons;
	extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	frame = gtk_frame_new("VE Table 3D display");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);

	drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(frame),drawing_area);

	gtk_widget_set_size_request (drawing_area, 
			DEFAULT_WIDTH, DEFAULT_HEIGHT);

	gl_config = get_gl_config();
	gtk_widget_set_gl_capability(drawing_area, gl_config, NULL, 
			TRUE, GDK_GL_RGBA_TYPE);

	GTK_WIDGET_SET_FLAGS(drawing_area,GTK_CAN_FOCUS);

	gtk_widget_add_events (drawing_area,
			GDK_BUTTON1_MOTION_MASK	|
			GDK_BUTTON2_MOTION_MASK	|
			GDK_BUTTON_PRESS_MASK	|
			GDK_KEY_PRESS_MASK		|
			GDK_KEY_RELEASE_MASK	|
			GDK_FOCUS_CHANGE_MASK	|
			GDK_VISIBILITY_NOTIFY_MASK);	

	/* Connect signal handlers to the drawing area */
	g_signal_connect_after(G_OBJECT (drawing_area), "realize",
			G_CALLBACK (ve_realize), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "configure_event",
			G_CALLBACK (ve_configure_event), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "expose_event",
			G_CALLBACK (ve_expose_event), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",
			G_CALLBACK (ve_motion_notify_event), NULL);	
	g_signal_connect (G_OBJECT (drawing_area), "button_press_event",
			G_CALLBACK (ve_button_press_event), NULL);	
	g_signal_connect(G_OBJECT (drawing_area), "key_press_event",
			G_CALLBACK (ve_key_press_event), NULL);	
	g_signal_connect(G_OBJECT (drawing_area), "focus_in_event",
			G_CALLBACK (ve_focus_in_event), NULL);	

	/* End of GL window, Now controls for it.... */
	frame = gtk_frame_new("3D Display Controls");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	button = gtk_button_new_with_label("Reset Display");
        gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
        g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(RESET_3D_VIEW));

	frame = gtk_frame_new("Commands");
        gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);

        table = gtk_table_new(1,2,FALSE);
        gtk_table_set_col_spacings(GTK_TABLE(table),5);
        gtk_table_set_row_spacings(GTK_TABLE(table),5);
        gtk_container_set_border_width (GTK_CONTAINER (table), 5);
        gtk_container_add(GTK_CONTAINER(frame),table);

        button = gtk_button_new_with_label("Get Data from ECU");
        gtk_tooltips_set_tip(tip,button,
                        "Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI",NULL);

        gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(READ_VE_CONST));

        button = gtk_button_new_with_label("Permanently Store Data in ECU");
        buttons.tuning_store_but = button;
        gtk_tooltips_set_tip(tip,button,
                        "Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
        gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
                        (GtkAttachOptions) (GTK_EXPAND),
                        (GtkAttachOptions) (0), 0, 0);
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(BURN_MS_FLASH));

	/* Probably want something meaningful here */
	return TRUE;
}
