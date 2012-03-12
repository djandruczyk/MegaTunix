INCLUDES = \
        -I$(top_srcdir)/include 

lib_LTLIBRARIES = libmtxyamlcpp.la

libmtxyamlcpp_la_SOURCES = aliasmanager.cpp \
				binary.cpp \
				conversion.cpp \
				directives.cpp \
				emitfromevents.cpp \
				emitter.cpp \
				emitterstate.cpp \
				emitterutils.cpp \
				exp.cpp \
				iterator.cpp \
				nodebuilder.cpp \
				node.cpp \
				nodeownership.cpp \
				null.cpp \
				ostream.cpp \
				parser.cpp \
				regex.cpp \
				scanner.cpp \
				scanscalar.cpp \
				scantag.cpp \
				scantoken.cpp \
				simplekey.cpp \
				singledocparser.cpp \
				stream.cpp \
				tag.cpp 

libmtxyamlcpp_la_LDFLAGS = -shared $(no_undefined) 
libmtxyamlcpp_la_LIBADD = @EXTRA_LIBS@ @GTK_LIBS@ @LIBXML2_LIBS@ 

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

if CROSS_COMPILING
no_undefined = -no-undefined
endif

AM_CPPFLAGS = $(COMMON_FLAGS) -Wall -Wextra -Wno-unused 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = 


