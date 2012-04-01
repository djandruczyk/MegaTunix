plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libsecu3.la 

libsecu3_la_SOURCES = \
	Makefile \
	secu3_plugin.c \
	secu3_plugin.h \
	secu3_gui_handlers.c \
	secu3_gui_handlers.h \
	secu3_menu_handlers.c \
	secu3_menu_handlers.h 

libsecu3_la_LDFLAGS = -shared $(no_undefined) -module
libsecu3_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CFLAGS += $(COMMON_FLAGS)

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


