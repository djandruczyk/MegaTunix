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
 *
 * Just about all of this was written by Richard Barrington, then rewritten
 * and extended by David Andruczyk
 * 
 * Large portions of this code are based on the examples provided with 
 * the GtkGlExt libraries.
 *
 * changelog
 * Ben Pierre 05/21/08
 * - wrote standard float rgb_from_hue() returns float triplet
 * - RGB3f color;
 * - uses rgb_from_hue(void)
 * - uses glColor3f() openGL native float color function
 * - uses rgb_from_hue(void)
 * - uses glColor3f() openGL native float color function
 * - set_shading_mode(void)
 * - drawFrameRate(void)
 *
 * David Andruczyk 6/24/08
 *  - drawFrameRate change to drawOrthoText
 */

/*!
  \file src/3d_vetable.c
  \ingroup SpecialStuff
  \brief Contains all the functions specific to the 3D table viewer/editor
  \author David Andruczyk
  \author Ben Pierre
  */

#include <3d_vetable.h>
#include <conversions.h>
#include <dashboard.h>
#include <debugging.h>
#include <gdk/gdkglglext.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <pango/pangoft2.h>
#include <gtk/gtkgl.h>
#include <gui_handlers.h>
#include <listmgmt.h>
#include <logviewer_gui.h>
#include <math.h>
#include <multi_expr_loader.h>
#include <plugin.h>
#include <rtv_processor.h>
#include <runtime_sliders.h>
#include <stdio.h>
#include <stdlib.h>
#include <tabloader.h>
#include <vetable_gui.h>
#include <widgetmgmt.h>
#ifdef _WIN32_
#include <windows.h>
#endif
#ifdef GDK_WINDOWING_QUARTZ
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define ONE_SECOND 	 1	/* one second */
#define DEFAULT_WIDTH  640
#define DEFAULT_HEIGHT 480                                                                                  
static GStaticMutex key_mutex = G_STATIC_MUTEX_INIT;
extern gconstpointer *global_data;

static gboolean delayed_expose(gpointer);
static gboolean delayed_reconfigure(gpointer);

/* New Font Stuff */
#ifdef _WIN32_
//static const char font_string[] = "Courier bold";
static const char font_string[] = "Sans";
#else
//static const char font_string[] = "Monospace";
static const char font_string[] = "Sans";
#endif

void gl_init(GtkWidget *);

/*!
  \brief Calculates the frames per second for the 3D display
  */
G_MODULE_EXPORT void CalculateFrameRate(GtkWidget *widget)
{
	Ve_View_3D *ve_view = NULL;
	GTimeVal  currentTime;
	ve_view = (Ve_View_3D*)OBJ_GET(widget,"ve_view");

	g_return_if_fail(ve_view);

	/* struct for the time value*/
	currentTime.tv_sec  = 0;
	currentTime.tv_usec = 0;

	/* gets the microseconds passed since app started*/
	g_get_current_time(&currentTime);

	/* Increase the frame counter*/
	ve_view->fps++;

	if (currentTime.tv_sec - ve_view->lasttime >= ONE_SECOND )
	{
		ve_view->lasttime = currentTime.tv_sec;
		/* Copy the frames per second into a string to display in the window*/
		sprintf(ve_view->strfps,_("Current Frames Per Second: %i"), (int)ve_view->fps);
		/* Reset the frames per second*/
		ve_view->fps = 0;
	}

	/* draw frame rate on screen */
	drawOrthoText(widget, ve_view->strfps, 1.0f, 1.0f, 1.0f, 0.025, 0.965 );
}

/*!
  \brief Draws simple 2d test on screen in orthographic projection
  \param str s the string to display
  \param r is the red value
  \param g is the green value
  \param b is the blue value
  \param x is the x coordinate
  \param y is the y coordinate
  */
G_MODULE_EXPORT void drawOrthoText(GtkWidget *widget, char *str, GLclampf r, GLclampf g, GLclampf b, GLfloat x, GLfloat y)
{
	GLint matrixMode;
	if (!str)
		return;

	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);  /* matrix mode? */

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_COLOR_BUFFER_BIT);       /* save current colour */
	glColor3f(r, g, b);
	glRasterPos3f(x, y, 0.0);
	gl_print_string(widget,str);
	glPopAttrib();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(matrixMode);
}


/*!
  \brief gets a color back from an angle passed in degrees.
  The degrees represent the arc around a color circle.
  \param hue (gfloat) degrees around the color circle
  \param sat (gfloat) col_sat from 0-1.0
  \param val (gfloat) col_val from 0-1.0
  \returns a RGB3f variable at the hue angle requested
  */
G_MODULE_EXPORT RGB3f rgb_from_hue(gfloat hue, gfloat sat, gfloat val)
{
	RGB3f color;
	gint i = 0;
	gfloat tmp = 0.0;
	gfloat fract = 0.0;
	gfloat S = sat;	/* using col_sat of 1.0*/
	gfloat V = val;	/* using Value of 1.0*/
	gfloat p = 0.0;
	gfloat q = 0.0;
	gfloat t = 0.0;
	gfloat r = 0.0;
	gfloat g = 0.0;
	gfloat b = 0.0;
	static GdkColormap *colormap = NULL;

	if (!colormap)
		colormap = gdk_colormap_get_system();

	while (hue > 360.0)
		hue -= 360.0;
        while (hue < 0.0)
                hue += 360.0;
	tmp = hue/60.0;
	i = floor(tmp);
	fract = tmp-i;

	p = V*(1.0-S);
	q = V*(1.0-(S*fract));
	t = V*(1.0-(S*(1.0-fract)));

	switch (i)
	{
		case 0:
			r = V;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = V;
			b = p;
			break;
		case 2:
			r = p;
			g = V;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = V;
			break;
		case 4:
			r = t;
			g = p;
			b = V;
			break;
		case 5:
			r = V;
			g = p;
			b = q;
			break;
	}
	color.red = r;
	color.green = g;
	color.blue = b;

	return (color);
}


/*!
  \brief create_ve3d_view does the initial work of creating the 3D vetable
  widget, it creates the datastructures, creates the window, initializes 
  OpenGL and binds all the handlers to the window that are needed.
  \param widget is the parent widget
  \param data is the unused
  \return TRUE on success
  */
