dashdir = $(datadir)/MegaTunix/$(DASH_DATA_DIR)

INCLUDES = \
	-I/usr/include \
        -I$(top_srcdir)/include \
	-I../widgets \
        @GTK_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


dashdesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@ @GLEXT_LIBS@ @WIN32_RESOURCES@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

dashdesigner_SOURCES = dashdesigner.c events.c events.h 

dashdesigner_DEPENDENCIES = @WIN32_RESOURCES@

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

bin_PROGRAMS = dashdesigner 

glade_files = dashdesigner.glade 

dash_DATA = $(glade_files)

EXTRA_DIST = README $(dash_DATA)
