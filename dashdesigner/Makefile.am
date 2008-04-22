dashdir = $(datadir)/MegaTunix/$(DASHDESIGNER_GLADE_DIR)

SUBDIRS = dashboards

INCLUDES = \
        -I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
        @GTK_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


dashdesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ @EXTRA_LIBS@ @GLEXT_LIBS@ @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la 

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -pedantic

CLEANFILES = *~ core gmon.out nohup.out 
DISTCLEANFILES = *~ core gmon.out nohup.out  *.bak *.gladep

dashdesigner_SOURCES = main.c events.c events.h xml.c xml.h rtv_parser.c rtv_parser.h loadsave.c loadsave.h

dashdesigner_DEPENDENCIES = @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

bin_PROGRAMS = dashdesigner 

glade_files = dashdesigner.glade 

dash_DATA = $(glade_files)

EXTRA_DIST = $(dash_DATA)
