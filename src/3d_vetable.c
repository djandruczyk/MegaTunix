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
#include <glib/gprintf.h>
#include <gui_handlers.h>
#include <ms_structures.h>
#include <serialio.h>
#include <structures.h>
#include <time.h>

static int grid = 8;
static GLuint font_list_base;

extern struct Runtime_Common *runtime;

#define DEFAULT_WIDTH  475
#define DEFAULT_HEIGHT 320                                                                                  
static gboolean winstat[3] = {FALSE,FALSE,FALSE};
extern struct DynamicButtons buttons;

gint create_3d_view(GtkWidget *widget, gpointer data)
{
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *drawing_area;
        GdkGLConfig *gl_config;
	struct Ve_View_3D *ve_view;
	extern GtkTooltips *tip;
	extern GtkWidget *ve_widgets[];
	extern GtkWidget *ign_widgets[];
	extern GList *store_controls;
	gchar *tmpbuf;
	gint tbl = (gint)data;

	if (winstat[tbl-1] == TRUE)
		return TRUE;
	else
		winstat[tbl-1] = TRUE;
	
	ve_view = g_malloc0(sizeof(struct Ve_View_3D));
	initialize_ve_view((void *)ve_view);
	ve_view->table = tbl;

	if(tbl == 3)	// Special case, ignition table
		tmpbuf = g_strdup("3D Spark Advance Table");
	else
		tmpbuf = g_strdup_printf("3D VE-Table for table %i",tbl);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), tmpbuf);
	g_free(tmpbuf);
	gtk_widget_set_size_request(window,DEFAULT_WIDTH,DEFAULT_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER(window),0);
	ve_view->window = window;
	g_object_set_data(G_OBJECT(window),"data",(gpointer)ve_view);
	/* Bind pointer to veview to an object for retrieval elsewhere */
	if (tbl == 1)
		g_object_set_data(G_OBJECT(ve_widgets[0]),
				"data",(gpointer)ve_view);
	else if (tbl == 2)
		g_object_set_data(G_OBJECT(ve_widgets[0+MS_PAGE_SIZE]),
				"data",(gpointer)ve_view);
	else if (tbl == 3)
		g_object_set_data(G_OBJECT(ign_widgets[0]),
				"data",(gpointer)ve_view);
	else
		fprintf(stderr,__FILE__": table sent is invalid (%i)\n",tbl);

	g_signal_connect_swapped(G_OBJECT(window), "delete_event",
			G_CALLBACK(reset_3d_winstat),
			(gpointer) window);
	g_signal_connect_swapped(G_OBJECT(window), "delete_event",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer) window);

	vbox = gtk_vbox_new(FALSE,0);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
        gtk_container_add(GTK_CONTAINER(window),vbox);

        hbox = gtk_hbox_new(FALSE,5);
        gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

        frame = gtk_frame_new("VE Table 3D display");
        gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

        drawing_area = gtk_drawing_area_new();
	g_object_set_data(G_OBJECT(drawing_area),"data",(gpointer)ve_view);
	ve_view->drawing_area = drawing_area;
        gtk_container_add(GTK_CONTAINER(frame),drawing_area);

        gl_config = get_gl_config();
        gtk_widget_set_gl_capability(drawing_area, gl_config, NULL,
                        TRUE, GDK_GL_RGBA_TYPE);

        GTK_WIDGET_SET_FLAGS(drawing_area,GTK_CAN_FOCUS);

        gtk_widget_add_events (drawing_area,
                        GDK_BUTTON1_MOTION_MASK |
                        GDK_BUTTON2_MOTION_MASK |
                        GDK_BUTTON3_MOTION_MASK |
                        GDK_BUTTON_PRESS_MASK   |
                        GDK_KEY_PRESS_MASK              |
                        GDK_KEY_RELEASE_MASK    |
                        GDK_FOCUS_CHANGE_MASK   |
                        GDK_VISIBILITY_NOTIFY_MASK);

        /* Connect signal handlers to the drawing area */
        g_signal_connect_after(G_OBJECT (drawing_area), "realize",
                        G_CALLBACK (ve_realize), GINT_TO_POINTER(tbl));
        g_signal_connect(G_OBJECT (drawing_area), "configure_event",
                        G_CALLBACK (ve_configure_event), GINT_TO_POINTER(tbl));
        g_signal_connect(G_OBJECT (drawing_area), "expose_event",
                        G_CALLBACK (ve_expose_event), GINT_TO_POINTER(tbl));
        g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",
                        G_CALLBACK (ve_motion_notify_event), GINT_TO_POINTER(tbl));
        g_signal_connect (G_OBJECT (drawing_area), "button_press_event",
                        G_CALLBACK (ve_button_press_event), GINT_TO_POINTER(tbl));
        g_signal_connect(G_OBJECT (drawing_area), "key_press_event",
                        G_CALLBACK (ve_key_press_event), GINT_TO_POINTER(tbl));
        g_signal_connect(G_OBJECT (drawing_area), "focus_in_event",
                        G_CALLBACK (ve_focus_in_event), GINT_TO_POINTER(tbl));

        /* End of GL window, Now controls for it.... */
        frame = gtk_frame_new("3D Display Controls");
        gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);

        vbox2 = gtk_vbox_new(FALSE,0);
        gtk_container_add(GTK_CONTAINER(frame),vbox2);

        button = gtk_button_new_with_label("Reset Display");
        gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);
	g_object_set_data(G_OBJECT(button),"data",(gpointer)ve_view);
        g_signal_connect_swapped(G_OBJECT (button), "clicked",
                        G_CALLBACK (reset_3d_view), (gpointer)button);

        button = gtk_button_new_with_label("Get Data from ECU");
        g_signal_connect(G_OBJECT(button), "clicked",
        		G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_VE_CONST));
        gtk_tooltips_set_tip(tip,button,
                        "Reads in the Constants and VEtable from the MegaSquirt ECU and populates the GUI",NULL);
        gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);


        button = gtk_button_new_with_label("Burn to ECU");
        g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(std_button_handler),
                        GINT_TO_POINTER(BURN_MS_FLASH));
        ve_view->burn_but = button;
        store_controls = g_list_append(store_controls,(gpointer)button);
        gtk_tooltips_set_tip(tip,button,
                        "Even though MegaTunix writes data to the MS as soon as its changed, it has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
        gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Start Reading RT Vars");
        g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(START_REALTIME));
        gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);

	button = gtk_button_new_with_label("Stop Reading RT vars");
        g_signal_connect(G_OBJECT (button), "clicked",
                        G_CALLBACK (std_button_handler), \
                        GINT_TO_POINTER(STOP_REALTIME));
        gtk_box_pack_start(GTK_BOX(vbox2),button,FALSE,FALSE,0);


        button = gtk_button_new_with_label("Close Window");
        gtk_box_pack_end(GTK_BOX(vbox2),button,FALSE,FALSE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
			G_CALLBACK(reset_3d_winstat),
			(gpointer) window);
        g_signal_connect_swapped(G_OBJECT(button), "clicked",
                        G_CALLBACK(gtk_widget_destroy),
			(gpointer) window);

	gtk_widget_show_all(window);

	return TRUE;
}
gint reset_3d_winstat(GtkWidget *widget)
{
	struct Ve_View_3D *ve_view;
	gint tbl = -1;
	extern GtkWidget *ve_widgets[];
	extern GtkWidget *ign_widgets[];
	extern GList *store_controls;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"data");
	store_controls = g_list_remove(store_controls,(gpointer)ve_view->burn_but);	
	tbl = ve_view->table;
	winstat[(ve_view->table)-1] = FALSE;
	free(ve_view);/* free up the memory */
	ve_view = NULL;
	if (tbl == 1)
		g_object_set_data(G_OBJECT(ve_widgets[0]),
				"data",NULL);
	else if (tbl == 2)
		g_object_set_data(G_OBJECT(ve_widgets[0+MS_PAGE_SIZE]),
				"data",NULL);
	else	// Ignition table 
		g_object_set_data(G_OBJECT(ign_widgets[0]),
				"data",NULL);

	return FALSE;  /* MUST return false otherwise 
			* other handlers WILL NOT run. */
}
	
