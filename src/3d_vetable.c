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
#include <assert.h>
#include <config.h>
#include <conversions.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gdk/gdkkeysyms.h>
#include <gui_handlers.h>
#include <gtk/gtkgl.h>
#include <listmgmt.h>
#include <../mtxmatheval/mtxmatheval.h>
#include <notifications.h>
#include <pango/pangoft2.h>
#include <rtv_processor.h>
#include <runtime_sliders.h>
#include <serialio.h>
#include <structures.h>
#include <tabloader.h>
#include <threads.h>
#include <time.h>
#include <widgetmgmt.h>

static GLuint font_list_base;


#define DEFAULT_WIDTH  475
#define DEFAULT_HEIGHT 320                                                                                  
static GHashTable *winstat = NULL;
/*!
 \brief create_ve3d_view does the initial work of creating the 3D vetable
 widget, it creates the datastructures, creates the window, initializes OpenGL
 and binds al lthe handlers to the window that are needed
 */
EXPORT gint create_ve3d_view(GtkWidget *widget, gpointer data)
{
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *drawing_area;
	GtkObject * object = NULL;
	GdkGLConfig *gl_config;
	struct Ve_View_3D *ve_view;
	extern GtkTooltips *tip;
	extern struct Firmware_Details *firmware;
	extern gboolean gl_ability;
	gint table_num =  -1;

	if (!gl_ability)
	{
		dbg_func(g_strdup(__FILE__": create_ve3d_view()\n\t GtkGLEXT Library initialization failed, no GL for you :(\n"),CRITICAL);
		return FALSE;
	}
	table_num = (gint)g_object_get_data(G_OBJECT(widget),"table_num");

	if (winstat == NULL)
		winstat = g_hash_table_new(NULL,NULL);

	if ((gboolean)g_hash_table_lookup(winstat,(gpointer)table_num) == TRUE)
		return TRUE;
	else
		g_hash_table_insert(winstat,(gpointer)table_num, (gpointer)TRUE);

	ve_view = initialize_ve3d_view();
	ve_view->x_source = g_strdup(g_object_get_data(G_OBJECT(widget),"x_source"));
	ve_view->y_source = g_strdup(g_object_get_data(G_OBJECT(widget),"y_source"));
	ve_view->z_source = g_strdup(g_object_get_data(G_OBJECT(widget),"z_source"));
	ve_view->x_suffix = g_strdup(firmware->table_params[table_num]->x_suffix);
	ve_view->y_suffix = g_strdup(firmware->table_params[table_num]->y_suffix);
	ve_view->z_suffix = g_strdup(firmware->table_params[table_num]->z_suffix);

	ve_view->x_conv_expr = g_strdup(firmware->table_params[table_num]->x_conv_expr);
	ve_view->y_conv_expr = g_strdup(firmware->table_params[table_num]->y_conv_expr);
	ve_view->z_conv_expr = g_strdup(firmware->table_params[table_num]->z_conv_expr);
	ve_view->x_eval = evaluator_create(ve_view->x_conv_expr);
	ve_view->y_eval = evaluator_create(ve_view->y_conv_expr);
	ve_view->z_eval = evaluator_create(ve_view->z_conv_expr);
	assert(ve_view->x_eval);
	assert(ve_view->y_eval);
	assert(ve_view->z_eval);
	
	ve_view->z_page = firmware->table_params[table_num]->z_page;
	ve_view->z_base = firmware->table_params[table_num]->z_base;
	ve_view->z_disp_float = firmware->table_params[table_num]->z_disp_float;
        ve_view->z_precision = firmware->table_params[table_num]->z_disp_precision;

	ve_view->x_page = firmware->table_params[table_num]->x_page;
	ve_view->x_base = firmware->table_params[table_num]->x_base;
	ve_view->x_bincount = firmware->table_params[table_num]->x_bincount;
	ve_view->x_disp_float = firmware->table_params[table_num]->x_disp_float;
        ve_view->x_precision = firmware->table_params[table_num]->x_disp_precision;

	ve_view->y_page = firmware->table_params[table_num]->y_page;
	ve_view->y_base = firmware->table_params[table_num]->y_base;
	ve_view->y_bincount = firmware->table_params[table_num]->y_bincount; 
	ve_view->y_disp_float = firmware->table_params[table_num]->y_disp_float;
        ve_view->y_precision = firmware->table_params[table_num]->y_disp_precision;

	ve_view->table_name = g_strdup(firmware->table_params[table_num]->table_name);
	ve_view->table_num = table_num;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), ve_view->table_name);
	gtk_widget_set_size_request(window,DEFAULT_WIDTH,DEFAULT_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER(window),0);
	ve_view->window = window;
	g_object_set_data(G_OBJECT(window),"ve_view",(gpointer)ve_view);

	/* Bind pointer to veview to an object for retrieval elsewhere */
	object = g_object_new(GTK_TYPE_INVISIBLE,NULL);
	g_object_set_data(G_OBJECT(object),"ve_view",(gpointer)ve_view);

	register_widget(g_strdup_printf("ve_view_%i",table_num),
			(gpointer)object);

	g_signal_connect_swapped(G_OBJECT(window), "delete_event",
			G_CALLBACK(free_ve3d_sliders),
			GINT_TO_POINTER(table_num));
	g_signal_connect_swapped(G_OBJECT(window), "delete_event",
			G_CALLBACK(free_ve3d_view),
			(gpointer) window);
	g_signal_connect_swapped(G_OBJECT(window), "delete_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer) window);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(window),vbox);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	frame = gtk_frame_new("VE/Spark Table 3D display");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	drawing_area = gtk_drawing_area_new();
	g_object_set_data(G_OBJECT(drawing_area),"ve_view",(gpointer)ve_view);
	ve_view->drawing_area = drawing_area;
	gtk_container_add(GTK_CONTAINER(frame),drawing_area);

	gl_config = get_gl_config();
	gtk_widget_set_gl_capability(drawing_area, gl_config, NULL,
			TRUE, GDK_GL_RGBA_TYPE);

	GTK_WIDGET_SET_FLAGS(drawing_area,GTK_CAN_FOCUS);

	gtk_widget_add_events (drawing_area,
			GDK_BUTTON1_MOTION_MASK	|
			GDK_BUTTON2_MOTION_MASK	|
			GDK_BUTTON3_MOTION_MASK	|
			GDK_BUTTON_PRESS_MASK	|
			GDK_KEY_PRESS_MASK	|
			GDK_KEY_RELEASE_MASK	|
			GDK_FOCUS_CHANGE_MASK	|
			GDK_VISIBILITY_NOTIFY_MASK);

	/* Connect signal handlers to the drawing area */
	g_signal_connect_after(G_OBJECT (drawing_area), "realize",
			G_CALLBACK (ve3d_realize), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "configure_event",
			G_CALLBACK (ve3d_configure_event), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "expose_event",
			G_CALLBACK (ve3d_expose_event), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",
			G_CALLBACK (ve3d_motion_notify_event), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "button_press_event",
			G_CALLBACK (ve3d_button_press_event), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "key_press_event",
			G_CALLBACK (ve3d_key_press_event), NULL);

	/* End of GL window, Now controls for it.... */
	frame = gtk_frame_new("3D Display Controls");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	button = gtk_button_new_with_label("Reset Display");
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
	g_object_set_data(G_OBJECT(button),"ve_view",(gpointer)ve_view);
	g_signal_connect_swapped(G_OBJECT (button), "clicked",
			G_CALLBACK (reset_3d_view), (gpointer)button);

	button = gtk_button_new_with_label("Get Data from ECU");
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(READ_VE_CONST));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	gtk_tooltips_set_tip(tip,button,
			"Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI",NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);


	button = gtk_button_new_with_label("Burn to ECU");
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(BURN_MS_FLASH));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	ve_view->burn_but = button;
	store_list("burners",g_list_append(
				get_list("burners"),(gpointer)button));

	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Start Reading RT Vars");
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(START_REALTIME));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), 
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Stop Reading RT vars");
	g_object_set_data(G_OBJECT(button),"handler",GINT_TO_POINTER(STOP_REALTIME));
	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler), 
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	table = gtk_table_new(4,2,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(vbox2),table,TRUE,TRUE,5);

	label = gtk_label_new("Edit\nPosition");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	register_widget(g_strdup_printf("x_active_label_%i",table_num),label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	register_widget(g_strdup_printf("y_active_label_%i",table_num),label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	register_widget(g_strdup_printf("z_active_label_%i",table_num),label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new("Runtime\nPosition");
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	register_widget(g_strdup_printf("x_runtime_label_%i",table_num),label);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	register_widget(g_strdup_printf("y_runtime_label_%i",table_num),label);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	register_widget(g_strdup_printf("z_runtime_label_%i",table_num),label);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);


	button = gtk_button_new_with_label("Close Window");
	gtk_box_pack_end(GTK_BOX(vbox2),button,FALSE,FALSE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
			G_CALLBACK(free_ve3d_sliders),
			GINT_TO_POINTER(table_num));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
			G_CALLBACK(free_ve3d_view),
			(gpointer) window);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer) window);


	frame = gtk_frame_new("Real-Time Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame),0);

	hbox = gtk_hbox_new(TRUE,5);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);
	register_widget(g_strdup_printf("ve3d_rt_table0_%i",table_num),table);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);
	register_widget(g_strdup_printf("ve3d_rt_table1_%i",table_num),table);

	load_ve3d_sliders(table_num);

	gtk_widget_show_all(window);

	return TRUE;
}