G_MODULE_EXPORT gboolean create_ve3d_view(GtkWidget *widget, gpointer data)
{
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *scale;
	GtkWidget *drawing_area;
	gfloat *mult = NULL;
	gfloat *add = NULL;
	gfloat tmpf = 0.0;
	GdkGLConfig *gl_config;
	gchar * tmpbuf = NULL;
	gint i = 0;
	gint j = 0;
	gint smallstep = 0;
	gint bigstep = 0;
	Ve_View_3D *ve_view;
	GHashTable *ve_view_hash = NULL;
	extern gboolean gl_ability;
	Firmware_Details *firmware = NULL;
	gint table_num =  -1;

	if (!gl_ability)
	{
		dbg_func(CRITICAL,g_strdup(__FILE__": create_ve3d_view()\n\t GtkGLEXT Library initialization failed, no GL for you :(\n"));
		return FALSE;
	}
	firmware = DATA_GET(global_data,"firmware");
	tmpbuf = (gchar *)OBJ_GET(widget,"table_num");
	table_num = (GINT)g_ascii_strtod(tmpbuf,NULL);
	ve_view_hash = DATA_GET(global_data,"ve_view_hash");

	g_return_val_if_fail(ve_view_hash,FALSE);
	g_return_val_if_fail(firmware,FALSE);
	g_return_val_if_fail(tmpbuf,FALSE);

	if (g_hash_table_lookup(ve_view_hash,GINT_TO_POINTER(table_num)))
		return TRUE;

	ve_view = initialize_ve3d_view();
	if (firmware->table_params[table_num]->x_multi_source)
	{
		ve_view->x_multi_source = firmware->table_params[table_num]->x_multi_source;
		ve_view->x_source_key = firmware->table_params[table_num]->x_source_key;
		ve_view->x_multi_hash = firmware->table_params[table_num]->x_multi_hash;
	}
	else
	{
		ve_view->x_source =
			g_strdup(firmware->table_params[table_num]->x_source);
		ve_view->x_suffix =
			g_strdup(firmware->table_params[table_num]->x_suffix);
		ve_view->x_precision =
			firmware->table_params[table_num]->x_precision;
	}
	if (firmware->table_params[table_num]->y_multi_source)
	{
		ve_view->y_multi_source = firmware->table_params[table_num]->y_multi_source;
		ve_view->y_source_key = firmware->table_params[table_num]->y_source_key;
		ve_view->y_multi_hash = firmware->table_params[table_num]->y_multi_hash;
	}
	else
	{
		ve_view->y_source =
			g_strdup(firmware->table_params[table_num]->y_source);
		ve_view->y_suffix =
			g_strdup(firmware->table_params[table_num]->y_suffix);
		ve_view->y_precision =
			firmware->table_params[table_num]->y_precision;
	}
	if (firmware->table_params[table_num]->z_multi_source)
	{
		ve_view->z_multi_source = firmware->table_params[table_num]->z_multi_source;
		ve_view->z_source_key = firmware->table_params[table_num]->z_source_key;
		ve_view->z_multi_hash = firmware->table_params[table_num]->z_multi_hash;
	}
	else
	{
		ve_view->z_source =
			g_strdup(firmware->table_params[table_num]->z_source);
		ve_view->z_suffix =
			g_strdup(firmware->table_params[table_num]->z_suffix);
		ve_view->z_precision =
			firmware->table_params[table_num]->z_precision;
		/* Z axis lookuptable dependancies */
		if (firmware->table_params[table_num]->z_depend_on)
			ve_view->z_depend_on = firmware->table_params[table_num]->z_depend_on;
	}

	mult = firmware->table_params[table_num]->x_fromecu_mult;	
	add = firmware->table_params[table_num]->x_fromecu_add;	
	tmpf = pow(10.0,(double)-(ve_view->x_precision));
	if (mult)
	{
		smallstep = (GINT)(tmpf / (*mult));
		bigstep = (GINT)(((10*tmpf) /(*mult)));
	}
	else
	{
		smallstep = (GINT)tmpf;
		bigstep = (GINT)(10*tmpf);
	}
	ve_view->x_smallstep = smallstep < 1 ? 1:smallstep;
	ve_view->x_bigstep = bigstep < 10 ? 10:bigstep;
	mult = firmware->table_params[table_num]->y_fromecu_mult;	
	add = firmware->table_params[table_num]->y_fromecu_add;	
	tmpf = pow(10.0,(double)-(ve_view->y_precision));
	if (mult)
	{
		smallstep = (GINT)(tmpf / (*mult));
		bigstep = (GINT)(((10*tmpf) /(*mult)));
	}
	else
	{
		smallstep = (GINT)tmpf;
		bigstep = (GINT)(10*tmpf);
	}
	ve_view->y_smallstep = smallstep < 1 ? 1:smallstep;
	ve_view->y_bigstep = bigstep < 10 ? 10:bigstep;
	mult = firmware->table_params[table_num]->z_fromecu_mult;	
	add = firmware->table_params[table_num]->z_fromecu_add;	
	tmpf = pow(10.0,(double)-(ve_view->z_precision));
	if (mult)
	{
		smallstep = (GINT)(tmpf / (*mult));
		bigstep = (GINT)(((10*tmpf) /(*mult)));
	}
	else
	{
		smallstep = (GINT)tmpf;
		bigstep = (GINT)(10*tmpf);
	}
	ve_view->z_smallstep = smallstep < 1 ? 1:smallstep;
	ve_view->z_bigstep = bigstep < 10 ? 10:bigstep;
	ve_view->x_page = firmware->table_params[table_num]->x_page;
	ve_view->x_base = firmware->table_params[table_num]->x_base;
	ve_view->x_size = firmware->table_params[table_num]->x_size;
	ve_view->x_mult = get_multiplier(ve_view->x_size);
	ve_view->x_bincount = firmware->table_params[table_num]->x_bincount;
	OBJ_SET(ve_view->x_container,"page",GINT_TO_POINTER(ve_view->x_page));
	OBJ_SET(ve_view->x_container,"size",GINT_TO_POINTER(ve_view->x_size));
	OBJ_SET(ve_view->x_container,"canID",GINT_TO_POINTER(firmware->canID));

	ve_view->y_page = firmware->table_params[table_num]->y_page;
	ve_view->y_base = firmware->table_params[table_num]->y_base;
	ve_view->y_size = firmware->table_params[table_num]->y_size;
	ve_view->y_mult = get_multiplier(ve_view->y_size);
	ve_view->y_bincount = firmware->table_params[table_num]->y_bincount;
	OBJ_SET(ve_view->y_container,"page",GINT_TO_POINTER(ve_view->y_page));
	OBJ_SET(ve_view->y_container,"size",GINT_TO_POINTER(ve_view->y_size));
	OBJ_SET(ve_view->y_container,"canID",GINT_TO_POINTER(firmware->canID));

	ve_view->z_page = firmware->table_params[table_num]->z_page;
	ve_view->z_base = firmware->table_params[table_num]->z_base;
	ve_view->z_size = firmware->table_params[table_num]->z_size;
	OBJ_SET(ve_view->z_container,"page",GINT_TO_POINTER(ve_view->z_page));
	OBJ_SET(ve_view->z_container,"size",GINT_TO_POINTER(ve_view->z_size));
	OBJ_SET(ve_view->z_container,"canID",GINT_TO_POINTER(firmware->canID));
	ve_view->z_mult = get_multiplier(ve_view->z_size);

	ve_view->table_name =
		g_strdup(firmware->table_params[table_num]->table_name);
	ve_view->table_num = table_num;

	ve_view->x_objects = g_new0(GObject *, firmware->table_params[table_num]->x_bincount);
	for (i=0;i<firmware->table_params[table_num]->x_bincount;i++)
	{
		ve_view->x_objects[i] = g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref_sink(ve_view->x_objects[i]);
		OBJ_SET(ve_view->x_objects[i],"page",GINT_TO_POINTER(ve_view->x_page));
		OBJ_SET(ve_view->x_objects[i],"offset",GINT_TO_POINTER(ve_view->x_base+(ve_view->x_mult * i)));
		OBJ_SET(ve_view->x_objects[i],"size",GINT_TO_POINTER(ve_view->x_size));
		OBJ_SET(ve_view->x_objects[i],"fromecu_mult",firmware->table_params[table_num]->x_fromecu_mult);
		OBJ_SET(ve_view->x_objects[i],"fromecu_add",firmware->table_params[table_num]->x_fromecu_add);
		OBJ_SET_FULL(ve_view->x_objects[i],"table_num",g_strdup_printf("%i",table_num),g_free);
		if (firmware->table_params[table_num]->x_multi_source)
		{
			OBJ_SET_FULL(ve_view->x_objects[i],"source_key",g_strdup(firmware->table_params[table_num]->x_source_key),g_free);
			OBJ_SET_FULL(ve_view->x_objects[i],"multi_expr_keys",g_strdup(firmware->table_params[table_num]->x_multi_expr_keys),g_free);
			OBJ_SET_FULL(ve_view->x_objects[i],"fromecu_mults",g_strdup(firmware->table_params[table_num]->x_fromecu_mults),g_free);
			OBJ_SET_FULL(ve_view->x_objects[i],"fromecu_adds",g_strdup(firmware->table_params[table_num]->x_fromecu_adds),g_free);
		}
	}
	ve_view->y_objects = g_new0(GObject *, firmware->table_params[table_num]->y_bincount);
	for (i=0;i<firmware->table_params[table_num]->y_bincount;i++)
	{
		ve_view->y_objects[i] = g_object_new(GTK_TYPE_INVISIBLE,NULL);
		g_object_ref_sink(ve_view->y_objects[i]);
		OBJ_SET(ve_view->y_objects[i],"page",GINT_TO_POINTER(ve_view->y_page));
		OBJ_SET(ve_view->y_objects[i],"offset",GINT_TO_POINTER(ve_view->y_base+(ve_view->y_mult * i)));
		OBJ_SET(ve_view->y_objects[i],"size",GINT_TO_POINTER(ve_view->y_size));
		OBJ_SET(ve_view->y_objects[i],"fromecu_mult",firmware->table_params[table_num]->y_fromecu_mult);
		OBJ_SET(ve_view->y_objects[i],"fromecu_add",firmware->table_params[table_num]->y_fromecu_add);
		OBJ_SET_FULL(ve_view->y_objects[i],"table_num",g_strdup_printf("%i",table_num),g_free);
		if (firmware->table_params[table_num]->y_multi_source)
		{
			OBJ_SET_FULL(ve_view->y_objects[i],"source_key",g_strdup(firmware->table_params[table_num]->y_source_key),g_free);
			OBJ_SET_FULL(ve_view->y_objects[i],"multi_expr_keys",g_strdup(firmware->table_params[table_num]->y_multi_expr_keys),g_free);
			OBJ_SET_FULL(ve_view->y_objects[i],"fromecu_mults",g_strdup(firmware->table_params[table_num]->y_fromecu_mults),g_free);
			OBJ_SET_FULL(ve_view->y_objects[i],"fromecu_adds",g_strdup(firmware->table_params[table_num]->y_fromecu_adds),g_free);
		}
	}

	ve_view->z_objects = g_new0(GObject **, firmware->table_params[table_num]->x_bincount);
	ve_view->quad_mesh = g_new0(Quad **, firmware->table_params[table_num]->x_bincount);
	for (i=0;i<firmware->table_params[table_num]->x_bincount;i++)
	{
		ve_view->quad_mesh[i] = g_new0(Quad *,firmware->table_params[table_num]->y_bincount);
		ve_view->z_objects[i] = g_new0(GObject *,firmware->table_params[table_num]->y_bincount);
		for (j=0;j<firmware->table_params[table_num]->y_bincount;j++)
		{
			ve_view->quad_mesh[i][j] = g_new0(Quad, 1);
			ve_view->z_objects[i][j] = g_object_new(GTK_TYPE_INVISIBLE,NULL);
			g_object_ref_sink(ve_view->z_objects[i][j]);
			OBJ_SET(ve_view->z_objects[i][j],"page",GINT_TO_POINTER(ve_view->z_page));
			if (firmware->capabilities & PIS)
				OBJ_SET(ve_view->z_objects[i][j],"offset",GINT_TO_POINTER(ve_view->z_base+(ve_view->z_mult * ((i*ve_view->y_bincount)+j))));
			else
				OBJ_SET(ve_view->z_objects[i][j],"offset",GINT_TO_POINTER(ve_view->z_base+(ve_view->z_mult * ((j*ve_view->y_bincount)+i))));
			OBJ_SET(ve_view->z_objects[i][j],"size",GINT_TO_POINTER(ve_view->z_size));
			OBJ_SET(ve_view->z_objects[i][j],"fromecu_mult",firmware->table_params[table_num]->z_fromecu_mult);
			OBJ_SET(ve_view->z_objects[i][j],"fromecu_add",firmware->table_params[table_num]->z_fromecu_add);
			OBJ_SET_FULL(ve_view->z_objects[i][j],"table_num",g_strdup_printf("%i",table_num),g_free);
			if (firmware->table_params[table_num]->z_multi_source)
			{
				OBJ_SET_FULL(ve_view->z_objects[i][j],"source_key",g_strdup(firmware->table_params[table_num]->z_source_key),g_free);
				OBJ_SET_FULL(ve_view->z_objects[i][j],"multi_expr_keys",g_strdup(firmware->table_params[table_num]->z_multi_expr_keys),g_free);
				OBJ_SET_FULL(ve_view->z_objects[i][j],"fromecu_mults",g_strdup(firmware->table_params[table_num]->z_fromecu_mults),g_free);
				OBJ_SET_FULL(ve_view->z_objects[i][j],"fromecu_adds",g_strdup(firmware->table_params[table_num]->z_fromecu_adds),g_free);
			}
			if (firmware->table_params[table_num]->z_depend_on)
			{
				OBJ_SET(ve_view->z_objects[i][j],"lookuptable",OBJ_GET(firmware->table_params[table_num]->z_object,"lookuptable"));
				OBJ_SET(ve_view->z_objects[i][j],"alt_lookuptable",OBJ_GET(firmware->table_params[table_num]->z_object,"alt_lookuptable"));
				OBJ_SET(ve_view->z_objects[i][j],"dep_object",OBJ_GET(firmware->table_params[table_num]->z_object,"dep_object"));
			}
		}
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _(ve_view->table_name));
	gtk_widget_set_size_request(window,DEFAULT_WIDTH,DEFAULT_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER(window),0);
	ve_view->window = window;
	OBJ_SET(window,"ve_view",(gpointer)ve_view);
	if  (firmware->table_params[table_num]->bind_to_list)
	{
		OBJ_SET_FULL(window,"bind_to_list", g_strdup(firmware->table_params[table_num]->bind_to_list),g_free);
		OBJ_SET(window,"match_type", GINT_TO_POINTER(firmware->table_params[table_num]->match_type));
		bind_to_lists(window,firmware->table_params[table_num]->bind_to_list);
	}

	/* Store it in ve_view_hash by table_num */
	g_hash_table_insert(ve_view_hash,GINT_TO_POINTER(table_num), ve_view);

	g_signal_connect(G_OBJECT(window), "delete_event",
			G_CALLBACK(ve3d_shutdown),
			NULL);
	gtk_widget_show(window);

	gl_config = get_gl_config();

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(window),vbox);

	hbox = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	frame = gtk_frame_new("3D Table display");
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	drawing_area = gtk_drawing_area_new();
	OBJ_SET(drawing_area,"ve_view",(gpointer)ve_view);
	ve_view->drawing_area = drawing_area;

	gtk_widget_set_can_focus(drawing_area,TRUE);
	gtk_widget_add_events (drawing_area,
			GDK_BUTTON_MOTION_MASK	|
			GDK_BUTTON_PRESS_MASK   |
			GDK_KEY_PRESS_MASK      |
			GDK_KEY_RELEASE_MASK    |
			GDK_VISIBILITY_NOTIFY_MASK);

	/* Connect signal handlers to the drawing area */
	g_signal_connect(G_OBJECT (drawing_area), "expose_event",
			G_CALLBACK (ve3d_expose_event), NULL);
	//g_signal_connect_after(G_OBJECT (drawing_area), "realize",
	g_signal_connect(G_OBJECT (drawing_area), "realize",
			G_CALLBACK (ve3d_realize), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "configure_event",
			G_CALLBACK (ve3d_configure_event), NULL);

	g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",
			G_CALLBACK (ve3d_motion_notify_event), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "button_press_event",
			G_CALLBACK (ve3d_button_press_event), NULL);
	g_signal_connect(G_OBJECT (drawing_area), "key_press_event",
			G_CALLBACK (ve3d_key_press_event), NULL);

	gtk_widget_set_gl_capability(drawing_area, gl_config, NULL,
			TRUE, GDK_GL_RGBA_TYPE);

	gtk_container_add(GTK_CONTAINER(frame),drawing_area);

	/* End of GL window, Now controls for it.... */
	frame = gtk_frame_new("3D Display Controls");
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);

	button = gtk_button_new_with_label("Reset Display");
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
	OBJ_SET(button,"ve_view",(gpointer)ve_view);
	g_signal_connect_swapped(G_OBJECT (button), "clicked",
			G_CALLBACK (reset_3d_view), (gpointer)button);

	button = gtk_button_new_with_label("Get Data from ECU");

	OBJ_SET(button,"handler",
			GINT_TO_POINTER(READ_VE_CONST));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	gtk_widget_set_tooltip_text(button,"Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI");

	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);


	button = gtk_button_new_with_label("Burn to ECU");

	OBJ_SET(button,"handler",
			GINT_TO_POINTER(BURN_MS_FLASH));
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			NULL);
	ve_view->burn_but = button;
	store_list("burners",g_list_prepend(get_list("burners"),(gpointer)button));

	gtk_widget_set_tooltip_text(button,"Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.");

	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Start Reading RT Vars");

	OBJ_SET(button,"handler",GINT_TO_POINTER(START_REALTIME));

	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Stop Reading RT vars");

	OBJ_SET(button,"handler",GINT_TO_POINTER(STOP_REALTIME));

	g_signal_connect(G_OBJECT (button), "clicked",
			G_CALLBACK (std_button_handler),
			NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);


	button = gtk_button_new_with_label("Close Window");
	OBJ_SET(button,"ve_view",ve_view);
	gtk_box_pack_end(GTK_BOX(vbox2),button,FALSE,FALSE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
			G_CALLBACK(ve3d_shutdown),
			(gpointer)window);

	/* Focus follows vertex toggle */
	button = gtk_check_button_new_with_label("Focus Follows Vertex\n with most Weight");
	ve_view->tracking_button = button;
	OBJ_SET(button,"ve_view",ve_view);
	gtk_box_pack_end(GTK_BOX(vbox2),button,FALSE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_tracking_focus),
			NULL);

	/* Table for rendering mode */
	table = gtk_table_new(2,2,TRUE);
	gtk_box_pack_end(GTK_BOX(vbox2),table,FALSE,FALSE,0);
	label = gtk_label_new("Rendering Mode");
	gtk_table_attach(GTK_TABLE(table),label,0,2,0,1,0,0,0,0);
	/* Wireframe toggle */
	button = gtk_radio_button_new_with_label(NULL,"Wireframe");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),ve_view->wireframe);
	OBJ_SET(button,"ve_view",ve_view);
	gtk_table_attach(GTK_TABLE(table),button,0,1,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_rendering_mode),
			NULL);

	/* Solid toggle */
	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),"Solid");
	OBJ_SET(button,"ve_view",ve_view);
	gtk_table_attach(GTK_TABLE(table),button,1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);


	/* Table for Scaling mode */
	table = gtk_table_new(2,2,TRUE);
	gtk_box_pack_end(GTK_BOX(vbox2),table,FALSE,FALSE,0);
	label = gtk_label_new("Axis Scale");
	gtk_table_attach(GTK_TABLE(table),label,0,2,0,1,0,0,0,0);

	/* Fixed Scale toggle */
	button = gtk_radio_button_new_with_label(NULL,"Fixed");
	OBJ_SET(button,"ve_view",ve_view);
	gtk_table_attach(GTK_TABLE(table),button,0,1,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_scaling_mode),
			NULL);

	/* Prop Scale toggle */
	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),"Proportional");

	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),!ve_view->fixed_scale);
	OBJ_SET(button,"ve_view",ve_view);
	gtk_table_attach(GTK_TABLE(table),button,1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);

	/* Opacity Slider */
	scale = gtk_hscale_new_with_range(0.1,1.0,0.001);
	OBJ_SET(scale,"ve_view",ve_view);
	g_signal_connect(G_OBJECT(scale), "value_changed",
			G_CALLBACK(set_opacity),
			NULL);
	gtk_range_set_value(GTK_RANGE(scale),ve_view->opacity);
	gtk_scale_set_draw_value(GTK_SCALE(scale),FALSE);
	gtk_box_pack_end(GTK_BOX(vbox2),scale,FALSE,TRUE,0);

	label = gtk_label_new("Opacity");
	gtk_box_pack_end(GTK_BOX(vbox2),label,FALSE,TRUE,0);

	/* Realtime var slider gauges */
	frame = gtk_frame_new("Real-Time Variables");
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame),0);

	hbox = gtk_hbox_new(TRUE,5);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	tmpbuf = g_strdup_printf("ve3d_rt_table0_%i",table_num);
	register_widget(tmpbuf,table);
	g_free(tmpbuf);

	table = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(hbox),table,TRUE,TRUE,0);

	tmpbuf = g_strdup_printf("ve3d_rt_table1_%i",table_num);
	register_widget(tmpbuf,table);
	g_free(tmpbuf);

	load_ve3d_sliders(table_num);

	gtk_widget_show_all(window);

	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	gdk_threads_add_timeout(500,delayed_reconfigure,ve_view);
	return TRUE;
}


/*!
  \brief ve3d_shutdown is called on close of the 3D vetable viewer/editor, it
  deallocates memory, disconencts handlers and then the widget is deleted 
  with gtk_widget_destroy
  \param widget is the widget to be destroyed
  \param data is the unused
  \returns FALSE so other handlers run
  */
G_MODULE_EXPORT gboolean ve3d_shutdown(GtkWidget *widget, gpointer data)
{
	GHashTable *ve_view_hash = NULL;
	GdkWindow *window = NULL;
	Ve_View_3D *ve_view;
	ve_view = (Ve_View_3D*)OBJ_GET(widget,"ve_view");
	ve_view_hash = DATA_GET(global_data,"ve_view_hash");
			
	g_return_val_if_fail(ve_view,FALSE);
	g_return_val_if_fail(ve_view_hash,FALSE);
	//printf("ve3d_shutdown, ve_view ptr is %p\n",ve_view);

	store_list("burners",g_list_remove(
				get_list("burners"),(gpointer)ve_view->burn_but));
	g_hash_table_remove(ve_view_hash,GINT_TO_POINTER(ve_view->table_num));
	free_ve3d_sliders(ve_view->table_num);
	if (ve_view->drawing_area)
	{
		window = gtk_widget_get_window(ve_view->drawing_area);
		gdk_window_unset_gl_capability(window);
		gtk_widget_destroy(ve_view->drawing_area);
	}
	gtk_widget_destroy(widget);
	g_free(ve_view->x_source);
	g_free(ve_view->y_source);
	g_free(ve_view->z_source);
	g_object_unref(ve_view->x_container);
	g_object_unref(ve_view->y_container);
	g_object_unref(ve_view->z_container);
	free(ve_view);/* free up the memory */
	ve_view = NULL;
	
	//printf("ve3d_shutdown complete, ve_view ptr is %p\n",ve_view);
	/* MUST return false otherwise other handlers won't run*/

	return FALSE;  
}



/*!
  \brief reset_3d_view resets the OpenGL widget to default position in
  case the user moves it or places it off the edge of the window and 
  can't find it...
  \param widget is the the container of the view
  */
G_MODULE_EXPORT void reset_3d_view(GtkWidget * widget)
{
	Ve_View_3D *ve_view;
	ve_view = (Ve_View_3D *)OBJ_GET(widget,"ve_view");
	ve_view->active_y = 0;
	ve_view->active_x = 0;
	ve_view->sphi = 45;
	ve_view->stheta = -63.75;
	ve_view->sdepth = -1.5;
	ve_view->zNear = 0;
	ve_view->zFar = 1.0;
	ve_view->aspect = 1.0;
	ve_view->h_strafe = -0.5;
	ve_view->v_strafe = -0.5;
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	ve_view->mesh_created = FALSE;
	ve3d_configure_event(ve_view->drawing_area, NULL,NULL);
	gdk_threads_add_timeout(500,delayed_expose,ve_view);
}