void reset_3d_view(GtkWidget * widget)
{
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"data");
	ve_view->active_load = 0;
	ve_view->active_rpm = 0;
	ve_view->dt = 0.008;
	ve_view->sphi = 35.0; 
	ve_view->stheta = 75.0; 
	ve_view->sdepth = 7.533;
	ve_view->zNear = 0.8;
	ve_view->zFar = 23;
	ve_view->h_strafe = 0;
	ve_view->v_strafe = 0;
	ve_view->aspect = 1.333;
	ve_configure_event(ve_view->drawing_area, NULL,NULL);
	ve_expose_event(ve_view->drawing_area, NULL,NULL);
}

GdkGLConfig* get_gl_config(void)
{
	GdkGLConfig* gl_config;                                                                                                                        
	/* Try double-buffered visual */
	gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
			GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE);
	if (gl_config == NULL)
	{
		g_print ("\n*** Cannot find the double-buffered visual.\n");
		g_print ("\n*** Trying single-buffered visual.\n");

		/* Try single-buffered visual */
		gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
				GDK_GL_MODE_DEPTH);
		if (gl_config == NULL)
		{
			/* Should make a non-GL basic drawing area version 
			   instead of dumping the user out of here, or at least 
			   render a non-GL found text on the drawing area */
			g_print ("*** No appropriate OpenGL-capable visual found.\n");
			exit (1);
		}
	}
	return gl_config;	
}

