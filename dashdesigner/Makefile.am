dashdir = $(datadir)/MegaTunix/$(DASH_DATA_DIR)

INCLUDES = \
	-I/usr/include \
        -I$(top_srcdir)/include \
	-I../widgets \
        @GTK_CFLAGS@ \
        @LIBGLADE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


dashdesigner_LDADD = @GTK_LIBS@ @LIBGLADE_LIBS@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la @EXTRA_LIBS@ @GLEXT_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

dashdesigner_SOURCES = dashdesigner.c


bin_PROGRAMS = dashdesigner 

glade_files = dashdesigner.glade 

dash_DATA = $(glade_files)

EXTRA_DIST = README $(dash_DATA)
