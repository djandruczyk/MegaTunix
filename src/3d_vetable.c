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

static int grid = 8;
static int beginX, beginY;
static int active_map, active_rpm = 0;
  
static float dt = 0.008;
static float sphi = 35.0; // 45.0
static float stheta = 75.0; 
static float sdepth = 7.533;
static float zNear = 0.8;
static float zFar = 23;
static float aspect = 1.0;
static float rpm_div=0.0, kpa_div=0.0,ve_div=0.0;
static int rpm_max=0, kpa_max=0, ve_max=0;
static GtkWidget *drawing_area;
static gchar font_string[] = "sans 10";
static GLuint font_list_base;
static gint font_height;
static gchar label[6];

extern struct Ve_Const_Std *ve_const_p0;
extern struct Ve_Const_Std *ve_const_p1;

void reset_3d_view()
{
	grid = 8;
	active_map = 0;
	active_rpm = 0;
	dt = 0.008;
	sphi = 35.0; 
	stheta = 75.0; 
	sdepth = 7.533;
	zNear = 0.8;
	zFar = 23;
	aspect = 1.333;
	ve_configure_event(drawing_area, NULL,NULL);
	ve_expose_event(drawing_area, NULL,NULL);
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

	GLfloat w = widget->allocation.width;
	GLfloat h = widget->allocation.height;

	#ifdef GLDEBUG
	//printf("%i Got Configure\n", clock());
	#endif
	
	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return FALSE;

	aspect = (float)w/(float)h;
	glViewport (0, 0, w, h);

	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/                                                                                                                  
	return TRUE;
}

gboolean ve_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	if (!GTK_WIDGET_HAS_FOCUS(widget)){
		gtk_widget_grab_focus(widget);
	}
	
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	
	#ifdef GLDEBUG
	//printf("%i Got Expose\n", clock());
	#endif
	
	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return FALSE;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(64.0, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0,0.0,-sdepth);
	glRotatef(-stheta, 1.0, 0.0, 0.0);
	glRotatef(sphi, 0.0, 0.0, 1.0);
	glTranslatef(-(float)((grid)/2-0.3), -(float)((grid)/2-1), -2.0);

	ve_calculate_scaling();
	ve_draw_ve_grid();
	ve_draw_active_indicator();
	ve_draw_axis();

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

	if (event->state & GDK_BUTTON1_MASK)
	{
		sphi += (float)(event->x - beginX) / 4.0;
		stheta += (float)(beginY - event->y) / 4.0;
		redraw = TRUE;
	}

	if (event->state & GDK_BUTTON2_MASK)
	{
		sdepth -= ((event->y - beginY)/(widget->allocation.height))*(grid);
		redraw = TRUE;
	}

	beginX = event->x;
	beginY = event->y;

	gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);

	return TRUE;
}