gboolean ve_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"data");

	GLfloat w = widget->allocation.width;
	GLfloat h = widget->allocation.height;

	#ifdef GLDEBUG
	//g_printf("%i Got Configure\n", clock());
	#endif
	
	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return FALSE;

	ve_view->aspect = (float)w/(float)h;
	glViewport (0, 0, w, h);

	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/                                                                                                                  
	return TRUE;
}

gboolean ve_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	struct Ve_View_3D *ve_view = NULL;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"data");

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
	glTranslatef(-(float)((grid/2)-0.3)-ve_view->h_strafe, -(float)((grid)/2-1)-ve_view->v_strafe, -2.0);

	ve_calculate_scaling(ve_view);
	ve_draw_ve_grid(ve_view);
	ve_draw_active_indicator(ve_view);
	ve_draw_runtime_indicator(ve_view);
	ve_draw_axis(ve_view);

	/* Swap buffers */
	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);
	else
		glFlush ();

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
	
	return TRUE; 
}

gboolean ve_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	gboolean redraw = FALSE;
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"data");

	// Left Button
	if (event->state & GDK_BUTTON1_MASK)
	{
		ve_view->sphi += (float)(event->x - ve_view->beginX) / 4.0;
		ve_view->stheta += (float)(ve_view->beginY - event->y) / 4.0;
		redraw = TRUE;
	}
	// Middle button (or both buttons for two button mice)
	if (event->state & GDK_BUTTON2_MASK)
	{
		ve_view->h_strafe -= (float)(event->x -ve_view->beginX) / 40.0;
		ve_view->v_strafe += (float)(event->y -ve_view->beginY) / 40.0;
//		g_printf("h_strafe %f, v_strafe %f\n",ve_view->h_strafe,ve_view->v_strafe);
		redraw = TRUE;
	}
	// Right Button
	if (event->state & GDK_BUTTON3_MASK)
	{
		ve_view->sdepth -= ((event->y - ve_view->beginY)/(widget->allocation.height))*(grid);
		redraw = TRUE;
	}

	ve_view->beginX = event->x;
	ve_view->beginY = event->y;

	gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);

	return TRUE;
}

gboolean ve_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)g_object_get_data(G_OBJECT(widget),"data");
	gtk_widget_grab_focus (widget);
	
	if (event->button != 0)
	{
		ve_view->beginX = event->x;
		ve_view->beginY = event->y;
		return TRUE;
	}

	return FALSE;
}

void ve_realize (GtkWidget *widget, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	GdkGLProc proc = NULL;

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
			g_print ("Sorry, glPolygonOffset() is not supported by this renderer.\n");
			exit (1);
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

	ve_load_font_metrics();
	
	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
}

void ve_calculate_scaling(void *ptr)
{
	int i=0;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_ptr = NULL;
	struct Ignition_Table *ign_ptr = NULL;
	struct Ve_View_3D *ve_view = NULL;

	ve_view = (struct Ve_View_3D *)ptr;
	if (ve_view->table == 1) /* all std code derivatives..*/
		ve_ptr = (struct Ve_Const_Std *) ms_data;
	else if (ve_view->table == 2)
		ve_ptr = (struct Ve_Const_Std *) (ms_data+MS_PAGE_SIZE);
	else if (ve_view->table == 3)
		ign_ptr = (struct Ignition_Table *) (ms_data+MS_PAGE_SIZE);
	else
		g_fprintf(stderr,__FILE__": Problem, ve_calculate_scaling(), table out of range..\n");
	
	ve_view->rpm_max = 0;
	ve_view->load_max = 0;
	ve_view->ve_max = 0;

	/* calculate scaling */
	if (ve_view->table != 3)	// Std VE-Tables
	{
		for (i=0;i<grid;i++) 
		{
			if (ve_ptr->rpm_bins[i] > ve_view->rpm_max) 
				ve_view->rpm_max = ve_ptr->rpm_bins[i];
			if (ve_ptr->load_bins[i] > ve_view->load_max) 
				ve_view->load_max = ve_ptr->load_bins[i];
		}
		for (i=0;i<grid*8;i++) {
			if (ve_ptr->ve_bins[i] > ve_view->ve_max) {
				ve_view->ve_max = ve_ptr->ve_bins[i];
			}
		}	
	}
	else				// Spark Table
	{
		for (i=0;i<grid;i++) 
		{
			if (ign_ptr->rpm_bins[i] > ve_view->rpm_max) 
				ve_view->rpm_max = ign_ptr->rpm_bins[i];
			if (ign_ptr->load_bins[i] > ve_view->load_max) 
				ve_view->load_max = ign_ptr->load_bins[i];
		}
		for (i=0;i<grid*8;i++) {
			if (((float)ign_ptr->spark_table[i]/2.84) > ve_view->ve_max) {
				ve_view->ve_max = ign_ptr->spark_table[i];
			}
		}	
	}

	ve_view->rpm_div = ((float)ve_view->rpm_max/8.0);
	ve_view->load_div = ((float)ve_view->load_max/8.0);
	ve_view->ve_div  = ((float)ve_view->ve_max/4.0);	
}

