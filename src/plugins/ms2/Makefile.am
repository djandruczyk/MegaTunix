plugindir = $(datadir)/MegaTunix/plugins

INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
	@LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


plugin_LTLIBRARIES = libms2.la 

libms2_la_SOURCES = \
	Makefile \
	ms2_gui_handlers.c \
	ms2_gui_handlers.h \
	ms2_plugin.c \
	ms2_plugin.h \
	ms2_tlogger.c \
	ms2_tlogger.h \
	user_outputs.c \
	user_outputs.h

libms2_la_LDFLAGS = -shared @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ @LIBGLADE_LIBS@ ../../../widgets/libmtxcombo_mask.la $(no_undefined) -module

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README


