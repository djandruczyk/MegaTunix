INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@

demo_SOURCES = demo.c gauge.h

piedemo_SOURCES = piedemo.c piegauge.h

apifix_SOURCES = apifix.c gauge.h

noinst_PROGRAMS = demo piedemo apifix

noinst_LTLIBRARIES = libmtxgauge.la

libmtxgauge_la_SOURCES = gauge.c gauge.h gauge-xml.c gauge-xml.h gauge-private.c gauge-private.h piegauge.c piegauge.h piegauge-private.c piegauge-private.h

libmtxgauge_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 

demo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

piedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

apifix_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -pedantic

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


