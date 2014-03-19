plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = liblibreems.la 

liblibreems_la_SOURCES = \
	Makefile \
	datamgmt.c \
	datamgmt.h \
	fileio.c \
	fileio.h \
	libreems_benchtest.c \
	libreems_benchtest.h \
	libreems_errors.c \
	libreems_errors.h \
	libreems_globaldefs.h \
	libreems_comms.c \
	libreems_comms.h \
	libreems_menu_handlers.c \
	libreems_menu_handlers.h \
	libreems_gui_handlers.c \
	libreems_gui_handlers.h \
	libreems_helpers.c \
	libreems_helpers.h \
	libreems_plugin.c \
	libreems_plugin.h \
	libreems_rtv_loader.c \
	libreems_rtv_loader.h \
	libreems_rtv_processor.c \
	libreems_rtv_processor.h \
	interrogate.c \
	interrogate.h \
	packet_handlers.c \
	packet_handlers.h \
	vetable_gui.c \
	vetable_gui.h 

liblibreems_la_LDFLAGS = -shared $(no_undefined) -module
liblibreems_la_LIBADD = ../../../mtxcommon/libmtxcommon.la ../../../widgets/libmtxcombo_mask.la ../../../widgets/libmtxcurve.la ../../../widgets/libmtxgauge.la @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@


COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CFLAGS += $(COMMON_FLAGS)

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


