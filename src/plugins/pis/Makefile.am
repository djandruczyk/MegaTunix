plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libpis.la 

libpis_la_SOURCES = \
	Makefile \
	pis_plugin.c \
	pis_plugin.h \
	pis_gui_handlers.c \
	pis_gui_handlers.h

libpis_la_LDFLAGS = -shared $(no_undefined) -module
libpis_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


