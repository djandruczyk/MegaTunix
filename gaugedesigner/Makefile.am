gaugedir = $(datadir)/MegaTunix/$(GAUGE_DATA_DIR)

INCLUDES = \
	-I/usr/include \
        -I$(top_srcdir)/include \
	-I../widgets \
        @GTK_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@

gaugedesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@ @GLEXT_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

gaugedesigner_SOURCES = main.c events.c events.h


bin_PROGRAMS = \
	       gaugedesigner

glade_files = \
	      gaugedesigner.glade

#xml_files = 

#gauge_DATA = $(glade_files) $(xml_files)
gauge_DATA = $(glade_files)

EXTRA_DIST = README $(gauge_DATA)