/*!
  \brief get_gl_config gets the OpenGL mode creates a GL config and returns it
  \returns pointer to a GdkGLConfig structure
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
		dbg_func(CRITICAL,g_strdup(__FILE__": get_gl_config()\n\t*** Cannot find the double-buffered visual.\n\t*** Trying single-buffered visual.\n"));

		/* Try single-buffered visual */
		gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
				GDK_GL_MODE_DEPTH);
		if (gl_config == NULL)
		{
			/* Should make a non-GL basic drawing area version
			   instead of dumping the user out of here, or at least 
			   render a non-GL found text on the drawing area */
			dbg_func(CRITICAL,g_strdup(__FILE__": get_gl_config()\n\t*** No appropriate OpenGL-capable visual found. EXITING!!\n"));
			exit (-1);
		}
	}
	return gl_config;
}


/*!
  \brief ve3d_configure_event is called when the window needs to be 
  drawn or redrawn after a resize. 
  \param widget is the pointer to the drawingarea
  \param event is the the Configure Event Structure
  \param data is unused
  \returns TRUE on completion, false if we couldn't get a GL drawable
  */
G_MODULE_EXPORT gboolean ve3d_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	Ve_View_3D *ve_view = NULL;
	GdkGLContext *glcontext = NULL;
	GdkGLDrawable *gldrawable = NULL;
	GtkAllocation allocation;
	GLfloat w;
	GLfloat h;

	gtk_widget_get_allocation(widget,&allocation);
	w = allocation.width;
	h = allocation.height;

	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_configure_event() Entered\n"));
	ve_view = (Ve_View_3D *)OBJ_GET(widget,"ve_view");
	if (ve_view->font_created)
		gl_destroy_font(widget);
	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);
	if (!glcontext)
		return TRUE;
	if (!gldrawable)
		return TRUE;

	dbg_func(OPENGL,g_strdup_printf(__FILE__": ve3d_configure_event() W %f, H %f\n",w,h));

	/*** OpenGL BEGIN ***/
	gdk_gl_drawable_gl_begin (gldrawable, glcontext);
	ve_view->aspect = 1.0;
	glViewport (0, 0, w, h);
	glMatrixMode(GL_MODELVIEW);
	gl_create_font(widget);
	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_configure_event() Complete!\n"));
	return TRUE;
}


/*!
  \brief ve3d_expose_event is called when the part or all of the GL area
  needs to be redrawn due to being "exposed" (uncovered), this kicks off 
  all the other renderers for updating the axis and status indicators. 
  \param widget is a pointer to the drawingarea for the view
  \param event is a pointer to the Expose Event structure
  \param data is unused
  \returns TRUE on success, FALSE if not realized or no GLdrawable
  */
G_MODULE_EXPORT gboolean ve3d_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GdkGLContext *glcontext = NULL;
	GdkGLDrawable *gldrawable = NULL;
	Ve_View_3D *ve_view = NULL;
	Cur_Vals *cur_vals = NULL;
	ve_view = (Ve_View_3D *)OBJ_GET(widget,"ve_view");

	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);

	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_expose_event() 3D View Expose Event\n"));
	g_return_val_if_fail(ve_view,FALSE);
	g_return_val_if_fail(glcontext,FALSE);
	g_return_val_if_fail(gldrawable,FALSE);

	if (!ve_view->gl_initialized)
	{
		ve_view->gl_initialized = TRUE;
		gl_init(widget);
	}

	/*** OpenGL BEGIN ***/
	gdk_gl_drawable_gl_begin (gldrawable, glcontext);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, 1.0, 0.1, 4);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* move model away from camera */
	glTranslatef(0,0.1875,ve_view->sdepth);

	/* Rotate around x/Y axis by sphi/stheta */
	glRotatef(ve_view->stheta, 1.0, 0.0, 0.0);
	glRotatef(ve_view->sphi, 0.0, 0.0, 1.0);

	/* Shift to middle of the screen or something like that */
	glTranslatef (-0.5, -0.5, -0.5);

	/* Draw everything */
	if (gtk_widget_is_sensitive(widget))
	{
		cur_vals = get_current_values(ve_view);
		ve3d_calculate_scaling(ve_view,cur_vals);
		if (!ve_view->mesh_created)
			generate_quad_mesh(ve_view,cur_vals);
		if (!DATA_GET(global_data,"offline"))
			ve3d_draw_runtime_indicator(ve_view,cur_vals);
		ve3d_draw_edit_indicator(ve_view,cur_vals);
		ve3d_draw_active_vertexes_marker(ve_view,cur_vals);
		ve3d_draw_ve_grid(ve_view,cur_vals);
		ve3d_draw_axis(ve_view,cur_vals);
		free_current_values(cur_vals);
		CalculateFrameRate(ve_view->drawing_area);
		/* Grey things out if we're supposed to be insensitive */
	}
	else
		ve3d_grey_window(ve_view);

	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);
	else
		glFlush ();

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_expose_event() Completed!\n"));
	return TRUE;
}


/*!
  \brief ve3d_motion_notify_event is called when the user moves
  the mouse inside the GL window, it causes the display to be 
  rotated/scaled/strafed depending on which button the user had held down.
  \param widget is the container of the view
  \param event is a pointer to the GdkEventMotion Structure
  \param data is unused
  \returns TRUE
  \see ve3d_button_press_event
  */
G_MODULE_EXPORT gboolean ve3d_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	Ve_View_3D *ve_view;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	ve_view = (Ve_View_3D *)OBJ_GET(widget,"ve_view");
	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
	window = gtk_widget_get_window(ve_view->drawing_area);

	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_motion_notify() 3D View Motion Notify\n"));
	/*printf("motion notify event\n");*/

	/* Left Button */
	if (event->state & GDK_BUTTON1_MASK)
	{
		ve_view->sphi += (gfloat)(event->x - ve_view->beginX) / 4.0;
		ve_view->stheta += (gfloat)(event->y - ve_view->beginY) / 4.0;
	}
	/* Middle button (or both buttons for two button mice) */
	if (event->state & GDK_BUTTON2_MASK)
	{
		ve_view->h_strafe -= (gfloat)(event->x - ve_view->beginX)/300.0;
		ve_view->v_strafe += (gfloat)(event->y - ve_view->beginY)/300.0;
	}
	/* Right Button */
	if (event->state & GDK_BUTTON3_MASK)
	{
		ve_view->sdepth -= (event->y - ve_view->beginY)/(allocation.height);
	}

	ve_view->beginX = event->x;
	ve_view->beginY = event->y;

	gdk_window_invalidate_rect (window,&allocation, FALSE);

	return TRUE;
}


/*!
  \brief ve3d_button_press_event is called when the user clicks a mouse 
  button The function grabs the location at which the button was clicked in 
  order to calculate what to change when rerendering
  \param widget is a pointer to the container of the view
  \param event is a pointer to the GdkEventButton Structure
  \param data is unused
  \return TRUE if a button is clicked, FALSE otherwise
  \see ve3d_motion_notify_event
  */
G_MODULE_EXPORT gboolean ve3d_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Ve_View_3D *ve_view;
	ve_view = (Ve_View_3D *)OBJ_GET(widget,"ve_view");
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_button_press_event()\n"));
	/*printf("button press event\n");*/

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
  \brief ve3d_realize is called when the window is created and sets the 
  main OpenGL parameters of the window (this only needs to be done once)
  \param widget is the drawingarea
  \param data is unused
  */
G_MODULE_EXPORT gint ve3d_realize (GtkWidget *widget, gpointer data)
{
	GdkWindow *window = gtk_widget_get_window(widget);
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_realize() 3D View Realization\n"));
	gdk_window_ensure_native(window);
	return TRUE;
}


void gl_init(GtkWidget *widget)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	g_return_if_fail(glcontext);
	g_return_if_fail(gldrawable);

	/*! Stuff we used to do in the realize handler... */
	/*** OpenGL BEGIN ***/
	gdk_gl_drawable_gl_begin (gldrawable, glcontext);

//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(65.0, 1.0, 0.1, 4);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();

	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glEnable (GL_LINE_SMOOTH);
	/*glEnable (GL_POLYGON_SMOOTH);*/
	glEnable (GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode(GL_MODELVIEW);
	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
}


/*!
  \brief When the display should be insensitive, this greys it out
  \param ve_view is the pointer to the current view
  */
G_MODULE_EXPORT void ve3d_grey_window(Ve_View_3D *ve_view)
{
	GdkPixmap *pmap = NULL;
	cairo_t * cr = NULL;
	GtkWidget * widget = ve_view->drawing_area;
	GdkWindow *window = gtk_widget_get_window(widget);
	GtkAllocation allocation;

	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);

	/* Not sure how to "grey" the window to make it appear insensitve */
	pmap=gdk_pixmap_new(window,
			allocation.width,
			allocation.height,
			gtk_widget_get_visual(widget)->depth);
	cr = gdk_cairo_create (pmap);
	cairo_set_source_rgba (cr, 0.3,0.3,0.3,0.5);
	cairo_rectangle (cr,
			0,0, 
			allocation.width, 
			allocation.height);
	cairo_fill(cr);
	cairo_destroy(cr);
	g_object_unref(pmap);
}


/*!
  \brief ve3d_calculate_scaling is called during a redraw to recalculate 
  the dimensions for the scales to make thing look pretty
  \param ve_view is a pointer to the view
  \param cur_val is a pointer to structure of current values for this iteration
  */
G_MODULE_EXPORT void ve3d_calculate_scaling(Ve_View_3D *ve_view, Cur_Vals *cur_val)
{
	gint i=0;
	gint j=0;
	gfloat min = 0.0;
	gfloat max = 0.0;
	gfloat tmpf = 0.0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_calculate_scaling()\n"));
	/*printf("CALC Scaling\n");*/

	min = 65535;
	max = 0;

	for (i=0;i<ve_view->x_bincount;i++)
	{
		tmpf = convert_after_upload((GtkWidget *)ve_view->x_objects[i]);
		if (tmpf > max)
			max = tmpf;
		if (tmpf < min)
			min = tmpf;
	}
	ve_view->x_trans = min;
	ve_view->x_max = max;
	ve_view->x_scale = 1.0/(max-min);
	min = 65535;
	max = 0;
	for (i=0;i<ve_view->y_bincount;i++)
	{
		tmpf = convert_after_upload((GtkWidget *)ve_view->y_objects[i]);
		if (tmpf > max)
			max = tmpf;
		if (tmpf < min)
			min = tmpf;
	}
	ve_view->y_trans = min;
	ve_view->y_max = max;
	ve_view->y_scale = 1.0/(max-min);
	min = 65535;
	max = 0;

	for (i=0;i<ve_view->x_bincount;i++)
	{
		for (j=0;j<ve_view->y_bincount;j++)
		{
			tmpf = convert_after_upload((GtkWidget *)ve_view->z_objects[i][j]);
			if (tmpf > max)
				max = tmpf;
			if (tmpf < min)
				min = tmpf;
		}
	}
	if (min == max)
	{
		min-=1;
		max+=1;
	}
	ve_view->z_trans = min-((max-min)*0.15);
	ve_view->z_max = max;
	ve_view->z_scale = 1.0/((max-min)/0.75);
	ve_view->z_offset = 0.0;
}

/*!
  \brief ve3d_draw_ve_grid is called during rerender and draws the 
  VEtable grid in 3D space
  \param ve_view is a pointer to the view
  \param cur_val is a pointer to structure of current values for this iteration
  */
G_MODULE_EXPORT void ve3d_draw_ve_grid(Ve_View_3D *ve_view, Cur_Vals *cur_val)
{
	gint x = 0;
	gint y = 0;
	Quad * quad = NULL;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_draw_ve_grid() \n"));

	/*printf("draw grid\n");*/

	/* Switch between solid and wireframe */
	if (ve_view->wireframe)
	{
		glLineWidth(1.75);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glLineWidth(1.25);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	/* Draw QUAD MESH */
	//for(y=0;y<ve_view->y_bincount-1;++y)
	for(y=ve_view->y_bincount-1;y >= 0;y--)
	{
		glBegin(GL_QUADS);
		for(x=ve_view->x_bincount-1;x >= 0;x--)
		{
			quad = ve_view->quad_mesh[x][y];
			/* (0x,0y) */
			glColor4f(quad->color[0].red, quad->color[0].green, quad->color[0].blue, ve_view->opacity);
			glVertex3f(quad->x[0],quad->y[0],quad->z[0]);
			/* (1x,0y) */
			glColor4f(quad->color[1].red, quad->color[1].green, quad->color[1].blue, ve_view->opacity);
			glVertex3f(quad->x[1],quad->y[1],quad->z[1]);
			/* (1x,1y) */
			glColor4f(quad->color[2].red, quad->color[2].green, quad->color[2].blue, ve_view->opacity);
			glVertex3f(quad->x[2],quad->y[2],quad->z[2]);
			/* (0x,1y) */
			glColor4f(quad->color[3].red, quad->color[3].green, quad->color[3].blue, ve_view->opacity);
			glVertex3f(quad->x[3],quad->y[3],quad->z[3]);
		}
		glEnd();
	}
	if (!ve_view->wireframe)  /* render black grid on main grid */
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for(y=ve_view->y_bincount-1;y>=0;y--)
		{
			glBegin(GL_QUADS);
			for(x=ve_view->x_bincount-1;x>=0;x--)
			{
				glColor4f(0,0,0, 1);
				quad = ve_view->quad_mesh[x][y];
				/* (0x,0y) */
				glVertex3f(quad->x[0],quad->y[0],quad->z[0]+0.001);
				/* (1x,0y) */
				glVertex3f(quad->x[1],quad->y[1],quad->z[1]+0.001);
				/* (1x,1y) */
				glVertex3f(quad->x[2],quad->y[2],quad->z[2]+0.001);
				/* (0x,1y) */
				glVertex3f(quad->x[3],quad->y[3],quad->z[3]+0.001);
			}
			glEnd();
			glBegin(GL_QUADS);
			for(x=ve_view->x_bincount-1;x>=0;x--)
			{
				glColor4f(0,0,0, 1);
				quad = ve_view->quad_mesh[x][y];
				/* (0x,0y) */
				glVertex3f(quad->x[0],quad->y[0],quad->z[0]-0.001);
				/* (1x,0y) */
				glVertex3f(quad->x[1],quad->y[1],quad->z[1]-0.001);
				/* (1x,1y) */
				glVertex3f(quad->x[2],quad->y[2],quad->z[2]-0.001);
				/* (0x,1y) */
				glVertex3f(quad->x[3],quad->y[3],quad->z[3]-0.001);
			}
			glEnd();
		}
	}

}


/*!
  \brief ve3d_draw_edit_indicator is called during rerender and draws 
  the red dot which tells where changes will be made to the table by the 
  user.  The user moves this with the arrow keys..
  \param ve_view is a pointer to the view
  \param cur_val is a pointer to structure of current values for this iteration
  */
