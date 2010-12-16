plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libfreeems.la 

libfreeems_la_SOURCES = \
	Makefile \
	freeems_comms.c \
	freeems_comms.h \
	freeems_plugin.c \
	freeems_plugin.h \
	interrogate.c \
	interrogate.h

libfreeems_la_LDFLAGS = -shared $(no_undefined) -module
libfreeems_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


