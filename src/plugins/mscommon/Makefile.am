plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libmscommon.la 

libmscommon_la_SOURCES = \
	Makefile \
	afr_calibrate.c \
	dataio.c \
	dataio.h \
	datamgmt.c \
	datamgmt.h \
	fileio.c \
	fileio.h \
	mscommon_comms.c \
	mscommon_comms.h \
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
	vex_support.c \
	vex_support.h


libmscommon_la_LDFLAGS = -shared ../../../mtxcommon/libmtxcommon.la @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ $(no_undefined) -module

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