G_MODULE_EXPORT void ve3d_draw_edit_indicator(Ve_View_3D *ve_view, Cur_Vals *cur_val)
{
	gfloat bottom = 0.0;
	GLfloat w = 0.0;
	GLfloat h = 0.0;
	GtkAllocation allocation;
	Firmware_Details *firmware = NULL;

	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
	w = allocation.width;
	h = allocation.height;
	firmware = DATA_GET(global_data,"firmware");
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_draw_edit_indicator()\n"));
	/*printf("draw edit indicator\n");*/
	
	drawOrthoText(ve_view->drawing_area,cur_val->x_edit_text, 1.0f, 0.2f, 0.2f, 0.025, 0.2 );
	drawOrthoText(ve_view->drawing_area,cur_val->y_edit_text, 1.0f, 0.2f, 0.2f, 0.025, 0.233 );
	drawOrthoText(ve_view->drawing_area,cur_val->z_edit_text, 1.0f, 0.2f, 0.2f, 0.025, 0.266 );
	drawOrthoText(ve_view->drawing_area,"Edit Position", 1.0f, 0.2f, 0.2f, 0.025, 0.300 );

	/* Render a red dot at the active VE map position */
	glPointSize(MIN(w,h)/55.0);
	glLineWidth(MIN(w,h)/300.0);
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_POINTS);

	if (ve_view->fixed_scale)
		glVertex3f(
				(gfloat)ve_view->active_x/((gfloat)ve_view->x_bincount-1.0),
				(gfloat)ve_view->active_y/((gfloat)ve_view->y_bincount-1.0),
				cur_val->z_edit_value);
	else
		glVertex3f(
				cur_val->x_edit_value,
				cur_val->y_edit_value,
				cur_val->z_edit_value);

	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor3f(1.0,0.0,0.0);

	if (ve_view->fixed_scale)
	{
		glVertex3f(
				(gfloat)ve_view->active_x/((gfloat)ve_view->x_bincount-1.0),
				(gfloat)ve_view->active_y/((gfloat)ve_view->y_bincount-1.0),
				cur_val->z_edit_value);
		glVertex3f(
				(gfloat)ve_view->active_x/((gfloat)ve_view->x_bincount-1.0),
				(gfloat)ve_view->active_y/((gfloat)ve_view->y_bincount-1.0),
				bottom);
		glVertex3f(
				0.0,
				(gfloat)ve_view->active_y/((gfloat)ve_view->y_bincount-1.0),
				bottom);
	}
	else
	{
		glVertex3f(
				cur_val->x_edit_value,
				cur_val->y_edit_value,
				cur_val->z_edit_value);
		glVertex3f(
				cur_val->x_edit_value,
				cur_val->y_edit_value,
				bottom);
		glVertex3f(
				0.0,
				cur_val->y_edit_value,
				bottom);
	}

	glEnd();

	glBegin(GL_LINES);
	if (ve_view->fixed_scale)
	{
		glVertex3f(
				(gfloat)ve_view->active_x/((gfloat)ve_view->x_bincount-1.0),
				(gfloat)ve_view->active_y/((gfloat)ve_view->y_bincount-1.0),
				bottom - ve_view->z_offset);

		glVertex3f(
				(gfloat)ve_view->active_x/((gfloat)ve_view->x_bincount-1.0),
				0.0,
				bottom);
	}
	else
	{
		glVertex3f(
				cur_val->x_edit_value,
				cur_val->y_edit_value,
				bottom - ve_view->z_offset);

		glVertex3f(
				cur_val->x_edit_value,
				0.0,
				bottom);
	}
	glEnd();
}


/*!
  \brief ve3d_draw_runtime_indicator is called during rerender and draws the
  green dot which tells where the engine is running at this instant.
  \param ve_view is a pointer to the view
  \param cur_val is a pointer to structure of current values for this iteration
  */
G_MODULE_EXPORT void ve3d_draw_runtime_indicator(Ve_View_3D *ve_view, Cur_Vals *cur_val)
{
	gchar * label = NULL;
	gfloat tmpf1 = 0.0;
	gfloat tmpf2 = 0.0;
	gfloat tmpf3 = 0.0;
	gfloat tmpf4 = 0.0;
	gfloat tmpf5 = 0.0;
	gfloat tmpf6 = 0.0;
	gfloat bottom = 0.0;
	gboolean out_of_bounds = FALSE;
	GLfloat w = 0.0;
	GLfloat h = 0.0;
	GtkAllocation allocation;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
	w = allocation.width;
	h = allocation.height;
	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_draw_runtime_indicator()\n"));
	/*printf("draw runtime indicator\n");*/

	if ((!ve_view->z_source) && (!ve_view->z_multi_source))
	{
		dbg_func(OPENGL|CRITICAL,g_strdup(__FILE__": ve3d_draw_runtime_indicator()\n\t\"z_source\" is NOT defined, check the .datamap file for\n\tmissing \"z_source\" key for [3d_view_button]\n"));
		return;
	}

	drawOrthoText(ve_view->drawing_area,cur_val->x_runtime_text, 0.2f, 1.0f, 0.2f, 0.025, 0.033 );
	drawOrthoText(ve_view->drawing_area,cur_val->y_runtime_text, 0.2f, 1.0f, 0.2f, 0.025, 0.066 );
	drawOrthoText(ve_view->drawing_area,cur_val->z_runtime_text, 0.2f, 1.0f, 0.2f, 0.025, 0.100 );
	drawOrthoText(ve_view->drawing_area,"Runtime Position", 0.2f, 1.0f, 0.2f, 0.025, 0.133 );

	bottom = 0.0;

	/* Tail, last val. */
	glLineWidth(MIN(w,h)/90.0);
	glColor3f(0.0,0.0,0.25);
	if (ve_view->fixed_scale)
	{
		tmpf4 = get_fixed_pos(ve_view,cur_val->p_x_vals[2],_X_);
		tmpf5 = get_fixed_pos(ve_view,cur_val->p_y_vals[2],_Y_);
	}
	else
	{
		tmpf4 = (cur_val->p_x_vals[2]-ve_view->x_trans)*ve_view->x_scale;
		tmpf5 = (cur_val->p_y_vals[2]-ve_view->y_trans)*ve_view->y_scale;
	}
	tmpf6 = (cur_val->p_z_vals[2]-ve_view->z_trans)*ve_view->z_scale;
	if ((tmpf4 > 1.0 ) || (tmpf4 < 0.0) ||(tmpf5 > 1.0 ) || (tmpf5 < 0.0))
		out_of_bounds = TRUE;
	else
		out_of_bounds = FALSE;

	tmpf4 = tmpf4 > 1.0 ? 1.0:tmpf4;
	tmpf4 = tmpf4 < 0.0 ? 0.0:tmpf4;
	tmpf5 = tmpf5 > 1.0 ? 1.0:tmpf5;
	tmpf5 = tmpf5 < 0.0 ? 0.0:tmpf5;
	tmpf6 = tmpf6 > 1.0 ? 1.0:tmpf6;
	tmpf6 = tmpf6 < 0.0 ? 0.0:tmpf6;

	/*
	glPointSize(MIN(w,h)/90.0);
	glBegin(GL_POINTS);
	glVertex3f(tmpf4,tmpf5,tmpf6);
	glEnd();
	*/

	/* Tail, second last val. */
	glColor3f(0.0,0.0,0.50);
	if (ve_view->fixed_scale)
	{
		tmpf1 = get_fixed_pos(ve_view,cur_val->p_x_vals[1],_X_);
		tmpf2 = get_fixed_pos(ve_view,cur_val->p_y_vals[1],_Y_);
	}
	else
	{
		tmpf1 = (cur_val->p_x_vals[1]-ve_view->x_trans)*ve_view->x_scale;
		tmpf2 = (cur_val->p_y_vals[1]-ve_view->y_trans)*ve_view->y_scale;
	}
	tmpf3 = (cur_val->p_z_vals[1]-ve_view->z_trans)*ve_view->z_scale;
	if ((tmpf1 > 1.0 ) || (tmpf1 < 0.0) ||(tmpf2 > 1.0 ) || (tmpf2 < 0.0))
		out_of_bounds = TRUE;
	else
		out_of_bounds = FALSE;

	tmpf1 = tmpf1 > 1.0 ? 1.0:tmpf1;
	tmpf1 = tmpf1 < 0.0 ? 0.0:tmpf1;
	tmpf2 = tmpf2 > 1.0 ? 1.0:tmpf2;
	tmpf2 = tmpf2 < 0.0 ? 0.0:tmpf2;
	tmpf3 = tmpf3 > 1.0 ? 1.0:tmpf3;
	tmpf3 = tmpf3 < 0.0 ? 0.0:tmpf3;

	/*
	glPointSize(MIN(w,h)/75.0);
	glBegin(GL_POINTS);
	glVertex3f(tmpf4,tmpf5,tmpf6);
	glEnd();
	*/

	glBegin(GL_LINE_STRIP);
	/* If anything out of bounds change color and clamp! */
	if (out_of_bounds)
		glColor3f(0.65,0.0,0.0);
	else
		glColor3f(0.0,0.0,0.65);

	glVertex3f(tmpf1,tmpf2,tmpf3);
	glVertex3f(tmpf4,tmpf5,tmpf6);
	glEnd();

	/* Tail, last val. */
	glColor3f(0.0,0.0,0.80);
	if (ve_view->fixed_scale)
	{
		tmpf4 = get_fixed_pos(ve_view,cur_val->p_x_vals[0],_X_);
		tmpf5 = get_fixed_pos(ve_view,cur_val->p_y_vals[0],_Y_);
	}
	else
	{
		tmpf4 = (cur_val->p_x_vals[0]-ve_view->x_trans)*ve_view->x_scale;
		tmpf5 = (cur_val->p_y_vals[0]-ve_view->y_trans)*ve_view->y_scale;
	}
	tmpf6 = (cur_val->p_z_vals[0]-ve_view->z_trans)*ve_view->z_scale;
	if ((tmpf4 > 1.0 ) || (tmpf4 < 0.0) ||(tmpf5 > 1.0 ) || (tmpf5 < 0.0))
		out_of_bounds = TRUE;
	else
		out_of_bounds = FALSE;

	tmpf4 = tmpf4 > 1.0 ? 1.0:tmpf4;
	tmpf4 = tmpf4 < 0.0 ? 0.0:tmpf4;
	tmpf5 = tmpf5 > 1.0 ? 1.0:tmpf5;
	tmpf5 = tmpf5 < 0.0 ? 0.0:tmpf5;
	tmpf6 = tmpf6 > 1.0 ? 1.0:tmpf6;
	tmpf6 = tmpf6 < 0.0 ? 0.0:tmpf6;

	/*
	glPointSize(MIN(w,h)/65.0);
	glBegin(GL_POINTS);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	glEnd();
	*/

	glBegin(GL_LINE_STRIP);
	/* If anything out of bounds change color and clamp! */
	if (out_of_bounds)
		glColor3f(0.80,0.0,0.0);
	else
		glColor3f(0.0,0.0,0.80);

	glVertex3f(tmpf1,tmpf2,tmpf3);
	glVertex3f(tmpf4,tmpf5,tmpf6);

	glEnd();

	/* Render a Blue dot at the active VE map position */
	glPointSize(MIN(w,h)/50.0);

	glColor3f(0.0,0.0,1.0);
	if (ve_view->fixed_scale)
	{
		tmpf1 = get_fixed_pos(ve_view,cur_val->x_val,_X_);
		tmpf2 = get_fixed_pos(ve_view,cur_val->y_val,_Y_);
	}
	else
	{
		tmpf1 = (cur_val->x_val-ve_view->x_trans)*ve_view->x_scale;
		tmpf2 = (cur_val->y_val-ve_view->y_trans)*ve_view->y_scale;
	}
	tmpf3 = (cur_val->z_val-ve_view->z_trans)*ve_view->z_scale;
	if ((tmpf1 > 1.0 ) || (tmpf1 < 0.0) ||(tmpf2 > 1.0 ) || (tmpf2 < 0.0))
		out_of_bounds = TRUE;
	else
		out_of_bounds = FALSE;

	tmpf1 = tmpf1 > 1.0 ? 1.0:tmpf1;
	tmpf1 = tmpf1 < 0.0 ? 0.0:tmpf1;
	tmpf2 = tmpf2 > 1.0 ? 1.0:tmpf2;
	tmpf2 = tmpf2 < 0.0 ? 0.0:tmpf2;
	tmpf3 = tmpf3 > 1.0 ? 1.0:tmpf3;
	tmpf3 = tmpf3 < 0.0 ? 0.0:tmpf3;

	glBegin(GL_POINTS);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	glEnd();

	glBegin(GL_LINE_STRIP);
	if (out_of_bounds)
		glColor3f(1.0,0.0,0.0);
	else
		glColor3f(0.0,0.0,1.0);

	glVertex3f(tmpf4,tmpf5,tmpf6);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	glEnd();

	glLineWidth(MIN(w,h)/300.0);
	glBegin(GL_LINE_STRIP);
	/* If anything out of bounds change color and clamp! */
	if (out_of_bounds)
		glColor3f(1.0,0.0,0.0);
	else
		glColor3f(0.0,1.0,0.0);


	glVertex3f(tmpf1,tmpf2,tmpf3);
	glVertex3f(tmpf1,tmpf2,bottom);

	glVertex3f(0.0,tmpf2,bottom);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(tmpf1,tmpf2,bottom - ve_view->z_offset);

	glVertex3f(tmpf1,0.0,bottom);
	glEnd();


	/* Live X axis marker */
	label = g_strdup_printf("%i",(GINT)cur_val->x_val);

	ve3d_draw_text(ve_view->drawing_area,label,tmpf1,-0.05,-0.05);
	g_free(label);


	/* Live Y axis marker */
	label = g_strdup_printf("%i",(GINT)cur_val->y_val);

	ve3d_draw_text(ve_view->drawing_area,label,-0.05,tmpf2,-0.05);
	g_free(label);

}


/*!
  \brief ve3d_draw_axis is called during rerender and draws the
  border axis scales around the VEgrid.
  \param ve_view is a pointer to the view
  \param cur_val is a pointer to structure of current values for this iteration
  */
G_MODULE_EXPORT void ve3d_draw_axis(Ve_View_3D *ve_view, Cur_Vals *cur_val)
{
	/* Set vars and an asthetically pleasing maximum value */
	gint i=0;
	gfloat tmpf = 0.0;
	gfloat tmpf1 = 0.0;
	gfloat tmpf2 = 0.0;
	gchar *label;

	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_draw_axis()\n"));
	/*printf("draw axis \n");*/

	/* Set line thickness and color */
	glLineWidth(1.0);
	glColor3f(0.3,0.3,0.3);

	/* Draw horizontal background grid lines
	   starting at 0 VE and working up to VE+10% */
	for (i=0;i<=100;i+=10)
	{
		glBegin(GL_LINE_STRIP);
		glVertex3f(0,1,(float)i/100.0);

		glVertex3f(1,1,(float)i/100.0);
		glVertex3f(1,0,(float)i/100.0);
		glEnd();
	}

	/* Draw vertical background grid lines along KPA axis */
	for (i=0;i<ve_view->y_bincount;i++)
	{
		glBegin(GL_LINES);
		if (ve_view->fixed_scale)
			tmpf2 = (gfloat)i/((gfloat)ve_view->y_bincount-1.0);
		else
			tmpf2 = (convert_after_upload((GtkWidget *)ve_view->y_objects[i])-ve_view->y_trans)*ve_view->y_scale;

		glVertex3f(1,tmpf2,0);
		glVertex3f(1,tmpf2,1);
		glEnd();
	}

	/* Draw vertical background lines along RPM axis */
	for (i=0;i<ve_view->x_bincount;i++)
	{
		glBegin(GL_LINES);

		if (ve_view->fixed_scale)
			tmpf1 = (gfloat)i/((gfloat)ve_view->x_bincount-1.0);
		else
			tmpf1 = (convert_after_upload((GtkWidget *)ve_view->x_objects[i])-ve_view->x_trans)*ve_view->x_scale;
		glVertex3f(tmpf1,1,0);

		glVertex3f(tmpf1,1,1);
		glEnd();
	}

	/* Add the back corner top lines */
	glBegin(GL_LINE_STRIP);
	glVertex3f(0,1,1);
	glVertex3f(1,1,1);
	glVertex3f(1,0,1);
	glEnd();

	/* Add front corner base lines */
	glBegin(GL_LINE_STRIP);
	glVertex3f(0,1,0);
	glVertex3f(0,0,0);
	glVertex3f(1,0,0);
	glVertex3f(1,1,0);
	glVertex3f(0,1,0);
	glEnd();

	/* Draw Z labels */
	for (i=0;i<=100;i+=10)
	{
		tmpf = (((float)i/100.0)/ve_view->z_scale)+ve_view->z_trans;
		label = g_strdup_printf("%1$.*2$f",tmpf,ve_view->z_precision);
		ve3d_draw_text(ve_view->drawing_area,label,-0.1,1,((float)i/100.0)-0.03);
		g_free(label);
	}
	return;

}


