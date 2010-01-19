INCLUDES = \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@

gaugedemo_SOURCES = gauge-demo.c gauge.h

curvedemo_SOURCES = curve-demo.c curve.h

piedemo_SOURCES = pie-demo.c piegauge.h

progressdemo_SOURCES = progress-demo.c progress.h

stripchartdemo_SOURCES = stripchart-demo.c stripchart.h

apifix_SOURCES = apifix.c gauge.h

noinst_PROGRAMS = gaugedemo piedemo apifix curvedemo progressdemo stripchartdemo

noinst_LTLIBRARIES = libmtxgauge.la libmtxcurve.la libmtxpie.la libmtxprogress.la libmtxstripchart.la

libmtxgauge_la_SOURCES = gauge.c gauge.h gauge-xml.c gauge-xml.h gauge-private.c gauge-private.h

libmtxcurve_la_SOURCES = curve.c curve.h curve-private.c curve-private.h

libmtxpie_la_SOURCES = piegauge.c piegauge.h piegauge-private.c piegauge-private.h

libmtxprogress_la_SOURCES = progress.c progress.h progress-private.c progress-private.h

libmtxstripchart_la_SOURCES = stripchart.c stripchart.h stripchart-private.c stripchart-private.h

libmtxgauge_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 
libmtxcurve_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 
libmtxpie_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 
libmtxprogress_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 
libmtxstripchart_la_LDFLAGS = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 

gaugedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

piedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxpie.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

progressdemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxprogress.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

curvedemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxcurve.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

stripchartdemo_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxstripchart.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

apifix_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@  ./libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

#AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic
AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