/*!
 \brief free_ve3d_view is called on close of the 3D vetable viewer/editor, it
 deallocates memory disconencts handlers and then the widget is deleted with
 gtk_widget_destroy
 */
gint free_ve3d_view(GtkWidget *widget)
{
	struct Ve_View_3D *ve_view;
	extern GHashTable *dynamic_widgets;
	gchar * tmpbuf = NULL;

	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"ve_view");
	store_list("burners",g_list_remove(
			get_list("burners"),(gpointer)ve_view->burn_but));
	g_hash_table_remove(winstat,(gpointer)ve_view->table_num);
	tmpbuf = g_strdup_printf("ve_view_%i",ve_view->table_num);
	g_object_set_data(g_hash_table_lookup(dynamic_widgets,tmpbuf),"ve_view",NULL);
	g_free(tmpbuf);
	deregister_widget(g_strdup_printf("ve_view_%i",ve_view->table_num));
	deregister_widget(g_strdup_printf("x_active_label_%i",ve_view->table_num));
	deregister_widget(g_strdup_printf("y_active_label_%i",ve_view->table_num));
	deregister_widget(g_strdup_printf("z_active_label_%i",ve_view->table_num));
	deregister_widget(g_strdup_printf("x_runtime_label_%i",ve_view->table_num));
	deregister_widget(g_strdup_printf("y_runtime_label_%i",ve_view->table_num));
	deregister_widget(g_strdup_printf("z_runtime_label_%i",ve_view->table_num));
	g_free(ve_view->x_source);
	g_free(ve_view->y_source);
	g_free(ve_view->z_source);
	free(ve_view);/* free up the memory */
	ve_view = NULL;

	return FALSE;  /* MUST return false otherwise 
			* other handlers WILL NOT run. */
}
	