/*!
  \brief ve3d_draw_text is called during rerender and draws the
  axis marker text
  \param text is the text to draw in 3d space
  \param x is the x coord
  \param y is the y coord
  \param z is the z coord
  */
G_MODULE_EXPORT void ve3d_draw_text(GtkWidget * widget, char* text, gfloat x, gfloat y, gfloat z)
{
	/* Experiment*/
	glColor3f(0.2,0.8,0.8);
	/* Set rendering postition */
	glRasterPos3f (x, y, z);
	/* Render each letter of text as stored in the display list */
	gl_print_string(widget, text);
}


/*!
  \brief ve3d_key_press_event is called whenever the user hits a key on the 3D
  view. It looks for arrow keys, Plus/Minus and Pgup/PgDown.  Arrows  move the
  red marker, +/- shift the value by 1 unit, Pgup/Pgdn shift the value by 10
  units
  \param widget is the pointer to the  parent widget of the view
  \param event is the pointer to the GdkEventkey structure
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ve3d_key_press_event (GtkWidget *widget, GdkEventKey
                                      *event, gpointer data)
{
	static gint (*get_ecu_data_f)(gpointer) = NULL;
	static void (*send_to_ecu_f)(gpointer, gint, gboolean) = NULL;
	gint offset = 0;
	gint x_bincount = 0;
	gint y_bincount = 0;
	gint y_page = 0;
	gint x_page = 0;
	gint z_page = 0;
	gint y_base = 0;
	gint x_base = 0;
	gint z_base = 0;
	gint x_mult = 0;
	gint y_mult = 0;
	gint z_mult = 0;
	gint cur = 0;
	GObject *x_container = NULL;
	GObject *y_container = NULL;
	GObject *z_container = NULL;
	gint i = 0;
	DataSize x_size = 0;
	DataSize y_size = 0;
	DataSize z_size = 0;
	gint max = 0;
	gint dload_val = 0;
	gint canID = 0;
	gint factor = 1;
	gboolean cur_state = FALSE;
	gboolean update_widgets = FALSE;
	Ve_View_3D *ve_view = NULL;
	Firmware_Details *firmware = NULL;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	firmware = DATA_GET(global_data,"firmware");
	if (!get_ecu_data_f)
		get_symbol("get_ecu_data",(void*)&get_ecu_data_f);

	if (!send_to_ecu_f)
		get_symbol("send_to_ecu",(void*)&send_to_ecu_f);

	ve_view = (Ve_View_3D *)OBJ_GET(widget,"ve_view");
	g_return_val_if_fail(get_ecu_data_f,FALSE);
	g_return_val_if_fail(send_to_ecu_f,FALSE);
	g_return_val_if_fail(ve_view,FALSE);

	dbg_func(OPENGL,g_strdup(__FILE__": ve3d_key_press_event()\n"));
	g_static_mutex_lock(&key_mutex);

	window = gtk_widget_get_window(ve_view->drawing_area);
	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);

	x_bincount = ve_view->x_bincount;
	y_bincount = ve_view->y_bincount;
	canID = firmware->canID;

	x_base = ve_view->x_base;
	y_base = ve_view->y_base;
	z_base = ve_view->z_base;

	x_page = ve_view->x_page;
	y_page = ve_view->y_page;
	z_page = ve_view->z_page;

	x_mult = ve_view->x_mult;
	y_mult = ve_view->y_mult;
	z_mult = ve_view->z_mult;

	x_size = ve_view->x_size;
	y_size = ve_view->y_size;
	z_size = ve_view->z_size;

	x_container = ve_view->x_container;
	y_container = ve_view->y_container;
	z_container = ve_view->z_container;

	g_return_val_if_fail(x_container,FALSE);
	g_return_val_if_fail(y_container,FALSE);
	g_return_val_if_fail(z_container,FALSE);

	if (event->state & GDK_SHIFT_MASK)
		factor = 10;
	else 
		factor = 1;
	switch (event->keyval)
	{
		case GDK_B:
		case GDK_b:
			g_signal_emit_by_name(ve_view->burn_but,"clicked");
			break;
		case GDK_J:
		case GDK_j:
		case GDK_Up:
			dbg_func(OPENGL,g_strdup("\t\"UP\"\n"));
			/* Ctrl+Up moves the Load axis up */
			if (event->state & GDK_CONTROL_MASK)
			{
				offset = y_base + (ve_view->active_y*y_mult);
				max = (GINT)pow(2,y_mult*8) - (ve_view->y_smallstep * factor);
				OBJ_SET(y_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(y_container);
				if (cur <= max)
				{
					dload_val = cur + (ve_view->y_smallstep *factor);
					send_to_ecu_f(y_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			else
			{
				if (ve_view->active_y < (y_bincount-1))
					ve_view->active_y += 1;
				gdk_window_invalidate_rect (window,&allocation, FALSE);
			}
			break;

		case GDK_K:
		case GDK_k:
		case GDK_Down:
			dbg_func(OPENGL,g_strdup("\t\"DOWN\"\n"));
			/* Ctrl+Down moves the Load axis down */
			if (event->state & GDK_CONTROL_MASK)
			{
				offset = y_base + (ve_view->active_y*y_mult);
				OBJ_SET(y_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(y_container);
				if (cur > (ve_view->y_smallstep * factor))
				{
					dload_val = cur - (ve_view->y_smallstep * factor);
					send_to_ecu_f(y_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			else
			{
				if (ve_view->active_y > 0)
					ve_view->active_y -= 1;
				gdk_window_invalidate_rect (window,&allocation, FALSE);
			}
			break;

		case GDK_H:
		case GDK_h:
		case GDK_Left:
			dbg_func(OPENGL,g_strdup("\t\"LEFT\"\n"));

			/* Ctrl+Down moves the RPM axis left (down) */
			if (event->state & GDK_CONTROL_MASK)
			{
				offset = x_base + (ve_view->active_x*x_mult);
				OBJ_SET(x_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(x_container);
				if (cur > (ve_view->x_smallstep * factor))
				{
					dload_val = cur - (ve_view->x_smallstep * factor);
					send_to_ecu_f(x_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			else
			{
				if (ve_view->active_x > 0)
					ve_view->active_x -= 1;
				gdk_window_invalidate_rect (window,&allocation, FALSE);
			}
			break;
		case GDK_L:
		case GDK_l:
		case GDK_Right:
			dbg_func(OPENGL,g_strdup("\t\"RIGHT\"\n"));
			/* Ctrl+Down moves the RPM axis right (up) */
			if (event->state & GDK_CONTROL_MASK)
			{
				offset = x_base + (ve_view->active_x*x_mult);
				max = (GINT)pow(2,x_mult*8) - (ve_view->x_smallstep * factor);
				OBJ_SET(x_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(x_container);
				if (cur <= max)
				{
					dload_val = cur + (ve_view->x_smallstep * factor);
					send_to_ecu_f(x_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			else
			{
				if (ve_view->active_x < (x_bincount-1))
					ve_view->active_x += 1;
				gdk_window_invalidate_rect (window,&allocation, FALSE);
			}
			break;
		case GDK_Page_Up:
			dbg_func(OPENGL,g_strdup("\t\"Page Up\"\n"));
			max = (GINT)pow(2,z_mult*8) - ve_view->z_bigstep;
			if (event->state & GDK_CONTROL_MASK)
			{
				/*printf("Ctrl-PgUp, big increase ROW!\n");*/
				for (i=0;i<x_bincount;i++)
				{
					offset = z_base+(((ve_view->active_y*y_bincount)+i)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur <= max)
					{
						dload_val = cur + ve_view->z_bigstep;
						send_to_ecu_f(z_container,dload_val, TRUE);
						update_widgets = TRUE;
					}
				}
				break;
			}
			else if (event->state & GDK_MOD1_MASK)
			{
				/*printf("Alt-PgUp, big increase COL!\n");*/
				for (i=0;i<y_bincount;i++)
				{
					offset = z_base+(((i*y_bincount)+ve_view->active_x)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur <= max)
					{
						dload_val = cur + ve_view->z_bigstep;
						send_to_ecu_f(z_container, dload_val, TRUE);
						update_widgets = TRUE;

					}
				}
				break;
			}
			else
			{
				offset = z_base+(((ve_view->active_y*y_bincount)+ve_view->active_x)*z_mult);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(z_container);
				if (cur <= max)
				{
					dload_val = cur + ve_view->z_bigstep;
					send_to_ecu_f(z_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			break;
		case GDK_plus:
		case GDK_KP_Add:
		case GDK_KP_Equal:
		case GDK_Q:
		case GDK_q:
		case GDK_equal:
			dbg_func(OPENGL,g_strdup("\t\"PLUS\"\n"));
			max = (GINT)pow(2,z_mult*8) - ve_view->z_smallstep;
			if (event->state & GDK_CONTROL_MASK)
			{
				/*printf("Ctrl-q/+/=, increase ROW!\n");*/
				for (i=0;i<x_bincount;i++)
				{
					offset = z_base+(((ve_view->active_y*y_bincount)+i)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur <= max)
					{
						dload_val = cur + ve_view->z_smallstep;
						send_to_ecu_f(z_container, dload_val, TRUE);
						update_widgets = TRUE;
					}
				}
				break;
			}
			else if (event->state & GDK_MOD1_MASK)
			{
				/*printf("Alt-q/+/=, increase COL!\n");*/
				for (i=0;i<y_bincount;i++)
				{
					offset = z_base+(((i*y_bincount)+ve_view->active_x)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur <= max)
					{
						dload_val = cur + ve_view->z_smallstep;
						send_to_ecu_f(z_container, dload_val, TRUE);
						update_widgets = TRUE;
					}
				}
				break;
			}
			else
			{
				offset = z_base+(((ve_view->active_y*y_bincount)+ve_view->active_x)*z_mult);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(z_container);
				if (cur < max)
				{
					dload_val = cur + ve_view->z_smallstep;
					send_to_ecu_f(z_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			break;
		case GDK_Page_Down:
			dbg_func(OPENGL,g_strdup("\t\"Page Down\"\n"));

			if (event->state & GDK_CONTROL_MASK)
			{
				/*printf("Ctrl-PgDn, big decrease ROW!\n");*/
				for (i=0;i<x_bincount;i++)
				{
					offset = z_base+(((ve_view->active_y*y_bincount)+i)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur >= ve_view->z_bigstep)
					{
						dload_val = cur - ve_view->z_bigstep;
						send_to_ecu_f(z_container,dload_val, TRUE);
						update_widgets = TRUE;
					}
				}
				break;
			}
			if (event->state & GDK_MOD1_MASK)
			{
				/*printf("Alt-PgDn, big decrease COL!\n");*/
				for (i=0;i<y_bincount;i++)
				{
					offset = z_base+(((i*y_bincount)+ve_view->active_x)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur >= ve_view->z_bigstep)
					{
						dload_val = cur - ve_view->z_bigstep;
						send_to_ecu_f(z_container, dload_val, TRUE);
						update_widgets = TRUE;
					}
				}
				break;
			}
			else
			{
				offset = z_base+(((ve_view->active_y*y_bincount)+ve_view->active_x)*z_mult);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(z_container);
				if (cur >= ve_view->z_bigstep)
				{
					dload_val = cur - ve_view->z_bigstep;
					send_to_ecu_f(z_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			break;
		case GDK_minus:
		case GDK_W:
		case GDK_w:
		case GDK_KP_Subtract:
			dbg_func(OPENGL,g_strdup("\t\"MINUS\"\n"));

			if (event->state & GDK_CONTROL_MASK)
			{
				/*printf("Ctrl-w/-, decrease ROW!\n");*/
				for (i=0;i<x_bincount;i++)
				{
					offset = z_base+(((ve_view->active_y*y_bincount)+i)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur > ve_view->z_smallstep)
					{
						dload_val = cur - ve_view->z_smallstep;
						send_to_ecu_f(z_container, dload_val, TRUE);
						update_widgets = TRUE;
					}
				}
				break;
			}
			else if (event->state & GDK_MOD1_MASK)
			{
				/*printf("ALT-w/-, decrease COL!\n");*/
				for (i=0;i<y_bincount;i++)
				{
					offset = z_base+(((i*y_bincount)+ve_view->active_x)*z_mult);
					OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
					cur = get_ecu_data_f(z_container);
					if (cur > ve_view->z_smallstep)
					{
						dload_val = cur - ve_view->z_smallstep;
						send_to_ecu_f(z_container, dload_val, TRUE);
						update_widgets = TRUE;

					}
				}
				break;
			}
			else
			{
				offset = z_base+(((ve_view->active_y*y_bincount)+ve_view->active_x)*z_mult);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(offset));
				cur = get_ecu_data_f(z_container);
				if (cur > ve_view->z_smallstep)
				{
					dload_val = cur - ve_view->z_smallstep;
					send_to_ecu_f(z_container,dload_val, TRUE);
					update_widgets = TRUE;
				}
			}
			break;
		case GDK_t:
		case GDK_T:
			dbg_func(OPENGL,g_strdup("\t\"t/T\"\n"));
			cur_state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ve_view->tracking_button));
			if (cur_state)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ve_view->tracking_button),FALSE);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ve_view->tracking_button),TRUE);
			break;

		default:
			dbg_func(OPENGL,g_strdup_printf(__FILE__": ve3d_key_press_event()\n\tKeypress not handled, code: %#.4X\"\n",event->keyval));
			g_static_mutex_unlock(&key_mutex);
			return FALSE;
	}
	if (update_widgets)
	{
		dbg_func(OPENGL,g_strdup(__FILE__": ve3d_key_press_event()\n\tupdating widget data in ECU\n"));

		queue_ve3d_update(ve_view);
		DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	}

	g_static_mutex_unlock(&key_mutex);
	return TRUE;
}


/*!
  \brief initialize_ve3d_view is called from create_ve3d_view to 
  intialize it's datastructure for use.  
  \returns pointer to an initialized Ve_View_3D structure
  \see Ve_View_3D
  */
G_MODULE_EXPORT Ve_View_3D * initialize_ve3d_view(void)
{
	Ve_View_3D *ve_view = NULL;
	ve_view= g_new0(Ve_View_3D,1) ;
	ve_view->x_source = NULL;
	ve_view->y_source = NULL;
	ve_view->z_source = NULL;
	ve_view->x_suffix = NULL;
	ve_view->y_suffix = NULL;
	ve_view->z_suffix = NULL;
	ve_view->x_source_key = NULL;
	ve_view->y_source_key = NULL;
	ve_view->z_source_key = NULL;
	ve_view->x_precision = 0;
	ve_view->y_precision = 0;
	ve_view->z_precision = 0;
	ve_view->beginX = 0;
	ve_view->beginY = 0;
	ve_view->sphi = 45.0;
	ve_view->stheta = -63.75;
	ve_view->sdepth = -1.5;
	ve_view->zNear = 0;
	ve_view->zFar = 10.0;
	ve_view->aspect = 1.0;
	ve_view->h_strafe = -0.5;
	ve_view->v_strafe = -0.5;
	ve_view->z_scale = 4.2;
	ve_view->active_x = 0;
	ve_view->active_y = 0;
	ve_view->z_offset = 0;
	ve_view->x_page = 0;
	ve_view->y_page = 0;
	ve_view->z_page = 0;
	ve_view->x_base = 0;
	ve_view->y_base = 0;
	ve_view->z_base = 0;
	ve_view->x_mult = 0;
	ve_view->y_mult = 0;
	ve_view->z_mult = 0;
	ve_view->z_minval = 0;
	ve_view->z_maxval = 0;
	ve_view->x_smallstep = 0;
	ve_view->x_bigstep = 0;
	ve_view->y_smallstep = 0;
	ve_view->y_bigstep = 0;
	ve_view->z_smallstep = 0;
	ve_view->z_bigstep = 0;
	ve_view->x_bincount = 0;
	ve_view->y_bincount = 0;
	ve_view->table_name = NULL;
	ve_view->opacity = 0.85;
	ve_view->wireframe = TRUE;
	ve_view->x_container = g_object_new(GTK_TYPE_INVISIBLE,NULL);
	g_object_ref_sink(ve_view->x_container);
	ve_view->y_container = g_object_new(GTK_TYPE_INVISIBLE,NULL);
	g_object_ref_sink(ve_view->y_container);
	ve_view->z_container = g_object_new(GTK_TYPE_INVISIBLE,NULL);
	g_object_ref_sink(ve_view->z_container);
	ve_view->mesh_created = FALSE;
	ve_view->font_created = FALSE;
	ve_view->font_ascent = -1;
	ve_view->font_descent = -1;
	ve_view->y_offset_bitmap_render_pango_units = -1;
	ve_view->ft2_context = NULL;
	ve_view->gl_initialized = FALSE;

	return ve_view;
}


/*!
  \brief update_ve3d_if_necessary is called from update_write_status to 
  redraw the 3D view if a variable is changed that is represented in the 
  3D view.  This function scans through the table params to see if the passed 
  page/offset is part of a table and then checks if the table is visible 
  if so it forces a redraw of that table. (convoluted and butt ugly, 
  but it works)
  \param page is the ECU page 
  \param offset is the ECU offset withing the above page
  */
G_MODULE_EXPORT void update_ve3d_if_necessary(int page, int offset)
{
	gint total_tables = 0;
	gboolean need_update = FALSE;
	gint i = 0;
	gint base = 0;
	gint mult = 0;
	DataSize size = MTX_U08;
	gint len = 0;
	Ve_View_3D *ve_view = NULL;
	Firmware_Details *firmware = NULL;
	GHashTable *ve_view_hash = NULL;
	GdkWindow *window = NULL;

	firmware = DATA_GET(global_data,"firmware");
	ve_view_hash = DATA_GET(global_data,"ve_view_hash");
	g_return_if_fail(firmware);
	g_return_if_fail(ve_view_hash);

	total_tables = firmware->total_tables;
	gboolean table_list[total_tables];

	for (i=0;i<total_tables;i++)
	{
		if (firmware->table_params[i]->x_page == page)
		{
			base = firmware->table_params[i]->x_base;
			size = firmware->table_params[i]->x_size;
			len = firmware->table_params[i]->x_bincount;
			mult = get_multiplier(size);
			if ((offset >= (base)) && (offset <= (base + (len * mult))))
			{
				need_update = TRUE;
				table_list[i] = TRUE;
			}
			else
				table_list[i] = FALSE;
		}
		if ((firmware->table_params[i]->y_page == page) && (!table_list[i]))
		{
			base = firmware->table_params[i]->y_base;
			size = firmware->table_params[i]->y_size;
			len = firmware->table_params[i]->y_bincount;
			mult = get_multiplier(size);
			if ((offset >= (base)) && (offset <= (base + (len * mult))))
			{
				need_update = TRUE;
				table_list[i] = TRUE;
			}
			else
				table_list[i] = FALSE;
		}
		if ((firmware->table_params[i]->z_page == page) && (!table_list[i]))
		{
			base = firmware->table_params[i]->z_base;
			size = firmware->table_params[i]->z_size;
			len = firmware->table_params[i]->y_bincount *firmware->table_params[i]->x_bincount;
			mult = get_multiplier(size);
			if ((offset >= (base)) && (offset <= (base + (len * mult))))
			{
				need_update = TRUE;
				table_list[i] = TRUE;
			}
			else
				table_list[i] = FALSE;
		}
	}
	if (!need_update)
		return;
	else
	{
		for (i=0;i<total_tables;i++)
		{
			if (table_list[i] == TRUE)
			{
				ve_view = g_hash_table_lookup(ve_view_hash,GINT_TO_POINTER(i));
				if ((ve_view) && (ve_view->drawing_area))
				{
					window = gtk_widget_get_window(ve_view->drawing_area);
					if (window)
						queue_ve3d_update(ve_view);
				}
			}
		}
	}
}


/*!
  \brief Queues a deferred redraw to the 3D view in 300 ms
  \param ve_view is the pointer to view
  */
G_MODULE_EXPORT void queue_ve3d_update(Ve_View_3D *ve_view)
{
	if (!DATA_GET(global_data,"ve3d_redraw_queued"))
	{
		DATA_SET(global_data,"ve3d_redraw_queued",GINT_TO_POINTER(TRUE));
		gdk_threads_add_timeout(300,sleep_and_redraw,ve_view);
	}

	return;
}


/*!
  \brief Invalidates the display which forces a redraw event
  \param data is the pointer to view
  \returns FALSE to cause timeout to expire
  */
G_MODULE_EXPORT gboolean sleep_and_redraw(gpointer data)
{
	Ve_View_3D *ve_view = (Ve_View_3D *)data;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	g_return_val_if_fail(ve_view,FALSE);
	window = gtk_widget_get_window(ve_view->drawing_area);
	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
	ve_view->mesh_created = FALSE;
	gdk_window_invalidate_rect (window, &allocation, FALSE);
	DATA_SET(global_data,"ve3d_redraw_queued",GINT_TO_POINTER(FALSE));
	return FALSE;
}



/*! 
  \brief Finds to current nearest vertexes and highlights them to indicate
  to the user the vertexes of influence.
  \param ve_view is the pointer to view
  \param cur_val is the pointer to structure of current values for this iteration
  */
G_MODULE_EXPORT void ve3d_draw_active_vertexes_marker(Ve_View_3D *ve_view,Cur_Vals *cur_val)
{
	gfloat tmpf1 = 0.0;
	gfloat tmpf2 = 0.0;
	gfloat tmpf3 = 0.0;
	gfloat max = 0.0;
	gint heaviest = -1;
	gint i = 0;
	gint bin[4] = {0,0,0,0};
	gfloat left_w = 0.0;
	gfloat right_w = 0.0;
	gfloat top_w = 0.0;
	gfloat bottom_w = 0.0;
	gfloat z_weight[4] = {0,0,0,0};
	gfloat left = 0.0;
	gfloat right = 0.0;
	gfloat top = 0.0;
	gfloat bottom = 0.0;
	GtkAllocation allocation;
	GLfloat w = 0.0;
	GLfloat h = 0.0;
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");

	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
	w = allocation.width;
	h = allocation.height;
	/*printf("draw active vertexes marker \n");*/
	glPointSize(MIN(w,h)/100.0);

	for (i=0;i<ve_view->x_bincount-1;i++)
	{
		if (convert_after_upload((GtkWidget *)ve_view->x_objects[i]) >= cur_val->x_val)
		{
			bin[0] = 0;
			bin[1] = 0;
			left_w = 1;
			right_w = 1;
			break;
		}
		left = convert_after_upload((GtkWidget *)ve_view->x_objects[i]);
		right = convert_after_upload((GtkWidget *)ve_view->x_objects[i+1]);

		if ((cur_val->x_val > left) && (cur_val->x_val <= right))
		{
			bin[0] = i;
			bin[1] = i+1;

			right_w = (float)(cur_val->x_val-left)/(float)(right-left);
			left_w = 1.0-right_w;
			break;

		}
		if (cur_val->x_val > right)
		{
			bin[0] = i+1;
			bin[1] = i+1;
			left_w = 1;
			right_w = 1;
		}
	}
	for (i=0;i<ve_view->y_bincount-1;i++)
	{
		if (convert_after_upload((GtkWidget *)ve_view->y_objects[i]) >= cur_val->y_val)
		{
			bin[2] = 0;
			bin[3] = 0;
			top_w = 1;
			bottom_w = 1;
			break;
		}
		bottom = convert_after_upload((GtkWidget *)ve_view->y_objects[i]);
		top = convert_after_upload((GtkWidget *)ve_view->y_objects[i+1]);

		if ((cur_val->y_val > bottom) && (cur_val->y_val <= top))
		{
			bin[2] = i;
			bin[3] = i+1;

			top_w = (float)(cur_val->y_val-bottom)/(float)(top-bottom);
			bottom_w = 1.0-top_w;
			break;

		}
		if (cur_val->y_val > top)
		{
			bin[2] = i+1;
			bin[3] = i+1;
			top_w = 1;
			bottom_w = 1;
		}
	}
	z_weight[0] = left_w*bottom_w;
	z_weight[1] = left_w*top_w;
	z_weight[2] = right_w*bottom_w;
	z_weight[3] = right_w*top_w;

	max=0;
	for (i=0;i<4;i++)
	{
		if (z_weight[i] > max)
		{
			max = z_weight[i];
			heaviest = i;
		}
	}
	glBegin(GL_POINTS);

	tmpf3 = z_weight[0]*1.35;
	glColor3f(tmpf3,1.0-tmpf3,1.0-tmpf3);

	if (ve_view->fixed_scale)
	{
		tmpf1 = (gfloat)bin[0]/((gfloat)ve_view->x_bincount-1.0);
		tmpf2 = (gfloat)bin[2]/((gfloat)ve_view->y_bincount-1.0);
	}
	else
	{
		tmpf1 = ((convert_after_upload((GtkWidget *)ve_view->x_objects[bin[0]])-ve_view->x_trans)*ve_view->x_scale);
		tmpf2 = ((convert_after_upload((GtkWidget *)ve_view->y_objects[bin[2]])-ve_view->y_trans)*ve_view->y_scale);
	}
	tmpf3 = (((convert_after_upload((GtkWidget *)ve_view->z_objects[bin[0]][bin[2]]))-ve_view->z_trans)*ve_view->z_scale);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	if ((ve_view->tracking_focus) && (heaviest == 0))
	{
		ve_view->active_x = bin[0];
		ve_view->active_y = bin[2];
	}

	tmpf3 = z_weight[1]*1.35;
	glColor3f(tmpf3,1.0-tmpf3,1.0-tmpf3);

	if (ve_view->fixed_scale)
		tmpf2 = (gfloat)bin[3]/((gfloat)ve_view->y_bincount-1.0);
	else
		tmpf2 = ((convert_after_upload((GtkWidget *)ve_view->y_objects[bin[3]])-ve_view->y_trans)*ve_view->y_scale);

	tmpf3 = (((convert_after_upload((GtkWidget *)ve_view->z_objects[bin[0]][bin[3]]))-ve_view->z_trans)*ve_view->z_scale);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	if ((ve_view->tracking_focus) && (heaviest == 1))
	{
		ve_view->active_x = bin[0];
		ve_view->active_y = bin[3];
	}

	tmpf3 = z_weight[2]*1.35;
	glColor3f(tmpf3,1.0-tmpf3,1.0-tmpf3);

	if (ve_view->fixed_scale)
	{
		tmpf1 = (gfloat)bin[1]/((gfloat)ve_view->x_bincount-1.0);
		tmpf2 = (gfloat)bin[2]/((gfloat)ve_view->y_bincount-1.0);
	}
	else
	{
		tmpf1 = ((convert_after_upload((GtkWidget *)ve_view->x_objects[bin[1]])-ve_view->x_trans)*ve_view->x_scale);
		tmpf2 = ((convert_after_upload((GtkWidget *)ve_view->y_objects[bin[2]])-ve_view->y_trans)*ve_view->y_scale);
	}
	tmpf3 = (((convert_after_upload((GtkWidget *)ve_view->z_objects[bin[1]][bin[2]]))-ve_view->z_trans)*ve_view->z_scale);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	if ((ve_view->tracking_focus) && (heaviest == 2))
	{
		ve_view->active_x = bin[1];
		ve_view->active_y = bin[2];
	}

	tmpf3 = z_weight[3]*1.35;
	glColor3f(tmpf3,1.0-tmpf3,1.0-tmpf3);

	if (ve_view->fixed_scale)
		tmpf2 = (gfloat)bin[3]/((gfloat)ve_view->y_bincount-1.0);
	else
		tmpf2 = ((convert_after_upload((GtkWidget *)ve_view->y_objects[bin[3]])-ve_view->y_trans)*ve_view->y_scale);
	tmpf3 = (((convert_after_upload((GtkWidget *)ve_view->z_objects[bin[1]][bin[3]]))-ve_view->z_trans)*ve_view->z_scale);
	glVertex3f(tmpf1,tmpf2,tmpf3);
	if ((ve_view->tracking_focus) && (heaviest == 3))
	{
		ve_view->active_x = bin[1];
		ve_view->active_y = bin[3];
	}

	glEnd();

}


/*!
  \brief get_current_values is a helper function that populates a structure
  of data comon to all the redraw subhandlers to avoid duplication of
  effort
  \param ve_view is the base structure
  \returns a Cur_Vals structure populted with appropriate fields some of which
  MUST be freed when done.
  \see Cur_Vals
  */
G_MODULE_EXPORT Cur_Vals * get_current_values(Ve_View_3D *ve_view)
{
	gfloat x_val = 0.0;
	gfloat y_val = 0.0;
	gfloat z_val = 0.0;
	gfloat tmp = 0.0;
	gint *algorithm = NULL;
	GHashTable *sources_hash = NULL;
	GHashTable *hash = NULL;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	MultiSource *multi = NULL;
	Cur_Vals *cur_val = NULL;
	cur_val = g_new0(Cur_Vals, 1);
	Firmware_Details *firmware = NULL;

	firmware = DATA_GET(global_data,"firmware");
	sources_hash = DATA_GET(global_data,"sources_hash");
	algorithm = (gint *)DATA_GET(global_data,"algorithm");

	/* X */
	/* Edit value */
	cur_val->x_edit_value = ((convert_after_upload((GtkWidget *)ve_view->x_objects[ve_view->active_x]))-ve_view->x_trans)*ve_view->x_scale;
	tmp = (cur_val->x_edit_value/ve_view->x_scale)+ve_view->x_trans;

	/* Runtime value */
	if (ve_view->x_multi_source)
	{
		hash = ve_view->x_multi_hash;
		key = ve_view->x_source_key;
		hash_key = g_hash_table_lookup(sources_hash,key);
		if (algorithm[ve_view->table_num] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = g_hash_table_lookup(hash,hash_key);
			else
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[ve_view->table_num] == ALPHA_N)
			multi = g_hash_table_lookup(hash,"DEFAULT");
		else if (algorithm[ve_view->table_num] == MAF)
		{
			multi = g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else
			multi = g_hash_table_lookup(hash,"DEFAULT");

		if (!multi)
			printf("BUG! multi is null!!\n");

		lookup_current_value(multi->source,&x_val);
		cur_val->x_val = x_val;
		lookup_previous_n_skip_x_values(multi->source,3,2,cur_val->p_x_vals);
		cur_val->x_runtime_text = g_strdup_printf("%1$.*2$f %3$s",x_val,multi->precision,multi->suffix);
		cur_val->x_edit_text = g_strdup_printf("%1$.*2$f %3$s",tmp,multi->precision,multi->suffix);
	}
	else
	{
		/* Runtime value */
		lookup_current_value(ve_view->x_source,&x_val);
		cur_val->x_val = x_val;
		lookup_previous_n_skip_x_values(ve_view->x_source,3,2,cur_val->p_x_vals);
		cur_val->x_edit_text = g_strdup_printf("%1$.*2$f %3$s",tmp,ve_view->x_precision,ve_view->x_suffix);
		cur_val->x_runtime_text = g_strdup_printf("%1$.*2$f %3$s",x_val,ve_view->x_precision,ve_view->x_suffix);
	}
	/* Y */
	cur_val->y_edit_value = ((convert_after_upload((GtkWidget *)ve_view->y_objects[ve_view->active_y]))-ve_view->y_trans)*ve_view->y_scale;
	tmp = (cur_val->y_edit_value/ve_view->y_scale)+ve_view->y_trans;
	if (ve_view->y_multi_source)
	{
		hash = ve_view->y_multi_hash;
		key = ve_view->y_source_key;
		hash_key = g_hash_table_lookup(sources_hash,key);
		if (algorithm[ve_view->table_num] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = g_hash_table_lookup(hash,hash_key);
			else
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[ve_view->table_num] == ALPHA_N)
		{
			multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[ve_view->table_num] == MAF)
		{
			multi = g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else
		{
			multi = g_hash_table_lookup(hash,"DEFAULT");
		}

		if (!multi)
			printf("BUG! multi is null!!\n");

		/* Edit value */
		tmp = (cur_val->y_edit_value/ve_view->y_scale)+ve_view->y_trans;
		cur_val->y_edit_text = g_strdup_printf("%1$.*2$f %3$s",tmp,multi->precision,multi->suffix);
		/* runtime value */
		lookup_current_value(multi->source,&y_val);
		cur_val->y_val = y_val;
		lookup_previous_n_skip_x_values(multi->source,3,2,cur_val->p_y_vals);
		cur_val->y_runtime_text = g_strdup_printf("%1$.*2$f %3$s",y_val,multi->precision,multi->suffix);
	}
	else
	{
		/* Runtime value */
		lookup_current_value(ve_view->y_source,&y_val);
		cur_val->y_val = y_val;
		lookup_previous_n_skip_x_values(ve_view->y_source,3,2,cur_val->p_y_vals);
		cur_val->y_edit_text = g_strdup_printf("%1$.*2$f %3$s",tmp,ve_view->y_precision,ve_view->y_suffix);
		cur_val->y_runtime_text = g_strdup_printf("%1$.*2$f %3$s",y_val,ve_view->y_precision,ve_view->y_suffix);
	}

	/* Z */
	cur_val->z_edit_value = ((convert_after_upload((GtkWidget *)ve_view->z_objects[ve_view->active_x][ve_view->active_y]))-ve_view->z_trans)*ve_view->z_scale;
	tmp = (cur_val->z_edit_value/ve_view->z_scale)+ve_view->z_trans;
	if (ve_view->z_multi_source)
	{
		hash = ve_view->z_multi_hash;
		key = ve_view->z_source_key;
		hash_key = g_hash_table_lookup(sources_hash,key);
		if (algorithm[ve_view->table_num] == SPEED_DENSITY)
		{
			if (hash_key)
				multi = g_hash_table_lookup(hash,hash_key);
			else
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else if (algorithm[ve_view->table_num] == ALPHA_N)
			multi = g_hash_table_lookup(hash,"DEFAULT");
		else if (algorithm[ve_view->table_num] == MAF)
		{
			multi = g_hash_table_lookup(hash,"AFM_VOLTS");
			if(!multi)
				multi = g_hash_table_lookup(hash,"DEFAULT");
		}
		else
			multi = g_hash_table_lookup(hash,"DEFAULT");

		if (!multi)
			printf("BUG! multi is null!!\n");

		/* Edit value */
		tmp = (cur_val->z_edit_value/ve_view->z_scale)+ve_view->z_trans;
		cur_val->z_edit_text = g_strdup_printf("%1$.*2$f %3$s",tmp,multi->precision,multi->suffix);
		printf("z_edit_val (multi) is %f\n",cur_val->z_edit_value);
		/* runtime value */
		lookup_current_value(multi->source,&z_val);
		cur_val->z_val = z_val;
		lookup_previous_n_skip_x_values(multi->source,3,2,cur_val->p_z_vals);
		cur_val->z_runtime_text = g_strdup_printf("%1$.*2$f %3$s",z_val,multi->precision,multi->suffix);
	}
	else
	{
		/* runtime value */
		lookup_current_value(ve_view->z_source,&z_val);
		cur_val->z_val = z_val;
		lookup_previous_n_skip_x_values(ve_view->z_source,3,2,cur_val->p_z_vals);
		cur_val->z_edit_text = g_strdup_printf("%1$.*2$f %3$s",tmp,ve_view->z_precision,ve_view->z_suffix);
		cur_val->z_runtime_text = g_strdup_printf("%1$.*2$f %3$s",z_val,ve_view->z_precision,ve_view->z_suffix);
	}

	return cur_val;
}


/*!
  \brief Enables or disables the tracking focus feature, where the active 
  editable vertex TRACKS the most inflential vertex
  \param widget is the togglebutton
  \param data is unused
  \return FALSE if no view, TRUE otherwise
  */
G_MODULE_EXPORT gboolean set_tracking_focus(GtkWidget *widget, gpointer data)
{
	Ve_View_3D *ve_view = NULL;

	ve_view = OBJ_GET(widget,"ve_view");
	if (!ve_view)
		return FALSE;
	ve_view->tracking_focus = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (!ve_view->tracking_focus)
	{
		ve_view->active_x = 0;
		ve_view->active_y = 0;
	}
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	return TRUE;
}


/*!
  \brief Sets the scaling mode to proportional or fixed
  \param widget is the togglebutton
  \param data is unused
  \return FALSE if no view, TRUE otherwise
  */
G_MODULE_EXPORT gboolean set_scaling_mode(GtkWidget *widget, gpointer data)
{
	Ve_View_3D *ve_view = NULL;

	ve_view = OBJ_GET(widget,"ve_view");
	if (!ve_view)
		return FALSE;
	ve_view->fixed_scale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	ve_view->mesh_created=FALSE;
	gdk_threads_add_timeout(500,delayed_expose,ve_view);
	return TRUE;
}


/*! 
  \brief Sets the rendering mode to wireframe or semi-transparent solid
  \param widget is the togglebutton
  \param data is unused
  \return FALSE if no view, TRUE otherwise
  */
G_MODULE_EXPORT gboolean set_rendering_mode(GtkWidget *widget, gpointer data)
{
	Ve_View_3D *ve_view = NULL;

	ve_view = OBJ_GET(widget,"ve_view");
	if (!ve_view)
		return FALSE;
	ve_view->wireframe = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	gdk_threads_add_timeout(500,delayed_expose,ve_view);
	return TRUE;
}


/*! 
  \brief sets the amount of opacity in the view.
  \param widget is the range/scale
  \param data is unused
  \return FALSE if no view, TRUE otherwise
  */
G_MODULE_EXPORT gboolean set_opacity(GtkWidget *widget, gpointer data)
{
	Ve_View_3D *ve_view = NULL;

	ve_view = OBJ_GET(widget,"ve_view");
	if (!ve_view)
		return FALSE;
	ve_view->opacity = gtk_range_get_value(GTK_RANGE(widget));
	DATA_SET(global_data,"forced_update",GINT_TO_POINTER(TRUE));
	gdk_threads_add_timeout(500,delayed_expose,ve_view);
	return TRUE;
}


/*!
  \brief Frees up data in the Cur_Vals structure
  \param cur_val is the pointer to the structure of current values
  */
G_MODULE_EXPORT void free_current_values(Cur_Vals *cur_val)
{
	if (cur_val->x_edit_text)
		g_free(cur_val->x_edit_text);
	if (cur_val->y_edit_text)
		g_free(cur_val->y_edit_text);
	if (cur_val->z_edit_text)
		g_free(cur_val->z_edit_text);
	if (cur_val->x_runtime_text)
		g_free(cur_val->x_runtime_text);
	if (cur_val->y_runtime_text)
		g_free(cur_val->y_runtime_text);
	if (cur_val->z_runtime_text)
		g_free(cur_val->z_runtime_text);
	g_free(cur_val);
}


/*!
  \brief Gets the  position between vertexes in fixed scale mode
  \param ve_view is the pointer to the view
  \param value is the value to search for
  \param axis is the enumeration for X or Y axis
  \returns position across the axis as a fraction of 0<->1.0
  */
G_MODULE_EXPORT gfloat get_fixed_pos(Ve_View_3D *ve_view,gfloat value,Axis axis)
{
	gfloat tmp1 = 0.0;
	gfloat tmp2 = 0.0;
	gfloat tmp3 = 0.0;
	gint i = 0;
	gint count = 0;
	GObject **widgets = NULL;

	switch (axis)
	{
		case _X_:
			widgets = ve_view->x_objects;
			count = ve_view->x_bincount;
			break;
		case _Y_:
			widgets = ve_view->y_objects;
			count = ve_view->y_bincount;
			break;
		default:
			printf(__FILE__": Error, default case, should NOT have ran\n");
			return 0;
			break;
	}
	for (i=0;i<count-1;i++)
	{
		tmp1 = convert_after_upload((GtkWidget *)widgets[i]);
		tmp2 = convert_after_upload((GtkWidget *)widgets[i+1]);
		if ((tmp1 <= value) && (tmp2 >= value))
			break;
	}
	tmp3 = ((gfloat)i/((gfloat)count-1))+(((value-tmp1)/(tmp2-tmp1))/10.0);
	return tmp3;

}


/*!
  \brief  Generates the quad mesh, this only needs to be done when the 
  underlying ECU data changes, so we do it once, hold onto it nad render it
  as needed
  \param ve_view is the pointer to the view
  \param cur_val is the pointer to the structure of current values for 
  this iteration
  */
G_MODULE_EXPORT void generate_quad_mesh(Ve_View_3D *ve_view, Cur_Vals *cur_val)
{
	static gint (*get_ecu_data_f)(gpointer) = NULL;
	gint x = 0;
	gint y = 0;
	gint z_mult = 0;
	gint z_page = 0;
	gint z_base = 0;
	gint tmpi = 0;
	gfloat scaler = 0.0;
	gint canID = 0;
	Quad * quad = NULL;
	GObject *z_container = NULL;
	Firmware_Details *firmware = NULL;

	if (!get_ecu_data_f)
		get_symbol("get_ecu_data",(void*)&get_ecu_data_f);

	firmware = DATA_GET(global_data,"firmware");
	g_return_if_fail(get_ecu_data_f);
	g_return_if_fail(firmware);

	z_container = ve_view->z_container;
	g_return_if_fail(get_ecu_data_f);
	g_return_if_fail(z_container);
	g_return_if_fail(firmware);

	canID = firmware->canID;

	dbg_func(OPENGL,g_strdup(__FILE__": generate_quad_mesh() \n"));

	z_base = ve_view->z_base;
	z_page = ve_view->z_page;
	z_mult = ve_view->z_mult;

	ve_view->z_minval=1000;
	ve_view->z_maxval=0;
	/* Draw QUAD MESH into stored grid (Calc'd once*/
	for(y=0;y<ve_view->x_bincount*ve_view->y_bincount;++y)
	{
		OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+(y*z_mult)));
		tmpi = get_ecu_data_f(z_container);
		if (tmpi >ve_view->z_maxval)
			ve_view->z_maxval = tmpi;
		if (tmpi < ve_view->z_minval)
			ve_view->z_minval = tmpi;
	}
	if (ve_view->z_maxval == ve_view->z_minval)
	{
		ve_view->z_minval-=10;
		ve_view->z_maxval+=10;
	}
	scaler = (256.0/((ve_view->z_maxval-ve_view->z_minval)*1.05));
	for(y=0;y<ve_view->y_bincount-1;++y)
	{
		for(x=0;x<ve_view->x_bincount-1;++x)
		{
			quad = ve_view->quad_mesh[x][y];
			if (ve_view->fixed_scale)
			{
				/* (0x,0y) */
				quad->x[0] = (gfloat)x/((gfloat)ve_view->x_bincount-1.0);
				quad->y[0] = (gfloat)y/((gfloat)ve_view->y_bincount-1.0);
				quad->z[0] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x][y]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+(((y*ve_view->y_bincount)+x)*z_mult)));
				quad->color[0] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
				/* (1x,0y) */
				quad->x[1] = ((gfloat)x+1.0)/((gfloat)ve_view->x_bincount-1.0);
				quad->y[1] = (gfloat)y/((gfloat)ve_view->y_bincount-1.0);
				quad->z[1] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x+1][y]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+(((y*ve_view->y_bincount)+x+1)*z_mult)));
				quad->color[1] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
				/* (1x,1y) */
				quad->x[2] = ((gfloat)x+1.0)/((gfloat)ve_view->x_bincount-1.0);
				quad->y[2] = ((gfloat)y+1.0)/((gfloat)ve_view->y_bincount-1.0);
				quad->z[2] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x+1][y+1]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+((((y+1)*ve_view->y_bincount)+x+1)*z_mult)));
				quad->color[2] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
				/* (0x,1y) */
				quad->x[3] = (gfloat)x/((gfloat)ve_view->x_bincount-1.0);
				quad->y[3] = ((gfloat)y+1.0)/((gfloat)ve_view->y_bincount-1.0);
				quad->z[3] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x][y+1]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+((((y+1)*ve_view->y_bincount)+x)*z_mult)));
				quad->color[3] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
			}
			else
			{
				/* (0x,0y) */
				quad->x[0] = ((convert_after_upload((GtkWidget *)ve_view->x_objects[x])-ve_view->x_trans)*ve_view->x_scale);
				quad->y[0] = ((convert_after_upload((GtkWidget *)ve_view->y_objects[y])-ve_view->y_trans)*ve_view->y_scale);
				quad->z[0] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x][y]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+(((y*ve_view->y_bincount)+x)*z_mult)));
				quad->color[0] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
				/* (1x,0y) */
				quad->x[1] = ((convert_after_upload((GtkWidget *)ve_view->x_objects[x+1])-ve_view->x_trans)*ve_view->x_scale);
				quad->y[1] = ((convert_after_upload((GtkWidget *)ve_view->y_objects[y])-ve_view->y_trans)*ve_view->y_scale);
				quad->z[1] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x+1][y]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+(((y*ve_view->y_bincount)+x+1)*z_mult)));
				quad->color[1] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
				/* (1x,1y) */
				quad->x[2] = ((convert_after_upload((GtkWidget *)ve_view->x_objects[x+1])-ve_view->x_trans)*ve_view->x_scale);
				quad->y[2] = ((convert_after_upload((GtkWidget *)ve_view->y_objects[y+1])-ve_view->y_trans)*ve_view->y_scale);
				quad->z[2] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x+1][y+1]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+((((y+1)*ve_view->y_bincount)+x+1)*z_mult)));
				quad->color[2] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
				/* (0x,1y) */
				quad->x[3] = ((convert_after_upload((GtkWidget *)ve_view->x_objects[x])-ve_view->x_trans)*ve_view->x_scale);
				quad->y[3] = ((convert_after_upload((GtkWidget *)ve_view->y_objects[y+1])-ve_view->y_trans)*ve_view->y_scale);
				quad->z[3] = (((convert_after_upload((GtkWidget *)ve_view->z_objects[x][y+1]))-ve_view->z_trans)*ve_view->z_scale);
				OBJ_SET(z_container,"offset",GINT_TO_POINTER(z_base+((((y+1)*ve_view->y_bincount)+x)*z_mult)));
				quad->color[3] = rgb_from_hue(256.0-((gfloat)get_ecu_data_f(z_container)-ve_view->z_minval)*scaler,0.75, 1.0);
			}
		}
	}
	ve_view->mesh_created = TRUE;
}