void ve_draw_ve_grid(void *ptr)
{
	int rpm=0, load=0;
	extern unsigned char *ms_data;
	struct Ve_Const_Std *ve_ptr = NULL;
	struct Ignition_Table *ign_ptr = NULL;
	struct Ve_View_3D *ve_view = NULL;

	ve_view = (struct Ve_View_3D *)ptr;

	if (ve_view->table == 1) /* all std code derivatives..*/
		ve_ptr = (struct Ve_Const_Std *) ms_data;
	else if (ve_view->table == 2)	/* DT code */
		ve_ptr = (struct Ve_Const_Std *) (ms_data+MS_PAGE_SIZE);
	else if (ve_view->table == 3)	/* Spark code */
		ign_ptr = (struct Ignition_Table *) (ms_data+MS_PAGE_SIZE);
	else
		g_fprintf(stderr,__FILE__": Problem, ve_draw_ve_grid(), table out of range..\n");
	

	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(1.5);
	
	if (ve_view->table != 3)	// Std code, non spark table
	{
		/* Draw lines on RPM axis */
		for(rpm=0;rpm<grid;rpm++)
		{
			glBegin(GL_LINE_STRIP);
			for(load=0;load<grid;load++) 
			{
				glVertex3f(
						(float)(ve_ptr->rpm_bins[rpm])
						/ve_view->rpm_div,
						(float)(ve_ptr->load_bins[load])
						/ve_view->load_div, 	 	
						(float)(ve_ptr->ve_bins[(load*8)+rpm])
						/ve_view->ve_div);
			}
			glEnd();
		}

		/* Draw lines on MAP axis */
		for(load=0;load<grid;load++)
		{
			glBegin(GL_LINE_STRIP);
			for(rpm=0;rpm<grid;rpm++)
			{
				glVertex3f(	
						(float)(ve_ptr->rpm_bins[rpm])
						/ve_view->rpm_div,
						(float)(ve_ptr->load_bins[load])
						/ve_view->load_div,
						(float)(ve_ptr->ve_bins[(load*8)+rpm])
						/ve_view->ve_div);	
			}
			glEnd();
		}
	}
	else				// Spark code
	{
		/* Draw lines on RPM axis */
		for(rpm=0;rpm<grid;rpm++)
		{
			glBegin(GL_LINE_STRIP);
			for(load=0;load<grid;load++) 
			{
				glVertex3f(
						(float)(ign_ptr->rpm_bins[rpm])
						/ve_view->rpm_div,
						(float)(ign_ptr->load_bins[load])
						/ve_view->load_div, 	 	
						(float)(ign_ptr->spark_table[(load*8)+rpm])/2.84
						/ve_view->ve_div);
			}
			glEnd();
		}

		/* Draw lines on MAP axis */
		for(load=0;load<grid;load++)
		{
			glBegin(GL_LINE_STRIP);
			for(rpm=0;rpm<grid;rpm++)
			{
				glVertex3f(	
						(float)(ign_ptr->rpm_bins[rpm])
						/ve_view->rpm_div,
						(float)(ign_ptr->load_bins[load])
						/ve_view->load_div,
						(float)(ign_ptr->spark_table[(load*8)+rpm])/2.84
						/ve_view->ve_div);	
			}
			glEnd();
		}
	}
}

