dnl Process this file with autoconf to produce a configure script.

dnl Version number here MUST be static (uggh, $%^&-you autoconf!!)
AC_INIT(megatunix,0.9.24)
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(/usr/local)
dnl Need to be able to do long paths (over 100chars)
AM_INIT_AUTOMAKE(tar-ustar)
AM_MAINTAINER_MODE([enable])

MTX_MAJOR_VERSION=0
MTX_MINOR_VERSION=9
MTX_MICRO_VERSION=24
AC_ARG_ENABLE([release], AS_HELP_STRING([--enable-release],[Enable Release versioning]))
if test "x$enable_release" != "xyes"; 
then
	RELEASE=0
else
	RELEASE=1
fi

AC_CONFIG_SUBDIRS(src/main.c)
dnl if test x"$MTX_VER_SUFFIX" != "x"; then
dnl 	MTX_VER_STRING="$MTX_MAJOR_VERSION.$MTX_MINOR_VERSION.$MTX_MICRO_VERSION-$MTX_VER_SUFFIX"
dnl else
MTX_VER_STRING="$MTX_MAJOR_VERSION.$MTX_MINOR_VERSION.$MTX_MICRO_VERSION"
CWD=`pwd |tr -s '/' '\\'`
dnl fi

dnl Checks for programs.

AC_PROG_LEX
if test "x$LEX" != "xflex"; 
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
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_MKDIR_P
AM_PROG_MKDIR_P
AC_PROG_MAKE_SET

dnl Gettext Goodness
AC_PROG_INTLTOOL(0.30)
INTLTOOL_PROF_RULE='%.prof: %.prof.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; sed -e "s/^_//g" $< > [$]@'
INTLTOOL_DATAMAP_RULE='%.datamap: %.datamap.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; sed -e "s/^_//g" $< > [$]@'
AC_SUBST(INTLTOOL_PROF_RULE)
AC_SUBST(INTLTOOL_DATAMAP_RULE)

GETTEXT_PACKAGE=megatunix
DATADIRNAME="MegaTunix/locale"
AC_SUBST(GETTEXT_PACKAGE)
AC_SUBST(DATADIRNAME)
AC_DEFINE([GETTEXT_PACKAGE], [], ["$GETTEXT_PACKAGE"])
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
ALL_LINGUAS="es fi fr de"
AM_GLIB_GNU_GETTEXT

MTXLOCALEDIR='${prefix}/share/MegaTunix/locale'
AC_SUBST(MTXLOCALEDIR)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(stdlib.h errno.h fcntl.h math.h stddef.h stdio.h string.h strings.h sys/poll.h sys/stat.h sys/types.h sys/ioctl.h sys/time.h termios.h unistd.h )
AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_FUNCS(bzero memset)
dnl check paths for necessary files

dnl MACRO definition to determine if running GTK>2.18 or not for use of cairo
AC_DEFUN([PKG_CHECK_GTK], 
[
	PKG_CHECK_MODULES(GTK,[gtk+-2.0 >= 2.24.0 gthread-2.0 gmodule-2.0 glib-2.0 >= 2.32.0],,)
dnl for GTK 3.x   	GTK_CFLAGS+="-DGSEAL_ENABLE"
	AC_SUBST(GTK_CFLAGS)
	AC_SUBST(GTK_LIBS)
])

PKG_CHECK_GTK()

dnl check for gtkgl library
PKG_CHECK_MODULES(GTKGL, [gtkgl-2.0])
AC_SUBST(GTKGL_CFLAGS)
AC_SUBST(GTKGL_LIBS)

dnl check for glade-2.x library
PKG_CHECK_MODULES(LIBGLADE,libglade-2.0 )
dnl if test `uname -s` != "Darwin"; then
dnl 	LIBGLADE_LIBS+="-Wl,--export-dynamic"
dnl fi
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

dnl AC_ARG_ENABLE(pis,
dnl [  --enable-pis         enable pis support [default=no]],, enable_pis=no)
dnl 
dnl if test "$enable_pis" == yes ; then
dnl 	AC_DEFINE([__PIS_SUPPORT__], [], [User requested PIS firmare support])
dnl fi