/*!
 \brief reset_3d_view resets the OpenGL widget to default position in
 case the user moves it or places it off the edge of the window and can't
 find it...
 */
void reset_3d_view(GtkWidget * widget)
{
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"ve_view");
	ve_view->active_y = 0;
	ve_view->active_x = 0;
	ve_view->dt = 0.008;
	ve_view->sphi = 35.0; 
	ve_view->stheta = 75.0; 
	ve_view->sdepth = 7.533;
	ve_view->zNear = 0.8;
	ve_view->zFar = 23;
	ve_view->h_strafe = 0;
	ve_view->v_strafe = 0;
	ve_view->aspect = 1.333;
	ve3d_configure_event(ve_view->drawing_area, NULL,NULL);
	ve3d_expose_event(ve_view->drawing_area, NULL,NULL);
}

/*!
 \brief ge_gl_config gets the OpenGL mode creates a GL config and returns it
 */
GdkGLConfig* get_gl_config(void)
{
	GdkGLConfig* gl_config;                                                                                                                        
	/* Try double-buffered visual */
	gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
			GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE);
	if (gl_config == NULL)
	{
		dbg_func(g_strdup(__FILE__": get_gl_config()\n\t*** Cannot find the double-buffered visual.\n\t*** Trying single-buffered visual.\n"),CRITICAL);

		/* Try single-buffered visual */
		gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
				GDK_GL_MODE_DEPTH);
		if (gl_config == NULL)
		{
			/* Should make a non-GL basic drawing area version 
			   instead of dumping the user out of here, or at least 
			   render a non-GL found text on the drawing area */
			dbg_func(g_strdup(__FILE__": get_gl_config()\n\t*** No appropriate OpenGL-capable visual found. EXITING!!\n"),CRITICAL);
			exit (-1);
		}
	}
	return gl_config;	
}

/*!
 \brief ve3d_configure_event is called when the window needs to be drawn
 after a resize. 
 */
gboolean ve3d_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"ve_view");

	GLfloat w = widget->allocation.width;
	GLfloat h = widget->allocation.height;

	dbg_func(g_strdup(__FILE__": ve3d_configure_event() 3D View Configure Event\n"),OPENGL);

	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return FALSE;

	ve_view->aspect = (gfloat)w/(gfloat)h;
	glViewport (0, 0, w, h);

	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/                                                                                                                  
	return TRUE;
}

/*!
 \brief ve3d_expose_event is called when the part or all of the GL area
 needs to be redrawn due to being "exposed" (uncovered), this kicks off all
 the other renderers for updating the axis and status indicators. This 
 method is NOT like I'd like it and is a CPU pig as 99.5% of the time we don't
 even need to redraw at all..  :(
 /bug this code is slow, and needs to be optimized or redesigned
 */
gboolean ve3d_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	extern gboolean forced_update;
	struct Ve_View_3D *ve_view = NULL;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"ve_view");

	dbg_func(g_strdup(__FILE__": ve3d_expose_event() 3D View Expose Event\n"),OPENGL);

	if (!GTK_WIDGET_HAS_FOCUS(widget)){
		gtk_widget_grab_focus(widget);
	}

	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return FALSE;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(64.0, ve_view->aspect, ve_view->zNear, ve_view->zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0,0.0,-ve_view->sdepth);
	glRotatef(-ve_view->stheta, 1.0, 0.0, 0.0);
	glRotatef(ve_view->sphi, 0.0, 0.0, 1.0);
	glTranslatef(-(gfloat)((ve_view->x_bincount/2)-0.3)-ve_view->h_strafe, -(gfloat)((ve_view->y_bincount)/2-1)-ve_view->v_strafe, -2.0);

	ve3d_calculate_scaling(ve_view);
	ve3d_draw_ve_grid(ve_view);
	ve3d_draw_active_indicator(ve_view);
	ve3d_draw_runtime_indicator(ve_view);
	ve3d_draw_axis(ve_view);

	/* Swap buffers */
	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);
	else
		glFlush ();

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/

	forced_update = TRUE;
	return TRUE; 
}

/*!
 \brief ve3d_motion_notify_event is called when the user clicks and drags the 
 mouse inside the GL window, it causes the display to be rotated/scaled/strafed
 depending on which button the user had held down.
 \see ve3d_button_press_event
 */
