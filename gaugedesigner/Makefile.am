INCLUDES = \
	-I/usr/include \
        -I$(top_srcdir)/include \
        @GTK_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


gaugedesigner_SOURCES = main.c events.c events.h ../widgets/gauge.h

noinst_PROGRAMS = gaugedesigner

gaugedesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ ../widgets/libmtxgauge.la @EXTRA_LIBS@ 
gaugedesigner_LDFLAGS =  -Wl,-export-dynamic

#gaugedesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ @EXTRA_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