void ve_draw_active_indicator(void *ptr)
{
	struct Ve_Const_Std *ve_ptr = NULL;
	struct Ignition_Table *ign_ptr = NULL;
	struct Ve_View_3D *ve_view = NULL;
	ve_view = (struct Ve_View_3D *)ptr;
	extern unsigned char * ms_data;

	if (ve_view->table == 1) /* all std code derivatives..*/
		ve_ptr = (struct Ve_Const_Std *) ms_data;
	else if (ve_view->table == 2)
		ve_ptr = (struct Ve_Const_Std *) (ms_data+MS_PAGE_SIZE);
	else if (ve_view->table == 3)
		ign_ptr = (struct Ignition_Table *) (ms_data+MS_PAGE_SIZE);
	else
		g_fprintf(stderr,__FILE__": Problem, ve_draw_active_indicator(), table out of range..\n");
	/* Render a red dot at the active VE map position */
	glPointSize(8.0);
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_POINTS);
	if (ve_view->table != 3)	// Std ve tables
	{
		glVertex3f(	
				(float)(ve_ptr->rpm_bins[ve_view->active_rpm])/ve_view->rpm_div,
				(float)(ve_ptr->load_bins[ve_view->active_load])/ve_view->load_div,	
				(float)(ve_ptr->ve_bins[(ve_view->active_load*8)+ve_view->active_rpm])/ve_view->ve_div);
	}
	else
	{
		glVertex3f(	
				(float)(ign_ptr->rpm_bins[ve_view->active_rpm])/ve_view->rpm_div,
				(float)(ign_ptr->load_bins[ve_view->active_load])/ve_view->load_div,	
				((float)ign_ptr->spark_table[(ve_view->active_load*8)+ve_view->active_rpm]/2.84)/ve_view->ve_div);
	}
	glEnd();	
}

void ve_draw_runtime_indicator(void *ptr)
{
	struct Ve_View_3D *ve_view;
	ve_view = (struct Ve_View_3D *)ptr;
	unsigned char actual_ve = 0;

	if (ve_view->table == 1) /* all std code derivatives..*/
		actual_ve = runtime->vecurr1;
	else if (ve_view->table == 2)
		actual_ve = runtime->vecurr2;
	else if (ve_view->table == 3)
		actual_ve = runtime->sparkangle;
	else
		g_fprintf(stderr,__FILE__": Problem, ve_draw_runtim_indicator(), table out of range..\n");

	/* Render a green dot at the active VE map position */
	glPointSize(8.0);
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_POINTS);
	glVertex3f(	
			(float)(runtime->rpm)/100/ve_view->rpm_div,
			(float)(runtime->map)/ve_view->load_div,	
			(float)(actual_ve)/ve_view->ve_div);
	glEnd();
}

