gaugedesignerdir = $(datadir)/MegaTunix/$(GAUGEDESIGNER_GLADE_DIR)

SUBDIRS = gauges

INCLUDES = \
        -I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
        @GTK_CFLAGS@ \
	@LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@

gaugedesigner_LDADD = ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la @GTK_LIBS@ @LIBXML2_LIBS@ @EXTRA_LIBS@ @WIN32_RESOURCES@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 

gaugedesigner_SOURCES = alerts.c gd_init.h gd_init.c main.c menu_handlers.c menu_handlers.h events.c handlers.c loadsave.c polygons.c tblocks.c tgroups.c warnings.c alerts.h events.h loadsave.h handlers.h polygons.h tblocks.h tgroups.h warnings.h 

gaugedesigner_DEPENDENCIES = @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

bin_PROGRAMS = \
	       gaugedesigner

ui_files = \
	      main.ui \
	      a_range.ui \
	      polygon.ui \
	      tblock.ui \
	      tgroup.ui \
	      w_range.ui


gaugedesigner_DATA = $(ui_files)

EXTRA_DIST = README $(gaugedesigner_DATA) gaugedesigner.ico
