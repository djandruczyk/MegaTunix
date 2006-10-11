INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @MEGATUNIX_DEFINES@


demo_SOURCES = gauge.c gauge.h gauge-private.h demo.c

noinst_PROGRAMS=demo

noinst_LTLIBRARIES = libmtxgauge.la

libmtxgauge_la_SOURCES = gauge.c gauge.h gauge-private.h

libmtxgauge_la_LDFLAGS = @GTK_LIBS@ @EXTRA_LIBS@

demo_LDADD = @GTK_LIBS@ @EXTRA_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