void ve_draw_axis(void *ptr)
{
	/* Set vars and an asthetically pleasing maximum value */
	int i=0, rpm=0, load=0;
	float top = 0.0;
	gchar *label;
	struct Ve_Const_Std *ve_ptr = NULL;
	struct Ignition_Table *ign_ptr = NULL;
	struct Ve_View_3D *ve_view = NULL;
	ve_view = (struct Ve_View_3D *)ptr;
	extern unsigned char *ms_data;

	if (ve_view->table == 1) /* all std code derivatives..*/
		ve_ptr = (struct Ve_Const_Std *) ms_data;
	else if (ve_view->table == 2)
		ve_ptr = (struct Ve_Const_Std *) (ms_data+MS_PAGE_SIZE);
	else if (ve_view->table == 3)
		ign_ptr = (struct Ignition_Table *) (ms_data+MS_PAGE_SIZE);
	else
		g_fprintf(stderr,__FILE__": Problem, ve_draw_axis(), table out of range..\n");
	
	top = ((float)(ve_view->ve_max+20))/ve_view->ve_div;
	/* Set line thickness and color */
	glLineWidth(1.0);
	glColor3f(0.7,0.7,0.7);
	
	if (ve_view->table != 3)	// Std VE table
	{
		/* Draw horizontal background grid lines  
		   starting at 0 VE and working up to VE+20% */
		for (i=0;i<(ve_view->ve_max+20);i = i + 10){
			glBegin(GL_LINE_STRIP);
			glVertex3f(
					((ve_ptr->rpm_bins[0])/ve_view->rpm_div),
					((ve_ptr->load_bins[7])/ve_view->load_div),		
					((float)i)/ve_view->ve_div);
			glVertex3f(
					((ve_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ve_ptr->load_bins[7])/ve_view->load_div),		
					((float)i)/ve_view->ve_div);
			glVertex3f(
					((ve_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ve_ptr->load_bins[0])/ve_view->load_div),		
					((float)i)/ve_view->ve_div);
			glEnd();	
		}

		/* Draw vertical background grid lines along KPA axis */
		for (i=0;i<8;i++){
			glBegin(GL_LINES);
			glVertex3f(
					((ve_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ve_ptr->load_bins[i])/ve_view->load_div),		
					0.0);
			glVertex3f(
					((ve_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ve_ptr->load_bins[i])/ve_view->load_div),		
					top);
			glEnd();
		}

		/* Draw vertical background lines along RPM axis */
		for (i=0;i<8;i++){
			glBegin(GL_LINES);
			glVertex3f(
					((ve_ptr->rpm_bins[i])/ve_view->rpm_div),		
					((ve_ptr->load_bins[7])/ve_view->load_div),
					0.0);
			glVertex3f(
					((ve_ptr->rpm_bins[i])/ve_view->rpm_div),
					((ve_ptr->load_bins[7])/ve_view->load_div),		
					top);
			glEnd();
		}

		/* Add the back corner top lines */
		glBegin(GL_LINE_STRIP);
		glVertex3f(
				((ve_ptr->rpm_bins[0])/ve_view->rpm_div),	
				((ve_ptr->load_bins[7])/ve_view->load_div),
				top);
		glVertex3f(
				((ve_ptr->rpm_bins[7])/ve_view->rpm_div),	
				((ve_ptr->load_bins[7])/ve_view->load_div),
				top);
		glVertex3f(
				((ve_ptr->rpm_bins[7])/ve_view->rpm_div),
				((ve_ptr->load_bins[0])/ve_view->load_div),	
				top);
		glEnd();

		/* Add front corner base lines */
		glBegin(GL_LINE_STRIP);
		glVertex3f(
				((ve_ptr->rpm_bins[0])/ve_view->rpm_div),	
				((ve_ptr->load_bins[7])/ve_view->load_div),
				0.0);
		glVertex3f(
				((ve_ptr->rpm_bins[0])/ve_view->rpm_div),	
				((ve_ptr->load_bins[0])/ve_view->load_div),
				0.0);
		glVertex3f(
				((ve_ptr->rpm_bins[7])/ve_view->rpm_div),
				((ve_ptr->load_bins[0])/ve_view->load_div),
				0.0);
		glEnd();

		/* Draw RPM and KPA labels */
		for (i=0;i<8;i++){
			rpm = (ve_ptr->rpm_bins[i])*100;
			load = (ve_ptr->load_bins[i]);
			label = g_strdup_printf("%i",load);
			ve_drawtext(label,
					((ve_ptr->rpm_bins[0])/ve_view->rpm_div),
					((ve_ptr->load_bins[i])/ve_view->load_div),
					0.0);
			g_free(label);

			label = g_strdup_printf("%i",rpm);
			ve_drawtext(label,
					((ve_ptr->rpm_bins[i])/ve_view->rpm_div),
					((ve_ptr->load_bins[0])/ve_view->load_div),
					0.0);
			g_free(label);
		}

		/* Draw VE labels */
		for (i=0;i<(ve_view->ve_max+20);i=i+10){
			label = g_strdup_printf("%i",i);
			ve_drawtext(label,
					((ve_ptr->rpm_bins[0])/ve_view->rpm_div),
					((ve_ptr->load_bins[7])/ve_view->load_div),
					(float)i/ve_view->ve_div);
			g_free(label);
		}
	}
	else			// Spark Tables
	{
		/* Draw horizontal background grid lines  
		   starting at 0 VE and working up to VE+20% */
		for (i=0;i<(ve_view->ve_max+20);i = i + 10){
			glBegin(GL_LINE_STRIP);
			glVertex3f(
					((ign_ptr->rpm_bins[0])/ve_view->rpm_div),
					((ign_ptr->load_bins[7])/ve_view->load_div),		
					((float)i)/ve_view->ve_div);
			glVertex3f(
					((ign_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ign_ptr->load_bins[7])/ve_view->load_div),		
					((float)i)/ve_view->ve_div);
			glVertex3f(
					((ign_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ign_ptr->load_bins[0])/ve_view->load_div),		
					((float)i)/ve_view->ve_div);
			glEnd();	
		}

		/* Draw vertical background grid lines along KPA axis */
		for (i=0;i<8;i++){
			glBegin(GL_LINES);
			glVertex3f(
					((ign_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ign_ptr->load_bins[i])/ve_view->load_div),		
					0.0);
			glVertex3f(
					((ign_ptr->rpm_bins[7])/ve_view->rpm_div),
					((ign_ptr->load_bins[i])/ve_view->load_div),		
					top);
			glEnd();
		}

		/* Draw vertical background lines along RPM axis */
		for (i=0;i<8;i++){
			glBegin(GL_LINES);
			glVertex3f(
					((ign_ptr->rpm_bins[i])/ve_view->rpm_div),		
					((ign_ptr->load_bins[7])/ve_view->load_div),
					0.0);
			glVertex3f(
					((ign_ptr->rpm_bins[i])/ve_view->rpm_div),
					((ign_ptr->load_bins[7])/ve_view->load_div),		
					top);
			glEnd();
		}

		/* Add the back corner top lines */
		glBegin(GL_LINE_STRIP);
		glVertex3f(
				((ign_ptr->rpm_bins[0])/ve_view->rpm_div),	
				((ign_ptr->load_bins[7])/ve_view->load_div),
				top);
		glVertex3f(
				((ign_ptr->rpm_bins[7])/ve_view->rpm_div),	
				((ign_ptr->load_bins[7])/ve_view->load_div),
				top);
		glVertex3f(
				((ign_ptr->rpm_bins[7])/ve_view->rpm_div),
				((ign_ptr->load_bins[0])/ve_view->load_div),	
				top);
		glEnd();

		/* Add front corner base lines */
		glBegin(GL_LINE_STRIP);
		glVertex3f(
				((ign_ptr->rpm_bins[0])/ve_view->rpm_div),	
				((ign_ptr->load_bins[7])/ve_view->load_div),
				0.0);
		glVertex3f(
				((ign_ptr->rpm_bins[0])/ve_view->rpm_div),	
				((ign_ptr->load_bins[0])/ve_view->load_div),
				0.0);
		glVertex3f(
				((ign_ptr->rpm_bins[7])/ve_view->rpm_div),
				((ign_ptr->load_bins[0])/ve_view->load_div),
				0.0);
		glEnd();

		/* Draw RPM and KPA labels */
		for (i=0;i<8;i++){
			rpm = (ign_ptr->rpm_bins[i])*100;
			load = (ign_ptr->load_bins[i]);
			label = g_strdup_printf("%i",load);
			ve_drawtext(label,
					((ign_ptr->rpm_bins[0])/ve_view->rpm_div),
					((ign_ptr->load_bins[i])/ve_view->load_div),
					0.0);
			g_free(label);

			label = g_strdup_printf("%i",rpm);
			ve_drawtext(label,
					((ign_ptr->rpm_bins[i])/ve_view->rpm_div),
					((ign_ptr->load_bins[0])/ve_view->load_div),
					0.0);
			g_free(label);
		}

		/* Draw VE labels */
		for (i=0;i<(ve_view->ve_max+20);i=i+10){
			label = g_strdup_printf("%i",i);
			ve_drawtext(label,
					((ign_ptr->rpm_bins[0])/ve_view->rpm_div),
					((ign_ptr->load_bins[7])/ve_view->load_div),
					(float)i/ve_view->ve_div);
			g_free(label);
		}
	}
	
}