gboolean ve3d_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	gboolean redraw = FALSE;
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"ve_view");

	dbg_func(g_strdup(__FILE__": ve3d_motion_notify() 3D View Motion Notify\n"),OPENGL);

	// Left Button
	if (event->state & GDK_BUTTON1_MASK)
	{
		ve_view->sphi += (gfloat)(event->x - ve_view->beginX) / 4.0;
		ve_view->stheta += (gfloat)(ve_view->beginY - event->y) / 4.0;
		redraw = TRUE;
	}
	// Middle button (or both buttons for two button mice)
	if (event->state & GDK_BUTTON2_MASK)
	{
		ve_view->h_strafe -= (gfloat)(event->x -ve_view->beginX) / 40.0;
		ve_view->v_strafe += (gfloat)(event->y -ve_view->beginY) / 40.0;
		redraw = TRUE;
	}
	// Right Button
	if (event->state & GDK_BUTTON3_MASK)
	{
		ve_view->sdepth -= ((event->y - ve_view->beginY)/(widget->allocation.height))*8;
		redraw = TRUE;
	}

	ve_view->beginX = event->x;
	ve_view->beginY = event->y;

	gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);

	return TRUE;
}

/*!
 \brief ve3d_button_press_event is called when the user clicks a mouse button
 The function grabs the location at which the button was clicked in order to
 calculate what to change when rerendering
 \see ve3d_motion_notify_event
 */
gboolean ve3d_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"ve_view");
	dbg_func(g_strdup(__FILE__": ve3d_button_press_event()\n"),OPENGL);

	gtk_widget_grab_focus (widget);

	if (event->button != 0)
	{
		ve_view->beginX = event->x;
		ve_view->beginY = event->y;
		return TRUE;
	}

	return FALSE;
}

/*!
 \brief ve3d_realize is called when the window is created and sets the main
 OpenGL parameters of the window (this only needs to be done once I think)
 */
void ve3d_realize (GtkWidget *widget, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	GdkGLProc proc = NULL;

	dbg_func(g_strdup(__FILE__": ve3d_realize() 3D View Realization\n"),OPENGL);

	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return;

	/* glPolygonOffsetEXT */
	proc = gdk_gl_get_glPolygonOffsetEXT();
	if (proc == NULL)
	{
		/* glPolygonOffset */
		proc = gdk_gl_get_proc_address ("glPolygonOffset");
		if (proc == NULL) {
			dbg_func(g_strdup(__FILE__": ve3d_realize()\n\tSorry, glPolygonOffset() is not supported by this renderer. EXITING!!!\n"),CRITICAL);
			exit (-11);
		}
	}

	glClearColor (0.0, 0.0, 0.0, 0.0);
	//gdk_gl_glPolygonOffsetEXT (proc, 1.0, 1.0);
	glShadeModel(GL_FLAT);
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	ve3d_load_font_metrics();

	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
}

/*!
 \brief ve3d_calculate_scaling is called during a redraw to recalculate the
 dimensions for the scales to make thing look pretty
 */
void ve3d_calculate_scaling(struct Ve_View_3D *ve_view)
{
	gint i=0;
	extern gint **ms_data;
	gint x_page = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_base = 0;
	gint y_base = 0;
	gint z_base = 0;

	dbg_func(g_strdup(__FILE__": ve3d_calculate_scaling()\n"),OPENGL);

	x_base = ve_view->x_base;
	y_base = ve_view->y_base;
	z_base = ve_view->z_base;

	x_page = ve_view->x_page;
	y_page = ve_view->y_page;
	z_page = ve_view->z_page;

	ve_view->x_max = 0;
	ve_view->y_max = 0;
	ve_view->z_max = 0;
	ve_view->z_min = 255;
	// Spark requires a divide by 2.84 to convert from ms units to degrees

	for (i=0;i<ve_view->x_bincount;i++) 
	{
		if (ms_data[x_page][x_base+i] > ve_view->x_max) 
			ve_view->x_max = ms_data[x_page][x_base+i];
	}

	for (i=0;i<ve_view->y_bincount;i++) 
	{
		if (ms_data[y_page][y_base+i] > ve_view->y_max) 
			ve_view->y_max = ms_data[y_page][y_base+i];
	}
	for (i=0;i<(ve_view->x_bincount*ve_view->y_bincount);i++) 
	{
		if (ms_data[z_page][z_base+i] > ve_view->z_max) 
			ve_view->z_max = ms_data[z_page][z_base+i];
		if (ms_data[z_page][z_base+i] < ve_view->z_min) 
			ve_view->z_min = ms_data[z_page][z_base+i];
	}

	ve_view->x_div = ((gfloat)ve_view->x_max/(gfloat)ve_view->x_bincount);
	ve_view->y_div = ((gfloat)ve_view->y_max/(gfloat)ve_view->y_bincount);
	ve_view->z_div = ((gfloat)ve_view->z_max-(gfloat)ve_view->z_min)/ve_view->z_scale;	
	ve_view->z_offset = ((gfloat)ve_view->z_min/ve_view->z_div);
}

/*!
 \brief ve3d_draw_ve_grid is called during rerender and draws trhe VEtable grid 
 in 3D space
 */
