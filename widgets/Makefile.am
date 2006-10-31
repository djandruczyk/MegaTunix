INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@


demo_SOURCES = demo.c gauge.h

noinst_PROGRAMS=demo

noinst_LTLIBRARIES = libmtxgauge.la

libmtxgauge_la_SOURCES = gauge.c gauge.h gauge-xml.c gauge-xml.h gauge-private.c gauge-private.h

libmtxgauge_la_LDFLAGS = @GTK_LIBS@ @LIBXML2_LIBS@ @EXTRA_LIBS@

demo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@ @EXTRA_LIBS@ libmtxgauge.la

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