void ve_drawtext(char* text, float x, float y, float z)
{
	glColor3f(0.7,0.7,0.7);
	/* Set rendering postition */
	glRasterPos3f (x, y, z);
	/* Render each letter of text as stored in the display list */
	while(*text) {
        glCallList(font_list_base+(*text));
        text++;
    };
}

void ve_load_font_metrics(void)
{
	PangoFontDescription *font_desc;
	PangoFont *font;
	PangoFontMetrics *font_metrics;
	gchar font_string[] = "sans 10";
	gint font_height;

	font_list_base = glGenLists (128);
	font_desc = pango_font_description_from_string (font_string);
	font = gdk_gl_font_use_pango_font (font_desc, 0, 128, font_list_base);
	if (font == NULL)
	{
		g_print ("*** Can't load font '%s'\n", font_string);
		exit (1);
	}
	font_metrics = pango_font_get_metrics (font, NULL);
	font_height = pango_font_metrics_get_ascent (font_metrics) +
		pango_font_metrics_get_descent (font_metrics);
	font_height = PANGO_PIXELS (font_height);
	pango_font_description_free (font_desc);
	pango_font_metrics_unref (font_metrics);
}

gboolean ve_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	gint value = 0;
	gint offset = 0;
	gint dload_val = 0;
	extern GtkWidget *ve_widgets[];
	extern GtkWidget *ign_widgets[];
	struct Ve_Const_Std *ve_ptr = NULL;
	struct Ignition_Table *ign_ptr = NULL;
	struct Ve_View_3D *ve_view = NULL;
	extern unsigned char *ms_data;
	ve_view = (struct Ve_View_3D *)g_object_get_data(
			G_OBJECT(widget),"data");

	if (ve_view->table == 1) /* all std code derivatives..*/
		ve_ptr = (struct Ve_Const_Std *) ms_data;
	else if (ve_view->table == 2)
		ve_ptr = (struct Ve_Const_Std *) (ms_data+MS_PAGE_SIZE);
	else if (ve_view->table == 3)
		ign_ptr = (struct Ignition_Table *) (ms_data+MS_PAGE_SIZE);
	else
		g_fprintf(stderr,__FILE__": Problem, ve_key_press_event(), table out of range..\n");