void ve3d_draw_ve_grid(struct Ve_View_3D *ve_view)
{
	extern gint **ms_data;
	gint x = 0;
	gint y = 0;
	gint x_page = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_base = 0;
	gint y_base = 0;
	gint z_base = 0;

	dbg_func(g_strdup(__FILE__": ve3d_draw_ve_grid() \n"),OPENGL);

	x_base = ve_view->x_base;
	y_base = ve_view->y_base;
	z_base = ve_view->z_base;

	x_page = ve_view->x_page;
	y_page = ve_view->y_page;
	z_page = ve_view->z_page;

	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(1.5);


	/* Draw lines on RPM axis */
	for(x=0;x<ve_view->x_bincount;x++)
	{
		glBegin(GL_LINE_STRIP);
		for(y=0;y<ve_view->y_bincount;y++) 
		{
			glVertex3f(
					(gfloat)(ms_data[x_page][x_base+x])/ve_view->x_div,
					
					(gfloat)(ms_data[y_page][y_base+y])/ve_view->y_div, 	 	
					((gfloat)(ms_data[z_page][z_base+(y*ve_view->y_bincount)+x])/ve_view->z_div)-ve_view->z_offset);
					
		}
		glEnd();
	}

	/* Draw lines on MAP axis */
	for(y=0;y<ve_view->y_bincount;y++)
	{
		glBegin(GL_LINE_STRIP);
		for(x=0;x<ve_view->x_bincount;x++)
		{
			glVertex3f(	
					(gfloat)(ms_data[x_page][x_base+x])/ve_view->x_div,
					(gfloat)(ms_data[y_page][y_base+y])/ve_view->y_div,
					((gfloat)(ms_data[z_page][z_base+(y*ve_view->y_bincount)+x])/ve_view->z_div)-ve_view->z_offset);
					
		}
		glEnd();
	}
}

/*!
 \brief ve3d_draw_active_indicator is called during rerender and draws the
 red dot which tells where changes will be made to the table by the user.  
 The user moves this with the arrow keys..
 */
void ve3d_draw_active_indicator(struct Ve_View_3D *ve_view)
{
	extern gint **ms_data;
	gint x_page = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_base = 0;
	gint y_base = 0;
	gint z_base = 0;
	gchar * tmpbuf = NULL;
	gchar * value = NULL;
	extern GHashTable *dynamic_widgets;

	dbg_func(g_strdup(__FILE__": ve3d_draw_active_indicator()\n"),OPENGL);

	x_base = ve_view->x_base;
	y_base = ve_view->y_base;
	z_base = ve_view->z_base;

	x_page = ve_view->x_page;
	y_page = ve_view->y_page;
	z_page = ve_view->z_page;


	/* Render a red dot at the active VE map position */
	glPointSize(8.0);
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_POINTS);
	glVertex3f(	
			(gfloat)(ms_data[x_page][x_base+ve_view->active_x])/ve_view->x_div,
			(gfloat)(ms_data[y_page][y_base+ve_view->active_y])/ve_view->y_div,	
			((gfloat)ms_data[z_page][z_base+(ve_view->active_y*ve_view->y_bincount)+ve_view->active_x]/ve_view->z_div)-ve_view->z_offset);
	glEnd();	

	tmpbuf = g_strdup_printf("x_active_label_%i",ve_view->table_num);
	value = g_strdup_printf("%1$.*2$f %3$s",evaluator_evaluate_x(ve_view->x_eval,ms_data[x_page][x_base+ve_view->active_x]),ve_view->x_precision,ve_view->x_suffix);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,tmpbuf)),value);
	g_free(tmpbuf);
	g_free(value);

	tmpbuf = g_strdup_printf("y_active_label_%i",ve_view->table_num);
	value = g_strdup_printf("%1$.*2$f %3$s",evaluator_evaluate_x(ve_view->y_eval,ms_data[y_page][y_base+ve_view->active_y]),ve_view->y_precision,ve_view->y_suffix);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,tmpbuf)),value);
	g_free(tmpbuf);
	g_free(value);

	tmpbuf = g_strdup_printf("z_active_label_%i",ve_view->table_num);
	value = g_strdup_printf("%1$.*2$f %3$s",evaluator_evaluate_x(ve_view->z_eval,ms_data[z_page][z_base+(ve_view->active_y*ve_view->y_bincount)+ve_view->active_x]),ve_view->z_precision,ve_view->z_suffix);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,tmpbuf)),value);
	g_free(tmpbuf);
	g_free(value);

}

/*!
 \brief ve3d_draw_runtiem_indicator is called during rerender and draws the
 green dot which tells where the engien is running at this instant.
 */
