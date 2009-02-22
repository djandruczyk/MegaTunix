INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@

gaugedemo_SOURCES = gauge-demo.c gauge.h

curvedemo_SOURCES = curve-demo.c curve.h

piedemo_SOURCES = piedemo.c piegauge.h

progressdemo_SOURCES = progressdemo.c progress.h

apifix_SOURCES = apifix.c gauge.h

noinst_PROGRAMS = gaugedemo piedemo apifix curvedemo progressdemo

noinst_LTLIBRARIES = libmtxgauge.la

libmtxgauge_la_SOURCES = gauge.c gauge.h gauge-xml.c gauge-xml.h gauge-private.c gauge-private.h piegauge.c piegauge.h piegauge-private.c piegauge-private.h curve.c curve.h curve-private.c curve-private.h progress.c progress.h progress-private.c progress-private.h

libmtxgauge_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 

gaugedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

piedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

progressdemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

curvedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

apifix_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -pedantic

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


