plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libfreeems.la 

libfreeems_la_SOURCES = \
	Makefile \
	datamgmt.c \
	datamgmt.h \
	freeems_globaldefs.h \
	freeems_comms.c \
	freeems_comms.h \
	freeems_gui_handlers.c \
	freeems_gui_handlers.h \
	freeems_helpers.c \
	freeems_helpers.h \
	freeems_plugin.c \
	freeems_plugin.h \
	interrogate.c \
	interrogate.h \
	packet_handlers.c \
	packet_handlers.h \
	vetable_gui.c \
	vetable_gui.h 

libfreeems_la_LDFLAGS = -shared $(no_undefined) -module
libfreeems_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


