dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/main.c)

MTX_MAJOR_VERSION=0
MTX_MINOR_VERSION=6
MTX_MICRO_VERSION=0

MTX_VERSION=$MTX_MAJOR_VERSION.$MTX_MINOR_VERSION.$MTX_MICRO_VERSION

AC_CONFIG_SUBDIRS(src/main.c)
AM_INIT_AUTOMAKE(megatunix, 0.6.0)
#AC_CONFIG_HEADERS(config.h)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CC
AM_PROG_LEX
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_YACC
AC_PROG_MAKE_SET

dnl check paths for necessary files
dnl pkg_modules="gtk+-2.0 gthread-2.0 libgnomecanvas-2.0 gtkglext-1.0"
pkg_modules="gtk+-2.0 gthread-2.0 gtkglext-1.0 libglade-2.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

dnl Checks for header files.
AC_HEADER_TIME
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h math.h stddef.h stdio.h string.h strings.h sys/poll.h sys/stat.h sys/types.h sys/ioctl.h sys/time.h termios.h unistd.h )

dnl optimizations
CFLAGS="-O2"
AC_SUBST(CFLAGS)

EXTRA_LIBS=""
AC_SUBST(EXTRA_LIBS)

dnl =================
dnl big/little endian
dnl =================
AC_C_BIGENDIAN(,,)


dnl ===============
dnl Debugging stuff
dnl ===============

dnl General Debugging code
AC_ARG_ENABLE(debug,
[  --enable-debug         enable debugging [default=no]],, enable_debug=no)

if test "$enable_debug" = yes; then
DEBUG_FLAGS="$DEBUG_FLAGS -g -DDEBUG"
fi
AC_SUBST(DEBUG_FLAGS)

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

LOOKUPTABLE_DATA_DIR=LookupTables
REALTIME_MAP_DATA_DIR=RealtimeMaps
INTERROGATOR_DATA_DIR=Interrogator
GUI_DATA_DIR=Gui
AC_SUBST(REALTIME_MAP_DATA_DIR)
AC_SUBST(LOOKUPTABLE_DATA_DIR)
AC_SUBST(INTERROGATOR_DATA_DIR)
AC_SUBST(GUI_DATA_DIR)

MEGATUNIX_DEFINES="$CFLAGS -DDATA_DIR=\\\"$datadir/MegaTunix\\\" -DLOOKUPTABLE_DIR=\\\"$LOOKUPTABLE_DATA_DIR\\\" -DINTERROGATOR_DIR=\\\"$INTERROGATOR_DATA_DIR\\\"  -DGUI_DIR=\\\"$GUI_DATA_DIR\\\" -DREALTIME_MAP_DIR=\\\"$REALTIME_MAP_DATA_DIR\\\" -DGTK_DISABLE_COMPAT_H -DGDK_DISABLE_DEPRECATED -D_MAJOR_=$MTX_MAJOR_VERSION -D_MINOR_=$MTX_MINOR_VERSION -D_MICRO_=$MTX_MICRO_VERSION "

AC_SUBST(MEGATUNIX_DEFINES)

AC_OUTPUT([
Makefile 
mtxmatheval/Makefile
src/Makefile
include/Makefile
LookupTables/Makefile
RealtimeMaps/Makefile
Interrogator/Makefile
Interrogator/Profiles/Makefile
Gui/Makefile
VERSION])

echo "
Configuration:

    Install Path:                       ${prefix}
    General Debugging:                  ${enable_debug}
    Profiling:                          ${enable_profiling}
"