AC_ARG_ENABLE(deprecated,
[  --enable-deprecated         enable deprecated functions [default=no]],, enable_deprecated=no)

if test "$enable_deprecated" == yes ; then
	DEPRECATED_CFLAGS=""
else
	DEPRECATED_CFLAGS="-Wdeprecated-declarations -DG_DISABLE_DEPRECATED -DG_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_COMPAT_H -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE "
fi
AC_SUBST(DEPRECATED_CFLAGS)
dnl ===============
dnl Debugging stuff
dnl ===============

dnl General Debugging code
AC_ARG_ENABLE(debug,
[  --enable-debug         enable debugging [default=no]],, enable_debug=no)

if test "$enable_debug" = yes ; then
DEBUG_FLAGS="$DEBUG_FLAGS -g -DDEBUG"
AM_CFLAGS="-O0 -Wall -Wno-long-long -Wno-unused -Wno-strict-aliasing -Wno-format-extra-args -std=gnu99"
else
DEBUG_FLAGS=""
AM_CFLAGS="-O2 -Wall -Wno-long-long -Wno-unused -Wno-strict-aliasing -Wno-format-extra-args -std=gnu99"
fi
AC_SUBST(DEBUG_FLAGS)
AC_SUBST(AM_CFLAGS)

AM_CONDITIONAL(CROSS_COMPILING, test $cross_compiling = yes)
if test "$cross_compiling" = yes ; then
	AC_CHECK_TOOL(WINDRES, windres)
	WIN32_RESOURCES="resources.o "
	MTXPLUGIN_DIR="dist/plugins"
	if test "$enable_debug" = yes ; then
		dnl EXTRA_LIBS=" -lgw32c -luuid -lole32 -lws2_32"
		dnl EXTRA_LIBS=" -luuid -lole32 -lws2_32"
		EXTRA_LIBS=" -lole32 -lws2_32"
	else
		dnl EXTRA_LIBS=" -mwindows -lgw32c -luuid -lole32 -lws2_32"
		dnl EXTRA_LIBS=" -mwindows -luuid -lole32 -lws2_32"
		EXTRA_LIBS=" -mwindows -lole32 -lws2_32"
	fi
else
	EXTRA_LIBS=" -lGLU -lm"
	MTXPLUGIN_DIR="$datarootdir/MegaTunix/plugins"
fi
AC_SUBST(MTXPLUGIN_DIR)
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

PREFIX=${prefix}
DASHDESIGNER_GLADE_DIR=dashdesigner
GAUGEDESIGNER_GLADE_DIR=gaugedesigner
GUI_DATA_DIR=Gui
GAUGES_DATA_DIR=Gauges
DASHES_DATA_DIR=Dashboards
INTERROGATOR_DATA_DIR=Interrogator
BACKUP_DATA_DIR=ECU_Backups
LOOKUPTABLES_DATA_DIR=LookupTables
REALTIME_MAPS_DATA_DIR=RealtimeMaps
RTSLIDERS_DATA_DIR=RuntimeSliders
RTTEXT_DATA_DIR=RuntimeText
RTSTATUS_DATA_DIR=RuntimeStatus
DATA_DIR=$datarootdir/MegaTunix
LOCALE_DIR=$datarootdir/MegaTunix/locale

AC_SUBST(PREFIX)
AC_SUBST(DATA_DIR)
AC_SUBST(LOCALE_DIR)
AC_SUBST(GUI_DATA_DIR)
AC_SUBST(BACKUP_DATA_DIR)
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