gboolean ve_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gtk_widget_grab_focus (widget);
	
	if (event->button == 1)
	{
		beginX = event->x;
		beginY = event->y;
		return TRUE;
	}

	if (event->button == 2)
	{
		beginX = event->x;
		beginY = event->y;
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

void ve_calculate_scaling(void)
{
	int i=0;
	rpm_max = 0;
	kpa_max = 0;
	ve_max = 0;
	
	/* calculate scaling */
	for (i=0;i<grid;i++) {
		if (ve_const_p0->rpm_bins[i] > rpm_max) {
			rpm_max=ve_const_p0->rpm_bins[i];
		}
		if (ve_const_p0->kpa_bins[i] > kpa_max) {
			kpa_max=ve_const_p0->kpa_bins[i];
		}
	}

	for (i=0;i<grid*8;i++) {
		if (ve_const_p0->ve_bins[i] > ve_max) {
			ve_max=ve_const_p0->ve_bins[i];
		}
	}	

	rpm_div = ((float)rpm_max/8.0);
	kpa_div = ((float)kpa_max/8.0);
	ve_div  = ((float)ve_max/4.0);	
}

void ve_draw_ve_grid(void)
{
	int rpm=0, map=0;

	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(1.5);
	
	/* Draw lines on RPM axis */
	for(rpm=0;rpm<grid;rpm++)
	{
		glBegin(GL_LINE_STRIP);
		for(map=0;map<grid;map++) {
			glVertex3f(
					(float)(ve_const_p0->rpm_bins[rpm])/rpm_div,			
					(float)(ve_const_p0->kpa_bins[map])/kpa_div, 	 	
					(float)(ve_const_p0->ve_bins[(map*8)+rpm])/ve_div);
		}
		glEnd();
	}
	
	/* Draw lines on MAP axis */
	for(map=0;map<grid;map++)
	{
		glBegin(GL_LINE_STRIP);
		for(rpm=0;rpm<grid;rpm++){
			glVertex3f(	
					(float)(ve_const_p0->rpm_bins[rpm])/rpm_div,
					(float)(ve_const_p0->kpa_bins[map])/kpa_div,			
					(float)(ve_const_p0->ve_bins[(map*8)+rpm])/ve_div);	
		}
		glEnd();
	}
}

void ve_draw_active_indicator(void)
{
	/* Render a red dot at the active VE map position */
	glPointSize(8.0);
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_POINTS);
	glVertex3f(	
			(float)(ve_const_p0->rpm_bins[active_rpm])/rpm_div,
			(float)(ve_const_p0->kpa_bins[active_map])/kpa_div,	
			(float)(ve_const_p0->ve_bins[(active_map*8)+active_rpm])/ve_div);
	glEnd();	
}