/*!
  \brief triggers an expose event
  \param data is the pointer to the ve_view structure
  \returns FALSE to disable timeout
  */
gboolean delayed_expose(gpointer data)
{
	Ve_View_3D *ve_view = (Ve_View_3D *)data;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	g_return_val_if_fail(ve_view,FALSE);
	window = gtk_widget_get_window(ve_view->drawing_area);
	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);

	gdk_window_invalidate_rect (window, &allocation, FALSE);
	ve3d_expose_event(ve_view->drawing_area,NULL,NULL);
	return FALSE;
}


/*!
  \brief hack to get around a weird GL init sequencing issue
  \param data is the pointer to the ve_view structure
  \returns FALSE to disable timeout
  */
gboolean delayed_reconfigure(gpointer data)
{
	Ve_View_3D *ve_view = (Ve_View_3D *)data;
	GtkAllocation allocation;
	GdkWindow *window = gtk_widget_get_window(ve_view->drawing_area);

	gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
	ve3d_configure_event(ve_view->drawing_area, NULL,NULL);
	gdk_window_invalidate_rect (window, &allocation, FALSE);
	ve3d_expose_event(ve_view->drawing_area,NULL,NULL);
	return FALSE;
}


/*!
  \brief updates the VE3D displays with new data from the ECU
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean update_ve3ds(gpointer data)
{
	gfloat x[2] = {0.0,0.0};
	gfloat y[2] = {0.0,0.0};
	gfloat z[2] = {0.0,0.0};
	gint i = 0;
	Ve_View_3D * ve_view = NULL;
	GHashTable *hash = NULL;
	gchar *key = NULL;
	gchar *hash_key = NULL;
	MultiSource *multi = NULL;
	TabIdent active_page;
	gint * algorithm;
	Firmware_Details *firmware = NULL;
	GHashTable *sources_hash = NULL;
	GHashTable *ve_view_hash = NULL;
	GtkAllocation allocation;
	GdkWindow *window = NULL;

	sources_hash = DATA_GET(global_data,"sources_hash");
	ve_view_hash = DATA_GET(global_data,"ve_view_hash");
	firmware = DATA_GET(global_data,"firmware");
	algorithm = DATA_GET(global_data,"algorithm");
	active_page = (TabIdent)DATA_GET(global_data,"active_page");

	g_return_val_if_fail(sources_hash,TRUE);
	g_return_val_if_fail(ve_view_hash,TRUE);
	g_return_val_if_fail(firmware,TRUE);
	g_return_val_if_fail(algorithm,TRUE);
	/* Update all the dynamic RT Sliders */

	if (DATA_GET(global_data,"leaving"))
		return FALSE;

	/* If OpenGL window is open, redraw it... */
	for (i=0;i<firmware->total_tables;i++)
	{
		ve_view = g_hash_table_lookup(ve_view_hash,GINT_TO_POINTER(i));
		if (ve_view)
		{
			gtk_widget_get_allocation(ve_view->drawing_area,&allocation);
			window = gtk_widget_get_window(ve_view->drawing_area);
		}
		if ((ve_view != NULL) && (window != NULL))
		{
			/* Get X values */
			if (ve_view->x_multi_source)
			{
				hash = ve_view->x_multi_hash;
				key = ve_view->x_source_key;
				hash_key = g_hash_table_lookup(sources_hash,key);
				if (algorithm[ve_view->table_num] == SPEED_DENSITY)
				{
					if (hash_key)
						multi = g_hash_table_lookup(hash,hash_key);
					else
						multi = g_hash_table_lookup(hash,"DEFAULT");
				}
				else if (algorithm[ve_view->table_num] == ALPHA_N)
					multi = g_hash_table_lookup(hash,"DEFAULT");
				else if (algorithm[ve_view->table_num] == MAF)
				{
					multi = g_hash_table_lookup(hash,"AFM_VOLTS");
					if(!multi)
						multi = g_hash_table_lookup(hash,"DEFAULT");
				}
				else
					multi = g_hash_table_lookup(hash,"DEFAULT");

				if (!multi)
					printf(_("multi is null!!\n"));

				lookup_previous_n_values(multi->source,2,x);
			}
			else
				lookup_previous_n_values(ve_view->x_source,2,x);

			/* Test X values, redraw if needed */
			if (((fabs(x[0]-x[1])/x[0]) > 0.01) || (DATA_GET(global_data,"forced_update")))
				goto redraw;

			/* Get Y values */
			if (ve_view->y_multi_source)
			{
				hash = ve_view->y_multi_hash;
				key = ve_view->y_source_key;
				hash_key = g_hash_table_lookup(sources_hash,key);
				if (algorithm[ve_view->table_num] == SPEED_DENSITY)
				{
					if (hash_key)
						multi = g_hash_table_lookup(hash,hash_key);
					else
						multi = g_hash_table_lookup(hash,"DEFAULT");
				}
				else if (algorithm[ve_view->table_num] == ALPHA_N)
					multi = g_hash_table_lookup(hash,"DEFAULT");
				else if (algorithm[ve_view->table_num] == MAF)
				{
					multi = g_hash_table_lookup(hash,"AFM_VOLTS");
					if(!multi)
						multi = g_hash_table_lookup(hash,"DEFAULT");
				}
				else
					multi = g_hash_table_lookup(hash,"DEFAULT");

				if (!multi)
					printf(_("multi is null!!\n"));

				lookup_previous_n_values(multi->source,2,y);
			}
			else
				lookup_previous_n_values(ve_view->y_source,2,y);

			/* Test Y values, redraw if needed */
			if (((fabs(y[0]-y[1])/y[0]) > 0.01) || (DATA_GET(global_data,"forced_update")))
				goto redraw;

			/* Get Z values */
			if (ve_view->z_multi_source)
			{
				hash = ve_view->z_multi_hash;
				key = ve_view->z_source_key;
				hash_key = g_hash_table_lookup(sources_hash,key);
				if (algorithm[ve_view->table_num] == SPEED_DENSITY)
				{
					if (hash_key)
						multi = g_hash_table_lookup(hash,hash_key);
					else
						multi = g_hash_table_lookup(hash,"DEFAULT");
				}
				else if (algorithm[ve_view->table_num] == ALPHA_N)
					multi = g_hash_table_lookup(hash,"DEFAULT");
				else if (algorithm[ve_view->table_num] == MAF)
				{
					multi = g_hash_table_lookup(hash,"AFM_VOLTS");
					if(!multi)
						multi = g_hash_table_lookup(hash,"DEFAULT");
				}
				else
					multi = g_hash_table_lookup(hash,"DEFAULT");

				if (!multi)
					printf(_("multi is null!!\n"));

				lookup_previous_n_values(multi->source,2,z);
			}
			else
				lookup_previous_n_values(ve_view->z_source,2,z);

			/* Test Z values, redraw if needed */
			if (((fabs(z[0]-z[1])/z[0]) > 0.01) || (DATA_GET(global_data,"forced_update")))
				goto redraw;
			continue;

redraw:
//			gdk_threads_enter();
			gdk_window_invalidate_rect (window, &allocation, FALSE);
//			gdk_threads_leave();
		}
	}

