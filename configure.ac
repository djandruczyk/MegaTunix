dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])

MTX_MAJOR_VERSION=0
MTX_MINOR_VERSION=9
MTX_MICRO_VERSION=20
MTX_VER_SUFFIX=~git
AC_SUBST(MTX_MAJOR_VERSION)
AC_SUBST(MTX_MINOR_VERSION)
AC_SUBST(MTX_MICRO_VERSION)
AC_SUBST(MTX_VER_SUFFIX)

AC_CONFIG_SUBDIRS(src/main.c)
if test x"$MTX_VER_SUFFIX" != "x"; then
	VER_STRING="$MTX_MAJOR_VERSION.$MTX_MINOR_VERSION.$MTX_MICRO_VERSION-$MTX_VER_SUFFIX"
else
	VER_STRING="$MTX_MAJOR_VERSION.$MTX_MINOR_VERSION.$MTX_MICRO_VERSION"
fi
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(megatunix, "$VER_STRING")

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_MKDIR_P
AM_PROG_MKDIR_P
AC_PROG_MAKE_SET
AM_GNU_GETTEXT([external])
AM_PROG_LEX
if test "$LEX" != "flex"; 
then
	echo "You need the 'flex' package"
	exit 1
fi
AC_PROG_YACC
if test "$YACC" != "bison -y";
then
	echo "You need the 'bison' package"
	exit 1
fi

dnl Checks for header files.
dnl AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h math.h stddef.h stdio.h string.h strings.h sys/poll.h sys/stat.h sys/types.h sys/ioctl.h sys/time.h termios.h unistd.h )
dnl check paths for necessary files

dnl MACRO definition to determine if running GTK>2.12 or not for use of cairo
AC_DEFUN([PKG_CHECK_GTK], 
[
	PKG_CHECK_MODULES(GTK,[gtk+-2.0 >= 2.12.0 gthread-2.0],,)
	AC_SUBST(GTK_CFLAGS)
	AC_SUBST(GTK_LIBS)
])

PKG_CHECK_GTK()

dnl check for gtkglext library
PKG_CHECK_MODULES(GLEXT, [gtkglext-1.0 gdkglext-1.0])
AC_SUBST(GLEXT_CFLAGS)
AC_SUBST(GLEXT_LIBS)

dnl check for glade-2.x library
PKG_CHECK_MODULES(LIBGLADE,libglade-2.0 )
if test `uname -s` != "Darwin"; then
	LIBGLADE_LIBS+="-Wl,--export-dynamic"
fi
AC_SUBST(LIBGLADE_CFLAGS)
AC_SUBST(LIBGLADE_LIBS)

dnl check for libxml-2.x library
PKG_CHECK_MODULES(LIBXML2,libxml-2.0 )
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)

AC_CHECK_SIZEOF([int *])
if test $ac_cv_sizeof_int_p -eq 8; then
	AC_DEFINE([__64BIT__], [], [OS has 64 bit pointers])
else
	AC_DEFINE([__32BIT__], [], [OS has 32 bit pointers])
fi


dnl ===============
dnl Device specific stuff
dnl ===============

AC_ARG_ENABLE(pis,
[  --enable-pis         enable pis support [default=no]],, enable_pis=no)

if test "$enable_pis" = yes ; then
	AC_DEFINE([__PIS_SUPPORT__], [], [User requested PIS firmare support])
fi
dnl ===============
dnl Debugging stuff
dnl ===============

dnl General Debugging code
AC_ARG_ENABLE(debug,
[  --enable-debug         enable debugging [default=no]],, enable_debug=no)

if test "$enable_debug" = yes ; then
DEBUG_FLAGS="$DEBUG_FLAGS -g -DDEBUG"
CFLAGS="-O0"
else
CFLAGS="-O2"
fi
AC_SUBST(DEBUG_FLAGS)
AC_SUBST(CFLAGS)

AM_CONDITIONAL(CROSS_COMPILING, test $cross_compiling = yes)
if test "$cross_compiling" = yes ; then
	AC_CHECK_TOOL(WINDRES, windres)
	WIN32_RESOURCES="resources.o "
	if test "$enable_debug" = yes ; then
		EXTRA_LIBS=" -lgw32c -luuid -lole32 -lws2_32"
	else
		EXTRA_LIBS=" -mwindows -lgw32c -luuid -lole32 -lws2_32"
	fi
else
	EXTRA_LIBS=""
fi

AC_SUBST(WIN32_RESOURCES)
AC_SUBST(EXTRA_LIBS)


dnl =================
dnl big/little endian
dnl =================
AC_C_BIGENDIAN(,,)

dnl ==============
dnl profiling
dnl =============

AC_ARG_ENABLE(profiling,
[  --enable-profiling     enable profiling ],, enable_profiling=no)

if test "$enable_profiling" = yes; then
PROF_FLAGS="-pg"
fi
AC_SUBST(PROF_FLAGS)

dnl ==============
dnl Output files for build
dnl ==============