void ve_draw_axis(void)
{
	/* Set vars and an asthetically pleasing maximum value */
	int i=0, rpm=0, map=0;
	float top = ((float)(ve_max+20))/ve_div;
	
	/* Set line thickness and color */
	glLineWidth(1.0);
	glColor3f(0.7,0.7,0.7);
	
	/* Draw horizontal background grid lines  
	   starting at 0 VE and working up to VE+20% */
	for (i=0;i<(ve_max+20);i = i + 10){
		glBegin(GL_LINE_STRIP);
		glVertex3f(
			((ve_const_p0->rpm_bins[0])/rpm_div),
			((ve_const_p0->kpa_bins[7])/kpa_div),		
			((float)i)/ve_div);
		glVertex3f(
			((ve_const_p0->rpm_bins[7])/rpm_div),
			((ve_const_p0->kpa_bins[7])/kpa_div),		
			((float)i)/ve_div);
		glVertex3f(
			((ve_const_p0->rpm_bins[7])/rpm_div),
			((ve_const_p0->kpa_bins[0])/kpa_div),		
			((float)i)/ve_div);
		glEnd();	
	}
	
	/* Draw vertical background grid lines along KPA axis */
	for (i=0;i<8;i++){
		glBegin(GL_LINES);
		glVertex3f(
			((ve_const_p0->rpm_bins[7])/rpm_div),
			((ve_const_p0->kpa_bins[i])/kpa_div),		
			0.0);
		glVertex3f(
			((ve_const_p0->rpm_bins[7])/rpm_div),
			((ve_const_p0->kpa_bins[i])/kpa_div),		
			top);
		glEnd();
	}
	
	/* Draw vertical background lines along RPM axis */
	for (i=0;i<8;i++){
		glBegin(GL_LINES);
		glVertex3f(
			((ve_const_p0->rpm_bins[i])/rpm_div),		
			((ve_const_p0->kpa_bins[7])/kpa_div),
			0.0);
		glVertex3f(
			((ve_const_p0->rpm_bins[i])/rpm_div),
			((ve_const_p0->kpa_bins[7])/kpa_div),		
			top);
		glEnd();
	}
	
	/* Add the back corner top lines */
	glBegin(GL_LINE_STRIP);
	glVertex3f(
		((ve_const_p0->rpm_bins[0])/rpm_div),	
		((ve_const_p0->kpa_bins[7])/kpa_div),
		top);
	glVertex3f(
		((ve_const_p0->rpm_bins[7])/rpm_div),	
		((ve_const_p0->kpa_bins[7])/kpa_div),
		top);
	glVertex3f(
		((ve_const_p0->rpm_bins[7])/rpm_div),
		((ve_const_p0->kpa_bins[0])/kpa_div),	
		top);
	glEnd();
	
	/* Add front corner base lines */
	glBegin(GL_LINE_STRIP);
	glVertex3f(
		((ve_const_p0->rpm_bins[0])/rpm_div),	
		((ve_const_p0->kpa_bins[7])/kpa_div),
		0.0);
	glVertex3f(
		((ve_const_p0->rpm_bins[0])/rpm_div),	
		((ve_const_p0->kpa_bins[0])/kpa_div),
		0.0);
	glVertex3f(
		((ve_const_p0->rpm_bins[7])/rpm_div),
		((ve_const_p0->kpa_bins[0])/kpa_div),
		0.0);
	glEnd();
	
	/* Draw RPM and KPA labels */
	for (i=0;i<8;i++){
		rpm = (ve_const_p0->rpm_bins[i])*100;
		map = (ve_const_p0->kpa_bins[i]);
		sprintf(label,"%i",map);
		ve_drawtext(label,
			((ve_const_p0->rpm_bins[0])/rpm_div),
			((ve_const_p0->kpa_bins[i])/kpa_div),
			0.0);
		
		sprintf(label,"%i",rpm);
		ve_drawtext(label,
			((ve_const_p0->rpm_bins[i])/rpm_div),
			((ve_const_p0->kpa_bins[0])/kpa_div),
			0.0);
	}
	
	/* Draw VE labels */
	for (i=0;i<(ve_max+20);i=i+10){
		sprintf(label,"%i",i);
		ve_drawtext(label,
			((ve_const_p0->rpm_bins[0])/rpm_div),
			((ve_const_p0->kpa_bins[7])/kpa_div),
			(float)i/ve_div);
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
	gint page = 0;
	gint value = 0;
	gint offset = 0;
	gint dload_val = 0;
	extern struct Ve_Widgets *page0_widgets;
//	extern struct Ve_Widgets *page1_widgets;
	#ifdef GLDEBUG	
	printf("Key press event\n");
	#endif
	switch (event->keyval)
	{
		case GDK_Up:
			#ifdef GLDEBUG
			printf("UP\n");
			#endif
			if (active_map < 7)
				active_map += 1;
			break;

		case GDK_Down:
			#ifdef GLDEBUG
			printf("DOWN\n");
			#endif
			if (active_map > 0)
				active_map -= 1;
			break;				

		case GDK_Left:
			#ifdef GLDEBUG
			printf("LEFT\n");
			#endif
			if (active_rpm > 0)
				active_rpm -= 1;
			break;					

		case GDK_Right:
			#ifdef GLDEBUG
			printf("RIGHT\n");
			#endif
			if (active_rpm < 7)
				active_rpm += 1;
			break;				

		case GDK_plus:
		case GDK_KP_Add:
			#ifdef GLDEBUG
			printf("PLUS\n");
			#endif
			if (ve_const_p0->ve_bins[(active_map*8)+active_rpm] < 255)
			{
				page = 0;  // < Change this when dualtable works
				offset = (active_map*8)+active_rpm;
				value = ve_const_p0->ve_bins[offset] + 1;
				dload_val = convert_before_download(offset,value,page);
				write_ve_const(dload_val,offset,page);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
						page0_widgets->widget[offset]),
						value);

			}
			break;				

		case GDK_minus:
		case GDK_KP_Subtract:
			#ifdef GLDEBUG
			printf("MINUS\n");
			#endif
			if (ve_const_p0->ve_bins[(active_map*8)+active_rpm] > 0)
			{
				page = 0;  // < Change this when dualtable works
				offset = (active_map*8)+active_rpm;
				value = ve_const_p0->ve_bins[offset] - 1;
				dload_val = convert_before_download(offset,value,page);
				write_ve_const(dload_val,offset,page);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(
						page0_widgets->widget[offset]),
						value);
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
