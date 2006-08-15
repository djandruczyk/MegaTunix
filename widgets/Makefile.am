INCLUDES = \
        -I$(top_srcdir)/include \
        @PACKAGE_CFLAGS@ \
        @MEGATUNIX_DEFINES@


demo_SOURCES = gauge.c gauge.h demo.c

bin_PROGRAMS=demo

demo_LDADD = @PACKAGE_LIBS@ @EXTRA_LIBS@

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 


EXTRA_DIST = README


