dashdesignerdir = $(datadir)/MegaTunix/$(DASHDESIGNER_GLADE_DIR)

SUBDIRS = dashboards

INCLUDES = \
        -I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
        @GTK_CFLAGS@ \
	@LIBXML2_CFLAGS@ \
        @MEGATUNIX_DEFINES@


dashdesigner_LDADD = @GTK_LIBS@ @LIBXML2_LIBS@ @EXTRA_LIBS@ @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la 

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused -pedantic

CLEANFILES = *~ core gmon.out nohup.out 
DISTCLEANFILES = *~ core gmon.out nohup.out  *.bak *.gladep

dashdesigner_SOURCES = \
		       main.c \
		       events.c \
		       events.h \
		       xml.c \
		       xml.h \
		       rtv_parser.c \
		       rtv_parser.h \
		       loadsave.c \
		       loadsave.h \
		       menu_handlers.c \
		       menu_handlers.h 

dashdesigner_DEPENDENCIES = @WIN32_RESOURCES@ ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

bin_PROGRAMS = dashdesigner 

ui_files = main.ui \
	   preview.ui \
	   propeditor.ui 

dashdesigner_DATA = $(ui_files)

EXTRA_DIST = $(dashdesigner_DATA) dashdesigner.ico
