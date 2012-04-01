plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libjimstim.la 

libjimstim_la_SOURCES = \
	Makefile \
	jimstim_plugin.c \
	jimstim_plugin.h \
	jimstim_gui_handlers.c \
	jimstim_gui_handlers.h \
	jimstim_menu_handlers.c \
	jimstim_menu_handlers.h \
	jimstim_sweeper.h \
	jimstim_sweeper.c 

libjimstim_la_LDFLAGS = -shared $(no_undefined) -module
libjimstim_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CFLAGS += $(COMMON_FLAGS)

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