//	gdk_threads_enter();
	{
		draw_ve_marker();
		update_tab_gauges();
	}
//	gdk_threads_leave();
	return TRUE;
}


void gl_create_font(GtkWidget *widget)
{
	PangoFontDescription *font_desc = NULL;
	PangoLayout *layout = NULL;
	PangoRectangle log_rect;
	int font_ascent_pango_units = 0;
	int font_descent_pango_units = 0;
	gint min = 0;
	GtkAllocation allocation;
	Ve_View_3D *ve_view = NULL;
	ve_view = (Ve_View_3D*)OBJ_GET(widget,"ve_view");

	g_return_if_fail(ve_view);
	if (ve_view->font_created)
		printf("Programming error: gl_create_font() was already called; you must call gl_destroy_font() before creating font again\n");
	else
		ve_view->font_created = TRUE;

	// This call is deprecated so we'll have to fix it sometime.
	ve_view->ft2_context = pango_ft2_get_context(72, 72);

	gtk_widget_get_allocation(widget,&allocation);
	min = MIN(allocation.width,allocation.height);
	font_desc = pango_font_description_from_string(font_string);
	pango_font_description_set_size(font_desc,(min/35)*PANGO_SCALE);

	pango_context_set_font_description(ve_view->ft2_context, font_desc);
	pango_font_description_free(font_desc);

	layout = pango_layout_new(ve_view->ft2_context);

	// I don't believe that's standard preprocessor syntax?
#if !PANGO_VERSION_CHECK(1,22,0)
	PangoLayoutIter *iter;  
	iter = pango_layout_get_iter(layout);
	font_ascent_pango_units = pango_layout_iter_get_baseline(iter);
	pango_layout_iter_free(iter);
#else
	font_ascent_pango_units = pango_layout_get_baseline(layout);
#endif

	pango_layout_get_extents(layout, NULL, &log_rect);
	g_object_unref(G_OBJECT(layout));
	font_descent_pango_units = log_rect.height - font_ascent_pango_units;

	ve_view->font_ascent = PANGO_PIXELS_CEIL(font_ascent_pango_units);
	ve_view->font_descent = PANGO_PIXELS_CEIL(font_descent_pango_units);
	ve_view->y_offset_bitmap_render_pango_units = (ve_view->font_ascent * PANGO_SCALE) - font_ascent_pango_units;
}


