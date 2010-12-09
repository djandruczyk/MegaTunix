plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
	-I$(top_srcdir)/mtxmatheval \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
	@LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libmscommon.la 

libmscommon_la_SOURCES = \
	Makefile \
	2d_table_editor.c \
	2d_table_editor.h \
	afr_calibrate.c \
	afr_calibrate.h \
	dataio.c \
	dataio.h \
	datamgmt.c \
	datamgmt.h \
	dep_loader.c \
	dep_loader.h \
	dep_processor.c \
	dep_processor.h \
	fileio.c \
	fileio.h \
	interrogate.c \
	interrogate.h \
	mscommon_comms.c \
	mscommon_comms.h \
	mscommon_gui_handlers.c \
	mscommon_gui_handlers.h \
	mscommon_helpers.c \
	mscommon_helpers.h \
	mscommon_plugin.c \
	mscommon_plugin.h \
	mtxsocket.c \
	mtxsocket.h \
	req_fuel.c \
	req_fuel.h \
	therm.c \
	therm.h \
	user_outputs.c \
	user_outputs.h \
	vex_support.c \
	vex_support.h


libmscommon_la_LDFLAGS = -shared ../../../mtxcommon/libmtxcommon.la ../../../widgets/libmtxcombo_mask.la ../../../widgets/libmtxcurve.la ../../../widgets/libmtxgauge.la @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@ $(no_undefined) -module

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic -fno-stack-protector
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