#ifdef GLDEBUG	
	g_printf("Key press event\n");
#endif
	switch (event->keyval)
	{
		case GDK_Up:
#ifdef GLDEBUG
			g_printf("UP\n");
#endif
			if (ve_view->active_load < 7)
				ve_view->active_load += 1;
			break;

		case GDK_Down:
#ifdef GLDEBUG
			g_printf("DOWN\n");
#endif
			if (ve_view->active_load > 0)
				ve_view->active_load -= 1;
			break;				

		case GDK_Left:
#ifdef GLDEBUG
			g_printf("LEFT\n");
#endif
			if (ve_view->active_rpm > 0)
				ve_view->active_rpm -= 1;
			break;					

		case GDK_Right:
#ifdef GLDEBUG
			g_printf("RIGHT\n");
#endif
			if (ve_view->active_rpm < 7)
				ve_view->active_rpm += 1;
			break;				

		case GDK_plus:
		case GDK_KP_Add:
#ifdef GLDEBUG
			g_printf("PLUS\n");
#endif
			if (ve_view->table != 3) // Std VE Table
			{
				if (ve_ptr->ve_bins[(ve_view->active_load*8)+ve_view->active_rpm] < 255)
				{
					offset = (ve_view->active_load*8)+ve_view->active_rpm;
					value = ve_ptr->ve_bins[offset] + 1;
					if (ve_view->table == 2)
						offset+=MS_PAGE_SIZE;
					dload_val = convert_before_download(offset,value,FALSE);
					write_ve_const(dload_val,offset,FALSE);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(
								ve_widgets[offset]),
							value);

				}
			}
			else		// Spark Table
			{
				if (ign_ptr->spark_table[(ve_view->active_load*8)+ve_view->active_rpm] < 255)
				{
					offset = (ve_view->active_load*8)+ve_view->active_rpm;
					value = ign_ptr->spark_table[offset] + 1;
					write_ve_const(value,offset,TRUE);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(ign_widgets[offset]),value/2.84);
				}
			}
			break;				

		case GDK_minus:
		case GDK_KP_Subtract:
#ifdef GLDEBUG
			g_printf("MINUS\n");
#endif
			if (ve_view->table != 3) // STD VE table
			{
				if (ve_ptr->ve_bins[(ve_view->active_load*8)+ve_view->active_rpm] > 0)
				{
					offset = (ve_view->active_load*8)+ve_view->active_rpm;
					value = ve_ptr->ve_bins[offset] - 1;
					if (ve_view->table == 2)
						offset+=MS_PAGE_SIZE;
					dload_val = convert_before_download(offset,value,FALSE);
					write_ve_const(dload_val,offset,FALSE);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(
								ve_widgets[offset]),
							value);
				}
			}
			else		// Spark Table
			{
				if (ign_ptr->spark_table[(ve_view->active_load*8)+ve_view->active_rpm] > 0)
				{
					offset = (ve_view->active_load*8)+ve_view->active_rpm;
					value = ign_ptr->spark_table[offset] - 1;
					write_ve_const(value,offset,TRUE);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(ign_widgets[offset]),value/2.84);
				}
			}
			break;							

		default:
			return FALSE;
	}

	gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);

	return TRUE;
}

gboolean ve_focus_in_event (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{	
	gtk_widget_grab_focus (widget);
	gtk_widget_map(widget);
	gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);
	return TRUE;
}
void initialize_ve_view(void *ptr)
{
	struct Ve_View_3D *ve_view; 
	ve_view = ptr;
	ve_view->beginX = 0;
	ve_view->beginY = 0;
	ve_view->active_load = 0;
	ve_view->active_rpm = 0;
	ve_view->dt = 0.008;
	ve_view->sphi = 35.0;
	ve_view->stheta = 75.0;
	ve_view->sdepth = 7.533;
	ve_view->zNear = 0.8;
	ve_view->zFar = 23.0;
	ve_view->aspect = 1.0;
	ve_view->rpm_div = 0.0;
	ve_view->load_div = 0.0;
	ve_view->ve_div = 0.0;
	ve_view->rpm_max = 0;
	ve_view->load_max = 0;
	ve_view->ve_max = 0;
	return;
}