void gl_destroy_font(GtkWidget *widget)
{
	Ve_View_3D *ve_view = NULL;
	ve_view = (Ve_View_3D*)OBJ_GET(widget,"ve_view");
	g_return_if_fail(ve_view);

	if (!ve_view->font_created) {
		printf("Programming error: gl_destroy_font() called when font does not exist\n");
	}
	ve_view->font_ascent = -1;
	ve_view->font_descent = -1;
	ve_view->y_offset_bitmap_render_pango_units = -1;
	g_object_unref(G_OBJECT(ve_view->ft2_context));
	ve_view->font_created = FALSE;
}


/*!
 \brief Renders the input text at the current location with the current color.
 The X position of the current location is used to place the left edge of 
 the text image, where the text image bounds are defined as the logical 
 extents of the line of text.  The Y position of the current location is 
 used to place the bottom of the text image.  You should offset the Y 
 position by the amount returned by gl_font_descent() if you 
 want to place the baseline of the text image at the current Y position.
 Note: A problem with this function is that if the lower left corner of 
 the text falls just a hair outside of the viewport (meaning the current 
 raster position is invalid), then no text will be rendered.  The solution 
 to this is a very hacky one.  You can search Google for "glDrawPixels 
 clipping".
*/
void gl_print_string(GtkWidget *widget, const gchar *s)
{
	PangoLayout *layout;
	PangoRectangle log_rect;
	FT_Bitmap bitmap;
	unsigned char *begin_bitmap_buffer;
	GLfloat color[4];
	GLint previous_unpack_alignment;
	GLboolean previous_blend_enabled;
	GLint previous_blend_func_src;
	GLint previous_blend_func_dst;
	GLfloat previous_red_bias;
	GLfloat previous_green_bias;
	GLfloat previous_blue_bias;
	GLfloat previous_alpha_scale;
	Ve_View_3D *ve_view = NULL;
	ve_view = (Ve_View_3D*)OBJ_GET(widget,"ve_view");

	g_return_if_fail(ve_view);

	if (!ve_view->font_created) {
		printf("Programming error: gl_print_string() called but font does not exist; you should have called glt_create_font() first\n");
	}

	layout = pango_layout_new(ve_view->ft2_context);
	pango_layout_set_width(layout, -1); // -1 no wrapping.  All text on one line.
	pango_layout_set_text(layout, s, -1); // -1 null-terminated string.
	pango_layout_get_extents(layout, NULL, &log_rect);

	if (log_rect.width > 0 && log_rect.height > 0) {
		bitmap.rows = ve_view->font_ascent + ve_view->font_descent;
		bitmap.width = PANGO_PIXELS_CEIL(log_rect.width);
		bitmap.pitch = -bitmap.width; // Rendering it "upside down" for OpenGL.
		begin_bitmap_buffer = (unsigned char *) g_malloc(bitmap.rows * bitmap.width);
		memset(begin_bitmap_buffer, 0, bitmap.rows * bitmap.width);
		bitmap.buffer = begin_bitmap_buffer + (bitmap.rows - 1) * bitmap.width; // See pitch above.
		bitmap.num_grays = 0xff;
		bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
		pango_ft2_render_layout_subpixel(&bitmap, layout, -log_rect.x,
				ve_view->y_offset_bitmap_render_pango_units);
		glGetFloatv(GL_CURRENT_COLOR, color);

		// Save state.  I didn't see any OpenGL push/pop operations for these.
		// Question: Is saving/restoring this state necessary?  Being safe.
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_unpack_alignment);
		previous_blend_enabled = glIsEnabled(GL_BLEND);
		glGetIntegerv(GL_BLEND_SRC, &previous_blend_func_src);
		glGetIntegerv(GL_BLEND_DST, &previous_blend_func_dst);
		glGetFloatv(GL_RED_BIAS, &previous_red_bias);
		glGetFloatv(GL_GREEN_BIAS, &previous_green_bias);
		glGetFloatv(GL_BLUE_BIAS, &previous_blue_bias);
		glGetFloatv(GL_ALPHA_SCALE, &previous_alpha_scale);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPixelTransferf(GL_RED_BIAS, color[0]);
		glPixelTransferf(GL_GREEN_BIAS, color[1]);
		glPixelTransferf(GL_BLUE_BIAS, color[2]);
		glPixelTransferf(GL_ALPHA_SCALE, color[3]);

		glDrawPixels(bitmap.width, bitmap.rows,
				GL_ALPHA, GL_UNSIGNED_BYTE, begin_bitmap_buffer);
		g_free(begin_bitmap_buffer);

		// Restore state in reverse order of how we set it.
		glPixelTransferf(GL_ALPHA_SCALE, previous_alpha_scale);
		glPixelTransferf(GL_BLUE_BIAS, previous_blue_bias);
		glPixelTransferf(GL_GREEN_BIAS, previous_green_bias);
		glPixelTransferf(GL_RED_BIAS, previous_red_bias);
		glBlendFunc(previous_blend_func_src, previous_blend_func_dst);
		if (!previous_blend_enabled) { glDisable(GL_BLEND); }
		glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);
	}
	g_object_unref(G_OBJECT(layout));
}
