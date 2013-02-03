INCLUDES = \
        -I$(top_srcdir)/include \
        `pkg-config --cflags glib-2.0`

lib_LTLIBRARIES = libmtx_glib_bug_override.la

libmtx_glib_bug_override_la_SOURCES = gthread-posix-override.c
libmtx_glib_bug_override_la_LDFLAGS = -shared $(no_undefined) -ldl -Wc,-nostdlib
libmtx_glib_bug_override_la_LIBADD = 

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CFLAGS = $(COMMON_FLAGS) -Wall -fPIC -DPIC

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = 