void ve3d_draw_runtime_indicator(struct Ve_View_3D *ve_view)
{
	gfloat x_val = 0.0;
	gfloat y_val = 0.0;
	gfloat z_val = 0.0;
	gchar * tmpbuf = NULL;
	gchar * value = NULL;
	extern GHashTable *dynamic_widgets;

	dbg_func(g_strdup(__FILE__": ve3d_draw_runtime_indicator()\n"),OPENGL);

	if (!ve_view->z_source)
	{
		dbg_func(g_strdup(__FILE__": ve3d_draw_runtime_indicator()\n\t\"z_source\" is NOT defined, check the .datamap file for\n\tmissing \"z_source\" key for [3d_view_button]\n"),CRITICAL);
		return;
	}

	lookup_current_value(ve_view->x_source,&x_val);
	lookup_current_value(ve_view->y_source,&y_val);
	lookup_current_value(ve_view->z_source,&z_val);

	/* Render a green dot at the active VE map position */
	glPointSize(8.0);
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_POINTS);
	glVertex3f(	
			x_val/ve_view->x_div,
			y_val/ve_view->y_div,	
			(z_val/ve_view->z_div)-ve_view->z_offset);
	glEnd();

	tmpbuf = g_strdup_printf("x_runtime_label_%i",ve_view->table_num);
	value = g_strdup_printf("%1$.*2$f %3$s",evaluator_evaluate_x(ve_view->x_eval,x_val),ve_view->x_precision,ve_view->x_suffix);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,tmpbuf)),value);
	g_free(tmpbuf);
	g_free(value);

	tmpbuf = g_strdup_printf("y_runtime_label_%i",ve_view->table_num);
	value = g_strdup_printf("%1$.*2$f %3$s",evaluator_evaluate_x(ve_view->y_eval,y_val),ve_view->y_precision,ve_view->y_suffix);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,tmpbuf)),value);
	g_free(tmpbuf);
	g_free(value);

	tmpbuf = g_strdup_printf("z_runtime_label_%i",ve_view->table_num);
	value = g_strdup_printf("%1$.*2$f %3$s",evaluator_evaluate_x(ve_view->z_eval,z_val),ve_view->z_precision,ve_view->z_suffix);
	gtk_label_set_text(GTK_LABEL(g_hash_table_lookup(dynamic_widgets,tmpbuf)),value);
	g_free(tmpbuf);
	g_free(value);

}

/*!
 \brief ve3d_draw_axis is called during rerender and draws the
 border axis scales around the VEgrid.
 */
