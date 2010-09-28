gaugedir = $(datadir)/MegaTunix/$(GAUGEDESIGNER_GLADE_DIR)

SUBDIRS = gauges

INCLUDES = \
        -I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
        @GTK_CFLAGS@ \
	@LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@

gaugedesigner_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@ @GLEXT_LIBS@ @WIN32_RESOURCES@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

gaugedesigner_SOURCES = alerts.c gd_init.h gd_init.c main.c events.c handlers.c loadsave.c polygons.c tblocks.c tgroups.c warnings.c alerts.h events.h loadsave.h handlers.h polygons.h tblocks.h tgroups.h warnings.h 

gaugedesigner_DEPENDENCIES = @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

bin_PROGRAMS = \
	       gaugedesigner

glade_files = \
	      main.glade \
	      a_range.glade \
	      polygon.glade \
	      tblock.glade \
	      tgroup.glade \
	      w_range.glade

#xml_files = 

#gauge_DATA = $(glade_files) $(xml_files)
gauge_DATA = $(glade_files)

EXTRA_DIST = README $(gauge_DATA) gaugedesigner.ico
