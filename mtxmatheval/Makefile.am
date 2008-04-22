# Copyright (C) 1999, 2002, 2003, 2004  Free Software Foundation, Inc.
#
# This file is part of GNU libmatheval
#
# GNU libmatheval is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2, or (at
# your option) any later version.
#
# GNU libmatheval is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with program; see the file COPYING. If not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# Taken from libmatheval with permission of author for use internally for
# MegaTunix.  Library name changed from libmatheval to libmtxmatheval to
# prevent name clash if library already installed on users system (not likely)
#

MAINTAINERCLEANFILES = Makefile.in parser.c parser.h scanner.c
CLEANFILES = parser.c parser.h scanner.c

EXTRA_DIST = parser.h

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall -pedantic

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

noinst_LTLIBRARIES = libmtxmatheval.la

libmtxmatheval_la_SOURCES = parser.y scanner.l error.c matheval.c	\
node.c symbol_table.c xmalloc.c xmath.c

libmtxmatheval_la_LDFLAGS = -static
libmtxmatheval_la_LIBADD = -lfl -lm 

noinst_HEADERS = mtxmatheval.h common.h error.h node.h \
	symbol_table.h xmalloc.h xmath.h

AM_YFLAGS = -d