void ve3d_draw_axis(struct Ve_View_3D *ve_view)
{
	/* Set vars and an asthetically pleasing maximum value */
	gint i=0;
	gfloat tmpf = 0.0;
	gfloat top = 0.0;
	gfloat bottom = 0.0;
	gchar *label;
	extern gint **ms_data;
	gint x_page = 0;
	gint y_page = 0;
	gint z_page = 0;
	gint x_base = 0;
	gint y_base = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	gint z_base = 0;

	dbg_func(g_strdup(__FILE__": ve3d_draw_axis()\n"),OPENGL);

	x_base = ve_view->x_base;
	y_base = ve_view->y_base;
	z_base = ve_view->z_base;

	x_page = ve_view->x_page;
	y_page = ve_view->y_page;
	z_page = ve_view->z_page;

	x_bincount = ve_view->x_bincount;
	y_bincount = ve_view->y_bincount;

	top = (ve_view->z_max+10)/ve_view->z_div;
	bottom = (ve_view->z_min-10)/ve_view->z_div;
	/* Set line thickness and color */
	glLineWidth(1.0);
	glColor3f(0.7,0.7,0.7);

	/* Draw horizontal background grid lines  
	   starting at 0 VE and working up to VE+10% */
	for (i=10*((gint)ve_view->z_min/10);i<(ve_view->z_max+10);i+=10)
	{
		glBegin(GL_LINE_STRIP);
		glVertex3f(
				((ms_data[x_page][x_base])/ve_view->x_div),
				((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),		
				(((gfloat)i)/ve_view->z_div)-ve_view->z_offset);
		glVertex3f(
				((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
				((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),		
				(((gfloat)i)/ve_view->z_div)-ve_view->z_offset);
		glVertex3f(
				((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
				((ms_data[y_page][y_base])/ve_view->y_div),		
				(((gfloat)i)/ve_view->z_div)-ve_view->z_offset);
		glEnd();	
	}

	/* Draw vertical background grid lines along KPA axis */
	for (i=0;i<y_bincount;i++)
	{
		glBegin(GL_LINES);
		glVertex3f(
				((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
				((ms_data[y_page][y_base+i])/ve_view->y_div),		
				bottom - ve_view->z_offset);
		glVertex3f(
				((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
				((ms_data[y_page][y_base+i])/ve_view->y_div),		
				top - ve_view->z_offset);
		glEnd();
	}

	/* Draw vertical background lines along RPM axis */
	for (i=0;i<x_bincount;i++)
	{
		glBegin(GL_LINES);
		glVertex3f(
				((ms_data[x_page][x_base+i])/ve_view->x_div),		
				((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
				bottom - ve_view->z_offset);
		glVertex3f(
				((ms_data[x_page][x_base+i])/ve_view->x_div),
				((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),		
				top - ve_view->z_offset);
		glEnd();
	}

	/* Add the back corner top lines */
	glBegin(GL_LINE_STRIP);
	glVertex3f(
			((ms_data[x_page][x_base])/ve_view->x_div),	
			((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
			top - ve_view->z_offset);
	glVertex3f(
			((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),	
			((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
			top - ve_view->z_offset);
	glVertex3f(
			((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
			((ms_data[y_page][y_base])/ve_view->y_div),	
			top - ve_view->z_offset);
	glEnd();

	/* Add front corner base lines */
	glBegin(GL_LINE_STRIP);
	glVertex3f(
			((ms_data[x_page][x_base])/ve_view->x_div),	
			((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
			bottom - ve_view->z_offset);
	glVertex3f(
			((ms_data[x_page][x_base])/ve_view->x_div),	
			((ms_data[y_page][y_base])/ve_view->y_div),
			bottom - ve_view->z_offset);
	glVertex3f(
			((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
			((ms_data[y_page][y_base])/ve_view->y_div),
			bottom - ve_view->z_offset);
	glVertex3f(
			((ms_data[x_page][x_base+x_bincount-1])/ve_view->x_div),
			((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
			bottom - ve_view->z_offset);
	glVertex3f(
			((ms_data[x_page][x_base])/ve_view->x_div),	
			((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
			bottom - ve_view->z_offset);
	glEnd();

	/* Draw RPM and KPA labels */
	for (i=0;i<y_bincount;i++)
	{
		tmpf = evaluator_evaluate_x(ve_view->y_eval,ms_data[y_page][y_base+i]);
		label = g_strdup_printf("%i",(gint)tmpf);
		ve3d_draw_text(label,
				((ms_data[x_page][x_base])/ve_view->x_div),
				((ms_data[y_page][y_base+i])/ve_view->y_div),
				bottom - ve_view->z_offset);
		g_free(label);
	}

	for (i=0;i<x_bincount;i++)
	{
		tmpf = evaluator_evaluate_x(ve_view->x_eval,ms_data[x_page][x_base+i]);
		label = g_strdup_printf("%i",(gint)tmpf);
		ve3d_draw_text(label,
				((ms_data[x_page][x_base+i])/ve_view->x_div),
				((ms_data[y_page][y_base])/ve_view->y_div),
				bottom - ve_view->z_offset);
		g_free(label);
	}

	/* Draw VE labels */
	for (i=10*((gint)ve_view->z_min/10);i<(ve_view->z_max+10);i=i+10)
	{
		tmpf = evaluator_evaluate_x(ve_view->z_eval,i);
		if (ve_view->z_disp_float)
			label = g_strdup_printf("%1$.*2$f",tmpf,ve_view->z_precision);
		else
			label = g_strdup_printf("%i",(gint)tmpf);
		ve3d_draw_text(label,
				((ms_data[x_page][x_base])/ve_view->x_div),
				((ms_data[y_page][y_base+y_bincount-1])/ve_view->y_div),
				(gfloat)i/ve_view->z_div - ve_view->z_offset);
		g_free(label);
	}

}

/*!
 \brief ve3d_draw_text is called during rerender and draws the
 axis marker text
 */
void ve3d_draw_text(char* text, gfloat x, gfloat y, gfloat z)
{
	glColor3f(0.1,0.8,0.8);
	/* Set rendering postition */
	glRasterPos3f (x, y, z);
	/* Render each letter of text as stored in the display list */

	while(*text) 
	{
		glCallList(font_list_base+(*text));
		text++;
	};
}

/*!
 \brief ve3d_load_font_metrics is called during ve3d_realize and loads the 
 fonts needed by OpenGL for rendering the text
 */
void ve3d_load_font_metrics(void)
{
	PangoFontDescription *font_desc;
	PangoFont *font;
	PangoFontMetrics *font_metrics;
	gchar font_string[] = "sans 10";
	gint font_height;

	dbg_func(g_strdup(__FILE__": ve3d_load_font_metrics()\n"),OPENGL);

	font_list_base = (GLuint) glGenLists (128);
	font_desc = pango_font_description_from_string (font_string);
	font = gdk_gl_font_use_pango_font (font_desc, 0, 128, (int)font_list_base);
	if (font == NULL)
	{
		dbg_func(g_strdup_printf(__FILE__": ve3d_load_font_metrics()\n\tCan't load font '%s' CRITICAL FAILURE\n",font_string),CRITICAL);
		exit (-1);
	}
	font_metrics = pango_font_get_metrics (font, NULL);
	font_height = pango_font_metrics_get_ascent (font_metrics) +
		pango_font_metrics_get_descent (font_metrics);
	font_height = PANGO_PIXELS (font_height);
	pango_font_description_free (font_desc);
	pango_font_metrics_unref (font_metrics);
}

/*!
 \brief ve3d_key_press_event is called whenever the user hits a key on the 3D
 view. It looks for arrow keys, Plus/Minus and Pgup/PgDown.  Arrows move the
 red marker, +/- shift the value by 1 unit, Pgup/Pgdn shift the value by 10
 units
 */
EXPORT gboolean ve3d_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	gint offset = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	gint y_page = 0;
	gint x_page = 0;
	gint z_page = 0;
	gint y_base = 0;
	gint x_base = 0;
	gint z_base = 0;
	gint dload_val = 0;
	gboolean update_widgets = FALSE;
	struct Ve_View_3D *ve_view = NULL;
	extern gint **ms_data;
	extern struct Firmware_Details *firmware;
	ve_view = (struct Ve_View_3D *)g_object_get_data(
			G_OBJECT(widget),"ve_view");

	dbg_func(g_strdup(__FILE__": ve3d_key_press_event()\n"),OPENGL);

	x_bincount = ve_view->x_bincount;
	y_bincount = ve_view->y_bincount;

	x_base = ve_view->x_base;
	y_base = ve_view->y_base;
	z_base = ve_view->z_base;

	x_page = ve_view->x_page;
	y_page = ve_view->y_page;
	z_page = ve_view->z_page;

	// Spark requires a divide by 2.84 to convert from ms units to degrees

	switch (event->keyval)
	{
		case GDK_Up:
			dbg_func(g_strdup("\t\"UP\"\n"),OPENGL);

			if (ve_view->active_y < (y_bincount-1))
				ve_view->active_y += 1;
			break;

		case GDK_Down:
			dbg_func(g_strdup("\t\"DOWN\"\n"),OPENGL);

			if (ve_view->active_y > 0)
				ve_view->active_y -= 1;
			break;				

		case GDK_Left:
			dbg_func(g_strdup("\t\"LEFT\"\n"),OPENGL);

			if (ve_view->active_x > 0)
				ve_view->active_x -= 1;
			break;					

		case GDK_Right:
			dbg_func(g_strdup("\t\"RIGHT\"\n"),OPENGL);

			if (ve_view->active_x < (x_bincount-1))
				ve_view->active_x += 1;
			break;				

		case GDK_Page_Up:
			dbg_func(g_strdup("\t\"Page Up\"\n"),OPENGL);

			offset = z_base+(ve_view->active_y*y_bincount)+ve_view->active_x;
			if (ms_data[z_page][offset] <= 245)
			{
				dload_val = ms_data[z_page][offset] + 10;
				update_widgets = TRUE;
			}
			break;				
		case GDK_plus:
		case GDK_KP_Add:
		case GDK_KP_Equal:
		case GDK_equal:
			dbg_func(g_strdup("\t\"PLUS\"\n"),OPENGL);

			offset = z_base+(ve_view->active_y*y_bincount)+ve_view->active_x;
			if (ms_data[z_page][offset] < 255)
			{
				dload_val = ms_data[z_page][offset] + 1;
				update_widgets = TRUE;
			}
			break;				
		case GDK_Page_Down:
			dbg_func(g_strdup("\t\"Page Down\"\n"),OPENGL);

			offset = z_base+(ve_view->active_y*y_bincount)+ve_view->active_x;
			if (ms_data[z_page][offset] >= 10)
			{
				dload_val = ms_data[z_page][offset] - 10;
				update_widgets = TRUE;
			}
			break;							


		case GDK_minus:
		case GDK_KP_Subtract:
			dbg_func(g_strdup("\t\"MINUS\"\n"),OPENGL);

			offset = z_base+(ve_view->active_y*y_bincount)+ve_view->active_x;
			if (ms_data[z_page][offset] > 0)
			{
				dload_val = ms_data[z_page][offset] - 1;
				update_widgets = TRUE;
			}
			break;							

		default:
			dbg_func(g_strdup_printf(__FILE__": ve3d_key_press_event()\n\tKeypress not handled, code: %#.4X\"\n",event->keyval),OPENGL);
			return FALSE;
	}
	if (update_widgets)
	{
		write_ve_const(widget,z_page,offset,dload_val,firmware->page_params[z_page]->is_spark, TRUE);
	}

	gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);

	return TRUE;
}


/*!
 \brief initialize_ve3d_view is called from create_ve3d_view to intialize it's
 datastructure for use.  
 \see Ve_View
 */
struct Ve_View_3D * initialize_ve3d_view()
{
	struct Ve_View_3D *ve_view = NULL; 
	ve_view= g_new0(struct Ve_View_3D,1);
	ve_view->x_source = NULL;
	ve_view->y_source = NULL;
	ve_view->z_source = NULL;
	ve_view->x_suffix = NULL;
	ve_view->y_suffix = NULL;
	ve_view->z_suffix = NULL;
	ve_view->x_conv_expr = NULL;
	ve_view->y_conv_expr = NULL;
	ve_view->z_conv_expr = NULL;
	ve_view->x_eval = NULL;
	ve_view->y_eval = NULL;
	ve_view->z_eval = NULL;
	ve_view->x_disp_float = FALSE;
	ve_view->y_disp_float = FALSE;
	ve_view->z_disp_float = FALSE;
	ve_view->x_precision = 0;
	ve_view->y_precision = 0;
	ve_view->z_precision = 0;
	ve_view->beginX = 0;
	ve_view->beginY = 0;
	ve_view->dt = 0.008;
	ve_view->sphi = 35.0;
	ve_view->stheta = 75.0;
	ve_view->sdepth = 7.533;
	ve_view->zNear = 0.8;
	ve_view->zFar = 23.0;
	ve_view->aspect = 1.0;
	ve_view->z_scale = 4.0;
	ve_view->x_div = 0.0;
	ve_view->x_max = 0.0;
	ve_view->x_min = 0.0;
	ve_view->active_x = 0;
	ve_view->y_div = 0.0;
	ve_view->y_max = 0.0;
	ve_view->y_min = 0.0;
	ve_view->active_y = 0;
	ve_view->z_div = 0.0;
	ve_view->z_max = 0.0;
	ve_view->z_min = 0.0;
	ve_view->z_offset = 0;
	ve_view->x_page = 0;
	ve_view->y_page = 0;
	ve_view->z_page = 0;
	ve_view->x_base = 0;
	ve_view->y_base = 0;
	ve_view->z_base = 0;
	ve_view->x_bincount = 0;
	ve_view->y_bincount = 0;
	ve_view->table_name = NULL;
	return ve_view;
}

