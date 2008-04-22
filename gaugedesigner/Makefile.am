gaugedir = $(datadir)/MegaTunix/$(GAUGEDESIGNER_GLADE_DIR)

SUBDIRS = gauges

INCLUDES = \
        -I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
        @GTK_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@

gaugedesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@ @GLEXT_LIBS@ @WIN32_RESOURCES@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

gaugedesigner_SOURCES = main.c events.c loadsave.c events.h loadsave.h handlers.c handlers.h

gaugedesigner_DEPENDENCIES = @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

bin_PROGRAMS = \
	       gaugedesigner

glade_files = \
	      gaugedesigner.glade

#xml_files = 

#gauge_DATA = $(glade_files) $(xml_files)
gauge_DATA = $(glade_files)

EXTRA_DIST = README $(gauge_DATA)