AC_SUBST(MTX_MAJOR_VERSION)
AC_SUBST(MTX_MINOR_VERSION)
AC_SUBST(MTX_MICRO_VERSION)
AC_SUBST(MTX_VER_STRING)
AC_SUBST(CWD)
AC_DEFINE_UNQUOTED([MTX_MAJOR],[$MTX_MAJOR_VERSION],[MegaTunix major version])
AC_DEFINE_UNQUOTED([MTX_MINOR],[$MTX_MINOR_VERSION],[MegaTunix minor version])
AC_DEFINE_UNQUOTED([MTX_MICRO],[$MTX_MICRO_VERSION],[MegaTunix micro version])
AC_DEFINE_UNQUOTED([MTX_VER_STRING],["$MTX_VER_STRING"],[MegaTunix version string])
AC_DEFINE_UNQUOTED([RELEASE],[$RELEASE],[MegaTunix release flag ])

MEGATUNIX_DEFINES=" -DMTXPLUGINDIR=\\\"$MTXPLUGIN_DIR\\\" -DLOCALEDIR=\\\"$LOCALE_DIR\\\" -DDATA_DIR=\\\"$DATA_DIR\\\" -DSNAPSHOTS_DATA_DIR=\\\"$SNAPSHOTS_DATA_DIR\\\" -DLOOKUPTABLES_DATA_DIR=\\\"$LOOKUPTABLES_DATA_DIR\\\" -DINTERROGATOR_DATA_DIR=\\\"$INTERROGATOR_DATA_DIR\\\"  -DGUI_DATA_DIR=\\\"$GUI_DATA_DIR\\\" -DGAUGES_DATA_DIR=\\\"$GAUGES_DATA_DIR\\\" -DDASHES_DATA_DIR=\\\"$DASHES_DATA_DIR\\\" -DGAUGEDESIGNER_GLADE_DIR=\\\"$GAUGEDESIGNER_GLADE_DIR\\\" -DDASHDESIGNER_GLADE_DIR=\\\"$DASHDESIGNER_GLADE_DIR\\\" -DREALTIME_MAPS_DATA_DIR=\\\"$REALTIME_MAPS_DATA_DIR\\\" -DRTSLIDERS_DATA_DIR=\\\"$RTSLIDERS_DATA_DIR\\\" -DRTTEXT_DATA_DIR=\\\"$RTTEXT_DATA_DIR\\\" -DRTSTATUS_DATA_DIR=\\\"$RTSTATUS_DATA_DIR\\\" "
AC_SUBST(MEGATUNIX_DEFINES)

AC_CONFIG_FILES([ po/Makefile.in
src/fix_glib_megatunix
Makefile 
include/Makefile
include/yaml-cpp/Makefile
yaml-cpp/Makefile
mtxcommon/Makefile
mtxmatheval/Makefile
src/Makefile
src/plugins/Makefile
src/plugins/libreems/Makefile
src/plugins/jimstim/Makefile
src/plugins/ms1/Makefile
src/plugins/ms2/Makefile
src/plugins/ms3/Makefile
src/plugins/mscommon/Makefile
src/plugins/null/Makefile
src/plugins/pis/Makefile
msloader/Makefile
msloader/resources.rc
gaugedesigner/resources.rc
gaugedesigner/Makefile
gaugedesigner/gauges/Makefile
gaugedesigner/gauges/LibreEMS_AM_SportComp/Makefile
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
Interrogator/Profiles/MS1/Makefile
Interrogator/Profiles/MS2/Makefile
Interrogator/Profiles/MS3-1.0/Makefile
Interrogator/Profiles/MS3-1.1/Makefile
Interrogator/Profiles/LibreEMS/Makefile
Interrogator/Profiles/PIS/Makefile
Interrogator/Profiles/JimStim/Makefile
Gui/Makefile
menu/Makefile
desktop/Makefile
category/Makefile
icons/Makefile
MegaTunix32.iss
MegaTunix64.iss
MegaTunix32_dbg.iss
MegaTunix64_dbg.iss
Doxyfile
WIN_NOTES.txt
])
AC_OUTPUT

echo "
MegaTunix $MTX_VER_STRING Configuration:

    Install Path:                       ${prefix}
    General Debugging:                  ${enable_debug}
    Allow Deprecated functions:         ${enable_deprecated}
    Profiling:                          ${enable_profiling}
"
dnl    PIS Support:                        ${enable_pis}
