plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libms1.la 

libms1_la_SOURCES = \
	Makefile \
	ms1_gui_handlers.c \
	ms1_gui_handlers.h \
	ms1_helpers.c \
	ms1_helpers.h \
	ms1_plugin.c \
	ms1_plugin.h \
	ms1_tlogger.c \
	ms1_tlogger.h

libms1_la_LDFLAGS = -shared @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ $(no_undefined) -module

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