DASHDESIGNER_GLADE_DIR=dashdesigner
GAUGEDESIGNER_GLADE_DIR=gaugedesigner
GUI_DATA_DIR=Gui
GAUGES_DATA_DIR=Gauges
DASHES_DATA_DIR=Dashboards
INTERROGATOR_DATA_DIR=Interrogator
SNAPSHOTS_DATA_DIR=ecu_snapshots
LOOKUPTABLES_DATA_DIR=LookupTables
REALTIME_MAPS_DATA_DIR=RealtimeMaps
RTSLIDERS_DATA_DIR=RuntimeSliders
RTTEXT_DATA_DIR=RuntimeText
RTSTATUS_DATA_DIR=RuntimeStatus
DATA_DIR=$datarootdir/MegaTunix
LOCALE_DIR=$datarootdir/MegaTunix/locale

AC_SUBST(DATA_DIR)
AC_SUBST(LOCALE_DIR)
AC_SUBST(GUI_DATA_DIR)
AC_SUBST(SNAPSHOTS_DATA_DIR)
AC_SUBST(DASHES_DATA_DIR)
AC_SUBST(GAUGES_DATA_DIR)
AC_SUBST(GAUGEDESIGNER_GLADE_DIR)
AC_SUBST(DASHDESIGNER_GLADE_DIR)
AC_SUBST(INTERROGATOR_DATA_DIR)
AC_SUBST(LOOKUPTABLES_DATA_DIR)
AC_SUBST(REALTIME_MAPS_DATA_DIR)
AC_SUBST(RTSLIDERS_DATA_DIR)
AC_SUBST(RTTEXT_DATA_DIR)
AC_SUBST(RTSTATUS_DATA_DIR)

MEGATUNIX_DEFINES="$CFLAGS -DLOCALEDIR=\\\"$LOCALE_DIR\\\" -DDATA_DIR=\\\"$DATA_DIR\\\" -DSNAPSHOTS_DATA_DIR=\\\"$SNAPSHOTS_DATA_DIR\\\" -DLOOKUPTABLES_DATA_DIR=\\\"$LOOKUPTABLES_DATA_DIR\\\" -DINTERROGATOR_DATA_DIR=\\\"$INTERROGATOR_DATA_DIR\\\"  -DGUI_DATA_DIR=\\\"$GUI_DATA_DIR\\\" -DGAUGES_DATA_DIR=\\\"$GAUGES_DATA_DIR\\\" -DDASHES_DATA_DIR=\\\"$DASHES_DATA_DIR\\\" -DGAUGEDESIGNER_GLADE_DIR=\\\"$GAUGEDESIGNER_GLADE_DIR\\\" -DDASHDESIGNER_GLADE_DIR=\\\"$DASHDESIGNER_GLADE_DIR\\\" -DREALTIME_MAPS_DATA_DIR=\\\"$REALTIME_MAPS_DATA_DIR\\\" -DRTSLIDERS_DATA_DIR=\\\"$RTSLIDERS_DATA_DIR\\\" -DRTTEXT_DATA_DIR=\\\"$RTTEXT_DATA_DIR\\\" -DRTSTATUS_DATA_DIR=\\\"$RTSTATUS_DATA_DIR\\\" -DGTK_DISABLE_COMPAT_H -D_MAJOR_=$MTX_MAJOR_VERSION -D_MINOR_=$MTX_MINOR_VERSION -D_MICRO_=$MTX_MICRO_VERSION -D_VER_SUFFIX_=\\\"$MTX_VER_SUFFIX\\\" "

AC_SUBST(MEGATUNIX_DEFINES)

AC_CONFIG_FILES([ po/Makefile.in
Makefile 
include/Makefile
mtxcommon/Makefile
mtxmatheval/Makefile
src/Makefile
msloader/Makefile
msloader/resources.rc
gaugedesigner/resources.rc
gaugedesigner/Makefile
gaugedesigner/gauges/Makefile
gaugedesigner/gauges/AutoGauge/Makefile
gaugedesigner/gauges/Speco/Makefile
gaugedesigner/gauges/SportComp/Makefile
gaugedesigner/gauges/UltraLite/Makefile
gaugedesigner/gauges/Cobalt/Makefile
gaugedesigner/gauges/Impact/Makefile
gaugedesigner/gauges/Honda/Makefile
gaugedesigner/gauges/Ghost/Makefile
gaugedesigner/gauges/MegaTune/Makefile
src/resources.rc
po/Makefile
widgets/Makefile
dashdesigner/Makefile
dashdesigner/dashboards/Makefile
dashdesigner/resources.rc
ecu_snapshots/Makefile
LookupTables/Makefile
RealtimeMaps/Makefile
RuntimeSliders/Makefile
RuntimeStatus/Makefile
RuntimeText/Makefile
Interrogator/Makefile
Interrogator/Profiles/Makefile
Gui/Makefile
menu/Makefile
desktop/Makefile
category/Makefile
icons/Makefile
MegaTunix.iss
WIN_NOTES.txt
])
AC_OUTPUT

echo "
Configuration:

    Install Path:                       ${prefix}
    General Debugging:                  ${enable_debug}
    Profiling:                          ${enable_profiling}
    PIS Support:                        ${enable_pis}
"
