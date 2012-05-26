INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @DEPRECATED_CFLAGS@ \
	@LIBGLADE_CFLAGS@ \
	@LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@

lib_LTLIBRARIES = libmtxcommon.la

libmtxcommon_la_SOURCES = getfiles.c configfile.c xmlbase.c

libmtxcommon_la_LDFLAGS = -shared $(no_undefined) 
libmtxcommon_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = 


